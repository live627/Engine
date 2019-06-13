////////////////////////////////////////////////////////////////////////////////
// Filename: textclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTCLASS_H_
#define _TEXTCLASS_H_


///////////////////////////////
// PRE-PROCESSING DIRECTIVES //
///////////////////////////////
// Windows is too helpful sometimes.
#define NOMINMAX


//////////////
// INCLUDES //
//////////////
#include <DirectXColors.h>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "fontmanager.h"
#include "fontshaderclass.h"
#include "LargeBitmap.h"
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
		size_t vertexCount, indexCount, maxLength, texidx;
		std::string text = "";
		DirectX::XMVECTORF32 color;
	};

public:
	TextClass(ID3D11Device*, ID3D11DeviceContext*, int, int, Fonts *, const DirectX::XMMATRIX &);

	void CreateColoredRects();
	void Render(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &);

	void SetMousePosition(int, int);
	void SetCameraPosition(const DirectX::XMFLOAT3 &);
	void SetFps(int, int);
	void SetCpu(int);

	void SetPausedState(bool);
	void ResizeBuffers(int, int);

private:
	void InitializeSentence(SentenceType &, int);
	void UpdateSentence(SentenceType &, const char *, float, float, const DirectX::XMVECTORF32 &);
	void RenderSentence(const SentenceType &, const DirectX::XMMATRIX &, const DirectX::XMMATRIX &);
	void RenderSentenceInstanced(const SentenceType &, const DirectX::XMMATRIX &, const DirectX::XMMATRIX &);

	ID3D11Device * device;
	ID3D11DeviceContext * deviceContext;
	Fonts* m_FontManager;
	ShaderClass m_FontShader;
	int m_screenWidth, m_screenHeight;
	DirectX::XMMATRIX m_baseViewMatrix;
	LargeBitmap m_Bitmap;
	std::vector<SentenceType> m_sentences;
};

#endif