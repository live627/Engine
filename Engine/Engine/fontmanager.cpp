////////////////////////////////////////////////////////////////////////////////
// Filename: fontmanager.cpp
////////////////////////////////////////////////////////////////////////////////
#include "fontmanager.h"


FontManager::FontManager(ID3D11Device * p_device, ID3D11DeviceContext * pdeviceContext)
{
	m_device = p_device;
	m_deviceContext = pdeviceContext;
}


bool FontManager::Initialize(HWND hwnd)
{
	bool result;


	FT_Init_FreeType(&m_library);
	LoadFonts("data\\fonts.dat");
	return true;
}


FontManager::~FontManager()
{
	while (!m_fonts.empty())
	{
		delete m_fonts.front();
		m_fonts.erase(m_fonts.begin());
	}
	FT_Done_FreeType(m_library);
}

bool FontManager::LoadFonts(const char* filename)
{
	bool result;


	std::ifstream file(filename, std::ios::binary);

	char numFonts = 0;
	file.read(&numFonts, sizeof(char));

  	for (int i = 1; i < (int)numFonts; i++)
	{
		long long fontLength = 0;
		file.read(reinterpret_cast<char*>(&fontLength), sizeof(long long));

		std::vector<FT_Byte> buffer(fontLength);
		file.read(reinterpret_cast<char*>(&buffer[0]), fontLength);
		result |= LoadFont(buffer.data(), fontLength);
	}

	file.close();

	return result;
}

bool FontManager::LoadFont(FT_Byte* m_buffer, long long m_length)
{
	Font* font = new Font(m_device, m_deviceContext);
	if (!font)
		return false;

	if (!font->LoadTTF(m_library, m_buffer, m_length))
	{
		delete font;
		return false;
	}

	// sets text (tmp)
	//font->RenderFont("ASDASD", Vec2<int>(200,  300));
	m_fonts.push_back(font);

	return true;
}

Font* FontManager::GetFont(int idx)
{
	return m_fonts[idx];
}


Font::Font(ID3D11Device * p_device, ID3D11DeviceContext * pdeviceContext)
{
	m_device = p_device;
	m_deviceContext = pdeviceContext;
}


Font::~Font()
{
}


