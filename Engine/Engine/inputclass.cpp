////////////////////////////////////////////////////////////////////////////////
// Filename: inputclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "inputclass.h"


InputClass::InputClass(HINSTANCE hinstance, HWND hwnd)
{
	this->hinstance = hinstance;
	this->hwnd = hwnd;


	// Initialize all the keys to being released and not pressed.
	for (int i = 0; i<256; i++)
		m_keys[i] = false;

	//// Initialize the location of the mouse on the screen.
	//m_mouseX = 0;
	//m_mouseY = 0;
	//m_mouseZ = 0;
}


bool InputClass::Initialize()
{
	HRESULT result;
	

	// Initialize the main direct input interface.
	result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Initialize the direct input interface for the mouse.
	result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// Set the data format for the mouse using the pre-defined mouse data format.
	result = m_mouse->SetDataFormat(&c_dfDIMouse2);
	if (FAILED(result))
	{
		return false;
	}

	// Set the cooperative level of the mouse to share with other programs.
	result = m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result))
	{
		return false;
	}

	// Acquire the mouse.
	result = m_mouse->Acquire();
	if (FAILED(result))
	{
		return false;
	}

	return true;
}


void InputClass::Shutdown()
{
	// Release the mouse.
	if (m_mouse)
	{
		m_mouse->Unacquire();
		m_mouse->Release();
		m_mouse = 0;
	}

	// Release the main interface to direct input.
	if (m_directInput)
	{
		m_directInput->Release();
		m_directInput = 0;
	}

	return;
}


void InputClass::Frame()
{
	HRESULT result;


	// Read the mouse device.
	result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE2), (LPVOID)&m_mouseState);
	if (FAILED(result))
	{
		// If the mouse lost focus or was not acquired then try to get control back.
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_mouse->Acquire();
		}
		else
		{
			// TODO Implement error output
		}
	}

	// Update the location of the mouse cursor based on the change of the mouse location during the frame.
	//m_mouseX = m_mouseState.lX;
	//m_mouseY = m_mouseState.lY;
	//m_mouseZ = m_mouseState.lZ;
	SetDeltaX(m_mouseState.lX);
	SetDeltaY(m_mouseState.lY);
	SetDeltaZ(m_mouseState.lZ);

	for (unsigned int i = 0; i < 8; i++)
		UpdateButtonState(i, m_mouseState.rgbButtons[i]);
}


void InputClass::GetMousePositionDelta(int& mouseX, int& mouseY)
{
	//mouseX = m_mouseX;
	//mouseY = m_mouseY;
	mouseX = GetDeltaX();
	mouseY = GetDeltaY();
}


void InputClass::GetMousePositionForDebug(int& mouseX, int& mouseY)
{
	POINT cursor;
	GetCursorPos(&cursor);
	mouseX = cursor.x;
	mouseY = cursor.y;
}


void InputClass::UpdateButtonState(unsigned int mouseButton, unsigned char di)
{
	if (di & 0x80 && !IsMouseButtonDown((MouseButtonID)mouseButton))
		buttons |= 1 << mouseButton; //turn the bit flag on
	else if (!(di & 0x80) && IsMouseButtonDown((MouseButtonID)mouseButton))
		buttons &= ~(1 << mouseButton); //turn the bit flag off
}


bool InputClass::IsMouseButtonDown(MouseButtonID mouseButton)
{
	// Return what state the MouseButton is in (pressed/not pressed).
	return buttons & (1L << mouseButton) == 0;
}


void InputClass::KeyDown(unsigned int input)
{
	// If a key is pressed then save that state in the key array.
	m_keys[input] = true;
}


void InputClass::KeyUp(unsigned int input)
{
	// If a key is released then clear that state in the key array.
	m_keys[input] = false;
}


bool InputClass::IsKeyDown(unsigned int key)
{
	// Return what state the key is in (pressed/not pressed).
	return m_keys[key];
}