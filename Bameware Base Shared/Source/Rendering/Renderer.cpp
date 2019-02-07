#include "../../Headers/Rendering/Renderer.h"

#include "../../Headers/Math.h"
#include "../../Headers/Utils.h"

#include <Dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

namespace BAMEWARE
{
	LRESULT APIENTRY WndProc(const HWND h_wnd, const UINT message, const WPARAM w_param, const LPARAM l_param)
	{
		if (h_wnd == g_renderer.GetWindowHandle())
			return g_renderer.EventCallback(message, w_param, l_param);

		return DefWindowProc(h_wnd, message, w_param, l_param);
	}

	LRESULT Renderer::EventCallback(const UINT message, const WPARAM w_param, const LPARAM l_param)
	{
		switch (message)
		{
		case WM_DESTROY:
			{
				m_should_close_window = true;
				PostQuitMessage(0);
				break;
			}
		case WM_SIZE: /// window resize
			{
				std::cout << m_window_size[0] << " " << m_window_size[1] << std::endl;

				if (!m_device_context || !m_render_target_view || !m_swap_chain || !m_device)
					return DefWindowProc(m_window_handle, message, w_param, l_param);

				m_device_context->OMSetRenderTargets(0, nullptr, nullptr);

				// Release all outstanding references to the swap chain's buffers.
				m_render_target_view->Release();

				// Preserve the existing buffer count and format.
				// Automatically choose the width and height to match the client rect for HWNDs.
				HRESULT result = m_swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
				if (FAILED(result))
					return DefWindowProc(m_window_handle, message, w_param, l_param);

				// Get buffer and create a render-target-view.
				ID3D11Texture2D* buffer = nullptr;
				result = m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&buffer));
				if (FAILED(result))
					return DefWindowProc(m_window_handle, message, w_param, l_param);

				result = m_device->CreateRenderTargetView(buffer, nullptr, &m_render_target_view);
				if (FAILED(result))
					return DefWindowProc(m_window_handle, message, w_param, l_param);

				buffer->Release();

				m_device_context->OMSetRenderTargets(1, &m_render_target_view, nullptr);

				/// set the viewport
				D3D11_VIEWPORT viewport;
				viewport.Width = float(m_window_size[0]);
				viewport.Height = float(m_window_size[1]);
				viewport.MinDepth = 0.0f;
				viewport.MaxDepth = 1.0f;
				viewport.TopLeftX = 0.f;
				viewport.TopLeftY = 0.f;
				m_device_context->RSSetViewports(1, &viewport);

				m_world_matrix = DirectX::XMMatrixIdentity();

				/// recompute the ortho matrix
				m_ortho_matrix = DirectX::XMMatrixOrthographicOffCenterLH(
					0.f, float(m_window_size[0]), float(m_window_size[1]), 0.f, m_screen_near, m_screen_depth);

				break;
			}
		default:
			{
				break;
			}
		}

		return DefWindowProc(m_window_handle, message, w_param, l_param);
	}

	bool Renderer::Initialize(const std::string& window_name, const Vector2DI& window_size, const bool transparent,
	                          const float screen_depth, const float screen_near)
	{
		if (m_window_handle)
			return false;

		m_window_name = window_name;
		m_window_position = Vector2DI({0, 0});
		m_window_size = window_size;
		m_screen_depth = screen_depth;
		m_screen_near = screen_near;
		m_is_transparent = transparent;

		/// creating the window
		{
			/// window class name is the window title with class at the end
			char window_class_name[256];
			sprintf_s(window_class_name, "%s%i class", window_name.c_str(), int(UTILS::RandomNumber(0.f, 99999.f)));

			/// create the window class
			WNDCLASSEX window_class;
			ZeroMemory(&window_class, sizeof(WNDCLASSEX));

			/// set the window class attributes
			window_class.cbSize = sizeof(WNDCLASSEX);
			window_class.style = CS_HREDRAW | CS_VREDRAW;
			window_class.lpfnWndProc = WndProc;
			window_class.hInstance = GetModuleHandle(nullptr);
			window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
			window_class.hbrBackground = HBRUSH(CreateSolidBrush(RGB(0, 0, 0)));
			window_class.lpszClassName = window_class_name;

			if (!RegisterClassEx(&window_class))
				return false;

			/// create the window
			m_window_handle = CreateWindowExA(
				(transparent ? (WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED) : 0),
				window_class_name,
				window_name.c_str(),
				WS_POPUP,
				0,
				0,
				window_size[0],
				window_size[1],
				nullptr,
				nullptr,
				GetModuleHandleA(nullptr),
				nullptr);

			if (!m_window_handle)
				return false;

			if (transparent)
			{
				/// dumb fix because of windows 10 creators update
				if (!SetLayeredWindowAttributes(m_window_handle, RGB(0, 0, 0), 255, LWA_ALPHA))
					return false;

				/// to make it transparent
				DWM_BLURBEHIND bb = {DWM_BB_ENABLE | DWM_BB_BLURREGION, true, CreateRectRgn(0, 0, -1, -1), true};
				if (DwmEnableBlurBehindWindow(m_window_handle, &bb) != S_OK)
					return false;
			}

			ShowWindow(m_window_handle, 1);
			SetWindowPos(m_window_handle, transparent ? HWND_TOPMOST : nullptr,
			             0, 0, window_size[0], window_size[1], 0);

			const MARGINS margin = {-1};
			if (DwmExtendFrameIntoClientArea(m_window_handle, &margin) != S_OK)
				return false;
		}

		/// initialize directx
		{
			// Initialize the swap chain description.
			DXGI_SWAP_CHAIN_DESC swap_chain_desc;
			ZeroMemory(&swap_chain_desc, sizeof(swap_chain_desc));

			// Set to a single back buffer.
			swap_chain_desc.BufferCount = 1;

			// Set the width and height of the back buffer.
			swap_chain_desc.BufferDesc.Width = window_size[0];
			swap_chain_desc.BufferDesc.Height = window_size[1];

			// Set regular 32-bit surface for the back buffer.
			swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

			swap_chain_desc.BufferDesc.RefreshRate.Numerator = 0;
			swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;

			// Set the usage of the back buffer.
			swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

			// Set the handle for the window to render to.
			swap_chain_desc.OutputWindow = m_window_handle;

			// Turn multisampling off.
			swap_chain_desc.SampleDesc.Count = 8;
			swap_chain_desc.SampleDesc.Quality = 0;

			swap_chain_desc.Windowed = true;

			// Set the scan line ordering and scaling to unspecified.
			swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

			// Discard the back buffer contents after presenting.
			swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

			// Don't set the advanced flags.
			swap_chain_desc.Flags = 0;

			D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;

			// Create the swap chain, Direct3D device, and Direct3D device context.
			HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
			                                               &feature_level, 1,
			                                               D3D11_SDK_VERSION, &swap_chain_desc, &m_swap_chain,
			                                               &m_device, nullptr, &m_device_context);
			if (FAILED(result))
				return false;

			// Get the pointer to the back buffer.
			ID3D11Texture2D* back_buffer_ptr = nullptr;
			if (FAILED(m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer_ptr))
			))
				return false;

			// Create the render target view with the back buffer pointer.
			if (FAILED(m_device->CreateRenderTargetView(back_buffer_ptr, NULL, &m_render_target_view)))
				return false;

			// Release pointer to the back buffer as we no longer need it.
			back_buffer_ptr->Release();
			back_buffer_ptr = nullptr;

			D3D11_TEXTURE2D_DESC depth_buffer_desc;
			// Initialize the description of the depth buffer.
			ZeroMemory(&depth_buffer_desc, sizeof(depth_buffer_desc));

			// Set up the description of the depth buffer.
			depth_buffer_desc.Width = window_size[0];
			depth_buffer_desc.Height = window_size[1];
			depth_buffer_desc.MipLevels = 1;
			depth_buffer_desc.ArraySize = 1;
			depth_buffer_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			depth_buffer_desc.SampleDesc.Count = swap_chain_desc.SampleDesc.Count;
			depth_buffer_desc.SampleDesc.Quality = 0;
			depth_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
			depth_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			depth_buffer_desc.CPUAccessFlags = 0;
			depth_buffer_desc.MiscFlags = 0;

			// Create the texture for the depth buffer using the filled out description.
			if (FAILED(m_device->CreateTexture2D(&depth_buffer_desc, NULL, &m_depth_stencil_buffer)))
				return false;

			// Initialize the description of the stencil state.
			D3D11_DEPTH_STENCIL_DESC depth_stencil_desc;
			ZeroMemory(&depth_stencil_desc, sizeof(depth_stencil_desc));

			// Set up the description of the stencil state.
			depth_stencil_desc.DepthEnable = false;
			depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;

			depth_stencil_desc.StencilEnable = true;
			depth_stencil_desc.StencilReadMask = 0xFF;
			depth_stencil_desc.StencilWriteMask = 0xFF;

			// Stencil operations if pixel is front-facing.
			depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
			depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			// Stencil operations if pixel is back-facing.
			depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
			depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

			// Create the depth stencil state.
			if (FAILED(m_device->CreateDepthStencilState(&depth_stencil_desc, &m_depth_stencil_state)))
				return false;

			// Set the depth stencil state.
			m_device_context->OMSetDepthStencilState(m_depth_stencil_state, 1);

			// Initialize the depth stencil view.
			D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc;
			ZeroMemory(&depth_stencil_view_desc, sizeof(depth_stencil_view_desc));

			// Set up the depth stencil view description.
			depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			depth_stencil_view_desc.Texture2D.MipSlice = 0;

			// Create the depth stencil view.
			if (FAILED(m_device->CreateDepthStencilView(m_depth_stencil_buffer, &depth_stencil_view_desc, &
				m_depth_stencil_view)))
				return false;

			// Bind the render target view and depth stencil buffer to the output render pipeline.
			m_device_context->OMSetRenderTargets(1, &m_render_target_view, m_depth_stencil_view);

			// Setup the raster description which will determine how and what polygons will be drawn.
			D3D11_RASTERIZER_DESC raster_desc;
			raster_desc.AntialiasedLineEnable = false;
			raster_desc.CullMode = D3D11_CULL_BACK;
			raster_desc.DepthBias = 0;
			raster_desc.DepthBiasClamp = 0.0f;
			raster_desc.DepthClipEnable = true;
			raster_desc.FillMode = D3D11_FILL_SOLID;
			raster_desc.FrontCounterClockwise = false;
			raster_desc.MultisampleEnable = false;
			raster_desc.ScissorEnable = true;
			raster_desc.SlopeScaledDepthBias = 0.0f;

			// Create the rasterizer state from the description we just filled out.
			if (FAILED(m_device->CreateRasterizerState(&raster_desc, &m_raster_state)))
				return false;

			// Now set the rasterizer state.
			m_device_context->RSSetState(m_raster_state);

			// Clear the blend state description.
			D3D11_BLEND_DESC blendStateDescription;
			ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));

			// Create an alpha enabled blend state description.
			blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
			blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
			blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			// Create the blend state using the description.
			result = m_device->CreateBlendState(&blendStateDescription, &m_alphaEnableBlendingState);
			if (FAILED(result))
				return false;

			// Setup the viewport for rendering.
			D3D11_VIEWPORT viewport;
			viewport.Width = float(window_size[0]);
			viewport.Height = float(window_size[1]);
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;
			viewport.TopLeftX = 0.0f;
			viewport.TopLeftY = 0.0f;

			// Create the viewport.
			m_device_context->RSSetViewports(1, &viewport);

			// Initialize the world matrix to the identity matrix.
			m_world_matrix = DirectX::XMMatrixIdentity();

			// Create an orthographic projection matrix for 2D rendering.
			m_ortho_matrix = DirectX::XMMatrixOrthographicOffCenterLH(0.f, float(window_size[0]), float(window_size[1]),
			                                                          0.f, screen_near, screen_depth);
		}

		/// initialize vertex buffer
		{
			// Create the index array.
			unsigned long indices[m_num_vertices];

			ZeroMemory(m_vertices, sizeof(m_vertices));

			for (size_t i = 0; i < m_num_vertices; i++)
				indices[i] = i;

			// Set up the description of the static vertex buffer.
			D3D11_BUFFER_DESC vertex_buffer_desc;
			vertex_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
			vertex_buffer_desc.ByteWidth = sizeof(m_vertices);
			vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertex_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			vertex_buffer_desc.MiscFlags = 0;
			vertex_buffer_desc.StructureByteStride = 0;

			// Give the subresource structure a pointer to the vertex data.
			D3D11_SUBRESOURCE_DATA vertex_data;
			vertex_data.pSysMem = m_vertices;
			vertex_data.SysMemPitch = 0;
			vertex_data.SysMemSlicePitch = 0;

			// Now create the vertex buffer.
			HRESULT result = m_device->CreateBuffer(&vertex_buffer_desc, &vertex_data, &m_vertex_buffer);
			if (FAILED(result))
				return false;

			// Set up the description of the static index buffer.
			D3D11_BUFFER_DESC index_buffer_desc;
			index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
			index_buffer_desc.ByteWidth = sizeof(indices);
			index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			index_buffer_desc.CPUAccessFlags = 0;
			index_buffer_desc.MiscFlags = 0;
			index_buffer_desc.StructureByteStride = 0;

			// Give the subresource structure a pointer to the index data.
			D3D11_SUBRESOURCE_DATA index_data;
			index_data.pSysMem = indices;
			index_data.SysMemPitch = 0;
			index_data.SysMemSlicePitch = 0;

			// Create the index buffer.
			result = m_device->CreateBuffer(&index_buffer_desc, &index_data, &m_index_buffer);
			if (FAILED(result))
				return false;
		}

		EndClipping();

		if (FAILED(m_shader.Initialize(m_device, m_window_handle)))
			return false;

		// Turn on the alpha blending.
		m_device_context->OMSetBlendState(m_alphaEnableBlendingState, nullptr, 0xffffffff);
		return true;
	}

	void Renderer::Release()
	{
		// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
		if (m_swap_chain)
			m_swap_chain->SetFullscreenState(false, nullptr);

		if (m_alphaEnableBlendingState)
		{
			m_alphaEnableBlendingState->Release();
			m_alphaEnableBlendingState = nullptr;
		}

		if (m_raster_state)
		{
			m_raster_state->Release();
			m_raster_state = nullptr;
		}

		if (m_depth_stencil_view)
		{
			m_depth_stencil_view->Release();
			m_depth_stencil_view = nullptr;
		}

		if (m_depth_stencil_state)
		{
			m_depth_stencil_state->Release();
			m_depth_stencil_state = nullptr;
		}

		if (m_depth_stencil_buffer)
		{
			m_depth_stencil_buffer->Release();
			m_depth_stencil_buffer = nullptr;
		}

		if (m_render_target_view)
		{
			m_render_target_view->Release();
			m_render_target_view = nullptr;
		}

		if (m_device_context)
		{
			m_device_context->Release();
			m_device_context = nullptr;
		}

		if (m_device)
		{
			m_device->Release();
			m_device = nullptr;
		}

		if (m_swap_chain)
		{
			m_swap_chain->Release();
			m_swap_chain = nullptr;
		}

		if (m_index_buffer)
		{
			m_index_buffer->Release();
			m_index_buffer = nullptr;
		}

		if (m_vertex_buffer)
		{
			m_vertex_buffer->Release();
			m_vertex_buffer = nullptr;
		}

		m_shader.Release();

		if (m_window_handle)
			DestroyWindow(m_window_handle);

		m_window_handle = nullptr;
	}

	bool Renderer::NextFrame(const ColorRGBA& color) const
	{
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Clear the back buffer.
		m_device_context->ClearRenderTargetView(m_render_target_view, ColorRGBA::GetFloatVec(color).Get());

		// Clear the depth buffer.
		m_device_context->ClearDepthStencilView(m_depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

		return !m_should_close_window;
	}

	void Renderer::EndFrame()
	{
		// Present as fast as possible.
		m_swap_chain->Present(0, 0);
		m_frame_count++;
	}

	void Renderer::SetDimensions(const Vector2DI& position, const Vector2DI& size)
	{
		m_window_size = size;
		m_window_position = position;

		SetWindowPos(m_window_handle, /*false ? HWND_TOPMOST :*/ nullptr,
		             position[0], position[1], m_window_size[0], m_window_size[1], 0);
	}

	void Renderer::SetDimensions(HWND parent_window)
	{
		Vector2DI offset({0, 0});

		RECT window_rect;
		GetWindowRect(parent_window, &window_rect);

		RECT client_rect;
		GetClientRect(parent_window, &client_rect);

		m_window_size[0] = client_rect.right - client_rect.left;
		m_window_size[1] = client_rect.bottom - client_rect.top;

		m_window_position[0] = window_rect.right - m_window_size[0] + offset[0];
		m_window_position[1] = window_rect.bottom - m_window_size[1] + offset[1];

		SetWindowPos(m_window_handle, /*false ? HWND_TOPMOST :*/ nullptr,
		             m_window_position[0], m_window_position[1], m_window_size[0], m_window_size[1], 0);
	}

	void Renderer::StartClipping(const Vector2DI& min, const Vector2DI& max) const
	{
		D3D11_RECT rect;
		rect.left = UTILS::Max(0, min[0] - 1);
		rect.top = UTILS::Max(0, min[1] - 1);
		rect.right = UTILS::Min(m_window_size[0], max[0]);
		rect.bottom = UTILS::Min(m_window_size[1], max[1]);

		m_device_context->RSSetScissorRects(1, &rect);
	}

	void Renderer::EndClipping()
	{
		D3D11_RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = m_window_size[0];
		rect.bottom = m_window_size[1];

		m_device_context->RSSetScissorRects(1, &rect);
	}

	void Renderer::RenderLine(const Vector2DI& position_1, const Vector2DI& position_2, const ColorRGBA& color)
	{
		m_vertices[0].m_position = position_1;
		m_vertices[1].m_position = position_2;

		m_vertices[0].m_color = ColorRGBA::GetFloatVec(color);
		m_vertices[1].m_color = ColorRGBA::GetFloatVec(color);

		RenderPrimitive(2, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	}

	void Renderer::RenderLine(const RenderVertex_t& vertex_1, const RenderVertex_t& vertex_2)
	{
		m_vertices[0] = vertex_1;
		m_vertices[1] = vertex_2;

		RenderPrimitive(2, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	}

	void Renderer::RenderTriangle(const Vector2DI& position_1, const Vector2DI& position_2, const Vector2DI& position_3,
	                              const ColorRGBA& color)
	{
		m_vertices[0].m_position = position_1;
		m_vertices[1].m_position = position_2;
		m_vertices[2].m_position = position_3;

		const auto float_vec = ColorRGBA::GetFloatVec(color);
		m_vertices[0].m_color = float_vec;
		m_vertices[1].m_color = float_vec;
		m_vertices[2].m_color = float_vec;

		RenderPrimitive(3, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	}

	void Renderer::RenderTriangle(const RenderVertex_t& vertex_1, const RenderVertex_t& vertex_2,
	                              const RenderVertex_t& vertex_3)
	{
		m_vertices[0] = vertex_1;
		m_vertices[1] = vertex_2;
		m_vertices[2] = vertex_3;

		RenderPrimitive(3, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	}

	void Renderer::RenderQuad(const Vector2DI& top_left, const Vector2DI& bottom_right, const const ColorRGBA& color,
	                          const bool filled)
	{
		const auto float_color = ColorRGBA::GetFloatVec(color);

		if (filled)
		{
			m_vertices[0].m_color = float_color;
			m_vertices[1].m_color = float_color;
			m_vertices[2].m_color = float_color;
			m_vertices[3].m_color = float_color;
			m_vertices[4].m_color = float_color;
			m_vertices[5].m_color = float_color;

			m_vertices[0].m_position = top_left;
			m_vertices[1].m_position = Vector2DI({bottom_right[0], top_left[1]});
			m_vertices[2].m_position = bottom_right;
			m_vertices[3].m_position = bottom_right;
			m_vertices[4].m_position = Vector2DI({top_left[0], bottom_right[1]});
			m_vertices[5].m_position = top_left;

			g_renderer.RenderPrimitive(6, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}
		else
		{
			m_vertices[0].m_color = float_color;
			m_vertices[1].m_color = float_color;
			m_vertices[2].m_color = float_color;
			m_vertices[3].m_color = float_color;
			m_vertices[4].m_color = float_color;

			m_vertices[0].m_position = top_left;
			m_vertices[1].m_position = Vector2DI({bottom_right[0], top_left[1]});
			m_vertices[2].m_position = bottom_right;
			m_vertices[3].m_position = Vector2DI({top_left[0], bottom_right[1]});
			m_vertices[4].m_position = Vector2DI({top_left[0], top_left[1] - 1});

			g_renderer.RenderPrimitive(5, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		}
	}

	void Renderer::RenderQuad(RenderVertex_t vertex_1, const RenderVertex_t& vertex_2, const RenderVertex_t& vertex_3,
	                          const RenderVertex_t& vertex_4, const bool filled)
	{
		if (filled)
		{
			m_vertices[0] = vertex_1;
			m_vertices[1] = vertex_2;
			m_vertices[2] = vertex_3;
			m_vertices[3] = vertex_3;
			m_vertices[4] = vertex_4;
			m_vertices[5] = vertex_1;

			g_renderer.RenderPrimitive(6, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}
		else
		{
			m_vertices[0] = vertex_1;
			m_vertices[1] = vertex_2;
			m_vertices[2] = vertex_3;
			m_vertices[3] = vertex_4;

			vertex_1.m_position[1] -= 1;
			m_vertices[4] = vertex_1;

			g_renderer.RenderPrimitive(5, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		}
	}

	void Renderer::RenderPrimitive(const size_t num_vertices, const D3D11_PRIMITIVE_TOPOLOGY topology) const
	{
		D3D11_MAPPED_SUBRESOURCE mapped_data;
		if (FAILED(m_device_context->Map(m_vertex_buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapped_data)))
			return;

		memcpy(mapped_data.pData, m_vertices, sizeof(RenderVertex_t) * num_vertices);
		m_device_context->Unmap(m_vertex_buffer, NULL);

		// Set vertex buffer stride and offset.
		unsigned int stride = sizeof(RenderVertex_t),
		             offset = 0;

		// Set the vertex buffer to active in the input assembler so it can be rendered.
		m_device_context->IASetVertexBuffers(0, 1, &m_vertex_buffer, &stride, &offset);

		// Set the index buffer to active in the input assembler so it can be rendered.
		m_device_context->IASetIndexBuffer(m_index_buffer, DXGI_FORMAT_R32_UINT, 0);

		// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
		m_device_context->IASetPrimitiveTopology(topology);

		m_shader.Render(m_device_context, num_vertices, m_world_matrix, GetViewMatrix(), GetOrthoMatrix());
	}

	bool Renderer::WorldToScreen(Vector2DI& screen, const Vector3DF& world, const Matrix4x4& view_matrix) const
	{
		screen[0] = view_matrix[0][0] * world[0] + view_matrix[0][1] * world[1] + view_matrix[0][2] * world[2] +
			view_matrix[0][3];
		screen[1] = view_matrix[1][0] * world[0] + view_matrix[1][1] * world[1] + view_matrix[1][2] * world[2] +
			view_matrix[1][3];

		const float w = view_matrix[3][0] * world[0] + view_matrix[3][1] * world[1] + view_matrix[3][2] * world[2] +
			view_matrix[3][3];
		if (w < 0.001f)
			return false;

		const auto width = float(m_window_size[0]),
		           height = float(m_window_size[1]);

		const float invw = 1.0f / w;
		const float mango_0 = float(screen[0]) * invw;
		const float mango_1 = float(screen[1]) * invw;

		float x = width / 2.f;
		float y = height / 2.f;
		x += 0.5f * mango_0 * width + 0.5f;
		y -= 0.5f * mango_1 * height + 0.5f;
		screen[0] = x;
		screen[1] = y;

		return true;
	}

	DirectX::XMMATRIX Renderer::GetViewMatrix() const
	{
		// Setup the vector that points upwards.
		DirectX::XMFLOAT3 up{0.f, 1.f, 0.f};

		// Load it into a XMVECTOR structure.
		const DirectX::XMVECTOR up_vector = XMLoadFloat3(&up);

		// Setup the position of the camera in the world.
		DirectX::XMFLOAT3 position{0.f, 0.f, -1.f};

		// Load it into a XMVECTOR structure.
		const DirectX::XMVECTOR position_vector = XMLoadFloat3(&position);

		// Setup where the camera is looking by default.
		DirectX::XMFLOAT3 look_at{0.f, 0.f, 1.f};

		// Load it into a XMVECTOR structure.
		const DirectX::XMVECTOR look_at_vector = XMLoadFloat3(&look_at);

		// Finally create the view matrix from the three updated vectors.
		return DirectX::XMMatrixLookAtLH(position_vector, look_at_vector, up_vector);
	}
}
