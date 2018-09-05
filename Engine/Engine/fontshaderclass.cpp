////////////////////////////////////////////////////////////////////////////////
// Filename: fontshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "fontshaderclass.h"



void ShaderClass::Render(int indexCount, const DirectX::XMMATRIX & worldMatrix,
	const DirectX::XMMATRIX & viewMatrix, const DirectX::XMMATRIX & projectionMatrix, 
	ID3D11ShaderResourceView* texture, const DirectX::XMVECTORF32 & pixelColor)
{
	// Set the shader parameters that it will use for rendering.
	SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix, texture, pixelColor);

	// Now render the prepared buffers with the shader.
	RenderShader(indexCount);
}


void ShaderClass::RenderInstanced(uint32_t indexCount, uint32_t instanceCount, const DirectX::XMMATRIX & worldMatrix,
	const DirectX::XMMATRIX & viewMatrix, const DirectX::XMMATRIX & projectionMatrix, 
	ID3D11ShaderResourceView* texture, const DirectX::XMVECTORF32 & pixelColor)
{
	// Set the shader parameters that it will use for rendering.
	SetShaderParameters(worldMatrix, viewMatrix, projectionMatrix, texture, pixelColor);

	// Now render the prepared buffers with the shader.
	RenderShaderInstanced(indexCount, instanceCount);
}


