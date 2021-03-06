////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "systemclass.h"


SystemClass::SystemClass()
{
	int screenWidth, screenHeight;

	try
	{
		// Initialize the windows api.
		InitializeWindows(screenWidth, screenHeight);

		m_Settings = Settings();
		m_Input = new InputClass(m_hinstance, m_hwnd);
		auto camera = new CameraClass(m_Input, screenWidth, screenHeight);
		m_Graphics = new GraphicsClass(camera, screenWidth, screenHeight, 1, m_hwnd, &m_Settings);

		m_gameObjects.push_back(new CpuClass(m_Input, camera, m_Graphics->GetText()));
		m_gameObjects.push_back(camera);
		m_gameObjects.push_back(m_Input);
		m_gameObjects.push_back(m_Graphics);

		std::ifstream file;
		file.open("autosave.bin", std::ios_base::binary);
		if (file.is_open())
		{
			BinaryReader reader(file);
			for (const auto & gameObject : m_gameObjects)
				gameObject->Load(reader);
		}
		file.close();

		// Fire up the autosave thread.
		thread_to_save_file = std::thread(&SystemClass::Autosave, this);

		// Allow the autosave loop to start.
		m_keepSavingFile.store(true);

		// Game can start now.
		Run();
	}
	catch (std::exception & e)
	{
		std::clog << e.what() << std::endl;
		char * buf = new char[1060];
		char * msg = "%s\n\nApplication will now quit.";
		sprintf_s(buf, 1060, msg, e.what());
		MessageBoxA(m_hwnd, buf, "Error", MB_OK | MB_ICONERROR);
	}
}


SystemClass::~SystemClass()
{
	// End the autosave loop.
	m_keepSavingFile.store(false);

	// All GameObjects must be ended.
	for (auto gameObject : m_gameObjects)
	{
 		gameObject->Shutdown();
		delete gameObject;
	}

	// Shutdown the window.
	ShutdownWindows();

	// Wait until the autosave thread finishes.	
	thread_to_save_file.join();
}


