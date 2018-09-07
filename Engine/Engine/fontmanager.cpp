////////////////////////////////////////////////////////////////////////////////
// Filename: fontmanager.cpp
////////////////////////////////////////////////////////////////////////////////
#include "fontmanager.h"


void Fonts::LoadFonts(const char* filename)
{
	std::ifstream file(filename, std::ios::binary);
	file.exceptions(std::fstream::failbit | std::fstream::badbit);
	BinaryReader reader(file);
	int32_t numFonts = reader.Get<int32_t>();
	m_fonts = std::make_unique<Font[]>(numFonts);

  	for (int i = 0; i < numFonts; i++)
	{
		int32_t fontLength = reader.Get<int32_t>();
		auto buffer = std::make_unique<FT_Byte[]>(fontLength);
		reader.Read(buffer.get(), fontLength);
		LoadFont(buffer.get(), fontLength, i);
	}

	file.close();
}


void Fonts::LoadFont(FT_Byte* m_buffer, int32_t m_length, int p_idx)
{
	try
	{
		Font font(m_device, m_deviceContext);

		if (!font.LoadTTF(m_library, m_buffer, static_cast<FT_Long>(m_length)))
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


bool Font::LoadTTF(FT_Library p_library, FT_Byte* m_buffer, FT_Long m_length)
{
	if (FT_New_Memory_Face(p_library, m_buffer, m_length, 0, &m_face))
		return false;

	if (FT_Set_Pixel_Sizes(m_face, 0, ui::ScaleX(16)))
		return false;

	uint32_t x = 0, y = 0, sx = 1, sy = 1;

	// Get total width
	uint32_t total_width = 0;
	uint32_t max_height = 0;
	m_glyphSlots = std::make_unique<GlyphInfo[]>(m_numGlyphs);
	auto glyphBuffers = std::make_unique<std::byte *[]>(m_numGlyphs);
	for (uint32_t i = 32; i < 127; i++) 
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

		// Advance is in 1/64 pixels, so bitshift by 6 to Get value in pixels (2^6 = 64).
		glyphInfo.ax = m_face->glyph->advance.x >> 6;
		glyphInfo.ay = m_face->glyph->advance.y >> 6;

		glyphInfo.bw = m_face->glyph->bitmap.width;
		glyphInfo.bh = m_face->glyph->bitmap.rows;

		int bl = m_face->glyph->bitmap_left;
		int bt = m_face->glyph->bitmap_top;

		std::byte * tempRow = (std::byte *)malloc(glyphInfo.bw * glyphInfo.bh * sizeof(std::byte));
		memcpy(
			tempRow,
			m_face->glyph->bitmap.buffer,
			glyphInfo.bw * glyphInfo.bh * sizeof(std::byte)
		);
		flip(
			tempRow,
			glyphInfo.bw, glyphInfo.bh
		);
		glyphBuffers[i - 32] = tempRow;

		glyphInfo.x = bl + x;
		glyphInfo.left = static_cast<float>(bl + x);
		glyphInfo.y = glyphInfo.bh - bt;

		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (glyphInfo.ax) * sx; // Bitshift by 6 to Get value in pixels (2^6 = 64)
		y += (glyphInfo.ay) * sy;
		total_width = x + glyphInfo.bw;
		max_height = std::max<uint32_t>(max_height, bt + glyphInfo.bh);
		glyphInfo.right = static_cast<float>(glyphInfo.x + glyphInfo.bw);
		m_glyphSlots[i - 32] = glyphInfo;
	}
	m_width = GetNextPow2(total_width);
	m_height = GetNextPow2(max_height);

	auto charmap = std::make_unique<std::byte[]>(m_width * m_height);

	for (uint32_t j = 0; j < 127-32; j++)
	{
		StitchGlyph(
			glyphBuffers[j],
			m_glyphSlots[j],
			m_glyphSlots[j].x,
			m_height / 4 - m_glyphSlots[j].y, 
			charmap.get()
		);
		m_glyphSlots[j].left /= m_width;
		m_glyphSlots[j].right /= m_width;
	}
	
	flip(charmap.get(), m_width, m_height);
	TextureClass tex(m_device, m_width, m_height, m_width, charmap.get(), DXGI_FORMAT_R8_UNORM);
	m_texture = tex.GetTexture();

	FT_Done_Face(m_face);

 	return true;
}


void Font::StitchGlyph(
	const std::byte * b,
	const GlyphInfo & g,
	uint32_t px,
	uint32_t py,
	std::byte * charmap)
{
	if (px + g.bw > m_width || py + g.bh > m_height)
		return; 

	for (uint32_t y = 0; y < g.bh; y++)
	{
		for (uint32_t x = 0; x < g.bw; x++)
		{
			charmap[(py + y) * m_width + (px + x)] = b[y * g.bw + x];
		}
	}
}

void Font::flip(std::byte * buffer, uint32_t width, uint32_t height)
{
	uint32_t rows = height / 2; // Iterate only half the buffer to Get a full flip
	std::byte * tempRow = (std::byte *)malloc(width * sizeof(std::byte));

	for (uint32_t rowIndex = 0; rowIndex < rows; rowIndex++)
	{
		memcpy(tempRow, buffer + rowIndex * width, width * sizeof(std::byte));
		memcpy(buffer + rowIndex * width, buffer + (height - rowIndex - 1) * width, width * sizeof(std::byte));
		memcpy(buffer + (height - rowIndex - 1) * width, tempRow, width * sizeof(std::byte));
	}

	free(tempRow);
}


void Font::BuildVertexArray(void* vertices, const char* sentence, float drawX, float drawY)
{
	VertexType* vertexPtr = (VertexType*)vertices;

	// Draw each letter onto a quad.
	uint32_t index = 0;
	for (uint32_t i = 0; i < strlen(sentence); i++)
	{
		uint32_t letter = static_cast<uint32_t>(sentence[i]) - 32;
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