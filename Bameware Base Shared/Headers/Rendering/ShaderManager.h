#pragma once
#include <directxmath.h>
#include <d3d11.h>


namespace BAMEWARE
{
	struct MatrixBufferType
	{
		DirectX::XMMATRIX m_world;
		DirectX::XMMATRIX m_view;
		DirectX::XMMATRIX m_projection;
	};

	class ShaderManager
	{
	public:
		bool Initialize(ID3D11Device* device, HWND window);
		void Release();

		bool Render(ID3D11DeviceContext* device_context, int index_count, DirectX::XMMATRIX world_matrix, DirectX::XMMATRIX view_matrix, DirectX::XMMATRIX projection_matrix) const;

	private:
		bool SetShaderParameters(ID3D11DeviceContext* device_context, DirectX::XMMATRIX world_matrix, DirectX::XMMATRIX view_matrix, DirectX::XMMATRIX projection_matrix) const;
		void RenderShader(ID3D11DeviceContext*, int) const;

	private:
		ID3D11VertexShader* m_vertex_shader = nullptr;
		ID3D11PixelShader* m_pixel_shader = nullptr;
		ID3D11InputLayout* m_layout = nullptr;
		ID3D11Buffer* m_matrix_buffer = nullptr;
	};
}
