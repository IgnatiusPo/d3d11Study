#pragma once
#include "VertexBuffer.h"
#include <vector>
#include "glm/glm.hpp"
#include <string>
#include "Shader.h"
#include "Texture.h"
enum class GBufferType
{
	Color,
	Normal,
	Position,
	Num
};

struct GLFWwindow;
class VertexBuffer;
class VertexShader;
class PixelShader;
class GeometryShader;
class Scene;
class Camera;
using MaterialID = int32_t;
class ParticleSystem;
class Renderer
{

public:
	void Init(GLFWwindow* window);
	void Init(HWND hwnd);

	void ClearBackbufferRTV();
	void ClearGBufferRTVs();
	void Present();
	void InitViewportFromWindow(GLFWwindow* window);
	void InitViewportFromWindow(HWND hwnd);
	void ResetViewport();
	void SetViewport(D3D11_VIEWPORT* viewport);
	void SetBackBufferRenderTarget();
	void SetRenderTargetsGBuffer();
	void SetSSAORenderTarget();
	void EnableDepthStencil();
	void DisableDepthStencil();
	void EnableDepthDisableWrite();
	void BindBlendState_NoBlend();

	// note: z-value of these would be 1.f, so make sure to disable depth test if you need overlayed fullscreen pass
	void DrawFullScreenQuad();
	void DrawFullScreenTriangle();

	void DrawCube();
	void Draw(unsigned int vertexCount, unsigned int start = 0);
	void Draw(const VertexBuffer* vBuffer, unsigned int start = 0);
	void SetVertexBuffer(const VertexBuffer* vBuffer);
	void SetVertexShader(const VertexShader* shader = nullptr);
	void SetPixelShader(const PixelShader* shader = nullptr);
	void SetGeometryShader(const GeometryShader* shader = nullptr);
	void SetInputLayout(ID3D11InputLayout* layout);
	void SetInputLayoutFromVertexShader(const VertexShader* shader);

	void SetClampSampler();

	struct View
	{
		glm::mat4 _view;
		glm::mat4 _projection;
		glm::vec4 _viewWorldPos;
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
	void UpdateViewBuffer(const glm::mat4& view, const glm::mat4& projection, const glm::vec4& viewWorldPos);
	void BindVertexViewBuffer();
	void BindPixelViewBuffer();
	void BindGeometryViewBuffer();

	void CreatePerModelBuffer();
	void UpdatePerModelBuffer(const glm::mat4& model);
	void BindPerModelBuffer();

	void CreateLightBuffer();
	void UpdateLightBuffer(const glm::vec3& direction);
	void BindLightBuffer();

	void CreateSSAOKernelBuffer();
	void BindSSAOKernelBuffer();




	// passes 
	void GeometryPass(const Scene* scene, const Camera* camera);
	void RenderLight(const glm::vec3& lightDirection);
	void RenderSSAO();
	void BlurPass(struct ID3D11ShaderResourceView* input, struct ID3D11ShaderResourceView*& output, struct ID3D11RenderTargetView* freeRTV = nullptr, struct ID3D11ShaderResourceView* freeSRVOut = nullptr);
	void RenderUnlitParticles(ParticleSystem* particleSystem);
public:
	// shaders
	ShaderID GetNDCVertexShader();
	ShaderID GetSimpleColorPixelShader();
	ShaderID GetFullScreenTriangleVertexShader();
	ShaderID GetBasicLightPixelShader();
	ShaderID GetDefaultModelVertexShader();
	ShaderID GetDefaultDeferredPixelShader();
	ShaderID GetSSAOPixelShader();
	ShaderID GetSimpleBlurPixelShader();
	ShaderID GetHorizontalBlurPixelShader();
	ShaderID GetVerticalBlurPixelShader();
	ShaderID AddNewPixelShader(const std::string& filename, const std::string& entryPoint);
	ShaderID AddNewVertexShader(const std::string& filename, const std::string& entryPoint);
	ShaderID AddNewGeometryShader(const std::string& filename, const std::string& entryPoint);

