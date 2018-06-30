////////////////////////////////////////////////////////////////////////////////
// Filename: systemclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_


///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
// target Windows 7 or later
#define _WIN32_WINNT 0x0601

#include <sdkddkver.h>
// The following #defines disable a bunch of unused windows stuff.
// See Windows.h, line 35, for explanation of these directives.
#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
//#define NOSYSMETRICS
#define NOMENUS
//#define NOICONS
//#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND

// ID3DX10Font wants typedef TEXTMETRIC definitions et al.
//#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE

#define STRICT


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