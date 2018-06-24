////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_


///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
#define WIN32_LEAN_AND_MEAN


//////////////
// INCLUDES //
//////////////
#include <windows.h>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <chrono>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "game.h"
#include "ui.h"

#include "inputclass.h"
#include "cameraclass.h"
#include "graphicsclass.h"

#include "cpuclass.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: SystemClass
////////////////////////////////////////////////////////////////////////////////
class SystemClass
{
public:
	SystemClass();
	~SystemClass() {}
	bool Initialize();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

	void Autosave();

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void InitializeScaling();
	void ShutdownWindows();

private:
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	InputClass* m_Input;
	GraphicsClass* m_Graphics;

	std::unordered_map<std::wstring, GameObject*> m_gameObjects; 

	std::string path_to_file = "whatever";
	std::atomic<bool> keep_saving_file;

	bool m_isGameActive = false;
	bool m_isGameHalted = false;
};


/////////////////////////
// FUNCTION PROTOTYPES //
/////////////////////////
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


/////////////
// GLOBALS //
/////////////
static SystemClass* ApplicationHandle = 0;


#endif