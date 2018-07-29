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


{
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