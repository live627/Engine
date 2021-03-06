////////////////////////////////////////////////////////////////////////////////
// Filename: bitmapclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _BITMAPCLASS_H_
#define _BITMAPCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl\client.h>


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "textureclass.h"
#include "game.h" 


////////////////////////////////////////////////////////////////////////////////
// Class name: BitmapClass
////////////////////////////////////////////////////////////////////////////////
class BitmapClass
{
private:
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
	    DirectX::XMFLOAT2 texture;
	};

public:
	BitmapClass(ID3D11Device *, ID3D11DeviceContext *, int, int, CHAR *);
	BitmapClass(ID3D11Device *, ID3D11DeviceContext *, int, int);
	void Render(RECT, float);
	~BitmapClass();
	void Render(RECT);

	void ResizeBuffers(int, int);

	int GetIndexCount() const { return m_indexCount; }
	auto GetTexture() const { return m_Texture.GetTexture(); }

private:
	void InitializeBuffers();
	void UpdateBuffers(RECT);
	void UpdateBuffers(RECT, float);
	void RenderBuffers();

private:
	ID3D11Device * device;
	ID3D11DeviceContext * deviceContext;
	ID3D11Buffer
		* m_vertexBuffer,
		* m_indexBuffer;
	size_t
		m_vertexCount, 
		m_indexCount, 
		m_screenWidth,
		m_screenHeight;
	TextureClass m_Texture;
	RECT m_previousPos;
};

#endif