#pragma once

#include <Windows.h>

#include <d3d11.h>
#include <directxmath.h>
#include <D3Dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "ShaderManager.h"

#include "../Vector.h"
#include "../Color.h"
#include "../Matrix.h"


namespace BAMEWARE
{
	typedef struct
	{
		Vector2DI m_position;
		Vector4DF m_color;
	} RenderVertex_t;

	class Renderer
	{
	public:
		bool Initialize(const std::string& window_name, const Vector2DI& window_size, bool transparent,
		                float screen_depth, float screen_near);
		void Release();

		bool NextFrame(const ColorRGBA& color) const;
		void EndFrame();

		void SetDimensions(const Vector2DI& position, const Vector2DI& size);
		void SetDimensions(HWND parent_window);

		void StartClipping(const Vector2DI& min, const Vector2DI& max) const;
		void EndClipping();

		void RenderLine(const Vector2DI& position_1, const Vector2DI& position_2, const ColorRGBA& color);
		void RenderLine(const RenderVertex_t& vertex_1, const RenderVertex_t& vertex_2);

		void RenderTriangle(const Vector2DI& position_1, const Vector2DI& position_2, const Vector2DI& position_3,
		                    const ColorRGBA& color);
		void RenderTriangle(const RenderVertex_t& vertex_1, const RenderVertex_t& vertex_2,
		                    const RenderVertex_t& vertex_3);

		void RenderQuad(const Vector2DI& top_left, const Vector2DI& bottom_right, const ColorRGBA& color, bool filled);
		void RenderQuad(RenderVertex_t vertex_1, const RenderVertex_t& vertex_2, const RenderVertex_t& vertex_3,
		                const RenderVertex_t& vertex_4, bool filled);

		void RenderPrimitive(size_t num_vertices, D3D11_PRIMITIVE_TOPOLOGY topology) const;

		bool WorldToScreen(Vector2DI& screen, const Vector3DF& world, const Matrix4x4& view_matrix) const;

		unsigned int GetFrameCount() const { return m_frame_count; }
		RenderVertex_t* GetVertices() { return m_vertices; }
		Vector2DI GetWindowSize() const { return m_window_size; }
		HWND GetWindowHandle() const { return m_window_handle; }

	public:
		LRESULT EventCallback(UINT message, WPARAM w_param, LPARAM l_param);

	private:
		DirectX::XMMATRIX GetOrthoMatrix() const { return m_ortho_matrix; }
		DirectX::XMMATRIX GetViewMatrix() const;

	private:
		HWND m_window_handle = nullptr;
		ShaderManager m_shader;
		std::string m_window_name;
		Vector2DI m_window_size,
		          m_window_position;
		float m_screen_depth = 0.f,
		      m_screen_near = 0.f;
		bool m_should_close_window = false,
		     m_is_transparent = false;
		unsigned int m_frame_count = 0;

		DirectX::XMMATRIX m_world_matrix{};
		DirectX::XMMATRIX m_ortho_matrix{};

		/// directx stuff
		IDXGISwapChain* m_swap_chain = nullptr;
		ID3D11Device* m_device = nullptr;
		ID3D11DeviceContext* m_device_context = nullptr;
		ID3D11RenderTargetView* m_render_target_view = nullptr;
		ID3D11Texture2D* m_depth_stencil_buffer = nullptr;
		ID3D11DepthStencilState* m_depth_stencil_state = nullptr;
		ID3D11DepthStencilView* m_depth_stencil_view = nullptr;
		ID3D11RasterizerState* m_raster_state = nullptr;
		ID3D11BlendState* m_alphaEnableBlendingState{};

		/// vertex buffer stuff
		static constexpr size_t m_num_vertices = 1024;
		RenderVertex_t m_vertices[m_num_vertices];
		ID3D11Buffer *m_vertex_buffer = nullptr,
		             *m_index_buffer = nullptr;
	};

	inline Renderer g_renderer;
}
