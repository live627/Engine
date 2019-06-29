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
	m_FontShader(device, deviceContext, "FontPixelShader"),
	m_Bitmap(device, deviceContext, p_FontShader, screenWidth, screenHeight),
	m_FontManager(p_fontManager)
{
	for (int i = 0; i < 5; i++)
	{
		try
		{
			auto sentence = SentenceType();
			sentence.texidx = i < 4 ? 1 : 2;
			InitializeSentence(sentence, 32);
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
	std::vector<Geometry::ColoredRect<int>> vec;
	vec.emplace_back(ui::ScaleX(30), ui::ScaleX(10), ui::ScaleX(160), ui::ScaleX(95), black);
	vec.emplace_back(0, top, m_screenWidth, ui::ScaleX(50), black, true);
	vec.emplace_back(0, top, m_screenWidth, ui::ScaleX(1), Colors::White, true);
	vec.emplace_back(0, top + height, m_screenWidth, ui::ScaleX(1), Colors::White, true);
	vec.emplace_back(0, m_screenHeight - height, m_screenWidth, ui::ScaleX(50), black);
	vec.emplace_back(0, m_screenHeight - height, m_screenWidth, ui::ScaleX(1), Colors::White);
	std::vector<const char *> items({ "test", "testt2", "test3" });
	Geometry::Rectangle<int> rect(0, m_screenHeight - ui::ScaleX(301), ui::ScaleX(200), ui::ScaleX(250));
	listView = std::make_unique<ListView>(rect, Colors::AntiqueWhite, Colors::DarkSlateGray);
	auto txtPos = listView->UpdateItems(std::move(items), m_FontManager->GetFont(2u));
	for (size_t i = 0; i < txtPos->size(); i++) 
		AddSentence(std::get<0>(txtPos->at(i)), std::get<1>(txtPos->at(i)),
			std::get<2>(txtPos->at(i)) + ui::ScaleX(15.0f), 2u);
	for (auto & sprite : listView->GetSprites())
	{
		vec.push_back(sprite);
	}
	m_Bitmap.UpdateColoredRects(std::move(vec));
}


void TextClass::RenderUI(const DirectX::XMMATRIX & worldMatrix, const DirectX::XMMATRIX & orthoMatrix)
{
	int i = 0;
	m_Bitmap.Render(worldMatrix, orthoMatrix, m_baseViewMatrix);
	int width = 0;
	for (auto & sentence : m_sentences)
	{
		if (i++ < 4)
			width = std::max(width, static_cast<int>(m_FontManager->GetFont(1)->MeasureString(sentence.text.c_str()).x));
		RenderSentence(sentence, worldMatrix, orthoMatrix);
	}
	m_Bitmap.UpdateColoredRect(0, { { ui::ScaleX(10), ui::ScaleX(10), width + ui::ScaleX(10), ui::ScaleX(85) },{ 0, 0, 0, 0.5f } });
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
	std::iota(indices.begin(), indices.end(), 0);

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
	static bool hasLoggedError = false;

	sentence.color = color;
	sentence.text = text;
		
	// Check for possible buffer overflow.
	if (strlen(text) > sentence.maxLength)
	{
		text = std::string(text).substr(0, sentence.maxLength - 1).c_str();
		if (!hasLoggedError)
		{
			throw std::out_of_range("Buffer overflow");
			hasLoggedError = true;
		}
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
	m_FontManager->GetFont(sentence.texidx)->BuildVertexArray(mappedResource.pData, text, drawX, drawY);

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
		orthoMatrix, m_FontManager->GetFont(sentence.texidx)->GetTexture(), sentence.color);
}

void TextClass::ResizeBuffers(int screenWidth, int screenHeight)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	CreateColoredRects();
}

void TextClass::AddSentence(const char * buf, float x, float y, size_t texidx)
{
	auto sentence = SentenceType();
	sentence.texidx = texidx;
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
	UpdateSentence(m_sentences[0], buf, ui::ScaleX(20.0f), ui::ScaleX(25.0f), DirectX::Colors::White);
}

void TextClass::SetCameraPosition(const DirectX::XMFLOAT3 & p_position)
{
	char buf[240];
	auto msg = "Location {%.f, %.f}";
	sprintf_s(buf, 240, msg, p_position.x, p_position.y);

	// Update the sentence vertex buffer with the new string information.
	UpdateSentence(m_sentences[1], buf, ui::ScaleX(20.0f), ui::ScaleX(45.0f), DirectX::Colors::White);
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
	UpdateSentence(m_sentences[2], buf, ui::ScaleX(20.0f), ui::ScaleX(65.0f), color);
}

void TextClass::SetCpu(int cpu)
{
	char buf[24];
	auto msg = "CPU: %d%%";
	sprintf_s(buf, 24, msg, cpu);

	// Update the sentence vertex buffer with the new string information.
	UpdateSentence(m_sentences[3], buf, ui::ScaleX(20.0f), ui::ScaleX(85.0f), { 0.0f, 1.0f, 0.0f });
}

void TextClass::SetPausedState(bool isGamePaused)
{
	auto buf = isGamePaused ? "Game Paused" : "";
	POINT size = m_FontManager->GetFont(2)->MeasureString(buf);
	float
		height = ui::ScaleX(50.0f),
		top = (m_screenHeight - height) / 3,
		left = (m_screenWidth - size.x) / 2;

	for (int i = 1; i < 4; i++)
		m_Bitmap.UpdateColoredRect(i, !isGamePaused);

	// Update the sentence vertex buffer with the new string information.
	UpdateSentence(m_sentences[4], buf, left, top + size.y / 2 + ui::ScaleX(25.0f), DirectX::Colors::White);
}