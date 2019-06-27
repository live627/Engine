////////////////////////////////////////////////////////////////////////////////
// Filename: fontmanager.cpp
////////////////////////////////////////////////////////////////////////////////
#include "fontmanager.h"


void Fonts::LoadFontBitmap(const char* p_texfilename, const char* p_datfilename)
{
	TextureClass tex(m_device, p_texfilename);
	m_textures[0] = tex.GetTexture();

	std::ifstream file(p_datfilename, std::ios::binary);
	file.exceptions(std::fstream::failbit | std::fstream::badbit);
	BinaryReader reader(file);
	int32_t numFonts = reader.Get<int32_t>();
	m_chars = std::make_unique<CharRecord[]>(numFonts);

  	for (int i = 0; i < 255; i++)
	{
		auto nn = reader.Get<int32_t>();
		m_chars[nn] =
		{
			reader.Get<float>(),
			reader.Get<float>(),
			reader.Get<float>(),
			reader.Get<float>(),
			reader.Get<float>(),
			reader.Get<float>(),
			reader.Get<float>(),
		};
	}

	file.close();
}


void Fonts::LoadFontBitmap2(const char* p_texfilename)
{
	TextureClass tex(m_device, p_texfilename);
	m_textures[1] = tex.GetTexture();

	std::vector<Character> characters_Consolas({
		{ ' ', 402, 239, 12, 12, 6, 6 },
		{ '!', 107, 76, 21, 57, -7, 50 },
		{ '"', 181, 239, 31, 27, -2, 50 },
		{ '#', 93, 133, 45, 53, 5, 47 },
		{ '$', 284, 0, 40, 66, 3, 53 },
		{ '%', 448, 0, 46, 57, 5, 51 },
		{ '&', 128, 76, 46, 56, 4, 49 },
		{ '\'', 212, 239, 19, 27, -8, 50 },
		{ '(', 17, 0, 30, 72, -3, 52 },
		{ ')', 47, 0, 30, 72, -2, 52 },
		{ '*', 37, 239, 37, 39, 1, 50 },
		{ '+', 750, 186, 42, 43, 3, 38 },
		{ ',', 74, 239, 28, 33, -1, 16 },
		{ '-', 324, 239, 31, 18, -2, 26 },
		{ '.', 274, 239, 23, 23, -6, 16 },
		{ '/', 370, 0, 39, 63, 2, 50 },
		{ '0', 288, 76, 42, 54, 3, 47 },
		{ '1', 638, 133, 39, 53, 2, 47 },
		{ '2', 677, 133, 39, 53, 1, 47 },
		{ '3', 615, 76, 38, 54, 1, 47 },
		{ '4', 138, 133, 45, 53, 5, 47 },
		{ '5', 653, 76, 37, 54, 0, 47 },
		{ '6', 495, 76, 40, 54, 2, 47 },
		{ '7', 438, 133, 40, 53, 2, 47 },
		{ '8', 535, 76, 40, 54, 2, 47 },
		{ '9', 478, 133, 40, 53, 3, 47 },
		{ ':', 406, 186, 22, 45, -7, 38 },
		{ ';', 217, 76, 27, 55, -2, 38 },
		{ '<', 177, 186, 37, 47, 2, 40 },
		{ '=', 142, 239, 39, 28, 2, 31 },
		{ '>', 214, 186, 36, 47, -1, 40 },
		{ '?', 75, 76, 32, 57, -3, 50 },
		{ '@', 77, 0, 47, 70, 6, 51 },
		{ 'A', 690, 76, 47, 53, 6, 47 },
		{ 'B', 716, 133, 39, 53, 1, 47 },
		{ 'C', 372, 76, 41, 54, 3, 47 },
		{ 'D', 272, 133, 42, 53, 3, 47 },
		{ 'E', 73, 186, 35, 53, 0, 47 },
		{ 'F', 108, 186, 35, 53, 0, 47 },
		{ 'G', 330, 76, 42, 54, 4, 47 },
		{ 'H', 356, 133, 41, 53, 3, 47 },
		{ 'I', 0, 186, 37, 53, 1, 47 },
		{ 'J', 143, 186, 34, 53, 0, 47 },
		{ 'K', 518, 133, 40, 53, 1, 47 },
		{ 'L', 37, 186, 36, 53, -1, 47 },
		{ 'M', 228, 133, 44, 53, 4, 47 },
		{ 'N', 558, 133, 40, 53, 2, 47 },
		{ 'O', 244, 76, 44, 54, 4, 47 },
		{ 'P', 755, 133, 39, 53, 1, 47 },
		{ 'Q', 324, 0, 46, 65, 4, 47 },
		{ 'R', 598, 133, 40, 53, 1, 47 },
		{ 'S', 575, 76, 40, 54, 3, 47 },
		{ 'T', 314, 133, 42, 53, 3, 47 },
		{ 'U', 413, 76, 41, 54, 3, 47 },
		{ 'V', 737, 76, 47, 53, 6, 47 },
		{ 'W', 183, 133, 45, 53, 5, 47 },
		{ 'X', 47, 133, 46, 53, 5, 47 },
		{ 'Y', 0, 133, 47, 53, 6, 47 },
		{ 'Z', 397, 133, 41, 53, 3, 47 },
		{ '[', 229, 0, 28, 70, -5, 51 },
		{ '\\', 409, 0, 39, 63, 1, 50 },
		{ ']', 257, 0, 27, 70, -3, 51 },
		{ '^', 102, 239, 40, 32, 2, 47 },
		{ '_', 355, 239, 47, 17, 6, -2 },
		{ '`', 297, 239, 27, 20, 0, 50 },
		{ 'a', 332, 186, 38, 45, 2, 38 },
		{ 'b', 618, 0, 39, 57, 1, 50 },
		{ 'c', 508, 186, 37, 44, 1, 38 },
		{ 'd', 657, 0, 39, 57, 3, 50 },
		{ 'e', 292, 186, 40, 45, 2, 38 },
		{ 'f', 494, 0, 42, 57, 3, 51 },
		{ 'g', 536, 0, 42, 57, 3, 38 },
		{ 'h', 38, 76, 37, 57, 1, 50 },
		{ 'i', 774, 0, 38, 57, 1, 51 },
		{ 'j', 124, 0, 35, 70, 2, 51 },
		{ 'k', 578, 0, 40, 57, 0, 50 },
		{ 'l', 0, 76, 38, 57, 1, 50 },
		{ 'm', 428, 186, 42, 44, 3, 38 },
		{ 'n', 545, 186, 37, 44, 1, 38 },
		{ 'o', 250, 186, 42, 45, 3, 38 },
		{ 'p', 696, 0, 39, 57, 1, 38 },
		{ 'q', 735, 0, 39, 57, 3, 38 },
		{ 'r', 470, 186, 38, 44, 0, 38 },
		{ 's', 370, 186, 36, 45, 0, 38 },
		{ 't', 454, 76, 41, 54, 4, 47 },
		{ 'u', 582, 186, 37, 44, 1, 37 },
		{ 'v', 664, 186, 43, 43, 4, 37 },
		{ 'w', 619, 186, 45, 43, 5, 37 },
		{ 'x', 707, 186, 43, 43, 4, 37 },
		{ 'y', 174, 76, 43, 56, 4, 37 },
		{ 'z', 0, 239, 37, 43, 1, 37 },
		{ '{', 159, 0, 35, 70, 1, 51 },
		{ '|', 0, 0, 17, 76, -9, 57 },
		{ '}', 194, 0, 35, 70, -1, 51 },
		{ '~', 231, 239, 43, 25, 4, 30 },
	});

	m_font_Consolas = { "Consolas", 64, 0, 0, 812, 282, 95, characters_Consolas };
}


