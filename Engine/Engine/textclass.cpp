///////////////////////////////////////////////////////////////////////////////
// Filename: textclass.cpp
///////////////////////////////////////////////////////////////////////////////
#include "textclass.h"


TextClass::TextClass(
	ID3D11Device * p_device, ID3D11DeviceContext * pdeviceContext, ShaderClass * p_FontShader,
	int screenWidth, int screenHeight, Fonts * p_fontManager, const DirectX::XMMATRIX & baseViewMatrix)
	:
	device(p_device),
	deviceContext(pdeviceContext),
	m_screenWidth(screenWidth),
	m_screenHeight(screenHeight),
	m_baseViewMatrix(baseViewMatrix),
	m_FontShader(device, deviceContext, "SDFPixelShader"),
	m_Bitmap(device, deviceContext, p_FontShader, screenWidth, screenHeight),
	m_FontManager(p_fontManager)
{
	for (int i = 0; i < 5; i++)
	{
		try
		{
			auto sentence = SentenceType();
			sentence.texidx = i < 4 ? 1 : 0;
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
	CreateColoredRects();
}


void TextClass::CreateColoredRects()
{
	int
		width = 400,
		height = ui::ScaleX(50),
		top = (m_screenHeight - height) / 3,
		left = (m_screenWidth - width) / 2;

	DirectX::XMFLOAT4 black = { 0, 0, 0, 0.5f };
	std::vector<ColoredRect> vec;
	vec.emplace_back(ui::ScaleX(30), ui::ScaleX(10), ui::ScaleX(160), ui::ScaleX(95), black);
	vec.emplace_back(0, top, m_screenWidth, ui::ScaleX(50), black, true);
	vec.emplace_back(0, top, m_screenWidth, ui::ScaleX(1), Colors::White, true);
	vec.emplace_back(0, top + height, m_screenWidth, ui::ScaleX(1), Colors::White, true);
	vec.emplace_back(0, m_screenHeight - height, m_screenWidth, ui::ScaleX(50), black);
	vec.emplace_back(0, m_screenHeight - height, m_screenWidth, ui::ScaleX(1), Colors::White);
	std::vector<const char *> items({ "test", "testt2", "test3" });
	RectangleI rect(0, m_screenHeight - ui::ScaleX(301), ui::ScaleX(200), ui::ScaleX(250));
	listView = std::make_unique<ListView>(rect, Colors::AntiqueWhite, Colors::DarkSlateGray);
	auto txtPos = listView->UpdateItems(std::move(items), m_FontManager);
	for (size_t i = 0; i < txtPos->size(); i++) 
		AddSentence(std::get<0>(txtPos->at(i)), std::get<1>(txtPos->at(i)),
			std::get<2>(txtPos->at(i)) + ui::ScaleX(15.0f));
	for (auto & sprite : listView->GetSprites())
	{
		vec.push_back(sprite);
	}
	m_Bitmap.UpdateColoredRects(std::move(vec));
}


void TextClass::RenderUI(const DirectX::XMMATRIX & worldMatrix, const DirectX::XMMATRIX & orthoMatrix)
{
	m_Bitmap.Render(worldMatrix, orthoMatrix, m_baseViewMatrix);
	int width = 0;
	for (auto & sentence : m_sentences)
	{
		width = std::max(width, static_cast<int>(m_FontManager->MeasureString2(sentence.text.c_str()).x));
		RenderSentence(sentence, worldMatrix, orthoMatrix);
	}
	m_Bitmap.UpdateColoredRect(0, { { ui::ScaleX(10), ui::ScaleX(10), width + ui::ScaleX(30), ui::ScaleX(95) },{ 0, 0, 0, 0.5f } });
}


void TextClass::InitializeSentence(SentenceType & sentence, int maxLength)
{
	sentence.maxLength = maxLength;
	sentence.vertexCount = 4 * maxLength;
	sentence.indexCount = 6 * maxLength;

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
	for (int i = 0, v = 0, iii = 0; i < maxLength; i++, v += 6, iii += 4)
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
		device->CreateBuffer(&indexBufferDesc, &indexData, sentence.indexBuffer.GetAddressOf()),
		"Could not create the index buffer."
	);
}


void TextClass::UpdateSentence(SentenceType & sentence, const char* text, 
	float positionX, float positionY, const DirectX::XMVECTORF32 & color)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	sentence.text.assign(text);

	// Store the color of the sentence.
	sentence.color = color;

	// Check for possible buffer overflow.
	if (strlen(text) > sentence.maxLength)
	{
		text = std::string(text).substr(0, sentence.maxLength - 1).c_str();
		//return false;
	}

	// Calculate the X and Y pixel position on the screen to start drawing to.
	float drawX = -(m_screenWidth >> 1) + positionX;
	float drawY = (m_screenHeight >> 1) - positionY;

	// Lock the vertex buffer so it can be written to.
	ThrowIfFailed(
		deviceContext->Map(sentence.vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource),
		"Could not lock the vertex buffer."
	);

	// Clear the vertex buffer.
	std::memset(mappedResource.pData, 0, (sizeof(VertexType) * sentence.vertexCount));

	// Use the font class to build the vertex array from the sentence text and sentence draw location.
	if (sentence.texidx == 0)
		m_FontManager->BuildVertexArray(mappedResource.pData, text, drawX, drawY);
	else
		m_FontManager->BuildVertexArray2(mappedResource.pData, text, drawX, drawY);

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
		orthoMatrix, m_FontManager->GetTexture(sentence.texidx), sentence.color);
}


