////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"


GraphicsClass::GraphicsClass(CameraClass * p_Camera,
	int screenWidth, int screenHeight, HWND p_hwnd)
	:
	m_Camera(p_Camera),
	m_screenWidth(screenWidth),
	m_screenHeight(screenHeight),
	m_hwnd(p_hwnd),
	m_D3D(screenWidth, screenHeight, p_hwnd, 
		VSYNC_ENABLED, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR),
	m_Font(m_D3D.GetDevice(), m_D3D.GetDeviceContext()),
	m_Shader(m_D3D.GetDevice(), m_D3D.GetDeviceContext()),
	m_Bitmap(
		m_D3D.GetDevice(), m_D3D.GetDeviceContext(),
		screenWidth, screenHeight, "../Engine/data/seafloor.dds"
	),
	m_Text(
		m_D3D.GetDevice(), m_D3D.GetDeviceContext(),
		screenWidth, screenHeight, &m_Font, m_Camera->GetViewMatrix()
	)
{}


void GraphicsClass::SetPausedState(bool isGamePaused)
{
	m_Text.SetPausedState(isGamePaused);
}


void GraphicsClass::Frame()
{
	// Finally render the graphics to the screen.
	Render();
}


bool GraphicsClass::Render()
{
	DirectX::XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;

	// Clear the buffers to begin the scene.
	m_D3D.BeginScene(DirectX::Colors::Black);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, projection, and ortho matrices from the camera and d3d objects.
	viewMatrix = m_Camera->GetViewMatrix();
	m_D3D.GetWorldMatrix(worldMatrix);
	m_D3D.GetProjectionMatrix(projectionMatrix);
	m_D3D.GetOrthoMatrix(orthoMatrix);

	// Turn off the Z buffer to begin all 2D rendering.
	m_D3D.TurnZBufferOff();

	// Put the bitmap vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_Bitmap.Render({ 100, 100, 256, 256 });

	// Render the bitmap with the texture shader.
	m_Shader.Render(m_Bitmap.GetIndexCount(), 
		worldMatrix, viewMatrix, orthoMatrix, m_Bitmap.GetTexture(), {});

	// Turn on the alpha blending before rendering the text.
	m_D3D.TurnOnAlphaBlending();
	
	// Render the text strings.
	m_Text.Render(worldMatrix, orthoMatrix);

	// Turn off alpha blending after rendering the text.
	m_D3D.TurnOffAlphaBlending();

	// Turn the Z buffer back on now that all 2D rendering has completed.
	m_D3D.TurnZBufferOn();

	// Present the rendered scene to the screen.
	m_D3D.EndScene();

	return true;
}


void GraphicsClass::ResizeBuffers(int width, int height)
{
	m_D3D.ResizeBuffers(static_cast<float>(width), static_cast<float>(height), SCREEN_DEPTH, SCREEN_NEAR);
	m_Bitmap.ResizeBuffers(width, height);
	m_Text.ResizeBuffers(width, height);
}