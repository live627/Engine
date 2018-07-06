////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "d3dclass.h"


D3DClass::D3DClass(float screenWidth, float screenHeight, HWND hwnd, bool vsync)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	m_hwnd = hwnd;
	m_vsync_enabled = vsync;
	FillDisplayModes();
}


void D3DClass::FillDisplayModes()
{
	HRESULT result;
	Microsoft::WRL::ComPtr<IDXGIFactory> factory;
	Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
	Microsoft::WRL::ComPtr<IDXGIOutput> adapterOutput;

	// Create a DirectX graphics interface factory.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	ThrowIfFailed(result, "Failed to create a DirectX graphics interface factory.");

	// Use the factory to create an adapter for the primary graphics interface (video card).
	result = factory->EnumAdapters(0, &adapter);
	ThrowIfFailed(result, "Failed to find the primary graphics interface (video card).");

	// Enumerate the primary adapter output (monitor).
	result = adapter->EnumOutputs(0, &adapterOutput);
	ThrowIfFailed(result, "Failed to enumerate the primary adapter output (monitor).");

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM
	// display format for the adapter output (monitor).
	unsigned int numModes = 0;
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	ThrowIfFailed(result, "Failed to get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).");

	// Now fill the display mode list structures.
	auto displayModeList = std::vector<DXGI_MODE_DESC>(numModes);
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList.data());
	ThrowIfFailed(result, "Failed to get the display modes");

	// Now go through all the display modes and find the one that matches the screen width and height.
	size_t screenSizeX = GetSystemMetrics(SM_CXSCREEN);
	size_t screenSizeY = GetSystemMetrics(SM_CYSCREEN);
	for (auto mode : displayModeList)
	{
		if (mode.Width == screenSizeX && mode.Height == screenSizeY)
		{
			m_currentMode = mode;
			break;
		}
	}
	
	// Get the adapter (video card) description.
	result = adapter->GetDesc(&m_adapterDesc);
} 


void D3DClass::Initialize(bool fullscreen, float screenDepth, float screenNear)
{
	HRESULT result;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
	D3D11_BLEND_DESC blendStateDescription;

	// Initialize the swap chain description.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = m_screenWidth;
	swapChainDesc.BufferDesc.Height = m_screenHeight;

	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the refresh rate of the back buffer.
	swapChainDesc.BufferDesc.RefreshRate = m_currentMode.RefreshRate;

	// Set the usage of the back buffer.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = m_hwnd;

	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	swapChainDesc.Windowed = !fullscreen;

	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;

	// Set the feature level to DirectX 11.
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Create the swap chain, Direct3D device, and Direct3D device context.
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 
		0, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc,
		m_swapChain.GetAddressOf(), m_device.GetAddressOf(), NULL, m_deviceContext.GetAddressOf());
	ThrowIfFailed(result, "Failed to create the swap chain, Direct3D device, and Direct3D device context.");

	CreateRenderTargetView();

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = m_screenWidth;
	depthBufferDesc.Height = m_screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, m_depthStencilBuffer.GetAddressOf());
	ThrowIfFailed(result, "Failed to create the depth buffer");

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	result = m_device->CreateDepthStencilState(&depthStencilDesc, m_depthStencilState.GetAddressOf());
	ThrowIfFailed(result, "Failed to create the depth stencil state");

	// Set the depth stencil state.
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 1);

	// Initialize the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = m_device->CreateDepthStencilView(
		m_depthStencilBuffer.Get(), &depthStencilViewDesc,
		m_depthStencilView.GetAddressOf());
	ThrowIfFailed(result, "Failed to create the depth stencil view");

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

	// Setup the raster description which will determine how and what polygons will be drawn.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	result = m_device->CreateRasterizerState(&rasterDesc, m_rasterState.GetAddressOf());
	ThrowIfFailed(result, "Failed to create the rasterizer state");

	// Now set the rasterizer state.
	m_deviceContext->RSSetState(m_rasterState.Get());

	SetViewport(m_screenWidth, m_screenHeight);
	CreateMatrices(m_screenWidth, m_screenHeight, screenDepth, screenNear);

	// Clear the second depth stencil state before setting the parameters.
	ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

	// Now create a second depth stencil state which turns off the Z buffer 
	// for 2D rendering. The only difference is that DepthEnable is set to
	// false, all other parameters are the same as the other depth stencil state.
	depthDisabledStencilDesc.DepthEnable = false;
	depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDisabledStencilDesc.StencilEnable = true;
	depthDisabledStencilDesc.StencilReadMask = 0xFF;
	depthDisabledStencilDesc.StencilWriteMask = 0xFF;
	depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the state using the device.
	result = m_device->CreateDepthStencilState(&depthDisabledStencilDesc, m_depthDisabledStencilState.GetAddressOf());
	ThrowIfFailed(result, "Failed to create a second depth stencil state which turns off the Z buffer for 2D rendering.  ");

	// Clear the blend state description.
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));

	// Create an alpha enabled blend state description.
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	// Create the blend state using the description.
	result = m_device->CreateBlendState(&blendStateDescription, m_alphaEnableBlendingState.GetAddressOf());
	ThrowIfFailed(result, "Failed to create an alpha enabled blend state");

	// Modify the description to create an alpha disabled blend state description.
	blendStateDescription.RenderTarget[0].BlendEnable = FALSE;

	// Create the blend state using the description.
	result = m_device->CreateBlendState(&blendStateDescription, m_alphaDisableBlendingState.GetAddressOf());
	ThrowIfFailed(result, "Failed to create an alpha disabled blend state");
}


