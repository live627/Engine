///////////////////////////////////////////////////////////////////////////////
// Filename: textclass.cpp
///////////////////////////////////////////////////////////////////////////////
#include "textclass.h"


TextClass::TextClass(
	ID3D11Device * p_device, ID3D11DeviceContext * pdeviceContext,
	int screenWidth, int screenHeight, Fonts * p_fontManager,
	const DirectX::XMMATRIX & baseViewMatrix)
	:
	device(p_device),
	deviceContext(pdeviceContext),

	m_Font(0),

	// Store the screen width and height.
	m_screenWidth(screenWidth),
	m_screenHeight(screenHeight),

	// Store the base view matrix.
	m_baseViewMatrix(baseViewMatrix),

	// Create the font shader object.
	m_FontShader(device, deviceContext, true),

	// Create the bitmap object.
	m_Bitmap(device, deviceContext, m_screenWidth, m_screenHeight, "")
{
	m_Font = p_fontManager->GetFont(1);

	for (int i = 0; i < 5; i++)
	{
		try
		{
			auto sentence = SentenceType();
			InitializeSentence(sentence, 24);
			m_sentences.push_back(sentence);
		}
		catch (std::exception & e)
		{
			throw std::runtime_error(
				FormatString(
					"%s\n\nCould not load sentence %d",
					e.what(), i
				).data()
			);
		}
	}
}


void TextClass::Render(const DirectX::XMMATRIX & worldMatrix, const DirectX::XMMATRIX & orthoMatrix)
{
	for (auto & sentence : m_sentences)
		RenderSentence(sentence, worldMatrix, orthoMatrix);

	int
		width = 400,
		height = 500,
		top = (m_screenHeight - height) / 2,
		left = (m_screenWidth - width) / 2;

	// Put the bitmap vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_Bitmap.Render({ left, top, width, height });

	// Create a pixel color vector with the input sentence color.
	auto pixelColor = DirectX::Colors::AntiqueWhite;

	// Render the text using the font shader.
	m_FontShader.Render(m_Bitmap.GetIndexCount(), worldMatrix, m_baseViewMatrix,
		orthoMatrix, m_Bitmap.GetTexture(), pixelColor);
}


void TextClass::InitializeSentence(SentenceType & sentence, int maxLength)
{
	sentence.maxLength = maxLength;
	sentence.vertexCount = 6 * maxLength;
	sentence.indexCount = sentence.vertexCount;

	// Create the vertex buffer.
	D3D11_BUFFER_DESC vertexBufferDesc =
	{
		sizeof(VertexType) * sentence.vertexCount,
		D3D11_USAGE_DYNAMIC,
		D3D11_BIND_VERTEX_BUFFER,
		D3D11_CPU_ACCESS_WRITE
	};
	D3D11_SUBRESOURCE_DATA vertexData = { std::vector<VertexType>(sentence.vertexCount).data() };
	ThrowIfFailed(
		device->CreateBuffer(&vertexBufferDesc, &vertexData, sentence.vertexBuffer.GetAddressOf()),
		"Could not create the vertex buffer."
	);

	D3D11_BUFFER_DESC indexBufferDesc = 
	{
		sizeof(unsigned long) * sentence.indexCount,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_INDEX_BUFFER
	};

	auto indices = std::vector<unsigned long>(sentence.indexCount);  
	for (size_t i = 0; i < sentence.indexCount; i++)
		indices[i] = i;

	D3D11_SUBRESOURCE_DATA indexData = { indices.data() };

	ThrowIfFailed(
		device->CreateBuffer(&indexBufferDesc, &indexData, sentence.indexBuffer.GetAddressOf()),
		"Could not create the index buffer."
	);
}


void TextClass::UpdateSentence(SentenceType & sentence, const char* text, 
	float positionX, float positionY, const DirectX::XMVECTORF32 & color)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;


	// Store the color of the sentence.
	sentence.color = color;

	// Check for possible buffer overflow.
	if (strlen(text) > sentence.maxLength)
	{
		text = std::string(text).substr(0, sentence.maxLength - 1).c_str();
		//return false;
	}

	// Create the vertex array.
	auto vertices = std::make_unique<VertexType[]>(sentence.vertexCount).get();

	// Calculate the X and Y pixel position on the screen to start drawing to.
	float drawX = -(m_screenWidth >> 1) + positionX;
	float drawY = (m_screenHeight >> 1) - positionY;

	// Use the font class to build the vertex array from the sentence text and sentence draw location.
	m_Font->BuildVertexArray((void*)vertices, text, drawX, drawY);

	// Lock the vertex buffer so it can be written to.
	ThrowIfFailed(
		deviceContext->Map(sentence.vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource),
		"Could not lock the vertex buffer."
	);

	// Get a pointer to the data in the vertex buffer.
	auto verticesPtr = (VertexType*)mappedResource.pData;

	// Copy the data into the vertex buffer.
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * sentence.vertexCount));

	// Unlock the vertex buffer.
	deviceContext->Unmap(sentence.vertexBuffer.Get(), 0);
}


