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
		DirectX::XMVECTORF32 color;
	};

	struct VertexType
	{
		DirectX::XMFLOAT3 position;
	    DirectX::XMFLOAT2 texture;
	};

public:
	TextClass(ID3D11Device*, ID3D11DeviceContext*);
	TextClass(const TextClass&);
	~TextClass();

	bool Initialize(HWND, int, int, DirectX::XMMATRIX, Fonts *);
	void Shutdown();
	bool Render(DirectX::XMMATRIX, DirectX::XMMATRIX);

	bool SetMousePosition(int, int);
	bool SetCameraPosition(const DirectX::XMFLOAT3 &);
	bool SetFps(int, float);
	bool SetCpu(int);

	bool SetPausedState(bool isGaamePaused);
	void ResizeBuffers(int width, int height);

private:
	bool InitializeSentence(SentenceType**, int);
	bool UpdateSentence(SentenceType*, const char*, float, float, const DirectX::XMVECTORF32 &);
	void ReleaseSentence(SentenceType**);
	bool RenderSentence(SentenceType * sentence, const DirectX::XMMATRIX & worldMatrix, const DirectX::XMMATRIX & orthoMatrix);

	ID3D11Device * device;
	ID3D11DeviceContext * deviceContext;
	Fonts* m_FontManager;
	Font* m_Font;
	ShaderClass* m_FontShader;
	int m_screenWidth, m_screenHeight;
	DirectX::XMMATRIX m_baseViewMatrix;
	BitmapClass* m_Bitmap;
	std::vector<SentenceType*> m_sentences;
};

#endif