bool Font::LoadTTF(FT_Library p_library, FT_Byte* m_buffer, long long m_length)
{
	if (FT_New_Memory_Face(p_library, m_buffer, m_length, 0, &m_face))
		return false;

	if (FT_Set_Pixel_Sizes(m_face, 0, ceilf(ui::ScaleX(18))))
		return false;

	float x = 0, y = 0, sx = 1, sy = 1;

	// Get total width
	int total_width = 0;
	int max_height = 0;
	for (int i = 32; i < 127; i++) {
		FT_UInt glyph_index = FT_Get_Char_Index(m_face, i);
		// Have to use FT_LOAD_RENDER.
		// If use FT_LOAD_DEFAULT, the actual glyph bitmap won't be loaded,
		// thus bitmap->rows will be incorrect, causing insufficient max_height.
		auto ret = FT_Load_Glyph(m_face, glyph_index, FT_LOAD_RENDER | FT_LOAD_COMPUTE_METRICS);
		if (ret != 0) {
			continue;
		}
		auto glyphInfo = GlyphInfo();
		glyphInfo.ax = m_face->glyph->advance.x >> 6;
		glyphInfo.ay = m_face->glyph->advance.y >> 6;

		glyphInfo.bw = m_face->glyph->bitmap.width;
		glyphInfo.bh = m_face->glyph->bitmap.rows;

		glyphInfo.bl = m_face->glyph->bitmap_left;
		glyphInfo.bt = m_face->glyph->bitmap_top;

		const int size = glyphInfo.bw*glyphInfo.bh;
		std::vector<unsigned char> outputBuffer;
		outputBuffer.assign(
			m_face->glyph->bitmap.buffer,
			m_face->glyph->bitmap.buffer + size
		);
		//outputBuffer.resize(size);
		//GenerateSigendDistanceFieldFrom(
		//	m_face->glyph->bitmap.buffer, 
		//	glyphInfo.bw, glyphInfo.bh,
		//	outputBuffer.data(), true
		//);
		//memcpy(
		//	&glyphInfo.img,
		//	&m_face->glyph->bitmap.buffer,
		//	sizeof(m_face->glyph->bitmap.buffer)
		//);
		glyphInfo.img = outputBuffer.data();

		glyphInfo.dib = FreeImage_ConvertFromRawBits(
			outputBuffer.data(), 
			m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows,
			m_face->glyph->bitmap.pitch, 8, 0, 0, 0
		);

		glyphInfo.left = glyphInfo.x = glyphInfo.bl + x;
		glyphInfo.y = glyphInfo.bh - glyphInfo.bt;

		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (glyphInfo.ax) * sx; // Bitshift by 6 to get value in pixels (2^6 = 64)
		y += (glyphInfo.ay) * sy;
		total_width = x + glyphInfo.bw;
		max_height = max(max_height, glyphInfo.bt + glyphInfo.bh);
		glyphInfo.right = glyphInfo.x + glyphInfo.bw;
		m_glyphSlots.push_back(glyphInfo);
	}
	m_width = GetNextPow2(total_width);
	m_height = GetNextPow2(max_height);

	const unsigned int size= m_width * m_height;
	//unsigned char  charmap[32];
	//unsigned char charmap = std::vector<unsigned char>(m_width * m_height * 8)[0];
	unsigned char * charmap = new unsigned char[m_width * m_height]();
	std::fill(charmap, charmap  + m_width * m_height, 0);
	FIBITMAP *dst = FreeImage_Allocate(m_width, m_height, 8);
	if (!dst) 
		return false;

	for (int j = 0; j < m_glyphSlots.size(); j++)
	{
 		//FreeImage_Paste(dst, m_glyphSlots[j].dib, 
		// 	m_glyphSlots[j].x, -m_glyphSlots[j].y + max_height / 2, 255);
		FreeImage_Unload(m_glyphSlots[j].dib);
		StitchGlyph(
			m_glyphSlots[j],
			//&m_face->glyph->bitmap,
			m_glyphSlots[j].x,
			-m_glyphSlots[j].y + max_height / 2, 
			//0xffffffu, 0x0u, 
			m_width, m_height,
			charmap
		);
		m_glyphSlots[j].left /= m_width;
		m_glyphSlots[j].right /= m_width;
	}
	
		dst = FreeImage_ConvertFromRawBits(
			charmap, m_width, m_height, m_width, 8, 0, 0, 0
		);
	unsigned int rowPitch = FreeImage_GetPitch(dst);
	unsigned char* imageData = FreeImage_GetBits(dst);

	if ((imageData == 0) || (m_width == 0) || (m_height == 0))
		return false;

	// Release the texture resource.
	if (m_texture)
	{
		m_texture->Release();
		m_texture = 0;
	}

	CreateShaderResourceView(m_width, m_height, rowPitch, imageData);
	CreateShaderResourceView(m_width, m_height, m_width, charmap);

	FreeImage_Save(FIF_BMP, dst, "charmap.bmp");
	FreeImage_Unload(dst);
	FT_Done_Face(m_face);

 	return true;
}


inline int Font::GetNextPow2(int a)
{
	int rval = 1;

	while (rval < a)
		rval <<= 1;

	return rval;
}


void Font::StitchGlyph(const GlyphInfo g,
	unsigned int px, unsigned int py, unsigned int total_width,
	unsigned int max_height, unsigned char * charmap)
{
	auto WIDTH = total_width, HEIGHT = max_height;

	for (int y = 0; y < g.bh; y++)
	{
		for (int x = 0; x < g.bw; x++)
		{
			//if (x >= WIDTH || y >= HEIGHT)
			//	continue; 

			charmap[(py + y) * WIDTH + (px + x)] = g.img[y * g.bw + x];
		}
	}
}


