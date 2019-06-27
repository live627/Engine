////////////////////////////////////////////////////////////////////////////////
// Filename: inputclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include <array>
#include "game.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: InputClass
////////////////////////////////////////////////////////////////////////////////
class InputClass : public IGameObject
{
public:
	InputClass(HINSTANCE hinstance, HWND hwnd);
	void Frame();

	POINT GetMousePositionDelta();
	POINT GetMousePosition();
	void GetMousePositionForDebug(int & mouseX, int & mouseY);
	void WndMouse(UINT, WPARAM);
	void WndMouseWheel(WPARAM delta) { wheel = delta; }

	bool IsKeyDown(uint8_t key) const { return m_keys[key]; }
	void WndKey(UINT, WPARAM, LPARAM);
	const std::vector<bool> GetKeys() const { return m_keys; }

private:
	HINSTANCE hinstance;
	HWND hwnd;
	std::vector<bool> m_keys;
	unsigned char wheel = 0;
	POINT cursor = {};
};

#endif