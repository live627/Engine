////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "systemclass.h"


SystemClass::SystemClass()
{
	m_Input = 0;
	m_Graphics = 0;
}


bool SystemClass::Initialize()
{
	int screenWidth, screenHeight;
	bool result;


	// Initialize the width and height of the screen to zero before sending the variables into the function.
	screenWidth = 0;
	screenHeight = 0;

	// Initialize the windows api.
	InitializeWindows(screenWidth, screenHeight);

	// Create the input object.  This object will be used to handle reading the keyboard input from the user.
	m_Input = new InputClass(m_hinstance, m_hwnd);
	if (!m_Input)
	{
		return false;
	}

	auto camera = new CameraClass(m_Input);

	// Create the graphics object.  This object will handle rendering all the graphics for this application.
	m_Graphics = new GraphicsClass(camera);
	if (!m_Graphics)
	{
		return false;
	}

	// Initialize the graphics object.
	result = m_Graphics->Initialize(screenWidth, screenHeight, m_hwnd);
	if (!result)
	{
		return false;
	}

	m_gameObjects.insert(make_pair(L"cpu", new CpuClass));
	m_gameObjects.insert(make_pair(L"input", m_Input));
	m_gameObjects.insert(make_pair(L"camera", camera));

	for (auto gameObject : m_gameObjects)
	{
		if (!gameObject.second->Initialize())
		{
			wchar_t buf[100];
			wchar_t * msg = L"Could not initialize the %s object.";
			swprintf_s(buf, wcslen(buf), msg, gameObject.first.c_str());
			MessageBox(m_hwnd, buf, L"Error", MB_OK);

			return false;
		}
	}
	ifstream file;
	file.open("autosave.bin", ios_base::binary);
	if (file.is_open())
	{
		for (auto gameObject : m_gameObjects)
		{
			gameObject.second->Load(file);
		}
	}
	file.close();

	// Allow the autosave loop to start.
	m_keepSavingFile.store(true);

	return true;
}


void SystemClass::Shutdown()
{
	// End the autosave loop.
	m_keepSavingFile.store(false);

	// All GameObjects must be ended.
	for (auto gameObject : m_gameObjects)
	{
		gameObject.second->Shutdown();
		delete gameObject.second;
	}

	// Release the graphics object.
	if (m_Graphics)
	{
		m_Graphics->Shutdown();
		delete m_Graphics;
		m_Graphics = 0;
	}

	// Shutdown the window.
	ShutdownWindows();

	return;
}


void SystemClass::Run()
{
	MSG msg;
	bool done, result;


	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));

	// Loop until there is a quit message from the window or the user.
	done = false;
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
			result = Frame();
			if (!result)
			{
				MessageBox(m_hwnd, L"Frame Processing Failed", L"Error", MB_OK);
				done = true;
			}
		}

		// Check if the user pressed escape and wants to exit the application.
		if (m_Input->IsKeyDown(VK_ESCAPE))
		{
			done = true;
		}
	}

	return;
}


bool SystemClass::Frame()
{
	bool result;
	int mouseX, mouseY;


	for (auto gameObject : m_gameObjects)
	{
		gameObject.second->Frame();
	}


	auto obj = static_cast<CpuClass*>(m_gameObjects.at(std::wstring(L"cpu")));
	auto cpuPercentage = obj->GetCpuPercentage();
	auto fps = obj->GetFps();
	auto frameTimeDelta = obj->GetTime();

	// Get the location of the mouse from the input object,
	m_Input->GetMousePositionForDebug(mouseX, mouseY);

	// Do the frame processing for the graphics object.
	result = m_Graphics->Frame(mouseX, mouseY, fps, cpuPercentage, frameTimeDelta, !m_isGameActive);
	if (!result)
	{
		return false;
	}

	// Finally render the graphics to the screen.
	result = m_Graphics->Render();
	if (!result)
	{
		return false;
	}

	return true;
}


void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
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
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);

	// Determine the resolution of the clients desktop screen.
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if (FULL_SCREEN)
	{
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

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
	m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
		dwStyle, posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// Hide the mouse cursor.
	//ShowCursor(false);

	return;
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
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_applicationName, m_hinstance);
	m_hinstance = NULL;

	// Release the pointer to this class.
	ApplicationHandle = NULL;

	return;
}


LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
	/*case WM_CREATE:
	{
		RAWINPUTDEVICE Rid[2];

		Rid[0].usUsagePage = 0x01;
		Rid[0].usUsage = 0x02;
		Rid[0].dwFlags = RIDEV_NOLEGACY;   // adds HID mouse and also ignores legacy mouse messages
		Rid[0].hwndTarget = hwnd;
		
		if (!RegisterRawInputDevices(&Rid[0], 1, sizeof(RAWINPUTDEVICE)))
			return -1;
	}
	return 0;*/

	// Check if a key has been pressed on the keyboard.
	case WM_KEYDOWN:
	{
		// If a key is pressed send it to the input object so it can record that state.
		m_Input->KeyDown((uint)wparam);
		return 0;
	}

	// Check if a key has been released on the keyboard.
	case WM_KEYUP:
	{
		// If a key is released then send it to the input object so it can unset the state for that key.
		m_Input->KeyUp((unsigned int)wparam);
		return 0;
	}

	// Check if the window is being closed.
	case WM_CLOSE:
	{
		if (MessageBox(hwnd, L"Are you sure you want to quit?", m_applicationName, MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2) == IDOK)
		{
			DestroyWindow(hwnd);
		}
		// Else: User canceled. Do nothing.
		return 0;
	}

	//case WM_ACTIVATE:
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

	//case WM_SIZE:
	//	if (!m_isGameHalted)
	//	{
	//		RECT rc;
	//		GetClientRect(hwnd, &rc);

	//		m_Graphics->ResizeBuffers(LOWORD(lparam), HIWORD(lparam));
	//	}
	//	break;

	case WM_SYSCOMMAND:
		switch (wparam)
		{
		case SC_MINIMIZE: {
			m_isGameHalted = true;
			break;
		}
		case SC_RESTORE: {
			m_isGameHalted = false;
			break;
		}
	}
		return DefWindowProc(hwnd, umsg, wparam, lparam);

	// Any other messages send to the default message handler as our application won't make use of them.
	default:
	{
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
	}
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		// Check if the window is being destroyed.
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	// All other messages pass to the message handler in the system class.
	default:
	{
		return ApplicationHandle->MessageHandler(hWnd, message, wParam, lParam);
	}
	return 0;
	}
}


void SystemClass::Autosave()
{
	bool canRetry = false;
	const uint retryTime = 1;
	const uint spinTime = 5;
	ofstream file;
	file.exceptions(std::fstream::failbit | std::fstream::badbit);

	while (m_keepSavingFile)
	{
		std::this_thread::sleep_for(
			std::chrono::duration<uint>(canRetry ? retryTime : spinTime)
		);
		canRetry = false;
		try
		{
			file.open("autosave.bin", ios_base::binary);
			for (auto gameObject : m_gameObjects)
				gameObject.second->Save(file);
			file.close();
		}
		catch (const std::ios_base::failure &e)
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
}