void Fonts::BuildVertexArray(void* vertices, const char* sentence, float drawX, float drawY)
{
	VertexType* vertexPtr = (VertexType*)vertices;

	// Draw each letter onto a quad.
	uint32_t index = 0;
	for (uint32_t i = 0; i < strlen(sentence); i++)
	{
		auto cr = m_chars[sentence[i]];
		auto Scale = ui::ScaleX(0.3f), x = drawX + (cr.xoffset * Scale), y = drawY + (cr.yoffset * Scale);
			vertexPtr[index] = { { x, y, 0 },{ cr.X / 1024.0f, cr.Y / 1024 } }; // Top left.
			index++;

			vertexPtr[index] = { { x + cr.Width * Scale, y, 0 },{ cr.X / 1024.0f + cr.Width / 1024.0f, cr.Y / 1024 } }; // Top right.
			index++;

			vertexPtr[index] = { { x, (y - cr.Height * Scale), 0 },{ cr.X / 1024.0f, cr.Y / 1024 + cr.Height / 1024 } }; // Bottom left.
			index++;

			vertexPtr[index] = { { (x + cr.Width * Scale), (y - cr.Height * Scale), 0 },{ cr.X / 1024.0f + cr.Width / 1024.0f, cr.Y / 1024 + cr.Height / 1024 } }; // Bottom right.
			index++;

		drawX += cr.xadvance * Scale;
	}
}


POINT && Fonts::MeasureString(const char* sentence)
{
	POINT index = { 0, 0 };
	for (uint32_t i = 0; i < strlen(sentence); i++)
	{
		auto cr = m_chars[sentence[i]];
		auto Scale = ui::ScaleX(0.3f);
		index.x += cr.xadvance * Scale;
	}

	return std::move(index);
}


POINT && Fonts::MeasureString2(const char* sentence)
{
	POINT index = { 0, 0 };
	for (uint32_t i = 0; i < strlen(sentence); i++)
	{
		auto cr = m_font_Consolas.characters[sentence[i] - 32];
		auto Scale = ui::ScaleX(0.25f);
		index.x += cr.Width * Scale;
	}

	return std::move(index);
}


void Fonts::BuildVertexArray2(void* vertices, const char* sentence, float drawX, float drawY)
{
	VertexType* vertexPtr = (VertexType*)vertices;

	// Draw each letter onto a quad.
	uint32_t index = 0;
	for (uint32_t i = 0; i < strlen(sentence); i++)
	{
		auto cr = m_font_Consolas.characters[sentence[i] - 32];
		auto Scale = ui::ScaleX(0.25f), x = drawX + (cr.xoffset * Scale), y = drawY + (cr.yoffset * Scale);
			vertexPtr[index] = { { x, y, 0 },{ cr.X / m_font_Consolas.width, cr.Y / m_font_Consolas.height } }; // Top left.
			index++;

			vertexPtr[index] = { { x + cr.Width * Scale, y, 0 },{ cr.X / m_font_Consolas.width + cr.Width / m_font_Consolas.width, cr.Y / m_font_Consolas.height } }; // Top right.
			index++;

			vertexPtr[index] = { { x, (y - cr.Height * Scale), 0 },{ cr.X / m_font_Consolas.width, cr.Y / m_font_Consolas.height + cr.Height / m_font_Consolas.height } }; // Bottom left.
			index++;

			vertexPtr[index] = { { (x + cr.Width * Scale), (y - cr.Height * Scale), 0 },{ cr.X / m_font_Consolas.width + cr.Width / m_font_Consolas.width, cr.Y / m_font_Consolas.height + cr.Height / m_font_Consolas.height } }; // Bottom right.
			index++;

		drawX += cr.Width * Scale;
	}
}