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


///////////////////////////////////////////////////////////////////////////////
// DDS struct
///////////////////////////////////////////////////////////////////////////////

enum {
	DDSD_CAPS = 0x00000001l,
	DDSD_HEIGHT = 0x00000002l,
	DDSD_WITH = 0x00000004l,
	DDSD_PITCH = 0x00000008l,
	DDSD_ALPHABITDEPTH = 0x00000080l,
	DDSD_PIXELFORMAT = 0x00001000l,
	DDSD_MIPMAPCOUNT = 0x00020000l,
	DDSD_LINEARSIZE = 0x00080000l,
	DDSD_DEPTH = 0x00800000l,

	DDPF_ALPHAPIXELS = 0x00000001l,
	DDPF_FOURCC = 0x00000004l,
	DDPF_RGB = 0x00000040l
};

struct DDPIXELFORMAT
{
	uint32_t    dwSize;
	uint32_t    dwFlags;
	uint32_t    dwFourCC;
	union
	{
		uint32_t    dwRGBBitCount;
		uint32_t    dwYUVBitCount;
		uint32_t    dwZBufferBitDepth;
		uint32_t    dwAlphaBitDepth;
	};
	union
	{
		uint32_t    dwRBitMask;
		uint32_t    dwYBitMask;
	};
	union
	{
		uint32_t    dwGBitMask;
		uint32_t    dwUBitMask;
	};
	union {
		uint32_t    dwBBitMask;
	};
	union
	{
		uint32_t    dwRGBAlphaBitMask;
		uint32_t    dwYUVAlphaBitMask;
	};
};
static_assert(sizeof(DDPIXELFORMAT) == 32);

struct DDSCAPS2
{
	uint32_t dwCaps1;
	uint32_t dwCaps2;
	uint32_t Reserved[2];
};
static_assert(sizeof(DDSCAPS2) == 16);

struct DDSURFACEDESC2
{
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwHeight;
	uint32_t dwWidth;
	uint32_t dwPitchOrLinearSize;
	uint32_t dwDepth;
	uint32_t dwMipMapCount;
	uint32_t dwReserved1[11];
	DDPIXELFORMAT ddpfPixelFormat;
	DDSCAPS2 ddsCaps;
	uint32_t dwReserved2;
};
static_assert(sizeof(DDSURFACEDESC2) == 124);

class DDS
{
public:
	DDS(const char* FilePath);
	auto GetPixels() { return m_pixels.data(); }
	constexpr auto GetFormat() { return m_format; }
	constexpr std::uint16_t GetWidth() const { return width; }
	constexpr std::uint16_t GetHeight() const { return height; }
	constexpr auto GetPitch() const { return m_pitch; }
	constexpr bool HasAlphaChannel() const { return bpp == 32u; }

private:
	constexpr uint32_t MakeFourCC(const uint8_t, const uint8_t, const uint8_t, const uint8_t) noexcept;
	constexpr uint32_t MakeFourCC(const char[5]) noexcept;
	constexpr bool IsBitmask(uint32_t, uint32_t, uint32_t, uint32_t, const DDPIXELFORMAT &) const noexcept;

	BinaryReader reader;
	std::ifstream m_file;
	std::vector<std::uint8_t> m_pixels;
	DXGI_FORMAT m_format;
	uint32_t m_pitch;
	uint16_t width, height, bpp;
};

#endif