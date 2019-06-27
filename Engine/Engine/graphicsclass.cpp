////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"


GraphicsClass::GraphicsClass(CameraClass * p_Camera,
	size_t screenWidth, size_t screenHeight, size_t scale, HWND p_hwnd,
	Settings * p_settings)
	:
	m_Camera(p_Camera),
	m_screenWidth(screenWidth),
	m_screenHeight(screenHeight),
	m_scale(scale),
	m_D3D(screenWidth, screenHeight, scale, p_hwnd, 
		VSYNC_ENABLED, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR),
	m_RenderTexture(
		m_D3D.GetDevice(), m_D3D.GetDeviceContext(),
		m_D3D.GetDepthStencilView(), screenWidth * scale, screenHeight * scale
	),
	m_Font(m_D3D.GetDevice(), m_D3D.GetDeviceContext()),
	m_Shader(m_D3D.GetDevice(), m_D3D.GetDeviceContext(), "TexturePixelShader"),
	m_Shader2(m_D3D.GetDevice(), m_D3D.GetDeviceContext(), "HSV2RGBPixelShader"),
	tiles(
		m_D3D.GetDevice(), m_D3D.GetDeviceContext(), &m_Shader,
		p_Camera, p_settings,
		screenWidth, screenHeight,55,55,50,6,8,8,1// width, height, chanceToStartAlive, smoothingIterations,
		//octaves, freq, seed
	),
	m_Bitmap(
		m_D3D.GetDevice(), m_D3D.GetDeviceContext(),
		screenWidth, screenHeight, "../Engine/data/seafloor.dds"
	),
	m_FontShader(m_D3D.GetDevice(), m_D3D.GetDeviceContext(), "FontPixelShader"),
	m_Bitmap2(
		m_D3D.GetDevice(), m_D3D.GetDeviceContext(), &m_Shader2,
		screenWidth, screenHeight
	),
	m_Text(
		m_D3D.GetDevice(), m_D3D.GetDeviceContext(), &m_FontShader,
		screenWidth, screenHeight, &m_Font, m_Camera->GetViewMatrix()
	)
{
	m_D3D.GetWorldMatrix(worldMatrix);
	m_D3D.GetProjectionMatrix(projectionMatrix);
	m_D3D.GetOrthoMatrix(orthoMatrix);
	m_Camera->ResizeBuffers(screenWidth, screenHeight, worldMatrix, orthoMatrix);
	baseviewMatrix = m_Camera->GetViewMatrix();
	m_gameObjects.push_back(&tiles);
	m_gameObjects.push_back(&m_Text);
	m_Bitmap2.MakeChart({ 8,8 }, std::vector<float>({ 0.2f, 0.3f, 0.4f, 0.1f }));
}


void GraphicsClass::SetPausedState(bool isGamePaused)
{
	m_Text.SetPausedState(isGamePaused);
}


void GraphicsClass::Frame()
{
	BeforeRender();
	Render();
	AfterRender();
}

void GraphicsClass::BeforeRender()
{
	if (m_scale > 1)
	{
		m_RenderTexture.SetRenderTarget();
		m_D3D.SetViewport(m_screenWidth * m_scale, m_screenHeight * m_scale);
		m_RenderTexture.ClearRenderTarget(0.0f, 0.0f, 1.0f, 1.0f);
	}
	else
		m_D3D.BeginScene(DirectX::Colors::Black);

 	m_D3D.TurnZBufferOff();
}

void GraphicsClass::AfterRender()
{
	if (m_scale > 1)
	{
		m_D3D.SetBackBufferRenderTarget();
		m_D3D.BeginScene(DirectX::Colors::Black);
		m_D3D.SetViewport(m_screenWidth, m_screenHeight);
		m_Bitmap.Render({ 0, 0, (LONG)m_screenWidth, (LONG)m_screenHeight });
		m_Shader.Render(m_Bitmap.GetIndexCount(),
			worldMatrix, baseviewMatrix, orthoMatrix, m_RenderTexture.GetShaderResourceView(), {});
	}
	m_D3D.TurnZBufferOn();

	// Present the rendered scene to the screen.
	m_D3D.EndScene();
}

void GraphicsClass::Render()
{
	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	viewMatrix = m_Camera->GetViewMatrix();	
	
	for (const auto & gameObject : m_gameObjects)
		gameObject->Render(worldMatrix, orthoMatrix, viewMatrix);

	m_Bitmap2.Render(worldMatrix, orthoMatrix, viewMatrix);

	m_D3D.TurnOnAlphaBlending();
	
	for (const auto & gameObject : m_gameObjects)
		gameObject->RenderUI(worldMatrix, orthoMatrix);

	m_D3D.TurnOffAlphaBlending();
}


void GraphicsClass::ResizeBuffers(int width, int height)
{
	m_D3D.ResizeBuffers(float(width), float(height), SCREEN_DEPTH, SCREEN_NEAR);
	m_D3D.GetWorldMatrix(worldMatrix);
	m_D3D.GetProjectionMatrix(projectionMatrix);
	m_D3D.GetOrthoMatrix(orthoMatrix);
	m_Camera->ResizeBuffers(width, height, worldMatrix, projectionMatrix);
	m_Bitmap.ResizeBuffers(width, height);
	m_Text.ResizeBuffers(width, height);
}


void GraphicsClass::Save(BinaryWriter & writer)
{
	for (const auto & gameObject : m_gameObjects)
		gameObject->Save(writer);
}

void GraphicsClass::Load(BinaryReader & reader)
{
	for (const auto & gameObject : m_gameObjects)
		gameObject->Load(reader);
}

void GraphicsClass::Click(const std::vector<bool> keys, POINT point)
{
	for (const auto & gameObject : m_gameObjects)
		gameObject->OnClick(keys, point);
}