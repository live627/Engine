///////////////////////////////////////////////////////////////////////////////
// Filename: LargeBitmap.cpp
///////////////////////////////////////////////////////////////////////////////
#include "LargeBitmap.h"


LargeBitmap::LargeBitmap(
	ID3D11Device * p_device, ID3D11DeviceContext * pdeviceContext, ShaderClass * p_FontShader,
	int screenWidth, int screenHeight, const char * filename)
	:
	LargeBitmap(p_device, pdeviceContext, p_FontShader, screenWidth, screenHeight)
{
	TextureClass tex(p_device, filename);
	m_texture = tex.GetTexture();
}

LargeBitmap::LargeBitmap(
	ID3D11Device * p_device, ID3D11DeviceContext * pdeviceContext, ShaderClass * p_FontShader,
	int screenWidth, int screenHeight)
	:
	device(p_device),
	deviceContext(pdeviceContext),
	m_screenWidth(screenWidth),
	m_screenHeight(screenHeight),
	m_FontShader(p_FontShader)
{
	TextureClass tex(p_device);
	m_texture = tex.GetTexture();
}


void LargeBitmap::UpdateColoredRects(const std::vector<ColoredRect> && coloredRects)
{
	m_rects = coloredRects;

	if (indexBuffer == nullptr)
		CreateBuffers();
	UpdateBuffers();
}


void LargeBitmap::UpdateColoredRect(int i, const ColoredRect & coloredRect)
{
	m_rects[i] = coloredRect;
	UpdateBuffers();
}

void LargeBitmap::UpdateColoredRect(int i, RECT val)
{
	m_rects[i].rect = val;
	UpdateBuffers();
}

void LargeBitmap::UpdateColoredRect(int i, bool val)
{
	m_rects[i].hidden = val;
	UpdateBuffers();
}


void LargeBitmap::Render(const DirectX::XMMATRIX & worldMatrix, const DirectX::XMMATRIX & orthoMatrix, const DirectX::XMMATRIX & viewMatrix)
{
	RenderBuffers();
	m_FontShader->Render(indexCount, worldMatrix, viewMatrix,
		orthoMatrix, m_texture.Get(), {1,1,1,1});
}


void LargeBitmap::CreateBuffers()
{
	vertexCount = 4 * m_rects.size();
	indexCount = 6 * m_rects.size();

	// Create the vertex buffer.
	D3D11_BUFFER_DESC vertexBufferDesc =
	{
		sizeof(VertexColorType) * vertexCount,
		D3D11_USAGE_DYNAMIC,
		D3D11_BIND_VERTEX_BUFFER,
		D3D11_CPU_ACCESS_WRITE
	};
	D3D11_SUBRESOURCE_DATA vertexData = { std::vector<VertexColorType>(vertexCount).data() };
	ThrowIfFailed(
		device->CreateBuffer(&vertexBufferDesc, &vertexData, vertexBuffer.GetAddressOf()),
		"Could not create the vertex buffer."
	);

	D3D11_BUFFER_DESC indexBufferDesc =
	{
		sizeof(unsigned long) * indexCount,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_INDEX_BUFFER
	};

	auto indices = std::vector<unsigned long>(indexCount);
	for (size_t i = 0, v = 0, iii = 0; i < m_rects.size(); i++, v += 6, iii += 4)
	{
		indices[v] = iii;
		indices[v + 1] = iii + 1;
		indices[v + 2] = iii + 2;

		indices[v + 3] = iii + 1;
		indices[v + 4] = iii + 3;
		indices[v + 5] = iii + 2;
	}

	D3D11_SUBRESOURCE_DATA indexData = { indices.data() };

	ThrowIfFailed(
		device->CreateBuffer(&indexBufferDesc, &indexData, indexBuffer.GetAddressOf()),
		"Could not create the index buffer."
	);
}


void LargeBitmap::UpdateBuffers()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the vertex buffer so it can be written to.
	ThrowIfFailed(
		deviceContext->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource),
		"Could not lock the vertex buffer."
	);

	// Clear the vertex buffer.
	std::memset(mappedResource.pData, 0, (sizeof(VertexColorType) * vertexCount));

	BuildVertexArray(mappedResource.pData);

	// Unlock the vertex buffer.
	deviceContext->Unmap(vertexBuffer.Get(), 0);
}


