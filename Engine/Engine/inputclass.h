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
class InputClass : public IGameObject
{
public:
	InputClass(HINSTANCE hinstance, HWND hwnd);
	bool Initialize() { return true; }
	void Frame();

	POINT GetMousePositionDelta();
	void GetMousePositionForDebug(int & mouseX, int & mouseY);
	void WndMouse(UINT, WPARAM);
	void WndMouseWheel(WPARAM delta) { wheel = delta; }

	bool IsKeyDown(uint8_t key) const { return m_keys[key]; }
	void WndKey(UINT, WPARAM, LPARAM);

private:
	HINSTANCE hinstance;
	HWND hwnd;
	bool m_keys[256] = {};
	unsigned char wheel = 0;
	POINT cursor = {};
};

#endif