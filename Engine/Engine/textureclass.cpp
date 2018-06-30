////////////////////////////////////////////////////////////////////////////////
// Filename: textureclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "textureclass.h"


TextureClass::TextureClass(ID3D11Device * p_device)
{
	m_device = p_device;

	m_texture = 0;
}


bool TextureClass::Initialize(const WCHAR* filename)
{
	HRESULT result;


	// Load the texture in.
	result = D3DX11CreateShaderResourceViewFromFile(
		m_device, filename, NULL, NULL, &m_texture, NULL);
	if(FAILED(result))
	{
		// Create a single-channel pixel. Use the
		//same pixel shader that we use to draw fonts.
		if (!CreateShaderResourceView())
			return false;
	}

	return true;
}


bool TextureClass::CreateShaderResourceView()
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = 1;
	textureDesc.Height = 1;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	ID3D11Texture2D* texture2D;

	static const byte s_pixel = 0xff;
	D3D11_SUBRESOURCE_DATA resourceData = { &s_pixel, sizeof(char), 0 };

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


void TextureClass::Shutdown()
{
	// Release the texture resource.
	if(m_texture)
	{
		m_texture->Release();
		m_texture = 0;
	}

	return;
}


ID3D11ShaderResourceView* TextureClass::GetTexture()
{
	return m_texture;
}