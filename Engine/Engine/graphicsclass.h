////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_


///////////////////////
// INCLUDES //
///////////////////////
#include "d3dclass.h"


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "cameraclass.h"
#include "fontshaderclass.h"
#include "bitmapclass.h"
#include "textclass.h"
#include "fontmanager.h"
#include "cpuclass.h"


/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;


////////////////////////////////////////////////////////////////////////////////
// Class name: GraphicsClass
////////////////////////////////////////////////////////////////////////////////
class GraphicsClass : public GameObject     
{
public:
	GraphicsClass(CameraClass *, CpuClass *, int, int, HWND);
	void Shutdown();
	void DebugMousePosition(int &, int &);
	void SetPausedState(bool);
	void UpdateDebugInfo();
	void Frame();
	bool Render();
	bool Initialize() { return true; }

	void ResizeBuffers(int, int);

private:
	D3DClass* m_D3D;
	Fonts* m_Font;
	CameraClass* m_Camera;
	CpuClass * m_dbg;
	ShaderClass* m_Shader;
	BitmapClass* m_Bitmap;
	TextClass* m_Text;
	int m_screenWidth, m_screenHeight;
	HWND m_hwnd;
};

#endif