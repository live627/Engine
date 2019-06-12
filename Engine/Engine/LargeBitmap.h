#pragma once



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
#include "bitmapclass.h"
#include "game.h"


	struct ColoredRect
	{
		RECT rect;
		DirectX::XMFLOAT4 color;
		bool hidden;
	};
////////////////////////////////////////////////////////////////////////////////
// Class name: LargeBitmap
////////////////////////////////////////////////////////////////////////////////
class LargeBitmap
{
private:

public:
	LargeBitmap(ID3D11Device*, ID3D11DeviceContext*, int, int, const DirectX::XMMATRIX &);
	void UpdateColoredRects(const std::vector<ColoredRect> &&);
	void UpdateColoredRect(int, const ColoredRect &);
	void Render(const DirectX::XMMATRIX &, const DirectX::XMMATRIX &);
	void ResizeBuffers(int, int);

private:
	void CreateBuffers();
	void UpdateBuffers();
	void BuildVertexArray(void *);
	void RenderBuffers();

	ID3D11Device * device;
	ID3D11DeviceContext * deviceContext;
	ShaderClass m_FontShader;
	int m_screenWidth, m_screenHeight;
	std::vector<ColoredRect> m_rects;
	DirectX::XMMATRIX m_baseViewMatrix;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer, indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	size_t vertexCount, indexCount;
};