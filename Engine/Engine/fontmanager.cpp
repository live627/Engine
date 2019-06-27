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
	height = m_face->height;
	max_advance_width = m_face->max_advance_width;
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

		// Advance is in 1/64 pixels, so bitshift by 6 to get value in pixels (2^6 = 64).
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
		glyphBuffers[i - 32] = tempRow;

		FT_Glyph aglyph;
		FT_Get_Glyph(m_face->glyph, &aglyph);
		FT_Glyph_Get_CBox(aglyph, ft_glyph_bbox_pixels, &glyphInfo.bbox);
		FT_Done_Glyph(aglyph);
		glyphInfo.x = bl + x;
		glyphInfo.left = static_cast<float>(bl + x);
		glyphInfo.y = bt;
		x += (glyphInfo.ax) * sx;
		y += (glyphInfo.ay) * sy;
		m_width = x + glyphInfo.bw;
		m_height = std::max<int32_t>(m_height, glyphInfo.bbox.yMax - glyphInfo.bbox.yMin);
		glyphInfo.right = static_cast<float>(glyphInfo.x + glyphInfo.bw);
		m_glyphSlots[i - 32] = glyphInfo;
	}

	auto charmap = std::make_unique<std::byte[]>(m_width * m_height);

	for (uint32_t j = 0; j < 127-32; j++)
	{
		StitchGlyph(
			glyphBuffers[j],
			m_glyphSlots[j],
			m_glyphSlots[j].x,
			0, 
			charmap.get()
		);
		m_glyphSlots[j].left /= m_width;
		m_glyphSlots[j].right /= m_width;
	}

	{
		std::ofstream f("test.pgm", std::ios_base::out
			| std::ios_base::binary
			| std::ios_base::trunc
		);

		int maxColorValue = 255;
		f << "P5\n" << m_width << " " << m_height << "\n" << maxColorValue << "\n";
		f.write(reinterpret_cast<const char*>(charmap.get()), m_width * m_height);
	}
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

	for (uint32_t y = 0u; y < g.bh; y++)
		for (uint32_t x = 0u; x < g.bw; x++)
			charmap[(py + y) * m_width + (px + x)] = b[y * g.bw + x];
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
			auto y = drawY + glyphSlot.y;
			vertexPtr[index++] = { { drawX, y, 0 },{ glyphSlot.left, 0 } }; // Top left.
			vertexPtr[index++] = { { (drawX + glyphSlot.bw), (y - m_height), 0 },{ glyphSlot.right, 1 } }; // Bottom right.
			vertexPtr[index++] = { { drawX, (y - m_height), 0 },{ glyphSlot.left, 1 } }; // Bottom left.
			vertexPtr[index++] = { { drawX, y, 0 },{ glyphSlot.left, 0 } }; // Top left.
			vertexPtr[index++] = { { drawX + glyphSlot.bw, y, 0 },{ glyphSlot.right, 0 } }; // Top right.
			vertexPtr[index++] = { { (drawX + glyphSlot.bw), (y - m_height), 0 },{ glyphSlot.right, 1 } }; // Bottom right.
		}

		drawX += glyphSlot.ax;
	}
}

POINT && Font::MeasureString(const char* sentence)
{
	POINT index = { 0, height >> 6 };
	for (auto i = 0u; i < strlen(sentence); i++)
	{
		auto glyphSlot = m_glyphSlots[sentence[i] - 32u];
		index.x += glyphSlot.ax;
		if (i == strlen(sentence) - 1)
			index.x += glyphSlot.bw;
	}

	return std::move(index);
}