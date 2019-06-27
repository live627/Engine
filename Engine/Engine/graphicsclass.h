////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_


///////////////////////
// INCLUDES //
///////////////////////


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "d3dclass.h"
#include "cameraclass.h"
#include "fontshaderclass.h"
#include "bitmapclass.h"
#include "textclass.h"
#include "fontmanager.h"
#include "cpuclass.h"
#include "tiles.h"
#include "LargeBitmap.h"


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
class GraphicsClass : public IGameObject     
{
public:
	GraphicsClass(CameraClass *, size_t, size_t, size_t, HWND, Settings *);
	void SetPausedState(bool);
	void Frame();
	void BeforeRender();
	void AfterRender();
	void Render();
	void ResizeBuffers(int, int);
	void Save(BinaryWriter &);
	void Load(BinaryReader &);
	void Click(const std::vector<bool>, POINT);
	auto GetText() { return &m_Text; }

private:
	D3DClass m_D3D;
	RenderTextureClass m_RenderTexture;
	Fonts m_Font;
	CameraClass* m_Camera;
	CpuClass * m_dbg;
	ShaderClass m_Shader;
	ShaderClass m_Shader2;
	ShaderClass m_FontShader;
	BitmapClass m_Bitmap;
	PieChart m_Bitmap2;
	TextClass m_Text;
	Tiles tiles;
	DirectX::XMMATRIX worldMatrix, baseviewMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	std::vector<IGameObject *> m_gameObjects;
	size_t m_screenWidth, m_screenHeight, m_scale;
};

#endif