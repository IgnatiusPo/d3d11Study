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
#include "GeometryShader.h"
#include "Scene.h"
#include "Camera.h"
#include <random>
#include "ParticleSystem.h"
//Initialize static variables
VertexBuffer Renderer::quad_vBuffer;
std::vector<VertexShader>		Renderer::VertexShaders;
std::vector<PixelShader>		Renderer::PixelShaders;
std::vector<GeometryShader>		Renderer::GeometryShaders;

std::vector<Texture>			Renderer::Textures;


 const float Renderer::quadPosTex[] = { //vertex attributes for a quad that fills the entire screen in NDC.
	-1.0f, -1.0f, 0.f, 1.f,
	-1.0f,  1.0f, 0.f, 0.f,
	 1.0f, -1.0f, 1.f, 1.f,

	 1.0f, -1.0f, 1.f, 1.f,
	-1.0f,  1.0f, 0.f, 0.f,
	 1.0f,  1.0f, 1.f, 0.f,
};
const float Renderer::cube_positions[] = {
	 // positions          
	 -1.0f,  1.0f, 1.0f,
	 -1.0f, -1.0f, 1.0f,
	  1.0f, -1.0f, 1.0f,
	  1.0f, -1.0f, 1.0f,
	  1.0f,  1.0f, 1.0f,
	 -1.0f,  1.0f, 1.0f,

	 -1.0f, -1.0f,  -1.0f,
	 -1.0f, -1.0f, 1.0f,
	 -1.0f,  1.0f, 1.0f,
	 -1.0f,  1.0f, 1.0f,
	 -1.0f,  1.0f, -1.0f,
	 -1.0f, -1.0f,  -1.0f,

	  1.0f, -1.0f, 1.0f,
	  1.0f, -1.0f,  -1.0f,
	  1.0f,  1.0f,  -1.0f,
	  1.0f,  1.0f,  -1.0f,
	  1.0f,  1.0f, 1.0f,
	  1.0f, -1.0f, 1.0f,

	 -1.0f, -1.0f,  -1.0f,
	 -1.0f,  1.0f,  -1.0f,
	  1.0f,  1.0f,  -1.0f,
	  1.0f,  1.0f,  -1.0f,
	  1.0f, -1.0f,  -1.0f,
	 -1.0f, -1.0f,  -1.0f,

	 -1.0f,  1.0f, 1.0f,
	  1.0f,  1.0f, 1.0f,
	  1.0f,  1.0f,  -1.0f,
	  1.0f,  1.0f,  -1.0f,
	 -1.0f,  1.0f,  -1.0f,
	 -1.0f,  1.0f, 1.0f,

	 -1.0f, -1.0f, 1.0f,
	 -1.0f, -1.0f,  -1.0f,
	  1.0f, -1.0f, 1.0f,
	  1.0f, -1.0f, 1.0f,
	 -1.0f, -1.0f,  -1.0f,
	  1.0f, -1.0f,  -1.0f
};

 float Renderer::background_colour[] = {  0.0f, 0.0f, 0.0f, 1.0f };




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
		&_swapchain,
		&_device,
		&feature_level,
		&_deviceContext);

	assert(S_OK == hr && _swapchain && _device && _deviceContext);
	
	//todo
	InitViewportFromWindow(window);
	ID3D11Texture2D* framebuffer;
	hr = _swapchain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**)&framebuffer);

	assert(SUCCEEDED(hr));

	hr = _device->CreateRenderTargetView(
		framebuffer, 0, &_backBufferRTV);

	assert(SUCCEEDED(hr));
	D3D11_TEXTURE2D_DESC framebufferDesc;
	framebuffer->GetDesc(&framebufferDesc);
	framebuffer->Release();

	//set rasterizer state
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	

	hr = _device->CreateRasterizerState(&rasterDesc, &_rasterizerState);
	assert(SUCCEEDED(hr));
	_deviceContext->RSSetState(_rasterizerState);

	//blend state(s)
	D3D11_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	for (int i = 0; i < 8; ++i)
	{
		blendDesc.RenderTarget[i].BlendEnable = false; // no blending
		blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	_device->CreateBlendState(&blendDesc, &_blendStateNoBlending);




	//create depth-stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = framebufferDesc.Width;
	descDepth.Height = framebufferDesc.Height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = _device->CreateTexture2D(&descDepth, NULL, &_depthStencilBuffer);

	//todo depth-stencil STATE
	D3D11_DEPTH_STENCIL_DESC enabledDepthStencilDesc;
	ZeroMemory(&enabledDepthStencilDesc, sizeof(enabledDepthStencilDesc));
	// Depth test parameters
	enabledDepthStencilDesc.DepthEnable = true;
	enabledDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	enabledDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	// Stencil test parameters
	enabledDepthStencilDesc.StencilEnable = false;
	enabledDepthStencilDesc.StencilReadMask = 0xFF;
	enabledDepthStencilDesc.StencilWriteMask = 0xFF;
	// Stencil operations if pixel is front-facing
	enabledDepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	enabledDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	enabledDepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	enabledDepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	enabledDepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	enabledDepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	enabledDepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	enabledDepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Create depth stencil states
	_device->CreateDepthStencilState(&enabledDepthStencilDesc, &_depthEnabledStencilState);
	D3D11_DEPTH_STENCIL_DESC disabledDepthStencilDesc;
	disabledDepthStencilDesc = enabledDepthStencilDesc;
	disabledDepthStencilDesc.DepthEnable = false;
	disabledDepthStencilDesc.StencilEnable = false;

	_device->CreateDepthStencilState(&disabledDepthStencilDesc, &_depthDisabledStencilState);

	D3D11_DEPTH_STENCIL_DESC _depthEnabledWriteDisabledDesc;
	_depthEnabledWriteDisabledDesc = enabledDepthStencilDesc;
	_depthEnabledWriteDisabledDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;


	_device->CreateDepthStencilState(&_depthEnabledWriteDisabledDesc, &_depthEnabledWriteDisabled);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthDescView;
	ZeroMemory(&depthDescView, sizeof(depthDescView));

	depthDescView.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDescView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthDescView.Texture2D.MipSlice = 0;
	hr = _device->CreateDepthStencilView(_depthStencilBuffer, &depthDescView, &_depthStencilView);

	D3D11_SHADER_RESOURCE_VIEW_DESC DepthSRV;

	DepthSRV.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	DepthSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	DepthSRV.Texture2D.MostDetailedMip = 0;
	DepthSRV.Texture2D.MipLevels = 1;

	hr = _device->CreateShaderResourceView(_depthStencilBuffer, &DepthSRV, &_depthStencilSRV);
	//@todo need to release?
	//_depthStencilBuffer->Release();


	/*  ******************************************************************************************* */
	// Create GBuffers
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = framebufferDesc.Width;
	textureDesc.Height = framebufferDesc.Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	for (int i = 0; i < (int)GBufferType::Num; ++i)
	{
		_device->CreateTexture2D(&textureDesc, NULL, &_GBufferTextureArray[i]);
	}

	// Setup the description of the render target views
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	
	for (int i = 0; i < (int)GBufferType::Num; ++i)
	{
		_device->CreateRenderTargetView(_GBufferTextureArray[i], &rtvDesc, &_GBufferRenderTargetViewArray[i]);
	}


	// Setup the description of the shader resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	for (int i = 0; i < (int)GBufferType::Num; ++i)
	{
		_device->CreateShaderResourceView(_GBufferTextureArray[i], &srvDesc, &_GBufferShaderResourceViewArray[i]);
	}

	//create SSAO texture
	{
		D3D11_TEXTURE2D_DESC SSAOTextureDesc;
		SSAOTextureDesc.Width = framebufferDesc.Width;
		SSAOTextureDesc.Height = framebufferDesc.Height;
		SSAOTextureDesc.MipLevels = 1;
		SSAOTextureDesc.ArraySize = 1;
		SSAOTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		SSAOTextureDesc.SampleDesc.Count = 1;
		SSAOTextureDesc.SampleDesc.Quality = 0;
		SSAOTextureDesc.Usage = D3D11_USAGE_DEFAULT;
		SSAOTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		SSAOTextureDesc.CPUAccessFlags = 0;
		SSAOTextureDesc.MiscFlags = 0;

		_device->CreateTexture2D(&SSAOTextureDesc, NULL, &_SSAOTexture);
		// blur
		_device->CreateTexture2D(&SSAOTextureDesc, NULL, &_BlurTexture);
		_device->CreateTexture2D(&SSAOTextureDesc, NULL, &_BlurHorizontalTexture);

		// Setup the description of the render target views
		D3D11_RENDER_TARGET_VIEW_DESC SSAORtvDesc;
		SSAORtvDesc.Format = SSAOTextureDesc.Format;
		SSAORtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		SSAORtvDesc.Texture2D.MipSlice = 0;

		_device->CreateRenderTargetView(_SSAOTexture, &SSAORtvDesc, &_SSAO_RTV);

		//blur
		_device->CreateRenderTargetView(_BlurTexture, &SSAORtvDesc, &_Blur_RTV);
		_device->CreateRenderTargetView(_BlurHorizontalTexture, &SSAORtvDesc, &_Blur_HorizontalRTV);



		// Setup the description of the shader resource view.
		D3D11_SHADER_RESOURCE_VIEW_DESC SSAOSrvDesc;

		SSAOSrvDesc.Format = textureDesc.Format;
		SSAOSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SSAOSrvDesc.Texture2D.MostDetailedMip = 0;
		SSAOSrvDesc.Texture2D.MipLevels = 1;

		//ssao
		_device->CreateShaderResourceView(_SSAOTexture, &SSAOSrvDesc, &_SSAO_SRV);
		//blur
		_device->CreateShaderResourceView(_BlurTexture, &SSAOSrvDesc, &_Blur_SRV);
		_device->CreateShaderResourceView(_BlurHorizontalTexture, &SSAOSrvDesc, &_Blur_HorizontalSRV);


		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); //  [0.0, 1.0]
		std::default_random_engine randomGenerator;
		auto lerp = [](float a, float b, float t)
		{
			return a * (1.f - t) + b * t;
		};
		int kernelSize = 32;
		for (unsigned int i = 0; i < kernelSize; ++i)
		{
			glm::vec4 sample(
				randomFloats(randomGenerator) * 2.0 - 1.0,
				randomFloats(randomGenerator) * 2.0 - 1.0,
				randomFloats(randomGenerator),
				0.f
			);
			sample = glm::normalize(sample);
			sample *= randomFloats(randomGenerator);

			float scale = (float)i / (float)kernelSize;
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			_ssaoKernel.push_back(sample);
		}

		//ssao noise
		std::vector<glm::vec3> ssaoNoise;
		int noiseSize = 4;
		for (unsigned int i = 0; i < noiseSize * noiseSize; i++)
		{
			glm::vec3 noise(
				randomFloats(randomGenerator) * 2.0 - 1.0,
				randomFloats(randomGenerator) * 2.0 - 1.0,
				0.0f);
			ssaoNoise.push_back(noise);
		}

		D3D11_TEXTURE2D_DESC SSAOTextureNoiseDesc;
		SSAOTextureNoiseDesc.Width = noiseSize;
		SSAOTextureNoiseDesc.Height = noiseSize;
		SSAOTextureNoiseDesc.MipLevels = 1;
		SSAOTextureNoiseDesc.ArraySize = 1;
		SSAOTextureNoiseDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		SSAOTextureNoiseDesc.SampleDesc.Count = 1;
		SSAOTextureNoiseDesc.SampleDesc.Quality = 0;
		SSAOTextureNoiseDesc.Usage = D3D11_USAGE_DEFAULT;
		SSAOTextureNoiseDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		SSAOTextureNoiseDesc.CPUAccessFlags = 0;
		SSAOTextureNoiseDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA SubData;
		ZeroMemory(&SubData, sizeof(SubData));
		SubData.pSysMem = ssaoNoise.data();
		SubData.SysMemPitch = noiseSize * 12;

		_device->CreateTexture2D(&SSAOTextureNoiseDesc, &SubData, &_SSAONoiseTexture);
		D3D11_SHADER_RESOURCE_VIEW_DESC SSAONoiseSrvDesc;

		SSAONoiseSrvDesc.Format = SSAOTextureNoiseDesc.Format;
		SSAONoiseSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SSAONoiseSrvDesc.Texture2D.MostDetailedMip = 0;
		SSAONoiseSrvDesc.Texture2D.MipLevels = 1;

		_device->CreateShaderResourceView(_SSAONoiseTexture, &SSAONoiseSrvDesc, &_SSAONoiseSRV);
		
	}


	// create known sampler states
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler states.
	_device->CreateSamplerState(&samplerDesc, &_samplerStateClamp);

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	_device->CreateSamplerState(&samplerDesc, &_samplerStateWrap);




	//initialize default vertex buffers
	D3D11_BUFFER_DESC vertex_buff_descr = {};
	vertex_buff_descr.ByteWidth = sizeof(Renderer::quadPosTex);
	vertex_buff_descr.Usage = D3D11_USAGE_DEFAULT;
	vertex_buff_descr.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	unsigned int stride = 4 * sizeof(float);
	unsigned int count = 6;

	quad_vBuffer.CreateBuffer(_device, vertex_buff_descr, Renderer::quadPosTex, stride, count);

	// create cbuffers
	CreateViewBuffer();
	CreatePerModelBuffer();
	CreateLightBuffer();
	CreateSSAOKernelBuffer();

	//create default shaders
	GetNDCVertexShader();
	GetSimpleColorPixelShader();
}
void Renderer::Init(HWND hwnd)
{
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
		&_swapchain,
		&_device,
		&feature_level,
		&_deviceContext);

	assert(S_OK == hr && _swapchain && _device && _deviceContext);

	//todo
	InitViewportFromWindow(hwnd);
	ID3D11Texture2D* framebuffer;
	hr = _swapchain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**)&framebuffer);

	assert(SUCCEEDED(hr));

	hr = _device->CreateRenderTargetView(
		framebuffer, 0, &_backBufferRTV);

	assert(SUCCEEDED(hr));
	D3D11_TEXTURE2D_DESC framebufferDesc;
	framebuffer->GetDesc(&framebufferDesc);
	framebuffer->Release();

	//set rasterizer state
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;


	hr = _device->CreateRasterizerState(&rasterDesc, &_rasterizerState);
	assert(SUCCEEDED(hr));
	_deviceContext->RSSetState(_rasterizerState);

	//create depth-stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = framebufferDesc.Width;
	descDepth.Height = framebufferDesc.Height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = _device->CreateTexture2D(&descDepth, NULL, &_depthStencilBuffer);

	//todo depth-stencil STATE
	D3D11_DEPTH_STENCIL_DESC enabledDepthStencilDesc;
	ZeroMemory(&enabledDepthStencilDesc, sizeof(enabledDepthStencilDesc));
	// Depth test parameters
	enabledDepthStencilDesc.DepthEnable = true;
	enabledDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	enabledDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	// Stencil test parameters
	enabledDepthStencilDesc.StencilEnable = false;
	enabledDepthStencilDesc.StencilReadMask = 0xFF;
	enabledDepthStencilDesc.StencilWriteMask = 0xFF;
	// Stencil operations if pixel is front-facing
	enabledDepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	enabledDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	enabledDepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	enabledDepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	enabledDepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	enabledDepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	enabledDepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	enabledDepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Create depth stencil states
	_device->CreateDepthStencilState(&enabledDepthStencilDesc, &_depthEnabledStencilState);
	D3D11_DEPTH_STENCIL_DESC disabledDepthStencilDesc;
	disabledDepthStencilDesc = enabledDepthStencilDesc;
	disabledDepthStencilDesc.DepthEnable = false;
	disabledDepthStencilDesc.StencilEnable = false;

	_device->CreateDepthStencilState(&disabledDepthStencilDesc, &_depthDisabledStencilState);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthDescView;
	ZeroMemory(&depthDescView, sizeof(depthDescView));

	depthDescView.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDescView.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthDescView.Texture2D.MipSlice = 0;
	hr = _device->CreateDepthStencilView(_depthStencilBuffer, &depthDescView, &_depthStencilView);

	D3D11_SHADER_RESOURCE_VIEW_DESC DepthSRV;

	DepthSRV.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	DepthSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	DepthSRV.Texture2D.MostDetailedMip = 0;
	DepthSRV.Texture2D.MipLevels = 1;

	hr = _device->CreateShaderResourceView(_depthStencilBuffer, &DepthSRV, &_depthStencilSRV);
	//@todo need to release?
	//_depthStencilBuffer->Release();


	/*  ******************************************************************************************* */
	// Create GBuffers
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = framebufferDesc.Width;
	textureDesc.Height = framebufferDesc.Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	for (int i = 0; i < (int)GBufferType::Num; ++i)
	{
		_device->CreateTexture2D(&textureDesc, NULL, &_GBufferTextureArray[i]);
	}

	// Setup the description of the render target views
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	for (int i = 0; i < (int)GBufferType::Num; ++i)
	{
		_device->CreateRenderTargetView(_GBufferTextureArray[i], &rtvDesc, &_GBufferRenderTargetViewArray[i]);
	}


	// Setup the description of the shader resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	for (int i = 0; i < (int)GBufferType::Num; ++i)
	{
		_device->CreateShaderResourceView(_GBufferTextureArray[i], &srvDesc, &_GBufferShaderResourceViewArray[i]);
	}

	//create SSAO texture
	{
		D3D11_TEXTURE2D_DESC SSAOTextureDesc;
		SSAOTextureDesc.Width = framebufferDesc.Width;
		SSAOTextureDesc.Height = framebufferDesc.Height;
		SSAOTextureDesc.MipLevels = 1;
		SSAOTextureDesc.ArraySize = 1;
		SSAOTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		SSAOTextureDesc.SampleDesc.Count = 1;
		SSAOTextureDesc.SampleDesc.Quality = 0;
		SSAOTextureDesc.Usage = D3D11_USAGE_DEFAULT;
		SSAOTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		SSAOTextureDesc.CPUAccessFlags = 0;
		SSAOTextureDesc.MiscFlags = 0;

		_device->CreateTexture2D(&SSAOTextureDesc, NULL, &_SSAOTexture);
		// blur
		_device->CreateTexture2D(&SSAOTextureDesc, NULL, &_BlurTexture);
		_device->CreateTexture2D(&SSAOTextureDesc, NULL, &_BlurHorizontalTexture);

		// Setup the description of the render target views
		D3D11_RENDER_TARGET_VIEW_DESC SSAORtvDesc;
		SSAORtvDesc.Format = SSAOTextureDesc.Format;
		SSAORtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		SSAORtvDesc.Texture2D.MipSlice = 0;

		_device->CreateRenderTargetView(_SSAOTexture, &SSAORtvDesc, &_SSAO_RTV);

		//blur
		_device->CreateRenderTargetView(_BlurTexture, &SSAORtvDesc, &_Blur_RTV);
		_device->CreateRenderTargetView(_BlurHorizontalTexture, &SSAORtvDesc, &_Blur_HorizontalRTV);



		// Setup the description of the shader resource view.
		D3D11_SHADER_RESOURCE_VIEW_DESC SSAOSrvDesc;

		SSAOSrvDesc.Format = textureDesc.Format;
		SSAOSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SSAOSrvDesc.Texture2D.MostDetailedMip = 0;
		SSAOSrvDesc.Texture2D.MipLevels = 1;

		_device->CreateShaderResourceView(_SSAOTexture, &SSAOSrvDesc, &_SSAO_SRV);
		//blur
		_device->CreateShaderResourceView(_BlurTexture, &SSAOSrvDesc, &_Blur_SRV);
		_device->CreateShaderResourceView(_BlurHorizontalTexture, &SSAOSrvDesc, &_Blur_HorizontalSRV);


		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); //  [0.0, 1.0]
		std::default_random_engine randomGenerator;
		auto lerp = [](float a, float b, float t)
		{
			return a * (1.f - t) + b * t;
		};
		int kernelSize = 32;
		for (unsigned int i = 0; i < kernelSize; ++i)
		{
			glm::vec4 sample(
				randomFloats(randomGenerator) * 2.0 - 1.0,
				randomFloats(randomGenerator) * 2.0 - 1.0,
				randomFloats(randomGenerator),
				0.f
			);
			sample = glm::normalize(sample);
			sample *= randomFloats(randomGenerator);

			float scale = (float)i / (float)kernelSize;
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			_ssaoKernel.push_back(sample);
		}

		//ssao noise
		std::vector<glm::vec3> ssaoNoise;
		int noiseSize = 4;
		for (unsigned int i = 0; i < noiseSize * noiseSize; i++)
		{
			glm::vec3 noise(
				randomFloats(randomGenerator) * 2.0 - 1.0,
				randomFloats(randomGenerator) * 2.0 - 1.0,
				0.0f);
			ssaoNoise.push_back(noise);
		}

		D3D11_TEXTURE2D_DESC SSAOTextureNoiseDesc;
		SSAOTextureNoiseDesc.Width = noiseSize;
		SSAOTextureNoiseDesc.Height = noiseSize;
		SSAOTextureNoiseDesc.MipLevels = 1;
		SSAOTextureNoiseDesc.ArraySize = 1;
		SSAOTextureNoiseDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		SSAOTextureNoiseDesc.SampleDesc.Count = 1;
		SSAOTextureNoiseDesc.SampleDesc.Quality = 0;
		SSAOTextureNoiseDesc.Usage = D3D11_USAGE_DEFAULT;
		SSAOTextureNoiseDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		SSAOTextureNoiseDesc.CPUAccessFlags = 0;
		SSAOTextureNoiseDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA SubData;
		ZeroMemory(&SubData, sizeof(SubData));
		SubData.pSysMem = ssaoNoise.data();
		SubData.SysMemPitch = noiseSize * 12;

		_device->CreateTexture2D(&SSAOTextureNoiseDesc, &SubData, &_SSAONoiseTexture);
		D3D11_SHADER_RESOURCE_VIEW_DESC SSAONoiseSrvDesc;

		SSAONoiseSrvDesc.Format = SSAOTextureNoiseDesc.Format;
		SSAONoiseSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SSAONoiseSrvDesc.Texture2D.MostDetailedMip = 0;
		SSAONoiseSrvDesc.Texture2D.MipLevels = 1;

		_device->CreateShaderResourceView(_SSAONoiseTexture, &SSAONoiseSrvDesc, &_SSAONoiseSRV);

	}


	// create known sampler states
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler states.
	_device->CreateSamplerState(&samplerDesc, &_samplerStateClamp);

	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

	_device->CreateSamplerState(&samplerDesc, &_samplerStateWrap);




	//initialize default vertex buffers
	D3D11_BUFFER_DESC vertex_buff_descr = {};
	vertex_buff_descr.ByteWidth = sizeof(Renderer::quadPosTex);
	vertex_buff_descr.Usage = D3D11_USAGE_DEFAULT;
	vertex_buff_descr.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	unsigned int stride = 4 * sizeof(float);
	unsigned int count = 6;

	quad_vBuffer.CreateBuffer(_device, vertex_buff_descr, Renderer::quadPosTex, stride, count);

	// create cbuffers
	CreateViewBuffer();
	CreatePerModelBuffer();
	CreateLightBuffer();
	CreateSSAOKernelBuffer();

	//create default shaders
	GetNDCVertexShader();
	GetSimpleColorPixelShader();
}


