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
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer, indexBuffer;
		size_t vertexCount, indexCount, maxLength;
		DirectX::XMVECTORF32 color;
	};

public:
	TextClass(ID3D11Device*, ID3D11DeviceContext*, int, int, const DirectX::XMMATRIX &);

	void Initialize(HWND, Fonts *);
	void Shutdown();

	void Render(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &);

	void SetMousePosition(int, int);
	void SetCameraPosition(const DirectX::XMFLOAT3 &);
	void SetFps(int, float);
	void SetCpu(int);

	void SetPausedState(bool);
	void ResizeBuffers(int, int);

private:
	void InitializeSentence(SentenceType &, int);
	void UpdateSentence(SentenceType &, const char *, float, float, const DirectX::XMVECTORF32 &);
	void RenderSentence(const SentenceType &, const DirectX::XMMATRIX &, const DirectX::XMMATRIX &);

	ID3D11Device * device;
	ID3D11DeviceContext * deviceContext;
	Fonts* m_FontManager;
	Font* m_Font;
	ShaderClass* m_FontShader;
	int m_screenWidth, m_screenHeight;
	DirectX::XMMATRIX m_baseViewMatrix;
	BitmapClass* m_Bitmap;
	std::vector<SentenceType> m_sentences;
};

#endif