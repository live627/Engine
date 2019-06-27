////////////////////////////////////////////////////////////////////////////////
// Filename: bitmapclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "bitmapclass.h"

BitmapClass::BitmapClass(
	ID3D11Device * p_device, ID3D11DeviceContext * pdeviceContext,
	int screenWidth, int screenHeight, CHAR * textureFilename)
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

BitmapClass::BitmapClass(
	ID3D11Device * p_device, ID3D11DeviceContext * pdeviceContext,
	int screenWidth, int screenHeight)
	:
	device(p_device),
	deviceContext(pdeviceContext),

	// Store the screen width and height.
	m_screenWidth(screenWidth),
	m_screenHeight(screenHeight),

	// Create the texture object.
	m_Texture(device),

	// Sttore an initial position that should be invalid.
	m_previousPos({ -1, -1, 0, 0 })
{
	// Initialize the vertex and index buffers.
	InitializeBuffers();
}


void BitmapClass::Render(RECT position, float scale)
{
	// If the position we are rendering this bitmap to has not
	// changed then don't update the vertex buffer since it
	// currently has the correct parameters.
	if (position != m_previousPos)
		UpdateBuffers(position, scale);

	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers();
}


void BitmapClass::Render(RECT position)
{
	// If the position we are rendering this bitmap to has not
	// changed then don't update the vertex buffer since it
	// currently has the correct parameters.
	//if (position != m_previousPos)
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
	D3D11_BUFFER_DESC vertexBufferDesc = {}, indexBufferDesc = {};
	D3D11_SUBRESOURCE_DATA vertexData = {}, indexData = {};


	// Set the number of vertices in the vertex array.
	m_vertexCount = 6;

	// Set the number of indices in the index array.
	m_indexCount = m_vertexCount;

	// Create the index array.
	std::vector<unsigned long> indices(m_indexCount);

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

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = std::vector<VertexType>(m_vertexCount).data();

	// Create the vertex buffer.
	ThrowIfFailed(
		device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer),
		"Could not create the vertex buffer."
	);

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices.data();

	// Create the index buffer.
	ThrowIfFailed(
		device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer),
		"Could not create the index buffer."
	);
}


BitmapClass::~BitmapClass()
{
	// Release the index buffer.
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}

	// Release the vertex buffer.
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}
}


void BitmapClass::UpdateBuffers(RECT position)
{
	m_previousPos = position;

	// Calculate the screen coordinates of the bitmap.
	float
		left = (float)position.left - (float)(m_screenWidth / 2),
		right = left + (float)position.right,
		top = (float)(m_screenHeight / 2) - (float)position.top,
		bottom = top - (float)position.bottom;

	// Create the vertex array.
	auto vertices = std::vector<VertexType>({
		// First triangle.
		{ { left, top, 0.0f }, { 0.0f, 0.0f } },  // Top left.
		{ { right, bottom, 0.0f }, { 1.0f, 1.0f } }, // Bottom right.
		{ { left, bottom, 0.0f }, { 0.0f, 1.0f } }, // Bottom left.

		// Second triangle.
		{ { left, top, 0.0f }, { 0.0f, 0.0f } }, // Top left.
		{ { right, top, 0.0f }, { 1.0f, 0.0f } }, // Top right.
		{ { right, bottom, 0.0f }, { 1.0f, 1.0f  } } // Bottom right.
	});

	// Lock the vertex buffer so it can be written to.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ThrowIfFailed(
		deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource),
		"Could not lock the vertex buffer."
	);

	// Copy the data into the vertex buffer.
	memcpy(mappedResource.pData, vertices.data(), (sizeof(VertexType) * m_vertexCount));
	
	// Unlock the vertex buffer.
	deviceContext->Unmap(m_vertexBuffer, 0);
}


void BitmapClass::UpdateBuffers(RECT position, float scale)
{
	m_previousPos = position;

	// Calculate the screen coordinates of the bitmap.
	float
		left = (float)position.left - (float)(m_screenWidth / 2),
		right = left + (float)position.right,
		top = (float)(m_screenHeight / 2) - (float)position.top,
		bottom = top - (float)position.bottom;

	// Create the vertex array.
	auto vertices = std::vector<VertexType>({
		// First triangle.
		{ { left, top, 0.0f }, { 0.0f, 0.0f } },  // Top left.
		{ { right * scale, bottom * scale, 0.0f }, { 1.0f, 1.0f } }, // Bottom right.
		{ { left, bottom * scale, 0.0f }, { 0.0f, 1.0f } }, // Bottom left.

		// Second triangle.
		{ { left, top, 0.0f }, { 0.0f, 0.0f } }, // Top left.
		{ { right * scale, top, 0.0f }, { 1.0f, 0.0f } }, // Top right.
		{ { right * scale, bottom * scale, 0.0f }, { 1.0f, 1.0f } } // Bottom right.
	}); 

	// Lock the vertex buffer so it can be written to.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ThrowIfFailed(
		deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource),
		"Could not lock the vertex buffer."
	);

	// Copy the data into the vertex buffer.
	memcpy(mappedResource.pData, vertices.data(), (sizeof(VertexType) * m_vertexCount));
	
	// Unlock the vertex buffer.
	deviceContext->Unmap(m_vertexBuffer, 0);
}


void BitmapClass::RenderBuffers()
{
	// Set vertex buffer stride and offset.
	auto stride = sizeof(VertexType), offset = 0u;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}