void TextClass::RenderSentence(
	const SentenceType & sentence, 
	const DirectX::XMMATRIX & worldMatrix,
	const DirectX::XMMATRIX & orthoMatrix
)
{
	auto stride = sizeof(VertexType), offset = 0u;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, sentence.vertexBuffer.GetAddressOf(), &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(sentence.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Render the text using the font shader.
	m_FontShader.Render(sentence.indexCount, worldMatrix, m_baseViewMatrix,
		orthoMatrix, m_Font->GetTexture(), sentence.color);
}


void TextClass::RenderSentenceInstanced(
	const SentenceType & sentence, 
	const DirectX::XMMATRIX & worldMatrix,
	const DirectX::XMMATRIX & orthoMatrix
)
{
	uint strides[2] = { sizeof(VertexType), sizeof(InstanceType) }, offsets[2] = { 0u };
	ID3D11Buffer* bufferPointers[2];

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, sentence.vertexBuffer.GetAddressOf(), strides, offsets);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(sentence.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Render the text using the font shader.
	m_FontShader.RenderInstanced(sentence.indexCount, m_sentences.size(), worldMatrix, m_baseViewMatrix,
		orthoMatrix, m_Font->GetTexture(), sentence.color);
}


void TextClass::ResizeBuffers(int screenWidth, int screenHeight)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
}


void TextClass::SetMousePosition(int mouseX, int mouseY)
{
	char buf[24];
	auto msg = "Mouse {%d, %d}";
	sprintf_s(buf, 24, msg, mouseX, mouseY);

	// Update the sentence vertex buffer with the new string information.
	UpdateSentence(m_sentences[0], buf, ceilf(ui::ScaleX(20)), ceilf(ui::ScaleX(10)), DirectX::Colors::White);
}


void TextClass::SetCameraPosition(const DirectX::XMFLOAT3 & p_position)
{
	char buf[240];
	auto msg = "Location {%.f, %.f}";
	sprintf_s(buf, 240, msg, p_position.x, p_position.y);

	// Update the sentence vertex buffer with the new string information.
	UpdateSentence(m_sentences[1], buf, ceilf(ui::ScaleX(20)), ceilf(ui::ScaleX(30)), DirectX::Colors::White);
}


void TextClass::SetFps(int fps, float frameTime)
{
	char buf[24];
	auto msg = "FPS: %d (t=%.fms)";
	sprintf_s(buf, 24, msg, std::min(fps, 9999), frameTime);

	// The green found in DirectXColors.h is too dark here.
	const DirectX::XMVECTORF32 green = { 0.0f, 1.0f, 0.0f };

	const auto color = fps < 60 
		? DirectX::Colors::Yellow
		: fps < 30 
		? DirectX::Colors::Red
		: green;
	
	// Update the sentence vertex buffer with the new string information.
	UpdateSentence(m_sentences[2], buf, ceilf(ui::ScaleX(20)), ceilf(ui::ScaleX(50)), color);
}


void TextClass::SetCpu(int cpu)
{
	char buf[24];
	auto msg = "CPU: %d%%";
	sprintf_s(buf, 24, msg, cpu);

	// Update the sentence vertex buffer with the new string information.
	UpdateSentence(m_sentences[3], buf, ceilf(ui::ScaleX(20)), ceilf(ui::ScaleX(70)), { 0.0f, 1.0f, 0.0f });
}


void TextClass::SetPausedState(bool isGamePaused)
{
	auto buf = isGamePaused ? "Game Paused" : "";

	// Update the sentence vertex buffer with the new string information.
	UpdateSentence(m_sentences[4], buf, ceilf(ui::ScaleX(200)), ceilf(ui::ScaleX(60)), DirectX::Colors::White);
}