void LargeBitmap::BuildVertexArray(void* vertices)
{
	VertexColorType* vertexPtr = (VertexColorType*)vertices;

	uint32_t index = 0;
	for (const ColoredRect & position : m_rects)
	{
		if (position.hidden)
		{
			index += 4;
			continue;
		}
		// Calculate the screen coordinates of the bitmap.
		float
			left = (float)position.rect.left - (float)(m_screenWidth / 2),
			right = left + (float)position.rect.right,
			top = (float)(m_screenHeight / 2) - (float)position.rect.top,
			bottom = top - (float)position.rect.bottom;

		// Create the vertex array.
		vertexPtr[index++] = { { left, top, 0.0f },{ 0.0f, 0.0f }, position.color };  // Top left.
		vertexPtr[index++] = { { right, top, 0.0f }, { 1.0f, 0.0f }, position.color }; // Top right.
		vertexPtr[index++] = { { left, bottom, 0.0f },{ 0.0f, 1.0f }, position.color }; // Bottom left.
		vertexPtr[index++] = { { right, bottom, 0.0f },{ 1.0f, 1.0f }, position.color }; // Bottom right.
	}
}


void LargeBitmap::RenderBuffers()
{
	auto stride = sizeof(VertexColorType), offset = 0u;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


void LargeBitmap::ResizeBuffers(int screenWidth, int screenHeight)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
}

Spritemap::Spritemap(
	ID3D11Device * p_device, ID3D11DeviceContext * pdeviceContext, ShaderClass * p_FontShader,
	int screenWidth, int screenHeight, const char * filename)
	:
	LargeBitmap(p_device, pdeviceContext, p_FontShader, screenWidth, screenHeight)
{
	TextureClass tex(p_device, filename);
	m_texture = tex.GetTexture();
}

void Spritemap::UpdateUvRects(const std::vector<RECT> && uvrects)
{
	m_uvrects = uvrects;
}

void Spritemap::SetRectUvMap(const std::vector<int> && uvrectmap)
{
	m_uvrectmap = uvrectmap;
}

void Spritemap::UpdateUvRectMap(int i, int uvrect)
{
	m_uvrectmap[i] = uvrect;
}

void Spritemap::BuildVertexArray(void * vertices)
{
	VertexColorType* vertexPtr = (VertexColorType*)vertices;

	uint32_t index = 0;
	for (int i = 0; i < m_rects.size(); i++)
	{
		auto position = m_rects[i];
		if (position.hidden || m_uvrectmap[i] > m_uvrects.size() - 1)
		{
			index += 4;
			continue;
		}
		// Calculate the screen coordinates of the bitmap.
		float
			left = (float)position.rect.left - (float)(m_screenWidth / 2),
			right = left + (float)position.rect.right,
			top = (float)(m_screenHeight / 2) - (float)position.rect.top,
			bottom = top - (float)position.rect.bottom,
			uvleft = (float)m_uvrects[m_uvrectmap[i]].left,
			uvright = uvleft + (float)m_uvrects[m_uvrectmap[i]].right,
			uvtop = (float)m_uvrects[m_uvrectmap[i]].top,
			uvbottom = uvtop + (float)m_uvrects[m_uvrectmap[i]].bottom;

		// Create the vertex array.
		vertexPtr[index++] = { { left, top, 0.0f },{ uvleft/2048,uvtop/2048.f }, position.color };  // Top left.
		vertexPtr[index++] = { { right, top, 0.0f }, { uvright/2048,uvtop/2048.f }, position.color }; // Top right.
		vertexPtr[index++] = { { left, bottom, 0.0f },{ uvleft/2048,uvbottom/2048.f }, position.color }; // Bottom left.
		vertexPtr[index++] = { { right, bottom, 0.0f },{ uvright/2048,uvbottom/2048.0f }, position.color }; // Bottom right.
	}
}