void D3DClass::ResizeBuffers(float screenWidth, float screenHeight, float screenDepth, float screenNear)
{
	if (m_swapChain)
	{
		m_deviceContext->OMSetRenderTargets(0, 0, 0);

		// Release all outstanding references to the swap chain's buffers.
		m_renderTargetView.Get()->Release();

		HRESULT hr;
		// Preserve the existing buffer count and format.
		// Automatically choose the width and height to match the client rect for HWNDs.
		hr = m_swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

		CreateRenderTargetView();
		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, NULL);
		SetViewport(screenWidth, screenHeight);
		CreateMatrices(screenWidth, screenHeight, screenDepth, screenNear);
	}

	return;
}


void D3DClass::CreateRenderTargetView()
{
	// Get the pointer to the back buffer.
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBufferPtr;
	ThrowIfFailed(
		m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr),
		"Failed to get the pointer to the back buffer."
	);

	// Create the render target view with the back buffer pointer.
	ThrowIfFailed(
		m_device->CreateRenderTargetView(backBufferPtr.Get(), NULL, &m_renderTargetView),
		"Failed to create the render target view"
	);
}


void D3DClass::SetViewport(float screenWidth, float screenHeight)
{
	// Set up the viewport.
	D3D11_VIEWPORT vp;
	vp.Width = screenWidth;
	vp.Height = screenHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_deviceContext->RSSetViewports(1u, &vp);
}


void D3DClass::CreateMatrices(float screenWidth, float screenHeight, float screenDepth, float screenNear)
{
	// Setup the projection matrix.
	float fieldOfView = DirectX::XM_PIDIV4;
	float screenAspect = screenWidth / screenHeight;

	// Create the projection matrix for 3D rendering.
	D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, fieldOfView, screenAspect, screenNear, screenDepth);

	// Initialize the world matrix to the identity matrix.
	D3DXMatrixIdentity(&m_worldMatrix);

	// Create an orthographic projection matrix for 2D rendering.
	D3DXMatrixOrthoLH(&m_orthoMatrix, screenWidth, screenHeight, screenNear, screenDepth);
}


void D3DClass::BeginScene(const DirectX::XMVECTORF32& color)
{
	// Clear the back buffer.
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), color);

	// Clear the depth buffer.
	m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);

	return;
}


void D3DClass::EndScene()
{
	// Present the back buffer to the screen since rendering is complete.
	m_swapChain->Present(m_vsync_enabled ? 1u : 0u, 0x0u);
}


ID3D11Device* D3DClass::GetDevice()
{
	return m_device.Get();
}


ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return m_deviceContext.Get();
}


void D3DClass::GetProjectionMatrix(D3DXMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
	return;
}


void D3DClass::GetWorldMatrix(D3DXMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
	return;
}


void D3DClass::GetOrthoMatrix(D3DXMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
	return;
}


void D3DClass::TurnZBufferOn()
{
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 1);
	return;
}


void D3DClass::TurnZBufferOff()
{
	m_deviceContext->OMSetDepthStencilState(m_depthDisabledStencilState.Get(), 1);
	return;
}


void D3DClass::TurnOnAlphaBlending()
{
	// Turn on the alpha blending.
	m_deviceContext->OMSetBlendState(m_alphaEnableBlendingState.Get(), {}, 0xffffffff);

	return;
}


void D3DClass::TurnOffAlphaBlending()
{
	// Turn off the alpha blending.
	m_deviceContext->OMSetBlendState(m_alphaDisableBlendingState.Get(), {}, 0xffffffff);

	return;
}