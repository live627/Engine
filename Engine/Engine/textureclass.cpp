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


TextureClass::TextureClass(ID3D11Device * p_device,
	unsigned int width, unsigned int height,
	unsigned int pitch, const std::byte * buffer,
	DXGI_FORMAT format)
	:
	m_device(p_device)
{
	CreateShaderResourceView(width, height, pitch, buffer, format);
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
		m_pitch = 16 * (width / 4);

		m_pixels.reserve(header.dwPitchOrLinearSize);
		reader.Read(m_pixels.data(), m_pixels.capacity());

		if (MakeFourCC('D', 'X', 'T', '1') == header.ddpfPixelFormat.dwFourCC)
		{
			m_pitch = 8 * (width / 4);
			m_format = DXGI_FORMAT_BC1_UNORM;
		}
		else if (MakeFourCC('D', 'X', 'T', '5') == header.ddpfPixelFormat.dwFourCC)
			m_format = DXGI_FORMAT_BC3_UNORM;
		else
			throw std::invalid_argument("Compressed format can only be either BC1 (DXT1) or BC3 (DXT5) UNORM.");
	}
	else if (header.ddpfPixelFormat.dwFlags & DDPF_RGB) {
		width = header.dwWidth & ~3;
		height = header.dwHeight & ~3;
		m_pitch = header.dwFlags & DDSD_PITCH
			? header.dwPitchOrLinearSize
			: header.dwWidth * header.ddpfPixelFormat.dwRGBBitCount / 8;
		m_pixels.reserve(header.dwHeight * m_pitch);
		reader.Read(m_pixels.data(), m_pixels.capacity());

		switch (header.ddpfPixelFormat.dwRGBBitCount)
		{
		case 32:
		{
			assert(header.ddpfPixelFormat.dwRBitMask == 0x00ff0000);
			assert(header.ddpfPixelFormat.dwGBitMask == 0x0000ff00);
			assert(header.ddpfPixelFormat.dwBBitMask == 0x000000ff);
			m_format = DXGI_FORMAT_B8G8R8A8_UNORM;
			if (header.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) {
				assert(header.ddpfPixelFormat.dwRGBAlphaBitMask == 0xff000000);
			//	m_format = DXGI_FORMAT_R8G8B8A8_UNORM;
			}
		}
		break;
		case 16:
		{
			assert(header.ddpfPixelFormat.dwRBitMask == 0xf800);
			assert(header.ddpfPixelFormat.dwGBitMask == 0x7e0);
			assert(header.ddpfPixelFormat.dwBBitMask == 0x1f);
			m_format = DXGI_FORMAT_B5G6R5_UNORM;
			break;
		}

		default:
			throw std::invalid_argument(
				FormatString(
					"%d bit image not supported",
					header.ddpfPixelFormat.dwRGBBitCount
				).data()
			);
			break;
		}
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


void RenderTextureClass::CreateShaderResourceView()
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = m_screenWidth;
	textureDesc.Height = m_screenHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2D;
	ThrowIfFailed(
		m_device->CreateTexture2D(&textureDesc, NULL, &texture2D),
		"Could not create the texture."
	);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = { textureDesc.Format, D3D11_RTV_DIMENSION_TEXTURE2D };
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	ThrowIfFailed(
		m_device->CreateRenderTargetView(texture2D.Get(), &renderTargetViewDesc, m_renderTargetView.GetAddressOf()),
		"Failed to create the render target view"
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

void RenderTextureClass::SetRenderTarget()
{
	// Bind the render target view and depth stencil buffer to the output render pipeline.
	deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), depthStencilView);
}

void RenderTextureClass::ClearRenderTarget(float red, float green, float blue, float alpha)
{
	float color[4];


	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), color);

	// Clear the depth buffer.
	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
