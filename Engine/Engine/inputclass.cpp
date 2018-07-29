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