void TextClass::ResizeBuffers(int screenWidth, int screenHeight)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	CreateColoredRects();
}

void TextClass::AddSentence(const char * buf, float x, float y)
{
	auto sentence = SentenceType();
	sentence.texidx = 0;
	InitializeSentence(sentence, 24);
	UpdateSentence(sentence, buf, x, y, DirectX::Colors::Black);
	m_sentences.push_back(sentence);

	//return m_sentences.end();
}

void TextClass::PopSentence()
{
	m_sentences.pop_back();
}

void TextClass::RemoveSentences(
	std::vector<SentenceType>::iterator iter1,
	std::vector<SentenceType>::iterator iter2)
{
	m_sentences.erase(iter1, iter2);
}


void TextClass::SetMousePosition(int mouseX, int mouseY)
{
	char buf[24];
	auto msg = "Mouse {%d, %d}";
	sprintf_s(buf, 24, msg, mouseX, mouseY);

	// Update the sentence vertex buffer with the new string information.
	UpdateSentence(m_sentences[0], buf, ui::ScaleX(20.0f), ui::ScaleX(30.0f), DirectX::Colors::White);
}

void TextClass::SetCameraPosition(const DirectX::XMFLOAT3 & p_position)
{
	char buf[240];
	auto msg = "Location {%.f, %.f}";
	sprintf_s(buf, 240, msg, p_position.x, p_position.y);

	// Update the sentence vertex buffer with the new string information.
	UpdateSentence(m_sentences[1], buf, ui::ScaleX(20.0f), ui::ScaleX(50.0f), DirectX::Colors::White);
}

void TextClass::SetFps(int fps, int frameTime)
{
	char buf[24];
	auto msg = "FPS: %d (t=%dms)";
	sprintf_s(buf, 24, msg, std::min(fps, 9999), frameTime);

	// The green found in DirectXColors.h is too dark here.
	const DirectX::XMVECTORF32 green = { 0.0f, 1.0f, 0.0f };

	const auto color = fps < 60
		? DirectX::Colors::Yellow
		: fps < 30
		? DirectX::Colors::Red
		: green;

	// Update the sentence vertex buffer with the new string information.
	UpdateSentence(m_sentences[2], buf, ui::ScaleX(20.0f), ui::ScaleX(70.0f), color);
}

void TextClass::SetCpu(int cpu)
{
	char buf[24];
	auto msg = "CPU: %d%%";
	sprintf_s(buf, 24, msg, cpu);

	// Update the sentence vertex buffer with the new string information.
	UpdateSentence(m_sentences[3], buf, ui::ScaleX(20.0f), ui::ScaleX(90.0f), { 0.0f, 1.0f, 0.0f });
}

void TextClass::SetPausedState(bool isGamePaused)
{
	auto buf = isGamePaused ? "Game Paused" : "";
	int
		width = static_cast<int>(m_FontManager->MeasureString(buf).x),
		height = ui::ScaleX(50),
		top = (m_screenHeight - height) / 3,
		left = (m_screenWidth - width) / 2;

	for (int i = 1; i < 4; i++)
		m_Bitmap.UpdateColoredRect(i, !isGamePaused);

	// Update the sentence vertex buffer with the new string information.
	UpdateSentence(m_sentences[4], buf, left, top + ui::ScaleX(30.0f), DirectX::Colors::White);
}