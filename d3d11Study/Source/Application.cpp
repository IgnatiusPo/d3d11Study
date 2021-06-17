#include "Application.h"
#include "windows.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "d3d11.h"
#include "dxgi.h"
#include "d3dcompiler.h"
#include "assert.h"
//#include "Model.h"

#pragma comment( lib, "user32" )          // link against the win32 library
#pragma comment( lib, "d3d11.lib" )       // direct3D library
#pragma comment( lib, "dxgi.lib" )        // directx graphics interface
#pragma comment( lib, "d3dcompiler.lib" ) // shader compiler


#include "VertexBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"

#include "Renderer.h"

//ID3D11Buffer* vertex_buffer_ptr = NULL;
VertexBuffer vBufferTri;
VertexBuffer vBufferCube;

VertexShader vertex_shader("Shaders/shaders.hlsl", "vs_main");
PixelShader pixel_shader("Shaders/shaders.hlsl", "ps_main");
VertexShader vertex_shader_deferred("Shaders/ShadersDeferred.hlsl", "vs_main");
PixelShader pixel_shader_deferred("Shaders/ShadersDeferred.hlsl", "ps_main");
ID3D11VertexShader* vertex_shader_ptr = NULL;
ID3D11PixelShader* pixel_shader_ptr = NULL;



// end test

