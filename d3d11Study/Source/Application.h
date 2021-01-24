#pragma once
class Renderer;
class Application
{
	// renderer* 
	// engine*
	//struct ID3D11Device* device_ptr;
	//struct ID3D11DeviceContext* device_context_ptr;
	//struct IDXGISwapChain* swap_chain_ptr;
	//struct ID3D11RenderTargetView* render_target_view_ptr;


	struct GLFWwindow* _window;
	Renderer* _renderer;
	float _wWidth = 1920.f, _wHeight = 1080.f;
	float _deltaTime = 0.f;
	float _lastFrame = 0.f;
	bool _shouldClose = false;

public:
	Application();

	int Init();
	void Tick();
	bool ShouldClose();
	void ProcessInput();
	void SwapBuffers();

};