void Font::StitchGlyph(FT_Bitmap* bitmap, int px, int py, unsigned int fontColor,
	unsigned int backgroundColor, unsigned int total_width,
	unsigned int max_height, unsigned char * video)
{
	unsigned int fontColorR = ((fontColor & 0x00FF0000) >> 16);
	unsigned int fontColorG = ((fontColor & 0x0000FF00) >> 8);
	unsigned int fontColorB = (fontColor & 0x000000FF);

	unsigned int backgroundColorR = ((backgroundColor & 0x00FF0000) >> 16);
	unsigned int backgroundColorG = ((backgroundColor & 0x0000FF00) >> 8);
	unsigned int backgroundColorB = (backgroundColor & 0x000000FF);

	float opacity;

	auto WIDTH = total_width, HEIGHT = max_height;

	for (int y = 0; y < bitmap->rows; y++)
	{
		for (int x = 0; x < bitmap->width; x++)
		{
			if (x >= WIDTH || y >= HEIGHT)
				continue;

			int i = (py + y) * WIDTH + (px + x);
			if (bitmap->buffer[y * bitmap->width + x] == 0)
			{
				/* Render background color. */
				video[i + 0] = backgroundColorR;
				video[i + 1] = backgroundColorG;
				video[i + 2] = backgroundColorB;
				video[i + 3] = 0xFF;
			}
			else
			{
				/* Calculate alpha (opacity). */
				opacity = bitmap->buffer[y * bitmap->width + x] / 255;

				video[i + 0] = fontColorR * opacity +
					(1 - opacity) * backgroundColorR;

				video[i + 1] = fontColorG * opacity +
					(1 - opacity) * backgroundColorG;

				video[i + 2] = fontColorB * opacity +
					(1 - opacity) * backgroundColorB;
				video[i + 3] = 0xFF;
			}
		}
	}
}


bool Font::CreateShaderResourceView(
	unsigned int width, unsigned int height,
	unsigned int pitch, const unsigned char * buffer)
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

	ID3D11Texture2D* texture2D;

	D3D11_SUBRESOURCE_DATA resourceData = { buffer, pitch, 0 };

	HRESULT res = m_device->CreateTexture2D(&textureDesc, &resourceData, &texture2D);
	if (FAILED(res))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	HRESULT result = m_device->CreateShaderResourceView(texture2D, &shaderResourceViewDesc, &m_texture);
	if (FAILED(result))
		return false;

	texture2D->Release();

	return true;
}


ID3D11ShaderResourceView* Font::GetShaderResourceView()
{
	return m_texture;
}


void Font::BuildVertexArray(void* vertices, const char* sentence, float drawX, float drawY)
{
	VertexType* vertexPtr;
	int index = 0;


	// Coerce the input vertices into a VertexType structure.
	vertexPtr = (VertexType*)vertices;

	// Draw each letter onto a quad.
	for (int i = 0; i < strlen(sentence); i++)
	{
		int letter = static_cast<int>(sentence[i]) - 32;

		if (letter > m_glyphSlots.size())
			continue;

		auto glyphSlot = m_glyphSlots[letter];

		// If the letter is a space then just move over three pixels.
		if (letter != 0)
		{
			// First triangle in quad.
			vertexPtr[index].position = D3DXVECTOR3(drawX, drawY, 0);  // Top left.
			vertexPtr[index].texture = D3DXVECTOR2(glyphSlot.left, 0);
			index++;

			vertexPtr[index].position = D3DXVECTOR3((drawX + glyphSlot.bw), (drawY - m_height), 0);  // Bottom right.
			vertexPtr[index].texture = D3DXVECTOR2(glyphSlot.right, 1);
			index++;

			vertexPtr[index].position = D3DXVECTOR3(drawX, (drawY - m_height), 0);  // Bottom left.
			vertexPtr[index].texture = D3DXVECTOR2(glyphSlot.left, 1);
			index++;

			// Second triangle in quad.
			vertexPtr[index].position = D3DXVECTOR3(drawX, drawY, 0);  // Top left.
			vertexPtr[index].texture = D3DXVECTOR2(glyphSlot.left, 0);
			index++;

			vertexPtr[index].position = D3DXVECTOR3(drawX + glyphSlot.bw, drawY, 0);  // Top right.
			vertexPtr[index].texture = D3DXVECTOR2(glyphSlot.right, 0);
			index++;

			vertexPtr[index].position = D3DXVECTOR3((drawX + glyphSlot.bw), (drawY - m_height), 0);  // Bottom right.
			vertexPtr[index].texture = D3DXVECTOR2(glyphSlot.right, 1);
			index++;
		}

		drawX += glyphSlot.ax;
	}

	return;
}