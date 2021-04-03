#pragma once
#include "VertexBuffer.h"
#include <vector>
#include "glm/glm.hpp"
#define GBUFFER_COUNT 2

struct GLFWwindow;
class VertexBuffer;
class VertexShader;
class PixelShader;
class Scene;
class Camera;
using ShaderID = int32_t;
using MaterialID = int32_t;
const ShaderID InvalidShaderID = -1;
class Renderer
{

public:
	void Init(GLFWwindow* window);

	void ClearRTV();
	void ClearGBufferRTVs();
	void Present();
	void InitViewportFromWindow(GLFWwindow* window);
	void ResetViewport();
	void SetViewport(D3D11_VIEWPORT* viewport);
	void SetBackBufferRenderTarget();
	void SetRenderTargetsGBuffer();
	void EnableDepthStencil();
	void DisableDepthStencil();

	void DrawQuadFS();
	void DrawCube();
	void Draw(unsigned int vertexCount, unsigned int start = 0);
	void Draw(const VertexBuffer* vBuffer, unsigned int start = 0);
	void SetVertexBuffer(const VertexBuffer* vBuffer);
	void SetVertexShader(const VertexShader* shader);
	void SetPixelShader(const PixelShader* shader);
	void SetInputLayoutFromVertexShader(const VertexShader* shader);

	struct View
	{
		glm::mat4 _view;
		glm::mat4 _projection;
	};
	struct PerModelData
	{
		glm::mat4 _model;
	};
	struct LightData
	{
		glm::vec3 _direction;
		char padding[4];
	};
	void CreateViewBuffer();
	void UpdateViewBuffer(const glm::mat4& view, const glm::mat4 projection);
	void BindVertexViewBuffer();

	void CreatePerModelBuffer();
	void UpdatePerModelBuffer(const glm::mat4& model);
	void BindPerModelBuffer();

	void CreateLightBuffer();
	void UpdateLightBuffer(const glm::vec3& direction);
	void BindLightBuffer();



	// passes 
	void GeometryPass(const Scene* scene, const Camera* camera);
	void RenderLight();
private:
	// shaders
	// create simple vertex shader that takes coordinated in NDC
	ShaderID GetNDCVertexShader();
	ShaderID GetSimpleColorPixelShader();
	ShaderID GetBasicLightPixelShader();
	ShaderID GetDefaultModelVertexShader();
	ShaderID GetDefaultDeferredPixelShader();

	void SetupLightingParameters(ShaderID PixelShaderID);
public:
	ShaderID vertexShaderNDC = InvalidShaderID;
	ShaderID pixelShaderSimpleColor = InvalidShaderID;
	ShaderID _pixelShaderLighting = InvalidShaderID;
	ShaderID _vertexShaderDefaultModel = InvalidShaderID;
	ShaderID _pixelShaderDefaultDeferred = InvalidShaderID;

	VertexShader* GetVertexShaderByID(ShaderID ID);
	PixelShader*	 GetPixelShaderByID(ShaderID ID);


	struct ID3D11Device* _device;
	struct ID3D11DeviceContext* _deviceContext;
	struct IDXGISwapChain* _swapchain;
	struct ID3D11RenderTargetView* _backBufferRTV;

	struct ID3D11DepthStencilState* _depthEnabledStencilState;
	struct ID3D11DepthStencilState* _depthDisabledStencilState;
	struct ID3D11DepthStencilView* _depthStencilView;
	struct ID3D11Texture2D* _depthStencilBuffer;


	struct ID3D11RasterizerState* _rasterizerState;

	struct ID3D11Texture2D*				_GBufferTextureArray[GBUFFER_COUNT];
	struct ID3D11RenderTargetView*		_GBufferRenderTargetViewArray[GBUFFER_COUNT];
	struct ID3D11ShaderResourceView*	_GBufferShaderResourceViewArray[GBUFFER_COUNT];

	struct ID3D11SamplerState*			_samplerStateClamp;
	struct D3D11_VIEWPORT				_viewport;

	// constant buffers
	//@todo create CBufferStruct to handle shit
	View _view;
	ID3D11Buffer* _viewBuffer;

	PerModelData _perModelData;
	ID3D11Buffer* _perModelBuffer;

	LightData _lightData;
	ID3D11Buffer* _lightBuffer;

	
	// static variables
	static VertexBuffer quad_vBuffer;

	static const float quadPosTex[24];
	static const float cube_positions[108];

	static float background_colour[4];

	static std::vector<VertexShader>	VertexShaders;
	static std::vector<PixelShader>		PixelShaders;
};
