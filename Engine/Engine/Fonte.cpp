#include "Fonte.h"


//The vertex type used for rendering fonts
struct FontVertex
{
	Vector2 position;
	Vector2 size;
	Vector2 startUV;
	Vector2 endUV;

	static const D3D11_INPUT_ELEMENT_DESC* GetLayoutDesc(int& count)
	{
		count = 4;

		static const D3D11_INPUT_ELEMENT_DESC pDesc[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "POSITION", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		return pDesc;
	}
};

//Just rendering the vertex buffer in the usual fashion
void StaticString::Render(int xpos, int ypos)
{
	Matrix4 world = Matrix4::CreateTranslation(xpos, ypos, 0.f);
	Material mat;
	mat.m_Decal = m_pFace->GetTexture();

	m_pDevice->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	m_pDevice->SetObjectProperties(world, mat);
	m_pDevice->SetVertexBuffer(m_pVerts);
	m_pDevice->SetLayout();
	m_pDevice->Draw();

	m_pDevice->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

//A helper function that converts a string into a vector that contains all the lines, where each line is a vector of all words
//Sidenote: is there really not a split function for strings in C++?!
std::vector> SplitToLinesOfWords(std::string text)
{
	//Inner vector contains all words of a line, outer contains all lines.
	std::vector> words;
	words.push_back(std::vector());

	std::string lastWord = "";

	for (int i = 0; i second.advance;

		int penX = 0;
		int penY = 0;

		std::vector vertices;

		//Nothing special going on here, I guess
		for (auto i = words.begin(); i != words.end(); ++i)
		{
			for (auto j = i->begin(); j != i->end(); ++j)
			{
				//Check if a new linebreak should be performed due to line overflow
				if (maxWidth && GetStringWidth(*j) + penX > maxWidth)
				{
					penX = 0;
					penY += m_Face->height;
				}

				for (int c = 0; c length(); ++c)
				{
					char c1 = j->at(c);
					auto dataIt = m_Coords.find(c1);

					if (dataIt != m_Coords.end())
					{
						GlyphData data = dataIt->second;
						FontVertex v;
						v.position = Vector2(penX + data.xOffset, penY + data.yOffset);
						v.size = data.size;
						v.startUV = data.start;
						v.endUV = data.end;

						vertices.push_back(v);

						char c2 = (j->length() - 1) - c > 0 ? j->at(c + 1) : ' ';

						penX += GetAdvance(c1, c2);

					}
				}

				penX += spaceWidth;
			}

			penX = 0;
			penY += m_Face->height;
		}

	StaticString str;
	str.m_pVerts = VertexBuffer::CreateStatic(m_pDevice, &vertices[0], sizeof(FontVertex), vertices.size());
	str.m_pFace = this;
	str.m_pDevice = m_pDevice;

	return str;
}

//Calculates the total width of a given string, assuming no line breaks
unsigned int FaceMap::GetStringWidth(std::string text)
{
	unsigned int width = 0;
	for (int i = 0; i second.size.x + 0.5f);
}
		}
	}

	return width;
}

//Returns the advance between two characters
unsigned int FaceMap::GetAdvance(char c1, char c2)
{
	auto it = m_Coords.find(c1);
	if (it != m_Coords.end())
	{
		return it->second.advance;
	}

	return 0;
}

bool FontManager::Initialize(Device* pDevice)
{
	m_pDevice = pDevice;

	int error = FT_Init_FreeType(&m_Library);

	if (error)
	{
		return false;
	}

	return true;

}

struct GlyphBitmap
{
	int glyphIndex;
	char* pGlyphBitmap;

	unsigned int width;
	unsigned int height;

	int xOffset;
	int yOffset;

	int advance;
};

//Create a list of all glyps, with FreeType bitmaps, that is ready to be packed
std::vector RenderGlyphs(FT_Face f, std::string chars)
{
	int error;

	std::vector res;
	res.reserve(chars.length());

	FT_GlyphSlot slot = f->glyph;

	for (int i = 0; i bitmap_left;

		bmp.yOffset = -slot->bitmap_top;
		bmp.width = slot->bitmap.width;
		bmp.height = slot->bitmap.rows;

		bmp.pGlyphBitmap = new char[bmp.width * bmp.height];

		for (int y = 0; y < bmp.height; ++y)
		{
			for (int x = 0; x bitmap.buffer[x + y * slot->bitmap.pitch];
		}
}

bmp.advance = slot->advance.x >> 6;

res.push_back(bmp);
	}

	return res;
}

//Do some clean up
void FreeGlyphBitmaps(std::vector& bmps)
{
	for (auto i = bmps.begin(); i != bmps.end(); ++i)
	{
		delete[] i->pGlyphBitmap;
	}
}

struct GlyphPos
{
	unsigned int x;
	unsigned int y;
};

struct PackedGlyphs
{
	std::vector positioning;

	unsigned int textureWidth;
	unsigned int textureHeight;
};

