////////////////////////////////////////////////////////////////////////////////
// Filename: textureclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "textureclass.h"


TextureClass::TextureClass(ID3D11Device * p_device, const char * filename)
	:
	m_device(p_device)
{
	try
	{
		// Load the texture in.
		Tga tga(filename);
		CreateShaderResourceView(tga.GetWidth(), tga.GetHeight(), tga.GetWidth() * 4 , tga.GetPixels(), DXGI_FORMAT_B8G8R8A8_UNORM);
	}
	catch (std::ios::failure & e)
	{
		// Create a single-channel pixel. Use the
		// same pixel shader that we use to draw fonts.
		static const byte s_pixel = 0xffu;
		CreateShaderResourceView(1, 1, 1, &s_pixel, DXGI_FORMAT_R8_UNORM);
		//throw;
	}
}


void TextureClass::CreateShaderResourceView(
	unsigned int width, unsigned int height,
	unsigned int pitch, const byte * buffer,
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


Tga::Tga(const char* FilePath)
	:
	m_file(FilePath, std::ios::in | std::ios::binary)
{
	m_file.exceptions(std::fstream::failbit | std::fstream::badbit);

	std::uint8_t header[18] = {};
	std::vector<std::uint8_t> imagedata;
	static std::uint8_t decompressed[12] = { 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
	static std::uint8_t iscompressed[12] = { 0x0, 0x0, 0xa, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

	m_file.read(reinterpret_cast<char*>(&header), sizeof(header));

	bpp = header[16];
	width = header[13] * 256 + header[12];
	height = header[15] * 256 + header[14];
	auto size = ((width * bpp + 31) / 32) * 4 * height;

	if (bpp != 24 && bpp != 32)
	{
		throw std::invalid_argument("invalid file format. required: 24 or 32 bit image.");
	}

	if (!std::memcmp(decompressed, &header, sizeof(decompressed)))
	{
		imagedata.resize(size);
		ImageCompressed = false;
		m_file.read(reinterpret_cast<char*>(imagedata.data()), size);
	}
	else if (!std::memcmp(iscompressed, &header, sizeof(iscompressed)))
	{
		PixelInfo pixel = {};
		int currentbyte = 0;
		std::size_t currentpixel = 0;
		ImageCompressed = true;
		std::uint8_t chunkheader = {};
		int bytesperpixel = bpp / 8;
		imagedata.resize(width * height * sizeof(PixelInfo));

		while (currentpixel < width * height)
		{
			m_file.read(reinterpret_cast<char*>(&chunkheader), sizeof(chunkheader));

			if (chunkheader < 128)
			{
				++chunkheader;
				for (int i = 0; i < chunkheader; ++i, ++currentpixel)
				{
					m_file.read(reinterpret_cast<char*>(&pixel), bytesperpixel);

					imagedata[currentbyte++] = pixel.r;
					imagedata[currentbyte++] = pixel.g;
					imagedata[currentbyte++] = pixel.b;
					imagedata[currentbyte++] = pixel.a;
				}
			}
			else
			{
				chunkheader -= 127;
				m_file.read(reinterpret_cast<char*>(&pixel), bytesperpixel);

				for (int i = 0; i < chunkheader; ++i, ++currentpixel)
				{
					imagedata[currentbyte++] = pixel.r;
					imagedata[currentbyte++] = pixel.g;
					imagedata[currentbyte++] = pixel.b;
					imagedata[currentbyte++] = pixel.a;
				}
			}
		}
	}
	else
	{
		throw std::invalid_argument("invalid file format. required: 24 or 32 bit tga file.");
	}

	m_pixels = imagedata;
}