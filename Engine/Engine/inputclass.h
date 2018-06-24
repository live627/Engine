////////////////////////////////////////////////////////////////////////////////
// Filename: inputclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_


///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
#define DIRECTINPUT_VERSION 0x0800


/////////////
// LINKING //
/////////////
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")


//////////////
// INCLUDES //
//////////////
#include <dinput.h>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "game.h"


//! Button ID for mouse devices
enum MouseButtonID {
	MB_Left = 0,
	MB_Right,
	MB_Middle,
	MB_Button3,
	MB_Button4,
	MB_Button5,
	MB_Button6,
	MB_Button7
};


////////////////////////////////////////////////////////////////////////////////
// Class name: InputClass
////////////////////////////////////////////////////////////////////////////////
class InputClass : public GameObject
{
public:
	InputClass(HINSTANCE hinstance, HWND hwnd);
	bool Initialize();
	void Shutdown();
	void Frame();

	void GetMousePositionDelta(int & mouseX, int & mouseY);
	void GetMousePositionForDebug(int & mouseX, int & mouseY);
	bool IsMouseButtonDown(MouseButtonID mouseButton);

	void KeyDown(unsigned int);
	void KeyUp(unsigned int);

	bool IsKeyDown(unsigned int);

private:
	HINSTANCE hinstance;
	HWND hwnd;
	IDirectInput8 * m_directInput;
	IDirectInputDevice8* m_mouse;
	DIMOUSESTATE2 m_mouseState;

	//int m_mouseX, m_mouseY, m_mouseZ;

	//! represents all keys - array position indicates virtual key from Windows meessages
	bool m_keys[256];

	//! represents all buttons - bit position indicates button down
	unsigned char buttons = 0x0u;

	//! represents delta of all mouse axis - values from Windows messages
	// leaast significant byte is delta X
	// next byte to the left is delta y
	// next byte to the left is delta z
	// most significant byte is unused
	unsigned int dword = 0x0u;

public:
	constexpr unsigned char GetDeltaZ() const { return(dword >> 16u) & 0xFFu; }
	constexpr unsigned char GetDeltaY() const { return(dword >> 8u) & 0xFFu; }
	constexpr unsigned char GetDeltaX() const { return dword & 0xFFu; }
	void SetDeltaZ(unsigned char deltaZ) { dword = (dword & 0xFF00FFFFu) | (deltaZ << 16u); }
	void SetDeltaY(unsigned char deltaY) { dword = (dword & 0xFFFF00FFu) | (deltaY << 8u); }
	void SetDeltaX(unsigned char deltaX) { dword = (dword & 0xFFFFFF00u) | deltaX; }

private:
	void UpdateButtonState(unsigned int mouseButton, unsigned char di);
};

#endif