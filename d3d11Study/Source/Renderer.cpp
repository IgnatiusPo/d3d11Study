#include "Renderer.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "d3d11.h"
#include "dxgi.h"
#include "d3dcompiler.h"
#include "assert.h"

#include "VertexShader.h"
#include "PixelShader.h"

//Initialize static variables
ID3D11InputLayout* Renderer::PositionOnlyInputLayout = nullptr;
 const float Renderer::QuadPositions[] = { //vertex attributes for a quad that fills the entire screen in NDC.
	-1.0f,  1.0f,
	-1.0f, -1.0f,
	 1.0f, -1.0f,

	-1.0f,  1.0f,
	 1.0f, -1.0f,
	 1.0f,  1.0f,
};
 float Renderer::background_colour[] = {  0x64 / 255.0f, 0x95 / 255.0f, 0xED / 255.0f, 1.0f };




void Renderer::Init(GLFWwindow* window)
{
	HWND hwnd = glfwGetWin32Window(window);
	if (hwnd == 0)
	{
		assert(false);
		return;
	}

	DXGI_SWAP_CHAIN_DESC swap_chain_descr = { 0 };
	swap_chain_descr.BufferDesc.RefreshRate.Numerator = 0;
	swap_chain_descr.BufferDesc.RefreshRate.Denominator = 1;
	swap_chain_descr.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swap_chain_descr.SampleDesc.Count = 1;
	swap_chain_descr.SampleDesc.Quality = 0;
	swap_chain_descr.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_descr.BufferCount = 1;
	swap_chain_descr.OutputWindow = hwnd;
	swap_chain_descr.Windowed = true;

	D3D_FEATURE_LEVEL feature_level;
	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		flags,
		NULL,
		0,
		D3D11_SDK_VERSION,
		&swap_chain_descr,
		&swap_chain_ptr,
		&device_ptr,
		&feature_level,
		&device_context_ptr);

	assert(S_OK == hr && swap_chain_ptr && device_ptr && device_context_ptr);
	
	ID3D11Texture2D* framebuffer;
	hr = swap_chain_ptr->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**)&framebuffer);

	assert(SUCCEEDED(hr));

	hr = device_ptr->CreateRenderTargetView(
		framebuffer, 0, &render_target_view_ptr);

	assert(SUCCEEDED(hr));
	framebuffer->Release();




	PositionOnlyInputLayout = NULL;
}

void Renderer::ClearRTV()
{
	device_context_ptr->ClearRenderTargetView(render_target_view_ptr, background_colour);

}

void Renderer::Present()
{
	swap_chain_ptr->Present(1, 0);
}

void Renderer::SetViewportFromWindow(GLFWwindow* window)
{
	RECT winRect;
	HWND hwnd = glfwGetWin32Window(window); // @todo: cache please
	GetClientRect(hwnd, &winRect);
	D3D11_VIEWPORT viewport = {
	  0.0f,
	  0.0f,
	  (FLOAT)(winRect.right - winRect.left),
	  (FLOAT)(winRect.bottom - winRect.top),
	  0.0f,
	  1.0f };
	device_context_ptr->RSSetViewports(1, &viewport);
}

void Renderer::SetRenderTarget()
{
	device_context_ptr->OMSetRenderTargets(1, &render_target_view_ptr, NULL);
}

void Renderer::DrawQuadFS()
{
		
}

void Renderer::Draw(unsigned int vertexCount, unsigned int start)
{
	device_context_ptr->Draw(vertexCount, start);
}

void Renderer::Draw(VertexBuffer* vBuffer, unsigned int start)
{
	Renderer::Draw(vBuffer->_count, start);
}

void Renderer::SetVertexBuffer(const VertexBuffer* vBuffer)
{
	ID3D11Buffer* vertex_buffer = vBuffer->GetD3DBuffer();
	device_context_ptr->IASetVertexBuffers(
		0,
		1,
		&vertex_buffer,
		&vBuffer->_stride,
		&vBuffer->_offset);

}

void Renderer::SetVertexShader(const VertexShader* shader)
{
	device_context_ptr->VSSetShader(shader->GetD3D11VertexShader(), NULL, 0);
}

void Renderer::SetPixelShader(const PixelShader* shader)
{
	device_context_ptr->PSSetShader(shader->GetD3D11PixelShader(), NULL, 0);
}

void Renderer::SetInputLayoutFromVertexShader(const VertexShader* shader)
{
	device_context_ptr->IASetInputLayout(shader->GetInputLayout());
}
