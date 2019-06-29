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


void LargeBitmap::UpdateColoredRects(const std::vector<Geometry::ColoredRect<int>> && coloredRects)
{
	m_rects = coloredRects;

	if (indexBuffer == nullptr)
		CreateBuffers();
	UpdateBuffers();
}


void LargeBitmap::UpdateColoredRect(int i, const Geometry::ColoredRect<int> & coloredRect)
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
size_t LargeBitmap::GetVertexCount()
{
	return 4 * m_rects.size();
}

size_t LargeBitmap::GetIndexCount()
{
	return 6 * m_rects.size();
}

std::vector<unsigned long> LargeBitmap::BuildIndexArray()
{
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

	return indices;
}

void LargeBitmap::CreateBuffers()
{
	vertexCount = GetVertexCount();
	indexCount = GetIndexCount();

	// Create the vertex buffer.
	D3D11_BUFFER_DESC vertexBufferDesc =
	{
		sizeof(VertexColorType) * vertexCount,
		D3D11_USAGE_DYNAMIC,
		D3D11_BIND_VERTEX_BUFFER,
		D3D11_CPU_ACCESS_WRITE
	};
	auto verts = std::make_unique<VertexColorType[]>(vertexCount);
	D3D11_SUBRESOURCE_DATA vertexData = { verts.get() };
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
	auto i = BuildIndexArray();
	D3D11_SUBRESOURCE_DATA indexData = { i.data() };

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
	for (const Geometry::ColoredRect<int> & position : m_rects)
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
	for (size_t i = 0u; i < m_rects.size(); i++)
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

#include <random>

PieChart::PieChart(
	ID3D11Device * p_device, ID3D11DeviceContext * pdeviceContext, ShaderClass * p_FontShader,
	int screenWidth, int screenHeight)
	:
	LargeBitmap(p_device, pdeviceContext, p_FontShader, screenWidth, screenHeight)
{
}

void PieChart::MakeChart(POINT origin, std::vector<float> dataPoints)
{
	int max = 10;
	float h = rand() / RAND_MAX;
	float s = 0.3f, v = 0.99f;
	auto c = std::vector<float>(max);
	std::generate(c.begin(), c.end(), [value = 0]() mutable { return value++; });
	std::shuffle(c.begin(), c.end(), std::mt19937{1729/* std::random_device{}()*/ });
	//dataPoints.clear();
	m_rects.clear();
	for (int i = 0; i < max; i++) {
		//dataPoints.emplace_back(00.1f);
		c[i] = 1.0f / (max + 1) * c[i];
	}
	auto radius = 100.0f;
	auto dangles = std::make_unique<float[]>(dataPoints.size());
	for (unsigned int i = 0; i < dataPoints.size(); i++)
	{
		dangles[i] = Lerp(0, DirectX::XM_2PI, dataPoints[i]);
		if (i != 0)
			dangles[i] += dangles[i - 1];
	}
	float angleStep = 10.0f / radius, prevx = 333, prevy = 222;

	int d = 0;
	for (float angle = 0; angle < DirectX::XM_2PI; angle += angleStep)
	{
		// Use the parametric definition of a circle: http://en.wikipedia.org/wiki/Circle#Cartesian_coordinates
		float x = 333 + radius * cos(angle);
		float y = 222 + radius * sin(angle);

		if (angle > dangles[d] && d < dataPoints.size() - 1)
			d++;

		m_rects.emplace_back(x, y, 0, 0, DirectX::XMFLOAT4{ c[d], 0.7f, 0.99f, 1 });
		m_rects.emplace_back(333, 222, 0, 0, DirectX::XMFLOAT4{ c[d], 0.6f, 0.99f, 1 } );
		m_rects.emplace_back(prevx, prevy, 0, 0, DirectX::XMFLOAT4{ c[d], 0.7f, 0.99f, 1 } );
		prevx = x;
		prevy = y;
	}
	m_rects[m_rects.size() - 3].rect = m_rects[0].rect;
	CreateBuffers();
	UpdateBuffers();
}

size_t PieChart::GetVertexCount()
{
	return m_rects.size();
}

size_t PieChart::GetIndexCount()
{
	return m_rects.size();
}

std::vector<unsigned long> PieChart::BuildIndexArray()
{
	auto indices = std::vector<unsigned long>(indexCount);
	std::iota(indices.begin(), indices.end(), 0);

	return indices;
}

void PieChart::BuildVertexArray(void* vertices)
{
	VertexColorType* vertexPtr = (VertexColorType*)vertices;

	uint32_t index = 0;
	for (const Geometry::ColoredRect<int> & position : m_rects)
	{
		float
			left = (float)position.rect.left - (float)(m_screenWidth / 2),
			top = (float)(m_screenHeight / 2) - (float)position.rect.top;

		vertexPtr[index++] = { { left, top, 0.0f },{ 0.0f, 0.0f }, position.color }; 
	}
}

void PieChart::RenderBuffers()
{
	auto stride = sizeof(VertexColorType), offset = 0u;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