//Pack the glyphs into a texture, only actual positioning will be calculated here
PackedGlyphs PackGlyphs(std::vector& glyphs, int maxTextureWidth = 64)
{
	//Not using a sophisticated packing algorithm... yet. Hopefully this one turns out to be
	//sufficiently good, glyphs are after all quite easy to pack.

	PackedGlyphs pg;
	pg.positioning.reserve(glyphs.size());

	int currentX = 0;
	int currentY = 0;
	unsigned int lineMaxSize = 0;

	for (auto i = glyphs.begin(); i != glyphs.end(); ++i)
	{
		if (currentX + i->width + 1 >= maxTextureWidth)
		{
			currentX = 0;
			currentY += lineMaxSize + 1;
			lineMaxSize = i->height;
		}

		GlyphPos data;
		data.x = currentX;
		data.y = currentY;

		currentX += i->width + 1;
		lineMaxSize = Max(i->height, lineMaxSize);

		pg.positioning.push_back(data);
	}

	pg.textureWidth = maxTextureWidth;
	pg.textureHeight = currentY + lineMaxSize + 1;

	return pg;
}

//Just a helper function for copying a glyph into a full texture
void CopyGlyphBitmap(char* dest, unsigned int destPitch, GlyphBitmap src, GlyphPos dstPos)
{
	for (int y = 0; y < src.height; ++y)
	{
		for (int x = 0; x < src.width; ++x)
		{
			dest[dstPos.x + x + (dstPos.y + y) * destPitch] = src.pGlyphBitmap[x + y * src.width];
		}
	}
}

//Creates the actual font texture
std::tr1::shared_ptr CreateFontTexture(Device* pDevice, PackedGlyphs& pg, std::vector& bmps)
{
	char* textureData = new char[pg.textureHeight * pg.textureWidth];
	ZeroMemory(textureData, pg.textureHeight * pg.textureWidth);

	for (int i = 0; i < bmps.size(); ++i)
	{
		CopyGlyphBitmap(textureData, pg.textureWidth, bmps[i], pg.positioning[i]);
	}

	auto res = Texture::CreateFromData(pDevice, pg.textureWidth, pg.textureHeight, TEXTURE_FORMAT_8BIT_UNORM, textureData);

	delete[] textureData;

	return res;
}

//A function for converting our packed glyphs and bitmaps into the data that we actually need.
std::vector BuildGlyphData(PackedGlyphs& pg, std::vector& bmps)
{
	std::vector gd;
	gd.reserve(bmps.size());

	for (int i = 0; i GetFile(fileName, RF_NORMAL);

		if (!pRes)
		{
			m_LastError = "Invalid resource pointer";
				return false;
		}

	FT_Face face;
		int error = FT_New_Memory_Face(m_Library, (FT_Byte*)pRes->GetData(), pRes->GetSize(), 0, &face);

		if (error)
		{
			m_LastError = "Unable to load font from file";
			return false;
		}

	std::string familyName = face->family_name;

	m_Faces.insert(make_pair(familyName, face));

	int numFaces = face->num_faces;
	for (int i = 1; i GetData(), pRes->GetSize(), i, &face);

	if (error)
	{
		continue;
	}

	std::string familyName = face->family_name;

	m_Faces.insert(make_pair(familyName, face));
}

return true;
}

FaceMap* FontManager::CreateFace(std::string familyName, std::string styleName, unsigned int size)
{
	m_LastError = 0;

	auto r = m_Faces.equal_range(familyName);

	FT_Face f = nullptr;

	for (auto i = r.first; i != r.second; ++i)
	{
		if (i->second->style_name == styleName)
		{
			f = i->second;
			break;
		}
	}

	if (!f)
	{
		//It could be a good idea to just take a random face or something here instead
		m_LastError = "Unable to find specified font face";
		return nullptr;
	}

	int error = 0;
	error = FT_Set_Pixel_Sizes(f, 0, size);

	if (error)
	{
		m_LastError = "Specified font size not supported by font";
		return nullptr;
	}

	FaceMap* pFace = new FaceMap();
	pFace->m_pDevice = m_pDevice;
	pFace->m_Size = size;
	pFace->m_Face = f;

	std::string str = "ABCDEFGHIJKLMNOPQRSTUVWXYZÅÄÖabcdefghijklmnopqrstuvwxyzåäö0123456789 ,.-!?$\"'";
	auto bmps = RenderGlyphs(f, str);
	auto packInfo = PackGlyphs(bmps);
	auto glyphData = BuildGlyphData(packInfo, bmps);
	auto pTexture = CreateFontTexture(m_pDevice, packInfo, bmps);
	FreeGlyphBitmaps(bmps);

	pFace->m_TexResource = m_pDevice->GetResources()->LoadTexture(pTexture);

	for (int i = 0; i m_Coords.insert(std::make_pair(str.at(i), glyphData[i]));

}

return pFace;
}