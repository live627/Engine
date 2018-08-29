////////////////////////////////////////////////////////////////////////////////
// Filename: textureclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTURECLASS_H_
#define _TEXTURECLASS_H_


//////////////
// INCLUDES //
//////////////
#include <cassert>
#include <fstream>
#include <sstream>
#include <d3d11.h>
#include <wrl\client.h>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "game.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: TextureClass
////////////////////////////////////////////////////////////////////////////////
class TextureClass
{
public:
	void CreateShaderResourceView(unsigned int, unsigned int, unsigned int, const byte *, DXGI_FORMAT);
	TextureClass(ID3D11Device *, const char *);
	TextureClass(ID3D11Device *);
	auto GetTexture() { return m_texture.Get(); }

private:
	ID3D11Device * m_device;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
};


typedef union PixelInfo
{
	std::uint32_t Colour;
	struct
	{
		std::uint8_t r, g, b, a;
	};
} *PPixelInfo;

class Tga
{
private:
	std::fstream m_file;
	std::vector<std::uint8_t> m_pixels;
	bool ImageCompressed;
	std::uint16_t width, height, bpp;

public:
	Tga(const char* FilePath);
	auto GetPixels() { return m_pixels.data(); }
	std::uint16_t GetWidth() const { return width; }
	std::uint16_t GetHeight() const { return height; }
	auto GetPitch() const { return width * (bpp / 8u); }
	bool HasAlphaChannel() const { return bpp == 32u; }
};

#endif