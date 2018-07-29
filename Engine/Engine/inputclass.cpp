////////////////////////////////////////////////////////////////////////////////
// Filename: inputclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "inputclass.h"


InputClass::InputClass(HINSTANCE hinstance, HWND hwnd)
{
	this->hinstance = hinstance;
	this->hwnd = hwnd;
}


void InputClass::Frame()
{


void InputClass::GetMousePositionForDebug(int& mouseX, int& mouseY)
{
	POINT cursor;
	GetCursorPos(&cursor);
	mouseX = cursor.x;
	mouseY = cursor.y;
}


void InputClass::WndMouse(UINT message, WPARAM wParam)
{
	if (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN || message == WM_MBUTTONDOWN || message == WM_XBUTTONDOWN)
	{
		SetCapture(hwnd);

		if (wParam & MK_LBUTTON)
			m_keys[VK_LBUTTON] = true;

		if (wParam & MK_RBUTTON)
			m_keys[VK_RBUTTON] = true;

		if (wParam & MK_MBUTTON)
			m_keys[VK_MBUTTON] = true;

		if (wParam & MK_XBUTTON1)
			m_keys[VK_XBUTTON1] = true;

		if (wParam & MK_XBUTTON2)
			m_keys[VK_XBUTTON2] = true;
	}
	else
	{
		m_keys[VK_LBUTTON] = false;
		m_keys[VK_RBUTTON] = false;
		m_keys[VK_MBUTTON] = false;
		m_keys[VK_XBUTTON1] = false;
		m_keys[VK_XBUTTON2] = false;

		ReleaseCapture();
	}
}


void InputClass::WndKey(UINT message, WPARAM wParam, LPARAM lParam)
{
	unsigned int scancode = (lParam >> 16) & 0xff;
	unsigned int extended = (lParam >> 24) & 0x1;
	bool pressed = message == WM_KEYDOWN || message == WM_SYSKEYDOWN;

	if (extended)
	{
		if (wParam == VK_CONTROL)
			wParam = VK_RCONTROL;
		else if (wParam == VK_MENU)
			wParam = VK_RMENU;
	}
	if (scancode == 0x2a)
		wParam = VK_LSHIFT;
	if (scancode == 0x36)
		wParam = VK_RSHIFT;

	m_keys[wParam] = pressed;
}