	TextureID AddNewTexture(const std::string& filename);
	ID3D11ShaderResourceView* CreateShaderResourceViewFromTexture(Texture* texture);

private:
	void SetupLightingParameters(ShaderID PixelShaderID, const glm::vec3& lightDirection);
	void SetupSSAOPatameters(ShaderID PixelShaderID);
	void SetupBlurPatameters(ShaderID PixelShaderID, struct ID3D11ShaderResourceView* input);
public:
	ShaderID vertexShaderNDC = InvalidShaderID;
	ShaderID pixelShaderSimpleColor = InvalidShaderID;
	ShaderID vertexShaderFSTriangle = InvalidShaderID;
	ShaderID _pixelShaderLighting = InvalidShaderID;
	ShaderID _vertexShaderDefaultModel = InvalidShaderID;
	ShaderID _pixelShaderDefaultDeferred = InvalidShaderID;
	ShaderID _pixelShaderSSAO = InvalidShaderID;
	ShaderID _pixelShaderSimpleBlur = InvalidShaderID;
	ShaderID _pixelShaderHorizontalBlur = InvalidShaderID;
	ShaderID _pixelShaderVerticalBlur = InvalidShaderID;


	static VertexShader*	 GetVertexShaderByID(ShaderID ID);
	static PixelShader*	 GetPixelShaderByID(ShaderID ID);
	static GeometryShader*	 GetGeometryShaderByID(ShaderID ID);

	static Texture*		 GetTextureByID(TextureID ID);


	struct ID3D11Device* _device;
	struct ID3D11DeviceContext* _deviceContext;
	struct IDXGISwapChain* _swapchain;
	struct ID3D11RenderTargetView* _backBufferRTV;

	struct ID3D11DepthStencilState* _depthEnabledStencilState;
	struct ID3D11DepthStencilState* _depthDisabledStencilState;
	struct ID3D11DepthStencilState* _depthEnabledWriteDisabled;
	struct ID3D11DepthStencilView* _depthStencilView;
	struct ID3D11ShaderResourceView* _depthStencilSRV;
	struct ID3D11Texture2D* _depthStencilBuffer;


	struct ID3D11RasterizerState* _rasterizerState;

	struct ID3D11BlendState* _blendStateNoBlending;

	struct ID3D11Texture2D*				_GBufferTextureArray[(int)GBufferType::Num];
	struct ID3D11RenderTargetView*		_GBufferRenderTargetViewArray[(int)GBufferType::Num];
	struct ID3D11ShaderResourceView*	_GBufferShaderResourceViewArray[(int)GBufferType::Num];

	struct ID3D11Texture2D* _SSAOTexture;
	struct ID3D11RenderTargetView* _SSAO_RTV;
	struct ID3D11ShaderResourceView* _SSAO_SRV;

	struct ID3D11Texture2D* _SSAONoiseTexture;
	struct ID3D11ShaderResourceView* _SSAONoiseSRV;


	struct ID3D11ShaderResourceView* _SSAO_BlurredSRV;


	struct ID3D11Texture2D* _BlurHorizontalTexture;
	struct ID3D11RenderTargetView* _Blur_HorizontalRTV;
	struct ID3D11ShaderResourceView* _Blur_HorizontalSRV;

	struct ID3D11Texture2D* _BlurTexture;
	struct ID3D11RenderTargetView* _Blur_RTV;
	struct ID3D11ShaderResourceView* _Blur_SRV;



	struct ID3D11SamplerState*			_samplerStateClamp;
	struct ID3D11SamplerState*			_samplerStateWrap;
	struct D3D11_VIEWPORT				_viewport;

	// constant buffers
	//@todo create CBufferStruct to handle shit
	View _view;
	ID3D11Buffer* _viewBuffer;

	PerModelData _perModelData;
	ID3D11Buffer* _perModelBuffer;

	LightData _lightData;
	ID3D11Buffer* _lightBuffer;

	std::vector<glm::vec4>	_ssaoKernel;
	ID3D11Buffer* _ssaoKernelBuffer;

	
	// static variables
	static VertexBuffer quad_vBuffer;

	static const float quadPosTex[24];
	static const float cube_positions[108];

	static float background_colour[4];

	static std::vector<VertexShader>		VertexShaders;
	static std::vector<PixelShader>			PixelShaders;
	static std::vector<GeometryShader>		GeometryShaders;

	static std::vector<Texture>				Textures;
};
