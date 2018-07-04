///////////////////////////////////////////////////////////////////////////////
// Filename: textclass.cpp
///////////////////////////////////////////////////////////////////////////////
#include "textclass.h"


TextClass::TextClass(ID3D11Device * p_device, ID3D11DeviceContext * pdeviceContext)
{
	device = p_device;
	deviceContext = pdeviceContext;

	m_Font = 0;
	m_FontShader = 0;
}

TextClass::TextClass(const TextClass& other)
{
}


TextClass::~TextClass()
{
}


bool TextClass::Initialize(HWND hwnd, int screenWidth, int screenHeight,
	D3DXMATRIX baseViewMatrix, FontManager * p_fontManager)
{
	bool result;


	// Store the screen width and height.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Store the base view matrix.
	m_baseViewMatrix = baseViewMatrix;

	m_Font = p_fontManager->GetFont(1);

	// Create the font shader object.
	m_FontShader = new ShaderClass(device, deviceContext, true);
	if (!m_FontShader)
	{
		return false;
	}

	// Initialize the font shader object.
	result = m_FontShader->Initialize();
	if (!result)
	{
		throw std::runtime_error("Could not initialize the font shader object.");
		return false;
	}

	// Create the bitmap object.
	m_Bitmap = new BitmapClass;
	if (!m_Bitmap)
	{
		return false;
	}

	// Initialize the bitmap object.
	result = m_Bitmap->Initialize(device, screenWidth, screenHeight, L"");
	if (!result)
	{
		throw std::runtime_error("Could not initialize the pixel object.");
		return false;
	}

	for (int i = 0; i < 5; i++)
	{
		auto sentence = new SentenceType();
		result = InitializeSentence(&sentence, 24);
		if (!result)
		{
			return false;
		}
		m_sentences.push_back(sentence);
	}

	return true;
}


void TextClass::Shutdown()
{
	for (auto sentence : m_sentences)
		ReleaseSentence(&sentence);

	// Release the pixel object.
	if (m_Bitmap)
	{
		m_Bitmap->Shutdown();
		delete m_Bitmap;
		m_Bitmap = 0;
	}

	// Release the font shader object.
	if (m_FontShader)
	{
		delete m_FontShader;
		m_FontShader = 0;
	}
}


bool TextClass::Render(D3DXMATRIX worldMatrix, D3DXMATRIX orthoMatrix)
{
	bool result = true;


	for (auto sentence : m_sentences)
		result &= RenderSentence(sentence, worldMatrix, orthoMatrix);

	int
		width = 400,
		height = 500,
		top = (m_screenHeight - height) / 2,
		left = (m_screenWidth - width) / 2;

	// Put the bitmap vertex and index buffers on the graphics pipeline to prepare them for drawing.
	result = m_Bitmap->Render(deviceContext, { left, top, width, height });
	if (!result)
	{
		return false;
	}

	// Create a pixel color vector with the input sentence color.
	D3DXVECTOR4 pixelColor = DirectX::Colors::AntiqueWhite;

	// Render the text using the font shader.
	return m_FontShader->Render(m_Bitmap->GetIndexCount(), worldMatrix, m_baseViewMatrix,
		orthoMatrix, m_Bitmap->GetTexture(), pixelColor);

	return result;
}


bool TextClass::InitializeSentence(SentenceType** sentence, int maxLength)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;


	// Create a new sentence object.
	*sentence = new SentenceType;
	if (!*sentence)
	{
		return false;
	}

	// Initialize the sentence buffers to null.
	(*sentence)->vertexBuffer = 0;
	(*sentence)->indexBuffer = 0;

	// Set the maximum length of the sentence.
	(*sentence)->maxLength = maxLength;

	// Set the number of vertices in the vertex array.
	(*sentence)->vertexCount = 6 * maxLength;

	// Set the number of indexes in the index array.
	(*sentence)->indexCount = (*sentence)->vertexCount;

	// Create the vertex array.
	vertices = new VertexType[(*sentence)->vertexCount];
	if (!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[(*sentence)->indexCount];
	if (!indices)
	{
		return false;
	}

	// Initialize vertex array to zeros at first.
	memset(vertices, 0, (sizeof(VertexType) * (*sentence)->vertexCount));

	// Initialize the index array.
	for (i = 0; i < (*sentence)->indexCount; i++)
	{
		indices[i] = i;
	}

	// Set up the description of the dynamic vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * (*sentence)->vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &(*sentence)->vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * (*sentence)->indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &(*sentence)->indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Release the vertex array as it is no longer needed.
	delete[] vertices;
	vertices = 0;

	// Release the index array as it is no longer needed.
	delete[] indices;
	indices = 0;

	return true;
}


