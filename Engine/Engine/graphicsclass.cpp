////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"


GraphicsClass::GraphicsClass(CameraClass * p_Camera, CpuClass * p_dbg, 
	int screenWidth, int screenHeight, HWND p_hwnd)
	:
	m_dbg(p_dbg),
	m_Camera(p_Camera),
	m_screenWidth(screenWidth),
	m_screenHeight(screenHeight),
	m_hwnd(p_hwnd)
{

	DirectX::XMMATRIX baseViewMatrix;

	m_D3D = new D3DClass(m_screenWidth, m_screenHeight, m_hwnd, 
		VSYNC_ENABLED, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);

	// Initialize a base view matrix with the camera for 2D user interface rendering.
	m_Camera->SetPosition(0.0f, 0.0f, -1.0f);
	m_Camera->Render();
	m_Camera->GetViewMatrix(baseViewMatrix);

	// Create the font object.
	m_Font = new Fonts(m_D3D->GetDevice(), m_D3D->GetDeviceContext());

	// Create the texture shader object.
	m_Shader = new ShaderClass(m_D3D->GetDevice(), m_D3D->GetDeviceContext());

	// Create the bitmap object.
	m_Bitmap = new BitmapClass(
		m_D3D->GetDevice(), m_D3D->GetDeviceContext(),
		m_screenWidth, m_screenHeight, "../Engine/data/seafloor.tga"
	);

	// Create the text object.
	m_Text = new TextClass(
		m_D3D->GetDevice(), m_D3D->GetDeviceContext(),
		m_screenWidth, m_screenHeight, m_Font, baseViewMatrix
	);
}


void GraphicsClass::Shutdown()
{
	// Release the bitmap object.
	if (m_Bitmap)
	{
		delete m_Bitmap;
		m_Bitmap = 0;
	}

	// Release the texture shader object.
	if (m_Shader)
	{
		delete m_Shader;
		m_Shader = 0;
	}

	// Release the font object.
	if (m_Font)
	{
		delete m_Font;
		m_Font = 0;
	}

	// Release the D3D object.
	if (m_D3D)
	{
		delete m_D3D;
		m_D3D = 0;
	}

	// Release the text object.
	if (m_Text)
	{
		delete m_Text;
		m_Text = 0;
	}
}


void GraphicsClass::DebugMousePosition(int& mouseX, int& mouseY)
{
	m_Text->SetMousePosition(mouseX, mouseY);
}


void GraphicsClass::SetPausedState(bool isGamePaused)
{
	m_Text->SetPausedState(isGamePaused);
}


void GraphicsClass::UpdateDebugInfo()
{
	m_Text->SetFps(m_dbg->GetFps(), m_dbg->GetFrameTimeDelta());
	m_Text->SetCpu(m_dbg->GetCpuPercentage());
	m_Text->SetCameraPosition(m_Camera->GetPosition());
}


void GraphicsClass::Frame()
{

	UpdateDebugInfo();

	// Finally render the graphics to the screen.
	Render();
}


bool GraphicsClass::Render()
{
	DirectX::XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;

	// Clear the buffers to begin the scene.
	m_D3D->BeginScene(DirectX::Colors::Black);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, projection, and ortho matrices from the camera and d3d objects.
	m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);
	m_D3D->GetOrthoMatrix(orthoMatrix);

	// Turn off the Z buffer to begin all 2D rendering.
	m_D3D->TurnZBufferOff();

	// Put the bitmap vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_Bitmap->Render({ 100, 100, 256, 256 });

	// Render the bitmap with the texture shader.
	m_Shader->Render(m_Bitmap->GetIndexCount(), 
		worldMatrix, viewMatrix, orthoMatrix, m_Bitmap->GetTexture(), {});

	// Turn on the alpha blending before rendering the text.
	m_D3D->TurnOnAlphaBlending();
	
	// Render the text strings.
	m_Text->Render(worldMatrix, orthoMatrix);

	// Turn off alpha blending after rendering the text.
	m_D3D->TurnOffAlphaBlending();

	// Turn the Z buffer back on now that all 2D rendering has completed.
	m_D3D->TurnZBufferOn();

	// Present the rendered scene to the screen.
	m_D3D->EndScene();

	return true;
}


void GraphicsClass::ResizeBuffers(int width, int height)
{
	if (this)
	{
		m_D3D->ResizeBuffers(width, height, SCREEN_DEPTH, SCREEN_NEAR);
		m_Bitmap->ResizeBuffers(width, height);
		m_Text->ResizeBuffers(width, height);
	}
}