#pragma once


///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
// Windows is too helpful sometimes.
#define NOMINMAX


///////////////////////
// INCLUDES //
///////////////////////
#include <map>
#include <queue>
#include "ft2build.h"
#include FT_FREETYPE_H
#include <wrl\client.h>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "fontshaderclass.h"
#include "game.h"


struct GlyphInfo {
	unsigned int ax; // advance.x
	unsigned int ay; // advance.y

	unsigned int bp; // bitmap.width;
	unsigned int bw; // bitmap.width;
	unsigned int bh; // bitmap.rows;

	unsigned int bl; // bitmap_left;
	unsigned int bt; // bitmap_top;

	unsigned int x, y; // Position of glyph on texture map in pixels.
	float left, right;
	std::vector<unsigned char> img;
};


////////////////////////////////////////////////////////////////////////////////
// Class name: Fonts
////////////////////////////////////////////////////////////////////////////////
class Font
{
private:

	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
	};

public:
	Font(ID3D11Device * p_device, ID3D11DeviceContext * p_deviceContext)
	:
		m_device(p_device),
		m_deviceContext(p_deviceContext)
	{}

	bool LoadTTF(FT_Library p_library, FT_Byte * m_buffer, long long m_length);
	ID3D11ShaderResourceView* GetShaderResourceView();
	void BuildVertexArray(void * vertices, const char * sentence, float drawX, float drawY);

private:
	int GetNextPow2(int a)
	{
		int rval = 2;

		while (rval < a)
			rval <<= 1;

		return rval;
	}

	void StitchGlyph(const GlyphInfo g, uint, uint, byte *);
	void flip(byte *, uint, uint);
	void CreateShaderResourceView(uint, uint, uint, const byte *);
	
	template <typename input_type, typename output_type>
	output_type chebyshev_distance(input_type from_x
		, input_type from_y
		, input_type to_x
		, input_type to_y)
	{
		input_type dx = std::abs(to_x - from_x);
		input_type dy = std::abs(to_y - from_y);

		return static_cast<output_type>(dx > dy ? dx : dy);
	}

	void GenerateSigendDistanceFieldFrom(const unsigned char* inputBuffer
		, unsigned int width
		, unsigned int height
		, unsigned char * outputBuffer
		, bool normalize = false)
	{
		for (uint iy = 0; iy < height; ++iy)
		{
			for (uint ix = 0; ix < width; ++ix)
			{
				uint index = iy * width + ix;
				unsigned char value = inputBuffer[index];
				uint indexMax = width * height;
				uint indexMin = 0;
				uint dfar = width > height ? width : height;
				bool found = false;
				for (uint distance = 1; distance < dfar; ++distance)
				{
					uint xmin = (ix - distance) >= 0 ? ix - distance : 0;
					uint ymin = (iy - distance) >= 0 ? iy - distance : 0;
					uint xmax = (ix + distance) < width ? ix + distance + 1 : width;
					uint ymax = (iy + distance) < height ? iy + distance + 1 : height;
					uint x = xmin;
					uint y = ymin;
					auto fCompareAndFill = [&]() -> bool
					{
						if (value != inputBuffer[y*width + x])
						{
							outputBuffer[index] = chebyshev_distance<int, float>(ix, iy, x, y);
							if (value < 0xff / 2) outputBuffer[index] *= -1;
							//outputBuffer[index] = distance;
							return true;
						}
						return false;
					};
					while (x < xmax)
					{
						if (fCompareAndFill())
						{
							found = true;
							break;
						}
						++x;
					}
					--x;
					if (found == true) { break; }
					while (y < ymax)
					{
						if (fCompareAndFill())
						{
							found = true;
							break;
						}
						++y;
					}
					--y;
					if (found == true) { break; }
					while (x >= xmin)
					{
						if (fCompareAndFill())
						{
							found = true;
							break;
						}
						--x;
					}
					++x;
					if (found == true) { break; }
					while (y >= ymin)
					{
						if (fCompareAndFill())
						{
							found = true;
							break;
						}
						--y;
					}
					if (found == true) { break; }

				} // for( int distance = 1; distance < far; ++distance )

			} // for( int ix = 0; ix < width; ++ix )

		} // for( int iy = 0; iy < height; ++iy )

		//if (normalize && inputBuffer != nullptr)
		//{
		//	float min = outputBuffer[0];
		//	float max = outputBuffer[0];
		//	for (int i = 0; i < width*height; ++i)
		//	{
		//		if (outputBuffer[i] < min)
		//			min = outputBuffer[i];
		//		if (outputBuffer[i] > max)
		//			max = outputBuffer[i];
		//	}
		//	float denominator = (max - min);
		//	float newMin = min / denominator;
		//	for (int i = 0; i < width*height; ++i)
		//	{
		//		outputBuffer[i] /= denominator;
		//		outputBuffer[i] -= newMin;
		//	}
		//}
	}

private:
	ID3D11Device * m_device;
	ID3D11DeviceContext * m_deviceContext;
	FT_Face m_face;
	std::vector<GlyphInfo> m_glyphSlots;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture = 0;
	size_t
		m_width,
		m_height;
};

class Fonts
{
public:
	Fonts(ID3D11Device * p_device, ID3D11DeviceContext * p_deviceContext)
		:
		m_device(p_device),
		m_deviceContext(p_deviceContext)
	{}

	bool Initialize();
	~Fonts();
	bool LoadFonts(const char * filename);
	bool LoadFont(FT_Byte * m_buffer, long long m_length, int);
	Font* GetFont(int);

private:
	ID3D11Device * m_device;
	ID3D11DeviceContext * m_deviceContext;
	FT_Library m_library;
	std::vector<Font*> m_fonts;
};