bool TextClass::UpdateSentence(SentenceType* sentence, const char* text, 
	float positionX, float positionY, const DirectX::XMVECTORF32& color)
{
	VertexType* vertices;
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;


	// Store the color of the sentence.
	sentence->color = static_cast<D3DXVECTOR4>(color);

	// Check for possible buffer overflow.
	if (strlen(text) > sentence->maxLength)
	{
		text = std::string(text).substr(0, sentence->maxLength - 1).c_str();
		//return false;
	}

	// Create the vertex array.
	vertices = new VertexType[sentence->vertexCount];
	if (!vertices)
	{
		return false;
	}

	// Initialize vertex array to zeros at first.
	memset(vertices, 0, (sizeof(VertexType) * sentence->vertexCount));

	// Calculate the X and Y pixel position on the screen to start drawing to.
	float drawX = -(m_screenWidth >> 1) + positionX;
	float drawY = (m_screenHeight >> 1) - positionY;

	// Use the font class to build the vertex array from the sentence text and sentence draw location.
	m_Font->BuildVertexArray((void*)vertices, text, drawX, drawY);

	// Lock the vertex buffer so it can be written to.
	result = deviceContext->Map(sentence->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the vertex buffer.
	verticesPtr = (VertexType*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * sentence->vertexCount));

	// Unlock the vertex buffer.
	deviceContext->Unmap(sentence->vertexBuffer, 0);

	// Release the vertex array as it is no longer needed.
	delete[] vertices;
	vertices = 0;

	return true;
}


void TextClass::ReleaseSentence(SentenceType** sentence)
{
	if (*sentence)
	{
		// Release the sentence vertex buffer.
		if ((*sentence)->vertexBuffer)
		{
			(*sentence)->vertexBuffer->Release();
			(*sentence)->vertexBuffer = 0;
		}

		// Release the sentence index buffer.
		if ((*sentence)->indexBuffer)
		{
			(*sentence)->indexBuffer->Release();
			(*sentence)->indexBuffer = 0;
		}

		// Release the sentence.
		delete *sentence;
		*sentence = 0;
	}

	return;
}


bool TextClass::RenderSentence(SentenceType* sentence, D3DXMATRIX worldMatrix,
	D3DXMATRIX orthoMatrix)
{
	unsigned int stride, offset;
	D3DXVECTOR4 pixelColor;
	bool result;


	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &sentence->vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(sentence->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Render the text using the font shader.
	return m_FontShader->Render(sentence->indexCount, worldMatrix, m_baseViewMatrix,
		orthoMatrix, m_Font->GetShaderResourceView(), sentence->color);
}


void TextClass::ResizeBuffers(int screenWidth, int screenHeight)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
}


bool TextClass::SetMousePosition(int mouseX, int mouseY)
{
	char buf[24];
	char * msg = "Mouse {%d, %d}";
	sprintf_s(buf, 24, msg, mouseX, mouseY);

	// Update the sentence vertex buffer with the new string information.
	return UpdateSentence(m_sentences[0], buf, ceilf(ui::ScaleX(20)), ceilf(ui::ScaleX(10)), DirectX::Colors::White);
}


bool TextClass::SetCameraPosition(D3DXVECTOR3 p_position)
{
	char buf[240];
	char * msg = "Location {%.f, %.f}";
	sprintf_s(buf, 240, msg, p_position.x, p_position.y);

	// Update the sentence vertex buffer with the new string information.
	return UpdateSentence(m_sentences[1], buf, ceilf(ui::ScaleX(20)), ceilf(ui::ScaleX(30)), DirectX::Colors::White);
}


bool TextClass::SetFps(int fps, float frameTime)
{
	char buf[24];
	char * msg = "FPS: %d (t=%.fms)";
	sprintf_s(buf, 24, msg, std::min(fps, 9999), frameTime);

	// The green found in DirectXColors.h is too dark here.
	const DirectX::XMVECTORF32 green = { 0.0f, 1.0f, 0.0f };

	const auto color = fps < 60 
		? DirectX::Colors::Yellow
		: fps < 30 
		? DirectX::Colors::Red
		: green;
	
	// Update the sentence vertex buffer with the new string information.
	return UpdateSentence(m_sentences[2], buf, ceilf(ui::ScaleX(20)), ceilf(ui::ScaleX(50)), color);
}


bool TextClass::SetCpu(int cpu)
{
	char buf[24];
	char * msg = "CPU: %d%%";
	sprintf_s(buf, 24, msg, cpu);

	// Update the sentence vertex buffer with the new string information.
	return UpdateSentence(m_sentences[3], buf, ceilf(ui::ScaleX(20)), ceilf(ui::ScaleX(70)), { 0.0f, 1.0f, 0.0f });
}


bool TextClass::SetPausedState(bool isGamePaused)
{
	char* buf = isGamePaused ? "Game Paused" : "";

	// Update the sentence vertex buffer with the new string information.
	return UpdateSentence(m_sentences[4], buf, ceilf(ui::ScaleX(200)), ceilf(ui::ScaleX(60)), DirectX::Colors::White);
}
