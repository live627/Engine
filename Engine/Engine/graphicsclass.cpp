////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"


GraphicsClass::GraphicsClass(CameraClass * p_Camera, int screenWidth, int screenHeight, HWND p_hwnd)
{
	m_D3D = 0;
	m_Camera = 0;
	m_Shader = 0;
	m_Bitmap = 0;
	m_Text = 0;

	m_Camera = p_Camera;
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	m_hwnd = p_hwnd;
}


bool GraphicsClass::Initialize()
{
	bool result;
	D3DXMATRIX baseViewMatrix;

	// Create the Direct3D object.
	m_D3D = new D3DClass(m_screenWidth, m_screenHeight, m_hwnd, VSYNC_ENABLED);

	// Initialize the Direct3D object.
	m_D3D->Initialize(FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);

	// Initialize a base view matrix with the camera for 2D user interface rendering.
	m_Camera->SetPosition(0.0f, 0.0f, -1.0f);
	m_Camera->Render();
	m_Camera->GetViewMatrix(baseViewMatrix);

	// Create the font object.
	m_Font = new FontManager(m_D3D->GetDevice(), m_D3D->GetDeviceContext());
	result = m_Font->Initialize();
	if (!result)
	{
		return false;
	}

	// Create the texture shader object.
	m_Shader = new ShaderClass(m_D3D->GetDevice(), m_D3D->GetDeviceContext());
	if (!m_Shader)
	{
		return false;
	}

	// Initialize the texture shader object.
	result = m_Shader->Initialize();
	if (!result)
	{
		throw std::runtime_error("Could not initialize the texture shader object.");
		return false;
	}

	// Create the bitmap object.
	m_Bitmap = new BitmapClass;

	// Initialize the bitmap object.
	result = m_Bitmap->Initialize(m_D3D->GetDevice(), m_screenWidth, m_screenHeight, L"../Engine/data/seafloor.dds");
	if (!result)
		throw std::runtime_error("Could not initialize the bitmap object.");

	// Create the text object.
	m_Text = new TextClass(m_D3D->GetDevice(), m_D3D->GetDeviceContext());

	// Initialize the text object.
	result = m_Text->Initialize(nullptr, m_screenWidth, m_screenHeight, baseViewMatrix, m_Font);
	if (!result)
		throw std::runtime_error("Could not initialize the text object.");

	return true;
}


void GraphicsClass::Shutdown()
{
	// Release the bitmap object.
	if (m_Bitmap)
	{
		m_Bitmap->Shutdown();
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
		m_Text->Shutdown();
		delete m_Text;
		m_Text = 0;
	}

	return;
}


bool GraphicsClass::Dbg(int mouseX, int mouseY, int fps, 
	int cpu, float frameTime, bool isGamePaused)
{
	bool result;

	// Set the frames per second.
	result = m_Text->SetFps(fps, frameTime);
	if (!result)
	{
		return false;
	}

	// Set the cpu usage.
	result = m_Text->SetCpu(cpu);
	if (!result)
	{
		return false;
	}

	// Set the location of the mouse.
	result = m_Text->SetMousePosition(mouseX, mouseY);
	if (!result)
	{
		return false;
	}

	// Set the position of the camera.
	result = m_Text->SetCameraPosition(m_Camera->GetPosition());
	if (!result)
	{
		return false;
	}

	result &= m_Text->SetPausedState(isGamePaused);

	return true;
}


bool GraphicsClass::Render()
{
	D3DXMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	bool result;

	// Clear the buffers to begin the scene.
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

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
	result = m_Bitmap->Render(m_D3D->GetDeviceContext(), { 100, 100, 256, 256 });
	if (!result)
	{
		return false;
	}

	// Render the bitmap with the texture shader.
	result = m_Shader->Render(m_Bitmap->GetIndexCount(), 
		worldMatrix, viewMatrix, orthoMatrix, m_Bitmap->GetTexture(), {});
	if (!result)
	{
		return false;
	}

	// Turn on the alpha blending before rendering the text.
	m_D3D->TurnOnAlphaBlending();
	
	// Render the text strings.
	result = m_Text->Render(worldMatrix, orthoMatrix);
	if (!result)
	{
		return false;
	}

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