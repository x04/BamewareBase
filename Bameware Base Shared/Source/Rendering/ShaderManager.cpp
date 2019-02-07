#include "../../Headers/Rendering/ShaderManager.h"
#include <D3Dcompiler.h>
#include <iostream>

static constexpr char VERTEX_SHADER_TEXT[] =
	R"(
/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};
//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
	int2 position : POSITION;
	float4 color : COLOR;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType ColorVertexShader(VertexInputType input)
{
	PixelInputType output;


	// Change the position vector to be 4 units for proper matrix calculations.
	output.position = float4(input.position, 0.f, 1.f);

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(output.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Store the input color for the pixel shader to use.
	output.color = input.color;

	return output;
}
)";

static constexpr char PIXEL_SHADER_TEXT[] =
	R"(
////////////////////////////////////////////////////////////////////////////////
// Filename: color.ps
////////////////////////////////////////////////////////////////////////////////


//////////////
// TYPEDEFS //
//////////////
struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};


////////////////////////////////////////////////////////////////////////////////
// Pixel Shader
////////////////////////////////////////////////////////////////////////////////
float4 ColorPixelShader(PixelInputType input) : SV_TARGET
{
    return input.color;
}
)";


namespace BAMEWARE
{
	bool ShaderManager::Initialize(ID3D11Device* device, HWND window)
	{
		// Initialize the pointers this function will use to null.
		ID3D10Blob* error_message = nullptr;
		ID3D10Blob* vertex_shader_buffer = nullptr;
		ID3D10Blob* pixel_shader_buffer = nullptr;

		/// compile the vertex shader
		HRESULT result = D3DCompile(static_cast<const void*>(VERTEX_SHADER_TEXT), sizeof(VERTEX_SHADER_TEXT), nullptr,
		                            nullptr,
		                            nullptr, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		                            &vertex_shader_buffer, &error_message);

		if (FAILED(result))
			return false;

		/// compile the pixel shader
		result = D3DCompile(static_cast<const void*>(PIXEL_SHADER_TEXT), sizeof(PIXEL_SHADER_TEXT), nullptr, nullptr,
		                    nullptr, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		                    &pixel_shader_buffer, &error_message);

		if (FAILED(result))
			return false;

		// Create the vertex shader from the buffer.
		result = device->CreateVertexShader(vertex_shader_buffer->GetBufferPointer(),
		                                    vertex_shader_buffer->GetBufferSize(), nullptr, &m_vertex_shader);
		if (FAILED(result))
			return false;

		// Create the pixel shader from the buffer.
		result = device->CreatePixelShader(pixel_shader_buffer->GetBufferPointer(),
		                                   pixel_shader_buffer->GetBufferSize(), nullptr, &m_pixel_shader);
		if (FAILED(result))
			return false;


		// Create the vertex input layout description.
		// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
		D3D11_INPUT_ELEMENT_DESC polygon_layout[2];
		polygon_layout[0].SemanticName = "POSITION";
		polygon_layout[0].SemanticIndex = 0;
		polygon_layout[0].Format = DXGI_FORMAT_R32G32_SINT;
		polygon_layout[0].InputSlot = 0;
		polygon_layout[0].AlignedByteOffset = 0;
		polygon_layout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygon_layout[0].InstanceDataStepRate = 0;

		polygon_layout[1].SemanticName = "COLOR";
		polygon_layout[1].SemanticIndex = 0;
		polygon_layout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		polygon_layout[1].InputSlot = 0;
		polygon_layout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		polygon_layout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygon_layout[1].InstanceDataStepRate = 0;

		// Get a count of the elements in the layout.
		const unsigned int num_elements = sizeof(polygon_layout) / sizeof(polygon_layout[0]);

		// Create the vertex input layout.
		result = device->CreateInputLayout(polygon_layout, num_elements, vertex_shader_buffer->GetBufferPointer(),
		                                   vertex_shader_buffer->GetBufferSize(), &m_layout);
		if (FAILED(result))
			return false;

		// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
		vertex_shader_buffer->Release();
		vertex_shader_buffer = nullptr;

		pixel_shader_buffer->Release();
		pixel_shader_buffer = nullptr;

		// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
		D3D11_BUFFER_DESC matrix_buffer_desc;
		matrix_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		matrix_buffer_desc.ByteWidth = sizeof(MatrixBufferType);
		matrix_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		matrix_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		matrix_buffer_desc.MiscFlags = 0;
		matrix_buffer_desc.StructureByteStride = 0;

		// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
		result = device->CreateBuffer(&matrix_buffer_desc, nullptr, &m_matrix_buffer);
		return !FAILED(result);
	}

	void ShaderManager::Release()
	{
		// Release the matrix constant buffer.
		if (m_matrix_buffer)
		{
			m_matrix_buffer->Release();
			m_matrix_buffer = nullptr;
		}

		// Release the layout.
		if (m_layout)
		{
			m_layout->Release();
			m_layout = nullptr;
		}

		// Release the pixel shader.
		if (m_pixel_shader)
		{
			m_pixel_shader->Release();
			m_pixel_shader = nullptr;
		}

		// Release the vertex shader.
		if (m_vertex_shader)
		{
			m_vertex_shader->Release();
			m_vertex_shader = nullptr;
		}
	}

	bool ShaderManager::Render(ID3D11DeviceContext* device_context, const int index_count,
	                           const DirectX::XMMATRIX world_matrix, const DirectX::XMMATRIX view_matrix,
	                           const DirectX::XMMATRIX projection_matrix) const
	{
		// Set the shader parameters that it will use for rendering.
		if (!SetShaderParameters(device_context, world_matrix, view_matrix, projection_matrix))
			return false;

		// Now render the prepared buffers with the shader.
		RenderShader(device_context, index_count);

		return true;
	}

	bool ShaderManager::SetShaderParameters(ID3D11DeviceContext* device_context, DirectX::XMMATRIX world_matrix,
	                                        DirectX::XMMATRIX view_matrix, DirectX::XMMATRIX projection_matrix) const
	{
		// Transpose the matrices to prepare them for the shader.
		world_matrix = XMMatrixTranspose(world_matrix);
		view_matrix = XMMatrixTranspose(view_matrix);
		projection_matrix = XMMatrixTranspose(projection_matrix);

		// Lock the constant buffer so it can be written to.
		D3D11_MAPPED_SUBRESOURCE mapped_resource;
		if (FAILED(device_context->Map(m_matrix_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource)))
			return false;

		// Get a pointer to the data in the constant buffer.
		auto data_ptr = static_cast<MatrixBufferType*>(mapped_resource.pData);

		// Copy the matrices into the constant buffer.
		data_ptr->m_world = world_matrix;
		data_ptr->m_view = view_matrix;
		data_ptr->m_projection = projection_matrix;

		// Unlock the constant buffer.
		device_context->Unmap(m_matrix_buffer, 0);

		// Finanly set the constant buffer in the vertex shader with the updated values.
		device_context->VSSetConstantBuffers(0, 1, &m_matrix_buffer);

		return true;
	}

	void ShaderManager::RenderShader(ID3D11DeviceContext* device_context, const int index_count) const
	{
		// Set the vertex input layout.
		device_context->IASetInputLayout(m_layout);

		// Set the vertex and pixel shaders that will be used to render this triangle.
		device_context->VSSetShader(m_vertex_shader, nullptr, 0);
		device_context->PSSetShader(m_pixel_shader, nullptr, 0);

		// Render the triangle.
		device_context->DrawIndexed(index_count, 0, 0);
	}
}
