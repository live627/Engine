////////////////////////////////////////////////////////////////////////////////
// Filename: inputclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "game.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: InputClass
////////////////////////////////////////////////////////////////////////////////
class InputClass : public GameObject
{
public:
	InputClass(HINSTANCE hinstance, HWND hwnd);
	bool Initialize() { return true; }
	void Frame();

	void GetMousePositionForDebug(int & mouseX, int & mouseY);
	bool IsMouseButtonDown(MouseButtonID mouseButton);

	void KeyDown(unsigned int);
	void KeyUp(unsigned int);

	bool IsKeyDown(unsigned int);

private:
	HINSTANCE hinstance;
	HWND hwnd;
	bool m_keys[256] = {};
};

#endif