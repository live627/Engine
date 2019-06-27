////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_


/////////////
// LINKING //
/////////////
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")


//////////////
// INCLUDES //
//////////////
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXColors.h>
#include <wrl\client.h>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "game.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: D3DClass
////////////////////////////////////////////////////////////////////////////////
class D3DClass
{
public:
	D3DClass(size_t, size_t, size_t, HWND, bool, bool, float, float);

	void ResizeBuffers(float, float, float, float);

	void BeginScene(const DirectX::XMVECTORF32 &);
	void EndScene();

	ID3D11Device* GetDevice() { return m_device.Get(); }
	ID3D11DeviceContext* GetDeviceContext() { return m_deviceContext.Get(); }
	DXGI_ADAPTER_DESC& GetAdapterDesc() { return m_adapterDesc; }

	void GetProjectionMatrix(DirectX::XMMATRIX &);
	void GetWorldMatrix(DirectX::XMMATRIX &);
	void GetOrthoMatrix(DirectX::XMMATRIX &);

	void TurnZBufferOn();
	void TurnZBufferOff();

	void TurnOnAlphaBlending();
	void TurnOffAlphaBlending();

	ID3D11DepthStencilView* GetDepthStencilView() { return m_depthStencilView.Get(); }
	void SetBackBufferRenderTarget();
	void SetViewport(float, float);

private:
	size_t m_screenWidth, m_screenHeight, m_scale;
	HWND m_hwnd;
	bool m_vsync_enabled;

	Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D11Device> m_device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthDisabledStencilState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_alphaEnableBlendingState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_alphaDisableBlendingState;
	DXGI_ADAPTER_DESC m_adapterDesc;
	DXGI_MODE_DESC m_currentMode;

	DirectX::XMMATRIX m_projectionMatrix;
	DirectX::XMMATRIX m_worldMatrix;
	DirectX::XMMATRIX m_orthoMatrix;

	void FillDisplayModes();
	void Initialize(bool, float, float);
	void CreateRenderTargetView();
	void CreateMatrices(float, float, float, float);
};

#endif