void Renderer::ClearBackbufferRTV()
{
	_deviceContext->ClearRenderTargetView(_backBufferRTV, background_colour);
	// Clear the depth buffer.
	_deviceContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

}

void Renderer::ClearGBufferRTVs()
{
	for (int i = 0; i < (int)GBufferType::Num; ++i)
	{
		_deviceContext->ClearRenderTargetView(_GBufferRenderTargetViewArray[i], background_colour);
	}
	// Clear the depth buffer.
	_deviceContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Renderer::Present()
{
	_swapchain->Present(1, 0);
}

void Renderer::InitViewportFromWindow(GLFWwindow* window)
{
	RECT winRect;
	HWND hwnd = glfwGetWin32Window(window); // @todo: cache please
	GetClientRect(hwnd, &winRect);
	_viewport  = {
	  0.0f,
	  0.0f,
	  (FLOAT)(winRect.right - winRect.left),
	  (FLOAT)(winRect.bottom - winRect.top),
	  0.0f,
	  1.0f };
}
void Renderer::InitViewportFromWindow(HWND hwnd)
{
	RECT winRect;
	GetClientRect(hwnd, &winRect);
	_viewport = {
	  0.0f,
	  0.0f,
	  (FLOAT)(winRect.right - winRect.left),
	  (FLOAT)(winRect.bottom - winRect.top),
	  0.0f,
	  1.0f };
}


void Renderer::ResetViewport()
{
	_deviceContext->RSSetViewports(1, &_viewport);
}

void Renderer::SetViewport(D3D11_VIEWPORT* viewport)
{
	_deviceContext->RSSetViewports(1, viewport);
}

void Renderer::SetBackBufferRenderTarget()
{
	_deviceContext->OMSetRenderTargets(1, &_backBufferRTV, _depthStencilView);
}

void Renderer::SetRenderTargetsGBuffer()
{
	//todo set viewport here?
	_deviceContext->OMSetRenderTargets((int)GBufferType::Num, _GBufferRenderTargetViewArray, _depthStencilView);
}

void Renderer::SetSSAORenderTarget()
{
	_deviceContext->OMSetRenderTargets(1, &_SSAO_RTV , _depthStencilView);
}

void Renderer::EnableDepthStencil()
{
	_deviceContext->OMSetDepthStencilState(_depthEnabledStencilState, 1);
}

void Renderer::DisableDepthStencil()
{
	_deviceContext->OMSetDepthStencilState(_depthDisabledStencilState, 1);
}

void Renderer::EnableDepthDisableWrite()
{
	_deviceContext->OMSetDepthStencilState(_depthEnabledWriteDisabled, 1);
}

void Renderer::BindBlendState_NoBlend()
{
	const FLOAT BlendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
	UINT SampleMask = 0xffffffff;
	_deviceContext->OMSetBlendState(_blendStateNoBlending, BlendFactor, SampleMask);
}

void Renderer::DrawFullScreenQuad()
{
	SetInputLayoutFromVertexShader(GetVertexShaderByID(GetNDCVertexShader()));
	SetVertexBuffer(&quad_vBuffer);

	// set the shaders
	SetVertexShader(GetVertexShaderByID(vertexShaderNDC));
	//@todo
	//pixel shader shouldn't be here, it should be already bound for FS pass, cmon
	//SetPixelShader(GetPixelShaderByID(pixelShaderSimpleColor));

	Draw(&quad_vBuffer, 0);
}

void Renderer::DrawFullScreenTriangle()
{
	SetInputLayout(NULL);
	SetVertexShader(GetVertexShaderByID(GetFullScreenTriangleVertexShader()));

	//@todo
	//pixel shader shouldn't be here, it should be already bound for FS pass, cmon
	//SetPixelShader(GetPixelShaderByID(pixelShaderSimpleColor));

	Draw(3, 0);
}

void Renderer::DrawCube()
{

}

void Renderer::Draw(unsigned int vertexCount, unsigned int start)
{
	_deviceContext->Draw(vertexCount, start);
}

void Renderer::Draw(const VertexBuffer* vBuffer, unsigned int start)
{
	Renderer::Draw(vBuffer->_count, start);
}

void Renderer::SetVertexBuffer(const VertexBuffer* vBuffer)
{
	ID3D11Buffer* vertex_buffer = vBuffer->GetD3DBuffer();
	_deviceContext->IASetVertexBuffers(
		0,
		1,
		&vertex_buffer,
		&vBuffer->_stride,
		&vBuffer->_offset);

}

void Renderer::SetVertexShader(const VertexShader* shader)
{
	if (shader && shader->GetD3D11VertexShader())
	{
		_deviceContext->VSSetShader(shader->GetD3D11VertexShader(), NULL, 0);
	}
	else
	{
		_deviceContext->VSSetShader(NULL, NULL, 0);
	}
}

void Renderer::SetPixelShader(const PixelShader* shader)
{
	if (shader && shader->GetD3D11PixelShader())
	{
		_deviceContext->PSSetShader(shader->GetD3D11PixelShader(), NULL, 0);
	}
	else
	{
		_deviceContext->PSSetShader(NULL, NULL, 0);
	}
}

void Renderer::SetGeometryShader(const GeometryShader* shader)
{
	if (shader && shader->GetD3D11GeometryShader())
	{
		_deviceContext->GSSetShader(shader->GetD3D11GeometryShader(), NULL, 0);
	}
	else
	{
		_deviceContext->GSSetShader(NULL, NULL, 0);
	}

}

void Renderer::SetInputLayoutFromVertexShader(const VertexShader* shader)
{
	_deviceContext->IASetInputLayout(shader->GetInputLayout());
}

void Renderer::SetClampSampler()
{
	_deviceContext->PSSetSamplers(0, 1, &_samplerStateClamp);
}

void Renderer::SetInputLayout(ID3D11InputLayout* layout)
{
	_deviceContext->IASetInputLayout(layout);
}


void Renderer::CreateViewBuffer()
{
	// create constant buffer
	D3D11_BUFFER_DESC view_buff_desc = {};
	view_buff_desc.ByteWidth = sizeof(_view);
	view_buff_desc.Usage = D3D11_USAGE_DEFAULT;
	view_buff_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HRESULT hr = _device->CreateBuffer(&view_buff_desc, nullptr, &_viewBuffer);

	assert(SUCCEEDED(hr));

}

void Renderer::UpdateViewBuffer(const glm::mat4& view, const glm::mat4& projection, const glm::vec4& viewWorldPos)
{
	_view._view = view;
	_view._projection = projection;
	_view._viewWorldPos = viewWorldPos;

	_deviceContext->UpdateSubresource(_viewBuffer, 0, 0, &_view, 0, 0);
}

void Renderer::BindVertexViewBuffer()
{
	_deviceContext->VSSetConstantBuffers(0, 1, &_viewBuffer);
}

void Renderer::BindPixelViewBuffer()
{
	_deviceContext->PSSetConstantBuffers(0, 1, &_viewBuffer);
}

void Renderer::BindGeometryViewBuffer()
{
	_deviceContext->GSSetConstantBuffers(0, 1, &_viewBuffer);
}

void Renderer::CreatePerModelBuffer()
{
	// create constant buffer
	D3D11_BUFFER_DESC perModel_buff_desc = {};
	perModel_buff_desc.ByteWidth = sizeof(_perModelData);
	perModel_buff_desc.Usage = D3D11_USAGE_DEFAULT;
	perModel_buff_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HRESULT hr = _device->CreateBuffer(&perModel_buff_desc, nullptr, &_perModelBuffer);

	assert(SUCCEEDED(hr));

}

void Renderer::UpdatePerModelBuffer(const glm::mat4& model)
{
	_perModelData._model = model;

	_deviceContext->UpdateSubresource(_perModelBuffer, 0, 0, &_perModelData, 0, 0);
}

void Renderer::BindPerModelBuffer()
{
	_deviceContext->VSSetConstantBuffers(1, 1, &_perModelBuffer);
}

void Renderer::CreateLightBuffer()
{
	// create constant buffer
	D3D11_BUFFER_DESC light_buff_desc = {};
	light_buff_desc.ByteWidth = sizeof(_lightData);
	light_buff_desc.Usage = D3D11_USAGE_DEFAULT;
	light_buff_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HRESULT hr = _device->CreateBuffer(&light_buff_desc, nullptr, &_lightBuffer);

	assert(SUCCEEDED(hr));

}

void Renderer::UpdateLightBuffer(const glm::vec3& direction)
{
	_lightData._direction = direction;

	_deviceContext->UpdateSubresource(_lightBuffer, 0, 0, &_lightData, 0, 0);
}

void Renderer::BindLightBuffer()
{
	_deviceContext->PSSetConstantBuffers(1, 1, &_lightBuffer);
}

void Renderer::CreateSSAOKernelBuffer()
{
	// create constant buffer
	D3D11_BUFFER_DESC kernel_buff_desc = {};
	kernel_buff_desc.ByteWidth = sizeof(glm::vec4) * _ssaoKernel.size();
	kernel_buff_desc.Usage = D3D11_USAGE_DEFAULT;
	kernel_buff_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HRESULT hr = _device->CreateBuffer(&kernel_buff_desc, nullptr, &_ssaoKernelBuffer);

	_deviceContext->UpdateSubresource(_ssaoKernelBuffer, 0, 0, _ssaoKernel.data(), 0, 0);
}

void Renderer::BindSSAOKernelBuffer()
{
	_deviceContext->PSSetConstantBuffers(1, 1, &_ssaoKernelBuffer);
}

void Renderer::GeometryPass(const Scene* scene, const Camera* camera)
{
	ResetViewport();
	EnableDepthStencil();
	SetRenderTargetsGBuffer();
	ClearGBufferRTVs();

	VertexShader* vertexShader = GetVertexShaderByID(GetDefaultModelVertexShader());
	SetInputLayoutFromVertexShader(vertexShader);
	SetVertexShader(vertexShader);
	PixelShader* pixelShader = GetPixelShaderByID(GetDefaultDeferredPixelShader());
	SetPixelShader(pixelShader);

	// camera
	glm::mat4 view = glm::mat4(1.0f);
	view = camera->GetViewMatrix();

	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspectiveFovLH_ZO(glm::radians(45.f), _viewport.Width, _viewport.Height, 0.1f, 1000.f); //todo setup near and far properly


	UpdateViewBuffer(view, projection, glm::vec4(camera->_position, 1.f));
	BindVertexViewBuffer();

	for (int i = 0; i < scene->_sceneObjects.size(); ++i)
	{
		UpdatePerModelBuffer(scene->_sceneObjects[i]._modelMatrix);
		BindPerModelBuffer();
		for (int j = 0; j < scene->_sceneObjects[i]._meshes.size(); ++j)
		{
			const VertexBuffer* vBuffer = scene->_sceneObjects[i]._meshes[j].GetVertexBuffer();
			SetVertexBuffer(vBuffer);
			Draw(vBuffer);
		}
	}
}

void Renderer::RenderLight(const glm::vec3& lightDirection)
{
	SetBackBufferRenderTarget();
	DisableDepthStencil();
	ResetViewport();
	ShaderID pixelShaderID = GetBasicLightPixelShader();
	SetupLightingParameters(pixelShaderID, lightDirection);

	DrawFullScreenTriangle();

	// unbind SRVs
	ID3D11ShaderResourceView* nullSRV = nullptr;
	_deviceContext->PSSetShaderResources(0, 1, &nullSRV);
	_deviceContext->PSSetShaderResources(1, 1, &nullSRV);
	_deviceContext->PSSetShaderResources(2, 1, &nullSRV);
	_deviceContext->PSSetShaderResources(3, 1, &nullSRV);

	EnableDepthStencil();
}

void Renderer::RenderSSAO()
{
	_deviceContext->ClearRenderTargetView(_SSAO_RTV, background_colour);

	DisableDepthStencil();
	ResetViewport();
	SetSSAORenderTarget();


	ShaderID pixelShaderID = GetSSAOPixelShader();
	SetupSSAOPatameters(pixelShaderID);

	DrawFullScreenTriangle();

	// unbind SRVs
	ID3D11ShaderResourceView* nullSRV = nullptr;
	_deviceContext->PSSetShaderResources(0, 1, &nullSRV);
	_deviceContext->PSSetShaderResources(1, 1, &nullSRV);
	_deviceContext->PSSetShaderResources(2, 1, &nullSRV);

	//blur

	BlurPass(_SSAO_SRV, _SSAO_BlurredSRV);
	//_deviceContext->ClearRenderTargetView(_Blur_RTV, background_colour);
	//_deviceContext->OMSetRenderTargets(1, &_Blur_RTV, _depthStencilView);

	//pixelShaderID = GetSimpleBlurPixelShader();
	//SetupBlurPatameters(pixelShaderID, _SSAO_SRV);

	//Draw(&quad_vBuffer);
	//_deviceContext->PSSetShaderResources(0, 1, &nullSRV);


}

void Renderer::BlurPass(ID3D11ShaderResourceView* input, ID3D11ShaderResourceView*& output, ID3D11RenderTargetView* freeRTV, ID3D11ShaderResourceView* freeSRVOut)
{
	_deviceContext->ClearRenderTargetView(_Blur_HorizontalRTV, background_colour);
	_deviceContext->OMSetRenderTargets(1, &_Blur_HorizontalRTV, nullptr);

	ShaderID pixelShaderHBlurID = GetHorizontalBlurPixelShader();
	SetupBlurPatameters(pixelShaderHBlurID, input);

	DrawFullScreenTriangle();

	if (freeRTV != nullptr && freeSRVOut != nullptr)
	{
		_deviceContext->ClearRenderTargetView(freeRTV, background_colour);
		_deviceContext->OMSetRenderTargets(1, &freeRTV, nullptr);
		output = freeSRVOut;
	}
	else
	{
		_deviceContext->ClearRenderTargetView(_Blur_RTV, background_colour);
		_deviceContext->OMSetRenderTargets(1, &_Blur_RTV, nullptr);
		output = _Blur_SRV;
	}
	ShaderID pixelShaderVBlurID = GetVerticalBlurPixelShader();
	SetupBlurPatameters(pixelShaderVBlurID, _Blur_HorizontalSRV);

	DrawFullScreenTriangle();

	ID3D11ShaderResourceView* nullSRV = nullptr;
	_deviceContext->PSSetShaderResources(0, 1, &nullSRV);
	_deviceContext->PSSetShaderResources(1, 1, &nullSRV);
	_deviceContext->PSSetShaderResources(2, 1, &nullSRV);


}

void Renderer::RenderUnlitParticles(ParticleSystem* particleSystem)
{
	SetBackBufferRenderTarget();
	ResetViewport();
	int numberOfEmitters = particleSystem->GetNumberOfEmitters();
	
	for (int i = 0; i < numberOfEmitters; ++i)
	{
		ParticleEmitter& emitter = particleSystem->GetEmitter(i);

		emitter.PreDraw(this);
		emitter.Draw(this);
		emitter.PostDraw(this);
	}
	EnableDepthStencil();

}

ShaderID Renderer::GetNDCVertexShader()
{
	if (vertexShaderNDC != InvalidShaderID)
	{
		return vertexShaderNDC;
	}

	ShaderID NewID = (ShaderID)Renderer::VertexShaders.size();

	VertexShader shader("Shaders/NDCVertexShader.hlsl", "vs_main");
	shader.CompileShader();
	shader.CreateVertexShader(_device);

	// layout
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
	  { "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	  { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	  /*
	  { "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	  { "NOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	  */
	};
	shader.CreateInputLayout(_device, inputElementDesc, 2);
	
	Renderer::VertexShaders.push_back(shader);
	vertexShaderNDC = NewID;
	return NewID;
}


ShaderID Renderer::GetFullScreenTriangleVertexShader()
{
	if (vertexShaderFSTriangle != InvalidShaderID)
	{
		return vertexShaderFSTriangle;
	}

	ShaderID NewID = (ShaderID)Renderer::VertexShaders.size();

	VertexShader shader("Shaders/FSTriangleVertexShader.hlsl", "vs_main");
	shader.CompileShader();
	shader.CreateVertexShader(_device);

	//// layout
	//D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
	//  { "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//  { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//  /*
	//  { "COL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//  { "NOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//  */
	//};
	//shader.CreateInputLayout(_device, inputElementDesc, 2);

	Renderer::VertexShaders.push_back(shader);
	vertexShaderFSTriangle = NewID;
	return NewID;

}

ShaderID Renderer::GetSimpleColorPixelShader()
{
	if (pixelShaderSimpleColor != InvalidShaderID)
	{
		return pixelShaderSimpleColor;
	}

	ShaderID NewID = (ShaderID)Renderer::PixelShaders.size();

	PixelShader shader("Shaders/SimpleColorPixelShader.hlsl", "ps_main");
	shader.CompileShader();
	shader.CreatePixelShader(_device);

	Renderer::PixelShaders.push_back(shader);
	pixelShaderSimpleColor = NewID;
	return NewID;
}

ShaderID Renderer::GetBasicLightPixelShader()
{
	if (_pixelShaderLighting != InvalidShaderID)
	{
		return _pixelShaderLighting;
	}

	ShaderID NewID = (ShaderID)Renderer::PixelShaders.size();

	PixelShader shader("Shaders/Lighting.hlsl", "ps_main");
	shader.CompileShader();
	shader.CreatePixelShader(_device);

	Renderer::PixelShaders.push_back(shader);
	_pixelShaderLighting = NewID;
	return NewID;


}

ShaderID Renderer::GetDefaultModelVertexShader()
{
	if (_vertexShaderDefaultModel != InvalidShaderID)
	{
		return _vertexShaderDefaultModel;
	}

	ShaderID NewID = (ShaderID)Renderer::VertexShaders.size();

	VertexShader shader("Shaders/ShadersDeferred.hlsl", "vs_main");
	shader.CompileShader();
	shader.CreateVertexShader(_device);

	// layout
	D3D11_INPUT_ELEMENT_DESC inputElementDescDeferred[] = {
	  { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	  { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

	shader.CreateInputLayout(_device, inputElementDescDeferred, 2);

	Renderer::VertexShaders.push_back(shader);
	_vertexShaderDefaultModel = NewID;
	return NewID;
}

ShaderID Renderer::GetDefaultDeferredPixelShader()
{
	if (_pixelShaderDefaultDeferred != InvalidShaderID)
	{
		return _pixelShaderDefaultDeferred;
	}

	ShaderID NewID = (ShaderID)Renderer::PixelShaders.size();

	PixelShader shader("Shaders/ShadersDeferred.hlsl", "ps_main");
	shader.CompileShader();
	shader.CreatePixelShader(_device);

	Renderer::PixelShaders.push_back(shader);
	_pixelShaderDefaultDeferred = NewID;
	return NewID;
}

ShaderID Renderer::GetSSAOPixelShader()
{
	if (_pixelShaderSSAO != InvalidShaderID)
	{
		return _pixelShaderSSAO;
	}

	ShaderID NewID = (ShaderID)Renderer::PixelShaders.size();

	PixelShader shader("Shaders/SSAOPixelShader.hlsl", "ps_main");
	shader.CompileShader();
	shader.CreatePixelShader(_device);

	Renderer::PixelShaders.push_back(shader);
	_pixelShaderSSAO = NewID;
	return NewID;

}

ShaderID Renderer::GetSimpleBlurPixelShader()
{
	if (_pixelShaderSimpleBlur != InvalidShaderID)
	{
		return _pixelShaderSimpleBlur;
	}

	ShaderID NewID = (ShaderID)Renderer::PixelShaders.size();

	PixelShader shader("Shaders/SimpleBlur.hlsl", "ps_main");
	shader.CompileShader();
	shader.CreatePixelShader(_device);

	Renderer::PixelShaders.push_back(shader);
	_pixelShaderSimpleBlur = NewID;
	return NewID;
}

ShaderID Renderer::GetHorizontalBlurPixelShader()
{
	if (_pixelShaderHorizontalBlur != InvalidShaderID)
	{
		return _pixelShaderHorizontalBlur;
	}

	ShaderID NewID = (ShaderID)Renderer::PixelShaders.size();

	PixelShader shader("Shaders/HorizontalBlur.hlsl", "ps_main");
	shader.CompileShader();
	shader.CreatePixelShader(_device);

	Renderer::PixelShaders.push_back(shader);
	_pixelShaderHorizontalBlur = NewID;
	return NewID;
}

ShaderID Renderer::GetVerticalBlurPixelShader()
{
	if (_pixelShaderVerticalBlur != InvalidShaderID)
	{
		return _pixelShaderVerticalBlur;
	}

	ShaderID NewID = (ShaderID)Renderer::PixelShaders.size();

	PixelShader shader("Shaders/VerticalBlur.hlsl", "ps_main");
	shader.CompileShader();
	shader.CreatePixelShader(_device);

	Renderer::PixelShaders.push_back(shader);
	_pixelShaderVerticalBlur = NewID;
	return NewID;
}

ShaderID Renderer::AddNewPixelShader(const std::string& filename, const std::string& entryPoint)
{
	ShaderID NewID = (ShaderID)Renderer::PixelShaders.size();

	PixelShader shader(filename, entryPoint);
	shader.CompileShader();
	shader.CreatePixelShader(_device);

	Renderer::PixelShaders.push_back(shader);
	return NewID;
}

ShaderID Renderer::AddNewVertexShader(const std::string& filename, const std::string& entryPoint)
{
	ShaderID NewID = (ShaderID)Renderer::VertexShaders.size();

	VertexShader shader(filename, entryPoint);
	shader.CompileShader();
	shader.CreateVertexShader(_device);

	Renderer::VertexShaders.push_back(shader);
	return NewID;

}

ShaderID Renderer::AddNewGeometryShader(const std::string& filename, const std::string& entryPoint)
{
	ShaderID NewID = (ShaderID)Renderer::GeometryShaders.size();

	GeometryShader shader(filename, entryPoint);
	shader.CompileShader();
	shader.CreateGeometryShader(_device);

	Renderer::GeometryShaders.push_back(shader);
	return NewID;

}

TextureID Renderer::AddNewTexture(const std::string& filename)
{
	TextureID NewID = (TextureID)Renderer::Textures.size();

	Texture texture;
	texture.Init(_device, filename);

	Renderer::Textures.push_back(texture);
	return NewID;
}

ID3D11ShaderResourceView* Renderer::CreateShaderResourceViewFromTexture(Texture* texture)
{
	return texture->CreateShaderResourceView(_device);
}

void Renderer::SetupLightingParameters(ShaderID PixelShaderID, const glm::vec3& lightDirection)
{
	PixelShader* pixelShader = GetPixelShaderByID(PixelShaderID);
	SetPixelShader(pixelShader);

	// GBuffer color
	_deviceContext->PSSetShaderResources(0, 1, &_GBufferShaderResourceViewArray[(unsigned int)GBufferType::Color]);
	// GBuffer normal
	_deviceContext->PSSetShaderResources(1, 1, &_GBufferShaderResourceViewArray[(unsigned int)GBufferType::Normal]);
	// GBuffer position
	_deviceContext->PSSetShaderResources(2, 1, &_GBufferShaderResourceViewArray[(unsigned int)GBufferType::Position]);
	// SSAO
	_deviceContext->PSSetShaderResources(3, 1, &_SSAO_BlurredSRV);

	_deviceContext->PSSetSamplers(0, 1, &_samplerStateClamp);

	UpdateLightBuffer(lightDirection);
	BindLightBuffer();
	BindPixelViewBuffer();
}

void Renderer::SetupSSAOPatameters(ShaderID PixelShaderID)
{
	PixelShader* pixelShader = GetPixelShaderByID(PixelShaderID);
	SetPixelShader(pixelShader);


	_deviceContext->PSSetShaderResources(0, 1, &_GBufferShaderResourceViewArray[(unsigned int)GBufferType::Normal]);
	_deviceContext->PSSetShaderResources(1, 1, &_GBufferShaderResourceViewArray[(unsigned int)GBufferType::Position]);
	_deviceContext->PSSetShaderResources(2, 1, &_SSAONoiseSRV);

	
	_deviceContext->PSSetSamplers(0, 1, &_samplerStateClamp);
	_deviceContext->PSSetSamplers(1, 1, &_samplerStateWrap);

	BindPixelViewBuffer();
	BindSSAOKernelBuffer();
}

void Renderer::SetupBlurPatameters(ShaderID PixelShaderID, ID3D11ShaderResourceView* input)
{
	PixelShader* pixelShader = GetPixelShaderByID(PixelShaderID);
	SetPixelShader(pixelShader);

	_deviceContext->PSSetShaderResources(0, 1, &input);
	_deviceContext->PSSetShaderResources(1, 1, &_depthStencilSRV);
	_deviceContext->PSSetShaderResources(2, 1, &_GBufferShaderResourceViewArray[(int)GBufferType::Position]);


	_deviceContext->PSSetSamplers(0, 1, &_samplerStateClamp);

}

VertexShader* Renderer::GetVertexShaderByID(ShaderID ID)
{
	//@todo invalid shader ID
	//if (ID >= Renderer::VertexShaders.size() || ID == Shader::InvalidShaderID)
	if (ID >= Renderer::VertexShaders.size())
	{
		assert(false);
		return nullptr;
	}
	return &Renderer::VertexShaders[ID];

}

PixelShader* Renderer::GetPixelShaderByID(ShaderID ID)
{
	//@todo invalid shader ID
//if (ID >= Renderer::VertexShaders.size() || ID == Shader::InvalidShaderID)
	if (ID >= Renderer::PixelShaders.size())
	{
		assert(false);
		return nullptr;
	}
	return &Renderer::PixelShaders[ID];
}

GeometryShader* Renderer::GetGeometryShaderByID(ShaderID ID)
{
	if (ID >= Renderer::GeometryShaders.size())
	{
		assert(false);
		return nullptr;
	}
	return &Renderer::GeometryShaders[ID];
}

Texture* Renderer::GetTextureByID(TextureID ID)
{
	if (ID >= Renderer::Textures.size())
	{
		assert(false);
		return nullptr;
	}
	return &Renderer::Textures[ID];

}
