////////////////////////////////////////////////////////////////////////////////
// Filename: fontmanager.cpp
////////////////////////////////////////////////////////////////////////////////
#include "fontmanager.h"


void Fonts::LoadFonts(const char* filename)
{
	std::ifstream file(filename, std::ios::binary);
	file.exceptions(std::fstream::failbit | std::fstream::badbit);
	char numFonts = 0;
	file.read(&numFonts, sizeof(char));
	m_fonts = std::make_unique<Font[]>((uint)numFonts);

  	for (uint i = 0; i < (uint)numFonts; i++)
	{
		long long fontLength = 0;
		file.read(reinterpret_cast<char*>(&fontLength), sizeof(long long));

		auto buffer = std::make_unique<FT_Byte[]>(fontLength);
		file.read(reinterpret_cast<char*>(&buffer[0]), fontLength);
		LoadFont(buffer.get(), fontLength, i);
	}

	file.close();
}


void Fonts::LoadFont(FT_Byte* m_buffer, long long m_length, int p_idx)
{
	try
	{
		Font font(m_device, m_deviceContext);

		if (!font.LoadTTF(m_library, m_buffer, m_length))
		{
			throw std::runtime_error(
				FormatString(
					"Could not load font %d", p_idx
				).data()
			);
		}

		m_fonts[p_idx] = std::move(font);
	}
	catch (std::exception & e)
	{
		throw std::runtime_error(
			FormatString(
				"%s\n\nCould not load font %d",
				e.what(), p_idx
			).data()
		);
	}
}


bool Font::LoadTTF(FT_Library p_library, FT_Byte* m_buffer, long long m_length)
{
	if (FT_New_Memory_Face(p_library, m_buffer, m_length, 0, &m_face))
		return false;

	if (FT_Set_Pixel_Sizes(m_face, 0, ceilf(ui::ScaleX(16))))
		return false;

	uint x = 0, y = 0, sx = 1, sy = 1;

	// Get total width
	uint total_width = 0;
	uint max_height = 0;
	m_glyphSlots = std::make_unique<GlyphInfo[]>(m_numGlyphs);
	auto glyphBuffers = std::make_unique<std::vector<byte>[]>(m_numGlyphs);
	for (uint i = 32; i < 127; i++) 
	{
		FT_UInt glyph_index = FT_Get_Char_Index(m_face, i);
		// Have to use FT_LOAD_RENDER.
		// If use FT_LOAD_DEFAULT, the actual glyph bitmap won't be loaded,
		// thus bitmap->rows will be incorrect, causing insufficient max_height.
		auto ret = FT_Load_Glyph(m_face, glyph_index, FT_LOAD_RENDER | FT_LOAD_COMPUTE_METRICS);
		if (ret != 0)
		{
			throw std::runtime_error(
				FormatString(
					"Could not load glyph %d (%c)", i, i
				).data()
			);
		}
		auto glyphInfo = GlyphInfo();

		// Advance is in 1/64 pixels, so bitshift by 6 to get value in pixels (2^6 = 64).
		glyphInfo.ax = m_face->glyph->advance.x >> 6;
		glyphInfo.ay = m_face->glyph->advance.y >> 6;

		glyphInfo.bw = m_face->glyph->bitmap.width;
		glyphInfo.bh = m_face->glyph->bitmap.rows;

		glyphInfo.bl = m_face->glyph->bitmap_left;
		glyphInfo.bt = m_face->glyph->bitmap_top;

		flip(
			m_face->glyph->bitmap.buffer,
			glyphInfo.bw, glyphInfo.bh
		);
		glyphBuffers[i - 32] = std::vector<byte>(
			m_face->glyph->bitmap.buffer,
			m_face->glyph->bitmap.buffer + glyphInfo.bw * glyphInfo.bh
		);

		glyphInfo.left = glyphInfo.x = glyphInfo.bl + x;
		glyphInfo.y = glyphInfo.bh - glyphInfo.bt;

		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (glyphInfo.ax) * sx; // Bitshift by 6 to get value in pixels (2^6 = 64)
		y += (glyphInfo.ay) * sy;
		total_width = x + glyphInfo.bw;
		max_height = std::max<uint>(max_height, glyphInfo.bt + glyphInfo.bh);
		glyphInfo.right = glyphInfo.x + glyphInfo.bw;
		m_glyphSlots[i - 32] = glyphInfo;
	}
	m_width = GetNextPow2(total_width);
	m_height = GetNextPow2(max_height);

	auto charmap = std::make_unique<byte[]>(m_width * m_height);

	for (uint j = 0; j < 127-32; j++)
	{
		StitchGlyph(
			glyphBuffers[j].data(),
			m_glyphSlots[j],
			m_glyphSlots[j].x,
			m_height / 4 - m_glyphSlots[j].y, 
			charmap.get()
		);
		m_glyphSlots[j].left /= m_width;
		m_glyphSlots[j].right /= m_width;
	}
	
	flip(charmap.get(), m_width, m_height);
	CreateShaderResourceView(m_width, m_height, m_width, charmap.get());

	FT_Done_Face(m_face);

 	return true;
}


