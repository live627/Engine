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
#include <d3dcompiler.h>
#include <fstream>
#include <string>
#include <iterator>
#include <wrl\client.h>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "game.h"


#pragma comment(lib, "d3dcompiler.lib")


struct VertexType
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 texture;
};


struct InstanceType
{
	DirectX::XMFLOAT3 position;
};


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
	};

public:
	ShaderClass(ID3D11Device * p_device, ID3D11DeviceContext * p_deviceContext, bool p_isFont = false)
		:
		m_device(p_device),
		m_deviceContext(p_deviceContext),
		m_isFont(p_isFont)
	{
		InitializeShader();
	}

	void Render(int, const DirectX::XMMATRIX &, const DirectX::XMMATRIX &, const DirectX::XMMATRIX &, ID3D11ShaderResourceView *, const DirectX::XMVECTORF32 &);
	void RenderInstanced(uint32_t, uint32_t, const DirectX::XMMATRIX &, const DirectX::XMMATRIX &, const DirectX::XMMATRIX &, ID3D11ShaderResourceView *, const DirectX::XMVECTORF32 &);

private:
	void InitializeShader();
	void SetShaderParameters(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &, const DirectX::XMMATRIX &, ID3D11ShaderResourceView *, const DirectX::XMVECTORF32 &);
	void RenderShader(int);
	void RenderShaderInstanced(uint32_t, uint32_t);

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