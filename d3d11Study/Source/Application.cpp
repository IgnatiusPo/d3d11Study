#include "Application.h"
#include "windows.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "d3d11.h"
#include "dxgi.h"
#include "d3dcompiler.h"
#include "assert.h"

#pragma comment( lib, "user32" )          // link against the win32 library
#pragma comment( lib, "d3d11.lib" )       // direct3D library
#pragma comment( lib, "dxgi.lib" )        // directx graphics interface
#pragma comment( lib, "d3dcompiler.lib" ) // shader compiler


#include "VertexBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"

#include "Renderer.h"

//ID3D11Buffer* vertex_buffer_ptr = NULL;
VertexBuffer vBuffer;
ID3DBlob* vs_blob_ptr = NULL, * ps_blob_ptr = NULL, * error_blob = NULL;
ID3D11InputLayout* input_layout_ptr = NULL;

VertexShader vertex_shader("shaders.hlsl", "vs_main");
PixelShader pixel_shader("shaders.hlsl", "ps_main");
ID3D11VertexShader* vertex_shader_ptr = NULL;
ID3D11PixelShader* pixel_shader_ptr = NULL;



// end test

namespace Callbacks
{
    void mouse_callback(GLFWwindow* window, double xpos, double ypos)
    {
        //Application* app = (Application*)(glfwGetWindowUserPointer(window));

        //static bool firstMouse = true;
        //if (firstMouse)
        //{
        //    app->_lastCameraX = (float)xpos;
        //    app->_lastCameraY = (float)ypos;
        //    firstMouse = false;
        //}

        //float xoffset = (float)xpos - app->_lastCameraX;
        //float yoffset = app->_lastCameraY - (float)ypos;
        //app->_lastCameraX = (float)xpos;
        //app->_lastCameraY = (float)ypos;

        //app->_camera.ProcessMouseInput(xoffset, yoffset);
    }

    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        //Application* app = (Application*)(glfwGetWindowUserPointer(window));
        //app->_camera.ProcessScrollInput((float)yoffset, app->_fov);
    }
    void FrameBufferSize(GLFWwindow* window, int Width, int Height)
    {
        //glViewport(0, 0, Width, Height);
    }
}

Application::Application()
{
    _window = NULL;
    //device_ptr = NULL;
    //device_context_ptr = NULL;
    //swap_chain_ptr = NULL;
    //render_target_view_ptr = NULL;
    _renderer = new Renderer();
}

int Application::Init()
{
	if (!glfwInit())
		return -1;


	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	_window = glfwCreateWindow((int)_wWidth, (int)_wHeight, "Application", NULL, NULL);
	if (!_window) {
		glfwTerminate();
		return -1;
	}
    // need for input processing
    glfwSetWindowUserPointer(_window, this);

    _renderer->Init(_window);

    // shader 
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG; // add more debug output
#endif

    vertex_shader.CompileShader();
    pixel_shader.CompileShader();

    // Run the message loop.
    vertex_shader_ptr = NULL;
     pixel_shader_ptr = NULL;

    vertex_shader.CreateVertexShader(_renderer->device_ptr);
    pixel_shader.CreatePixelShader(_renderer->device_ptr);

    // layout
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
      { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      /*
      { "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "NOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      */
    };
    vertex_shader.CreateInputLayout(_renderer->device_ptr, inputElementDesc, 1);

    //vertex data
    float vertex_data_array[] = {
       0.0f,  0.5f,  0.0f, // point at top
       0.5f, -0.5f,  0.0f, // point at bottom-right
      -0.5f, -0.5f,  0.0f, // point at bottom-left
    };
     UINT vertex_stride = 3 * sizeof(float);
     UINT vertex_offset = 0;
     UINT vertex_count = 3;

    { /*** load mesh data into vertex buffer **/
        D3D11_BUFFER_DESC vertex_buff_descr = {};
        vertex_buff_descr.ByteWidth = sizeof(vertex_data_array);
        vertex_buff_descr.Usage = D3D11_USAGE_DEFAULT;
        vertex_buff_descr.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        
        vBuffer.CreateBuffer(_renderer->device_ptr, vertex_buff_descr, vertex_data_array, vertex_stride, vertex_count, vertex_offset);
    }


    _shouldClose = false;

	return 0;
}

void Application::Tick()
{
	///**** handle user input and other window events ****/
    //MSG msg = {};
	//if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
	//    TranslateMessage(&msg);
	//    DispatchMessage(&msg);
	//}
	//if (msg.message == WM_QUIT) { _shouldClose = true; }
    glfwPollEvents();
    ProcessInput();


	/* clear the back buffer to cornflower blue for the new frame */
    _renderer->ClearRTV();

    _renderer->SetViewportFromWindow(_window);

    _renderer->SetRenderTarget();

    //@todo handle input layout
    _renderer->device_context_ptr->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    _renderer->SetInputLayoutFromVertexShader(&vertex_shader);
    _renderer->SetVertexBuffer(&vBuffer);

	// set the shaders
    _renderer->SetVertexShader(&vertex_shader);
    _renderer->SetPixelShader(&pixel_shader);

	// draw 
    _renderer->Draw(&vBuffer);

	// present 
    _renderer->Present();

}

bool Application::ShouldClose()
{
	return _shouldClose;
}

void Application::ProcessInput()
{
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(_window, Callbacks::mouse_callback);
    glfwSetScrollCallback(_window, Callbacks::scroll_callback);

    if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        _shouldClose = true;
        //glfwSetWindowShouldClose(_window, GLFW_TRUE);
    }

}

void Application::SwapBuffers()
{
}