void Font::StitchGlyph(
	const byte * b,
	const GlyphInfo & g,
	uint px,
	uint py,
	byte * charmap)
{
	if (px + g.bw > m_width || py + g.bh > m_height)
		return; 

	for (uint y = 0; y < g.bh; y++)
	{
		for (uint x = 0; x < g.bw; x++)
		{
			charmap[(py + y) * m_width + (px + x)] = b[y * g.bw + x];
		}
	}
}

void Font::flip(byte * buffer, uint width, uint height)
{
	uint rows = height / 2; // Iterate only half the buffer to get a full flip
	byte * tempRow = (byte *)malloc(width * sizeof(byte));

	for (uint rowIndex = 0; rowIndex < rows; rowIndex++)
	{
		memcpy(tempRow, buffer + rowIndex * width, width * sizeof(byte));
		memcpy(buffer + rowIndex * width, buffer + (height - rowIndex - 1) * width, width * sizeof(byte));
		memcpy(buffer + (height - rowIndex - 1) * width, tempRow, width * sizeof(byte));
	}

	free(tempRow);
}


void Font::CreateShaderResourceView(
	uint width, uint height,
	uint pitch, const byte * buffer)
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2D;
	D3D11_SUBRESOURCE_DATA resourceData = { buffer, pitch, 0 };
	ThrowIfFailed(
		m_device->CreateTexture2D(&textureDesc, &resourceData, &texture2D),
		"Could not create the texture."
	);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	ThrowIfFailed(
		m_device->CreateShaderResourceView(texture2D.Get(), &shaderResourceViewDesc, &m_texture),
		"Could not create the shader resource view."
	);
}


void Font::BuildVertexArray(void* vertices, const char* sentence, float drawX, float drawY)
{
	 VertexType* vertexPtr = (VertexType*)vertices;

	// Draw each letter onto a quad.
	uint index = 0;
	for (uint i = 0; i < strlen(sentence); i++)
	{
		uint letter = static_cast<uint>(sentence[i]) - 32;
/*
		if (letter > m_glyphSlots.size())
			continue;
*/
		auto glyphSlot = m_glyphSlots[letter];

		// If the letter is a space then just move over three pixels.
		if (letter != 0)
		{
			// First triangle in quad.
			vertexPtr[index] = { { drawX, drawY, 0 }, { glyphSlot.left, 0} }; // Top left.
			index++;

			vertexPtr[index] = { { (drawX + glyphSlot.bw), (drawY - m_height), 0},{glyphSlot.right, 1} };; // Bottom right.
			index++;

			vertexPtr[index] = { {drawX, (drawY - m_height), 0	},{glyphSlot.left, 1} }; // Bottom left.
			index++;

			// Second triangle in quad.
			vertexPtr[index] = { {drawX, drawY, 0},{glyphSlot.left, 0} }; // Top left.
			index++;

			vertexPtr[index] = { {drawX + glyphSlot.bw, drawY, 0},{glyphSlot.right, 0 } }; // Top right.
			index++;

			vertexPtr[index] = { {(drawX + glyphSlot.bw), (drawY - m_height), 0},{glyphSlot.right, 1} }; // Bottom right.
			index++;
		}

		drawX += glyphSlot.ax;
	}
}