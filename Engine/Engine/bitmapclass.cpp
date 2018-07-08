////////////////////////////////////////////////////////////////////////////////
// Filename: bitmapclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "bitmapclass.h"


BitmapClass::BitmapClass(
	ID3D11Device * p_device, ID3D11DeviceContext * pdeviceContext,
	int screenWidth, int screenHeight, CHAR* textureFilename)
	:
	device(p_device),
	deviceContext(pdeviceContext),

	// Store the screen width and height.
	m_screenWidth(screenWidth),
	m_screenHeight(screenHeight),

	// Create the texture object.
	m_Texture(device, textureFilename),

	// Sttore an initial position that should be invalid.
	m_previousPos({ -1, -1, 0, 0 })
{

	// Initialize the vertex and index buffers.
	InitializeBuffers();
}


void BitmapClass::Render(RECT position)
{
	// Re-build the dynamic vertex buffer for rendering to possibly a different location on the screen.
	UpdateBuffers(position);

	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers();
}


void BitmapClass::ResizeBuffers(int screenWidth, int screenHeight)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	m_previousPos = { -1, -1, 0, 0 };
}


void BitmapClass::InitializeBuffers()
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;


	// Set the number of vertices in the vertex array.
	m_vertexCount = 6;

	// Set the number of indices in the index array.
	m_indexCount = m_vertexCount;

	// Create the index array.
	auto indices = std::vector<unsigned long>(m_indexCount);

	// Initialize the index array.
	for (size_t i = 0; i < m_indexCount; i++)
	{
		indices[i] = i;
	}

	// Set up the description of the dynamic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = std::vector<VertexType>(m_vertexCount).data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Create the vertex buffer.
	ThrowIfFailed(
		device->CreateBuffer(&vertexBufferDesc, &vertexData, m_vertexBuffer.GetAddressOf()),
		"Could not create the vertex buffer."
	);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	ThrowIfFailed(
		device->CreateBuffer(&indexBufferDesc, &indexData, m_indexBuffer.GetAddressOf()),
		"Could not create the index buffer."
	);
}


void BitmapClass::UpdateBuffers(RECT position)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;
	HRESULT result;


	// If the position we are rendering this bitmap to has not
	// changed then don't update the vertex buffer since it
	// currently has the correct parameters.
	if (position == m_previousPos)
		return;

	// If it has changed then update the position it is being rendered to.
	m_previousPos = position;

	// Calculate the screen coordinates of the bitmap.
	float
		left = (float)-(m_screenWidth / 2) + (float)position.left,
		right = left + (float)position.right,
		top = (float)(m_screenHeight / 2) - (float)position.top,
		bottom = top - (float)position.bottom;

	// Create the vertex array.
	auto vertices = std::vector<VertexType>(m_vertexCount).data();

	// Load the vertex array with data.
	// First triangle.
	vertices[0].position = DirectX::XMFLOAT3(left, top, 0.0f);  // Top left.
	vertices[0].texture = DirectX::XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = DirectX::XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[1].texture = DirectX::XMFLOAT2(1.0f, 1.0f);

	vertices[2].position = DirectX::XMFLOAT3(left, bottom, 0.0f);  // Bottom left.
	vertices[2].texture = DirectX::XMFLOAT2(0.0f, 1.0f);

	// Second triangle.
	vertices[3].position = DirectX::XMFLOAT3(left, top, 0.0f);  // Top left.
	vertices[3].texture = DirectX::XMFLOAT2(0.0f, 0.0f);

	vertices[4].position = DirectX::XMFLOAT3(right, top, 0.0f);  // Top right.
	vertices[4].texture = DirectX::XMFLOAT2(1.0f, 0.0f);

	vertices[5].position = DirectX::XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[5].texture = DirectX::XMFLOAT2(1.0f, 1.0f);

	// Lock the vertex buffer so it can be written to.
	ThrowIfFailed(
		deviceContext->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource),
		"Could not lock the vertex buffer."
	);

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (VertexType*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

	// Unlock the vertex buffer.
	deviceContext->Unmap(m_vertexBuffer.Get(), 0);
}


void BitmapClass::RenderBuffers()
{
	// Set vertex buffer stride and offset.
	auto stride = sizeof(VertexType), offset = 0u;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}