void ShaderClass::InitializeShader()
{
	HRESULT result;
	Microsoft::WRL::ComPtr<ID3DBlob>
		errorMessage,
		vertexShaderBuffer,
		pixelShaderBuffer;
	uint32_t flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif
	{
		// Compile the vertex shader code.
		std::ifstream infile("VertexShader.hlsl");
		std::string lines((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
		result = D3DCompile(lines.c_str(), lines.size(), NULL, NULL, NULL,
			"TextureVertexShader", "vs_5_0", flags, 0,
			&vertexShaderBuffer, &errorMessage);
		if (FAILED(result) && errorMessage)
		{
			// Pop a message up on the screen to notify the user to check the text file for compile errors.
			throw std::runtime_error(
				FormatString(
					"Error compiling vertex shader.\n\n%s",
					(char*)(errorMessage->GetBufferPointer())
				).data()
			);
		}
	}
	{
		// Compile the pixel shader code.
		std::ifstream infile("PixelShader.hlsl");
		std::string lines((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
		result = D3DCompile(lines.c_str(), lines.size(), NULL, NULL, NULL,
			m_isFont ? "FontPixelShader" : "TexturePixelShader", "ps_5_0", flags, 0,
			&pixelShaderBuffer, &errorMessage);
		if (FAILED(result) && errorMessage)
		{
			// Pop a message up on the screen to notify the user to check the text file for compile errors.
			throw std::runtime_error(
				FormatString(
					"Error compiling pixel shader.\n\n%s",
					(char*)(errorMessage->GetBufferPointer())
				).data()
			);
		}
	}
	// Create the vertex shader from the buffer.
	ThrowIfFailed(
		m_device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
			vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader),
		"Could not create the vertex shader buffer."
	);

	ThrowIfFailed(
		m_device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
			pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader),
		"Could not create the pixel shader buffer."
	);

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[] =
	{
		//Vertex Buffer
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		//Instance buffer
		{ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};

	// Get a count of the elements in the layout.
	auto numElements = std::size(polygonLayout);

	// Create the vertex input layout.
	ThrowIfFailed(
		m_device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
			vertexShaderBuffer->GetBufferSize(), &m_layout),
		"Could not create the vertex shader layout."
	);

	// Setup the description of the dynamic constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC constantBufferDesc =
	{
		sizeof(ConstantBufferType),
		D3D11_USAGE_DYNAMIC,
		D3D11_BIND_CONSTANT_BUFFER,
		D3D11_CPU_ACCESS_WRITE
	};

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	ThrowIfFailed(
		m_device->CreateBuffer(&constantBufferDesc, NULL, &m_constantBuffer),
		"Could not create the vertex constant buffer pointer."
	);

	// Create a texture sampler state description.
	D3D11_SAMPLER_DESC samplerDesc = 
	{
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		0.0f, 1, D3D11_COMPARISON_ALWAYS,
		0, D3D11_FLOAT32_MAX
	};

	// Create the texture sampler state.
	ThrowIfFailed(
		m_device->CreateSamplerState(&samplerDesc, &m_sampleState),
		"Could not create the texture sampler state."
	);

	// Setup the description of the dynamic pixel constant buffer that is in the pixel shader.
	D3D11_BUFFER_DESC pixelBufferDesc =
	{
		sizeof(PixelBufferType),
		D3D11_USAGE_DYNAMIC,
		D3D11_BIND_CONSTANT_BUFFER,
		D3D11_CPU_ACCESS_WRITE
	};

	// Create the pixel constant buffer pointer so we can access the pixel shader constant buffer from within this class.
	ThrowIfFailed(
		m_device->CreateBuffer(&pixelBufferDesc, NULL, &m_pixelBuffer),
		"Could not create the pixel constant buffer pointer."
	);
}


void ShaderClass::SetShaderParameters(const DirectX::XMMATRIX & worldMatrix,
	const DirectX::XMMATRIX & viewMatrix, const DirectX::XMMATRIX & projectionMatrix,
	ID3D11ShaderResourceView* texture, const DirectX::XMVECTORF32 & pixelColor)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;


	// Lock the constant buffer so it can be written to.
	ThrowIfFailed(
		m_deviceContext->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource),
		"Could not lock the vertex constant buffer pointer."
	);

	// Get a pointer to the data in the constant buffer.
	auto dataPtr = (ConstantBufferType*)mappedResource.pData;

	// Transpose the matrices to prepare them for the shader.
	dataPtr->world = DirectX::XMMatrixTranspose(worldMatrix);
	dataPtr->view = DirectX::XMMatrixTranspose(viewMatrix);
	dataPtr->projection = DirectX::XMMatrixTranspose(projectionMatrix);

	// Unlock the constant buffer.
	m_deviceContext->Unmap(m_constantBuffer.Get(), 0);

	// Now set the constant buffer in the vertex shader with the updated values.
	m_deviceContext->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

	// Set shader texture resource in the pixel shader.
	m_deviceContext->PSSetShaderResources(0, 1, &texture);

	// Lock the pixel constant buffer so it can be written to.
	ThrowIfFailed(
		m_deviceContext->Map(m_pixelBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource),
		"Could not lock the pixel constant buffer pointer."
	);

	// Get a pointer to the data in the pixel constant buffer.
	auto dataPtr2 = (PixelBufferType*)mappedResource.pData;

	// Copy the pixel color into the pixel constant buffer.
	dataPtr2->pixelColor = static_cast<DirectX::XMFLOAT4>(pixelColor);

	// Unlock the pixel constant buffer.
	m_deviceContext->Unmap(m_pixelBuffer.Get(), 0);

	// Now set the pixel constant buffer in the pixel shader with the updated value.
	m_deviceContext->PSSetConstantBuffers(0, 1, m_pixelBuffer.GetAddressOf());
}


void ShaderClass::RenderShader(int indexCount)
{
	// Set the vertex input layout.
	m_deviceContext->IASetInputLayout(m_layout.Get());

	// Set the vertex and pixel shaders that will be used to render the triangles.
	m_deviceContext->VSSetShader(m_vertexShader.Get(), NULL, 0);
	m_deviceContext->PSSetShader(m_pixelShader.Get(), NULL, 0);

	// Set the sampler state in the pixel shader.
	m_deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangles.
	m_deviceContext->DrawIndexed(indexCount, 0, 0);
}


void ShaderClass::RenderShaderInstanced(uint32_t indexCount, uint32_t instanceCount)
{
	// Set the vertex input layout.
	m_deviceContext->IASetInputLayout(m_layout.Get());

	// Set the vertex and pixel shaders that will be used to render the triangles.
	m_deviceContext->VSSetShader(m_vertexShader.Get(), NULL, 0);
	m_deviceContext->PSSetShader(m_pixelShader.Get(), NULL, 0);

	// Set the sampler state in the pixel shader.
	m_deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangles.
	m_deviceContext->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
}