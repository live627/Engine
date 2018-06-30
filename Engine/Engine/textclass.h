////////////////////////////////////////////////////////////////////////////////
// Filename: textclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTCLASS_H_
#define _TEXTCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <DirectXColors.h>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "fontmanager.h"
#include "fontshaderclass.h"
#include "bitmapclass.h"
#include "game.h"


////////////////////////////////////////////////////////////////////////////////
// Class name: TextClass
////////////////////////////////////////////////////////////////////////////////
class TextClass
{
private:
	struct SentenceType
	{
		ID3D11Buffer *vertexBuffer, *indexBuffer;
		size_t vertexCount, indexCount, maxLength;
		D3DXVECTOR4 color;
	};

	struct VertexType
	{
		D3DXVECTOR3 position;
	    D3DXVECTOR2 texture;
	};

public:
	TextClass(ID3D11Device*, ID3D11DeviceContext*);
	TextClass(const TextClass&);
	~TextClass();

	bool Initialize(HWND, int, int, D3DXMATRIX, FontManager *);
	void Shutdown();
	bool Render(D3DXMATRIX, D3DXMATRIX);

	bool SetMousePosition(int, int);
	bool SetCameraPosition(D3DXVECTOR3);
	bool SetFps(int, float);
	bool SetCpu(int);

	bool SetPausedState(bool isGaamePaused);
	void ResizeBuffers(int width, int height);

private:
	bool InitializeSentence(SentenceType**, int);
	bool UpdateSentence(SentenceType*, const char*, float, float, const DirectX::XMVECTORF32&);
	void ReleaseSentence(SentenceType**);
	bool RenderSentence(SentenceType*, D3DXMATRIX, D3DXMATRIX);

	ID3D11Device * device;
	ID3D11DeviceContext * deviceContext;
	FontManager* m_FontManager;
	Font* m_Font;
	FontShaderClass* m_FontShader;
	int m_screenWidth, m_screenHeight;
	D3DXMATRIX m_baseViewMatrix;
	BitmapClass* m_Bitmap;
	std::vector<SentenceType*> m_sentences;
};

#endif