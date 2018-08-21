////////////////////////////////////////////////////////////////////////////////
// Filename: fontmanager.cpp
////////////////////////////////////////////////////////////////////////////////
#include "fontmanager.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"


void Fonts::LoadFonts(const char* filename)
{
	std::ifstream file(filename, std::ios::binary);
	file.exceptions(std::fstream::failbit | std::fstream::badbit);
	char numFonts = 0;
	file.read(&numFonts, sizeof(char));
	m_fonts = std::make_unique<Font[]>((uint)numFonts);

		long long fontLength = 0;
		file.read(reinterpret_cast<char*>(&fontLength), sizeof(long long));

		auto buffer = std::make_unique<uint8_t[]>(fontLength);
		file.read(reinterpret_cast<char*>(&buffer[0]), fontLength);
  	for (uint i = 0; i < (uint)numFonts; i++)
	{
		LoadFont(buffer.get(), fontLength, i);
	}

	file.close();
}


void Fonts::LoadFont(uint8_t * buffer, uint64_t length, int p_idx)
{
	try
	{
		Font font(m_device, m_deviceContext);

		if (!font.LoadTTF(buffer, length))
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


bool Font::LoadTTF(uint8_t * buffer, uint64_t length)
{
	stbtt_fontinfo info;
	if (!stbtt_InitFont(&info, buffer, 0))
		return false;

	int l_h = ui::ScaleX(20); /* line height */

	float scale = stbtt_ScaleForPixelHeight(&info, l_h);

	int ascent, descent, lineGap;
	stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
	ascent *= scale;
	descent *= scale;
	m_glyphSlots = std::vector<GlyphInfo>(m_numGlyphs);
	for (int i = 32, ix0, iy0, ix1, iy1, ax; i < 127; i++)
	{
		stbtt_GetCodepointBitmapBox(&info, i, scale, scale, &ix0, &iy0, &ix1, &iy1);
		stbtt_GetCodepointHMetrics(&info, i, &ax, nullptr);
		auto glyphInfo = GlyphInfo();

		glyphInfo.ax = ax * scale;
		glyphInfo.ay = ascent - descent;

		glyphInfo.bw = ix1 - ix0;
		glyphInfo.bh = iy1 - iy0;

		glyphInfo.left = glyphInfo.x = m_width;
		glyphInfo.y = ascent + iy0;

		glyphInfo.right = glyphInfo.x + glyphInfo.bw;
		m_glyphSlots[i - 32] = glyphInfo;
		m_width += ax * scale;
		m_height = ascent - descent;
	}

	auto charmap = std::make_unique<byte[]>(m_width * m_height);
	for (int i = 0, x = 0; i < m_numGlyphs; i++)
	{
		auto g = m_glyphSlots[i];
		int byteOffset = x + g.y * m_width;
		stbtt_MakeCodepointBitmap(&info, charmap.get() + byteOffset, g.bw, g.bh, m_width, scale, scale, i + 32);

		x += g.ax;
		m_glyphSlots[i].left /= m_width;
		m_glyphSlots[i].right /= m_width;
	}
	CreateShaderResourceView(m_width, m_height, m_width, charmap.get());

 	return true;
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
			vertexPtr[index] = { { drawX, drawY, 0 },{ glyphSlot.left, 0 } }; // Top left.
			index++;

			vertexPtr[index] = { { (drawX + glyphSlot.bw), (drawY - m_height), 0 },{ glyphSlot.right, 1 } }; // Bottom right.
			index++;

			vertexPtr[index] = { { drawX, (drawY - m_height), 0 },{ glyphSlot.left, 1 } }; // Bottom left.
			index++;

			// Second triangle in quad.
			vertexPtr[index] = { { drawX, drawY, 0 },{ glyphSlot.left, 0 } }; // Top left.
			index++;

			vertexPtr[index] = { { drawX + glyphSlot.bw, drawY, 0 },{ glyphSlot.right, 0 } }; // Top right.
			index++;

			vertexPtr[index] = { { (drawX + glyphSlot.bw), (drawY - m_height), 0 },{ glyphSlot.right, 1 } }; // Bottom right.
			index++;
		}

		drawX += glyphSlot.ax;
	}
}