void SystemClass::Run()
{
	MSG msg;

	// Loop until there is a quit message from the window or the user.
	bool done = false;
	while (!done)
	{
		// Handle the windows messages.
		if (!m_isGameHalted && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Oh, but we minimized, so freeze while waiting for a message.
		else if (m_isGameHalted && GetMessage(&msg, 0, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else if (!m_isGameHalted)
		{
			// Otherwise do the frame processing.  If frame processing fails then exit.
			try
			{
				m_Graphics->SetPausedState(!m_isGameActive);

				for (const auto & gameObject : m_gameObjects)
				{
					gameObject->Frame();
				}
			}
			catch (std::exception & e)
			{
				char * buf = new char[1060];
				char * msg = "Failed to process frames:\n\n%s\n\nApplication will now quit.";
				sprintf_s(buf, 1060, msg, e.what());
				MessageBoxA(m_hwnd, buf, "Error", MB_OK | MB_ICONERROR);
				done = true;
			}
		}

		// Check if the user pressed escape and wants to exit the application.
		if (m_Input->IsKeyDown(VK_ESCAPE))
			done = true;
	}
}


void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	int posX, posY;
	int dwStyle = WS_VISIBLE;
	InitializeScaling();

	// Get an external pointer to this object.	
	ApplicationHandle = this;

	// Get the instance of this application.
	m_hinstance = GetModuleHandle(NULL);

	// Give the application a name.
	m_applicationName = L"Random Game Engine";

	// Setup the windows class with default settings.
	WNDCLASS wc = {};
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = m_applicationName;

	// Register the window class.
	if (!RegisterClass(&wc))
		throw std::system_error(
			std::error_code(
				GetLastError(),
				std::system_category()
			),
			"Win32 error occured when trying to register the window class"
		);

	// Determine the resolution of the clients desktop screen.
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if (FULL_SCREEN)
	{
		// Set the position of the window to the top left corner.
		posX = posY = 0;

		// Clip any child windows to this window's bounds.
		dwStyle |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		// If windowed then set it to 800x600 resolution.
		screenWidth = static_cast<UINT>(ceil(ui::ScaleX(800)));
		screenHeight = static_cast<UINT>(ceil(ui::ScaleX(600)));

		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;

		// Draw a standard window with the complete titlebar etc.
		dwStyle |= WS_OVERLAPPEDWINDOW;
	}

	// Create the window with the screen settings and get the handle to it.
	m_hwnd = CreateWindow(m_applicationName, m_applicationName,
		dwStyle, posX, posY, screenWidth, screenHeight, 
		NULL, NULL, m_hinstance, NULL
	);

	if (!m_hwnd)
		throw std::system_error(
			std::error_code(
				GetLastError(),
				std::system_category()
			),
			"Win32 error occured when trying to create window"
		);
}


void SystemClass::InitializeScaling()
{
	HDC screen = GetDC(0);
	ui::scaleX = GetDeviceCaps(screen, LOGPIXELSX) / 96.0;
	ui::scaleY = GetDeviceCaps(screen, LOGPIXELSY) / 96.0;
	ReleaseDC(0, screen);
}


void SystemClass::ShutdownWindows()
{
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// Release the pointer to this class.
	ApplicationHandle = NULL;
}

LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		m_Input->WndKey(umsg, wparam, lparam);
		break;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
		m_Input->WndMouse(umsg, wparam);
		m_Graphics->Click(m_Input->GetKeys(), m_Input->GetMousePosition());
		break;

	case WM_MOUSEWHEEL:
		m_Input->WndMouseWheel(GET_WHEEL_DELTA_WPARAM(wparam));
		m_Input->WndMouse(umsg, GET_KEYSTATE_WPARAM(wparam));
		break;

	case WM_CLOSE:
		if (MessageBox(hwnd, L"Are you sure you want to quit?", m_applicationName, MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2) == IDOK)
			DestroyWindow(hwnd);
		break;

	case WM_ACTIVATEAPP:
		m_isGameActive = wparam != 0;
		break;

	case WM_EXITSIZEMOVE:
		if (!m_isGameHalted)
		{
			RECT rc;
			GetWindowRect(hwnd, &rc);

			m_Graphics->ResizeBuffers(rc.right - rc.left, rc.bottom - rc.top);
		}
		break;

	case WM_SYSCOMMAND:
		switch (wparam)
		{
		case SC_MINIMIZE: 
			m_isGameHalted = true;
			break;
		case SC_RESTORE: 
			m_isGameHalted = false;
			break;
		case SC_KEYMENU:
			break;
		}
		if (wparam == SC_CLOSE && (lparam >> 16) <= 0) 
			break;

		return DefWindowProc(hwnd, umsg, wparam, lparam);

	default:
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	return 0;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	// Check if the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	// All other messages pass to the message handler in the system class.
	default:
		return ApplicationHandle->MessageHandler(hWnd, message, wParam, lParam);
	}
	return 0;
}


void SystemClass::Autosave()
{
	using namespace std::chrono_literals;
	bool canRetry = false;
	const auto retryTime = 1s;
	auto spinTime = 5s;
	std::ofstream file;
	file.exceptions(std::fstream::failbit | std::fstream::badbit);

	while (true)
	{
		std::this_thread::sleep_for(canRetry ? retryTime : spinTime);
		canRetry = false;
		if (m_keepSavingFile)
		{
			try
			{
				file.open("autosave.bin", std::ios_base::binary);
				BinaryWriter writer(file);
				for (const auto & gameObject : m_gameObjects)
					gameObject->Save(writer);
				file.close();
			}
			catch (const std::ios_base::failure & e)
			{
				switch (MessageBoxA(
					m_hwnd, e.what(), "Autosave error",
					MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_DEFBUTTON2
				))
				{
				case IDABORT:
					m_keepSavingFile = false;
					break;

				case IDRETRY:
					canRetry = true;
					break;
				}
			}
		}
		else
			break;
	}
}


double ui::scaleX = 0;
double ui::scaleY = 0;