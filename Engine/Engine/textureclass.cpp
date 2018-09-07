////////////////////////////////////////////////////////////////////////////////
// Filename: textureclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "textureclass.h"


TextureClass::TextureClass(ID3D11Device * p_device, const char * filename)
	:
	m_device(p_device)
{
	// Load the texture in.
	DDS dds(filename);
	CreateShaderResourceView(dds.GetWidth(), dds.GetHeight(), dds.GetPitch(), dds.GetPixels(), dds.GetFormat());
}


TextureClass::TextureClass(ID3D11Device * p_device)
	:
	m_device(p_device)
{
	// Create a single-channel pixel. Use the
	// same pixel shader that we use to draw fonts.
	static const std::byte s_pixel = std::byte(0xffu);
	CreateShaderResourceView(1, 1, 1, &s_pixel, DXGI_FORMAT_R8_UNORM);
}


void TextureClass::CreateShaderResourceView(
	unsigned int width, unsigned int height,
	unsigned int pitch, const std::byte * buffer,
	DXGI_FORMAT format)
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = format;
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


TextureClass::DDS::DDS(const char* FilePath)
	:
	m_file(FilePath, std::ios::binary),
	reader(m_file)
{
	m_file.exceptions(std::fstream::failbit | std::fstream::badbit);

	// magic number
	if (reader.Get<uint32_t>() != MakeFourCC("DDS "))
		throw std::invalid_argument("Magic number (fourCC) not found.");

	// DDSURFACEDESC2
	DDSURFACEDESC2 header = reader.Get<DDSURFACEDESC2>();
	if (header.ddpfPixelFormat.dwFlags & DDPF_FOURCC) 
	{
		width = header.dwWidth & ~3;
		height = header.dwHeight & ~3;
		m_pitch = 8 * (width / 4);

		m_pixels.reserve(header.dwPitchOrLinearSize);
		reader.Read(m_pixels.data(), m_pixels.capacity());

		if (MakeFourCC('D', 'X', 'T', '1') == header.ddpfPixelFormat.dwFourCC)
			m_format = DXGI_FORMAT_BC1_UNORM;
		else if (MakeFourCC('D', 'X', 'T', '5') == header.ddpfPixelFormat.dwFourCC)
			m_format = DXGI_FORMAT_BC5_UNORM;
		else
			throw std::invalid_argument("Compressed format can only be either BC1 (DXT1) or BC3 (DXT5) UNORM.");
	}
	else
		throw std::invalid_argument("Only compressed formats are supported.");
}

// the first argument (a) is the least significant byte of the fourcc (endianness doesn't matter)
// the function is evaluated at compile time if the arguments are known (no run-time overhead).
constexpr uint32_t TextureClass::DDS::MakeFourCC(const uint8_t a, const uint8_t b, const uint8_t c, const uint8_t d) noexcept
{
	return (d << 24) | (c << 16) | (b << 8) | a;
}

// the last character of the argument is the most significant byte of the fourcc
// the function is evaluated at compile time if the string argument is known (no run-time overhead).
constexpr uint32_t TextureClass::DDS::MakeFourCC(const char p[5]) noexcept
{
	return (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
}

constexpr bool TextureClass::DDS::IsBitmask(uint32_t r, uint32_t g, uint32_t b, uint32_t a, const DDPIXELFORMAT & ddsPixelFormat) const noexcept
{
	return
		ddsPixelFormat.dwRBitMask == r
		&& ddsPixelFormat.dwGBitMask == g
		&& ddsPixelFormat.dwBBitMask == b
		&& ddsPixelFormat.dwRGBAlphaBitMask == a;
}