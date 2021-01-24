#pragma once
#include "VertexBuffer.h"

struct GLFWwindow;
class VertexBuffer;
class VertexShader;
class PixelShader;
class Renderer
{

public:
	void Init(GLFWwindow* window);

	void ClearRTV();
	void Present();
	void SetViewportFromWindow(GLFWwindow* window);
	void SetRenderTarget();

	void DrawQuadFS();
	void Draw(unsigned int vertexCount, unsigned int start = 0);
	void Draw(VertexBuffer* vBuffer, unsigned int start = 0);
	void SetVertexBuffer(const VertexBuffer* vBuffer);
	void SetVertexShader(const VertexShader* shader);
	void SetPixelShader(const PixelShader* shader);
	void SetInputLayoutFromVertexShader(const VertexShader* shader);


	// static variables
	static VertexBuffer QuadVB;
	static ID3D11InputLayout* PositionOnlyInputLayout;

	struct ID3D11Device* device_ptr;
	struct ID3D11DeviceContext* device_context_ptr;
	struct IDXGISwapChain* swap_chain_ptr;
	struct ID3D11RenderTargetView* render_target_view_ptr;
	

	static const float QuadPositions[12];
	static float background_colour[4];
};