namespace Callbacks
{
    void mouse_callback(GLFWwindow* window, double xpos, double ypos)
    {
        Application* app = (Application*)(glfwGetWindowUserPointer(window));
        app->_camera.ProcessMouseInput(xpos, ypos);
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

Application::Application() : _camera(glm::vec3(0.f, 0.f, -10.f))
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
    vertex_shader_deferred.CompileShader();
    pixel_shader_deferred.CompileShader();

    // Run the message loop.
    vertex_shader_ptr = NULL;
     pixel_shader_ptr = NULL;

    vertex_shader.CreateVertexShader(_renderer->_device);
    pixel_shader.CreatePixelShader(_renderer->_device);
    vertex_shader_deferred.CreateVertexShader(_renderer->_device);
    pixel_shader_deferred.CreatePixelShader(_renderer->_device);

    // layout
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
      { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      /*
      { "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "NOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      */
    };
    D3D11_INPUT_ELEMENT_DESC inputElementDescDeferred[] = {
  { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    vertex_shader.CreateInputLayout(_renderer->_device, inputElementDesc, 1);
    vertex_shader_deferred.CreateInputLayout(_renderer->_device, inputElementDescDeferred, 2);

    //vertex data
    float vertex_data_array[] = {
       0.0f,  0.5f,  0.0f, 12.f, // point at top and garbage for test :)
      -0.5f, -0.5f,  0.0f, 42.f, // point at bottom-left
       0.5f, -0.5f,  0.0f, 53.f,// point at bottom-right
    };
     UINT vertex_stride = 4 * sizeof(float);
     UINT vertex_offset = 0;
     UINT vertex_count = 3;

    { /*** load mesh data into vertex buffer **/
        D3D11_BUFFER_DESC vertex_buff_descr = {};
        vertex_buff_descr.ByteWidth = sizeof(vertex_data_array);
        vertex_buff_descr.Usage = D3D11_USAGE_DEFAULT;
        vertex_buff_descr.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        
        vBufferTri.CreateBuffer(_renderer->_device, vertex_buff_descr, vertex_data_array, vertex_stride, vertex_count, vertex_offset);
    }
    {
        // cube
        D3D11_BUFFER_DESC vertex_buff_descr = {};
        vertex_buff_descr.ByteWidth = sizeof(Renderer::cube_positions);
        vertex_buff_descr.Usage = D3D11_USAGE_DEFAULT;
        vertex_buff_descr.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        UINT vertex_stride = 3 * sizeof(float);
        UINT vertex_count = sizeof(Renderer::cube_positions) / (3 * sizeof(float));

        vBufferCube.CreateBuffer(_renderer->_device, vertex_buff_descr, Renderer::cube_positions, vertex_stride, vertex_count);
    }



    _shouldClose = false;

    // initialize scene 
    _scene.AddObjectToScene(LoadModel<ModelType::STL>("res/stl/Einstein.stl", _renderer));
    _scene.AddObjectToScene(LoadModel<ModelType::STL>("res/stl/moon_city_final.stl ", _renderer));

    glm::mat4 modelMat = glm::mat4(1.f);

    modelMat = glm::scale(modelMat, glm::vec3(0.2f, 0.2f, 0.2f));
    modelMat = glm::translate(modelMat, glm::vec3(0.f, -35.f, 400.f));
    modelMat = glm::rotate(modelMat, glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
    modelMat = glm::rotate(modelMat, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
    _scene._sceneObjects[0].SetModelMatrix(modelMat);
    modelMat = glm::mat4(1.f);
    modelMat = glm::translate(modelMat, glm::vec3(0.f, -50.f, 100.f));
    modelMat = glm::rotate(modelMat, glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
    modelMat = glm::rotate(modelMat, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
    _scene._sceneObjects[1].SetModelMatrix(modelMat);


	return 0;
}

void Application::Tick()
{
    float CurrentFrame = (float)glfwGetTime();
    _deltaTime = CurrentFrame - _lastFrame;
    _lastFrame = CurrentFrame;

	///**** handle user input and other window events ****/
    //MSG msg = {};
	//if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
	//    TranslateMessage(&msg);
	//    DispatchMessage(&msg);
	//}
	//if (msg.message == WM_QUIT) { _shouldClose = true; }
    glfwPollEvents();
    ProcessInput();


	/* clear the back buffer */
    _renderer->SetBackBufferRenderTarget();
    _renderer->ClearBackbufferRTV();

    _renderer->InitViewportFromWindow(_window);
    _renderer->ResetViewport();

    _renderer->EnableDepthStencil();

    _renderer->_deviceContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



    // draw triangle
    _renderer->SetInputLayoutFromVertexShader(&vertex_shader);
    _renderer->SetVertexBuffer(&vBufferTri);

	// set the shaders
    _renderer->SetVertexShader(&vertex_shader);
    _renderer->SetPixelShader(&pixel_shader);
    //_renderer->Draw(&vBufferTri);

    _renderer->SetInputLayoutFromVertexShader(&vertex_shader);
    _renderer->SetVertexBuffer(&vBufferCube);
    _renderer->SetVertexShader(&vertex_shader);
    _renderer->SetPixelShader(&pixel_shader);

    glm::mat4 view = glm::mat4(1.0f);
    view = _camera.GetViewMatrix();
    //view = glm::lookAtLH(glm::vec3(0.f, 0.f, -10.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f));

    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspectiveFovLH_ZO(glm::radians(45.f), _wWidth , _wHeight, 0.1f, 1000.f); //todo setup near and far properly
    //projection = glm::perspective(glm::radians(45.f), _wWidth / _wHeight, 0.1f, 4000.f); //todo setup near and far properly
    // glm uses OpenGL convention for projection, making Z in [-1, 1]. D3D needs range [0, 1]
    // @todo write my own perpective projection
    //projection = glm::translate(glm::scale(projection, glm::vec3(0.f, 0.f, 0.5f)), glm::vec3(0.f, 0.f, 0.5f));
    //projection = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.5f)) * glm::scale(glm::mat4(1.f), glm::vec3(0.f, 0.f, 0.5f)) * projection;
    
    _renderer->UpdateViewBuffer(view, projection, glm::vec4(_camera._position, 1.f));
    _renderer->BindVertexViewBuffer();

    //_renderer->Draw(&vBufferCube);
    

    // not deferred pass
    //_renderer->SetInputLayoutFromVertexShader(&vertex_shader);
    //_renderer->SetVertexBuffer(&_scene._sceneObjects[0]._meshes[0].GetVertexBuffer());
    //_renderer->SetVertexShader(&vertex_shader);

    //_renderer->SetPixelShader(&pixel_shader);

    //_renderer->Draw(&_scene._sceneObjects[0]._meshes[0].GetVertexBuffer());
    //_renderer->Draw(_scene, view, projection);

    // deferred pass
    //_renderer->InitViewportFromWindow(_window);
    //_renderer->ResetViewport();
    //_renderer->EnableDepthStencil();
    //_renderer->SetRenderTargetsGBuffer();
    //_renderer->ClearGBufferRTVs();

    //_renderer->SetInputLayoutFromVertexShader(&vertex_shader_deferred);
    //_renderer->SetVertexBuffer(_scene._sceneObjects[0]._meshes[0].GetVertexBuffer());
    //_renderer->SetVertexShader(&vertex_shader_deferred);

    //_renderer->UpdateViewBuffer(view, projection);
    //_renderer->BindVertexViewBuffer();


    //_renderer->UpdatePerModelBuffer(_scene._sceneObjects[0]._modelMatrix);
    //_renderer->BindPerModelBuffer();

    //_renderer->SetPixelShader(&pixel_shader_deferred);
    //_renderer->Draw(_scene._sceneObjects[0]._meshes[0].GetVertexBuffer());
    _renderer->GeometryPass(&_scene, &_camera);

    //if (_useSSAO)
    //{
        _renderer->RenderSSAO();
    //}
    _renderer->RenderLight();
    // draw quad
    _renderer->DrawQuadFS();

    _renderer->DrawCube();
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

    if (glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS)
        _camera.ProcessKeyboardInput(CameraMovement::FORWARD, _deltaTime);
    if (glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS)
        _camera.ProcessKeyboardInput(CameraMovement::BACKWARD, _deltaTime);
    if (glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS)
        _camera.ProcessKeyboardInput(CameraMovement::LEFT, _deltaTime);
    if (glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS)
        _camera.ProcessKeyboardInput(CameraMovement::RIGHT, _deltaTime);
    if (glfwGetKey(_window, GLFW_KEY_F1) == GLFW_PRESS)
        _useSSAO = !_useSSAO;


}

void Application::SwapBuffers()
{
}
