////////////////////////////////////////////////////////////////////////////////
// Filename: fontshaderclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _SHADERCLASS_H_
#define _SHADERCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dx11async.h>
#include <fstream>
#include <wrl\client.h>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "shaders.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: ShaderClass
////////////////////////////////////////////////////////////////////////////////
class ShaderClass
{
private:
	struct ConstantBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	};

	struct PixelBufferType
	{
		DirectX::XMFLOAT4 pixelColor;
		///float textureWidth;
	};

public:
	ShaderClass(ID3D11Device * p_device, ID3D11DeviceContext * p_deviceContext, bool p_isFont = false)
		:
		m_device(p_device),
		m_deviceContext(p_deviceContext),
		m_isFont(p_isFont)
	{}

	bool Initialize();
	bool Render(int indexCount, const DirectX::XMMATRIX & worldMatrix, const DirectX::XMMATRIX & viewMatrix, const DirectX::XMMATRIX & projectionMatrix, ID3D11ShaderResourceView * texture, const DirectX::XMVECTORF32 & pixelColor);

private:
	bool InitializeShader();
	void OutputShaderErrorMessage(ID3D10Blob*);
	bool SetShaderParameters(const DirectX::XMMATRIX & worldMatrix, const DirectX::XMMATRIX & viewMatrix, const DirectX::XMMATRIX & projectionMatrix, ID3D11ShaderResourceView * texture, const DirectX::XMVECTORF32 & pixelColor);
	void RenderShader(int);

private:
	ID3D11Device * m_device;
	ID3D11DeviceContext* m_deviceContext;
	bool m_isFont;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_layout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampleState;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pixelBuffer;
};

#endif