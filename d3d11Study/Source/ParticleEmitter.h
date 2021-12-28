#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "d3d11.h"
#include "Shader.h"
#include "Texture.h"

enum EmitterType
{
	Points = 1,
	Quads = 2
};
class Renderer;
using ShaderTexture = std::pair<TextureID, int>	; //texture to shader location
static const int reservedPixelShaderResourceLocations = 0; // update these when update overall particle shader
static const int reservedVertexShaderResourceLocations = 1;
class ParticleEmitter
{
	ID3D11Buffer* _GPUDataBuffer;
	ID3D11Buffer* _PerParticleBuffer;
	ID3D11ShaderResourceView* _SRVGPUData;
	ID3D11BlendState* _blendState;

	ShaderID _vertexShader = InvalidShaderID;
	ShaderID _pixelShader = InvalidShaderID;
	ShaderID _geometryShader = InvalidShaderID;

	std::vector<ShaderTexture>	_texturesVertexShader; // texture to shader location
	std::vector<ShaderTexture>	_texturesPixelShader;

	int _particlesCount;
	struct PerParticleData
	{
		glm::vec3 _origin;
		char padding[4];
	} _PerParticleData;
	struct GPUParticleData
	{
		glm::vec3 _position;
		glm::vec3 _velocity;
	};
	std::vector<GPUParticleData> _GPUData;

	bool _PerParticleDataDirty = true;
public:
	ParticleEmitter(int particlesCount, EmitterType type);
	EmitterType _type;

	void Init(ID3D11Device* device, glm::vec3 origin);
	void CreateGPUDataBuffer(ID3D11Device* device);
	void CreatePerParticleBuffer(ID3D11Device* device);
	void CreateSRV(ID3D11Device* device);
	//void CreateUAV();

	void PreDraw(Renderer* renderer);
	void Draw(Renderer* renderer);
	void PostDraw(Renderer* renderer);

	ID3D11Buffer* GetGPUDataBuffer() const { return _GPUDataBuffer; }
	ID3D11Buffer* GetOriginBuffer() const { return _PerParticleBuffer; }
	ID3D11ShaderResourceView* GetSRVGPUData() const { return _SRVGPUData; }

	void UpdateOrigin(glm::vec3 newOrigin);
	void UpdatePerParticleBuffer(ID3D11DeviceContext* deviceContext);
	void BindOriginBuffer(ID3D11DeviceContext* deviceContext);
	void BindSRVGPUData(ID3D11DeviceContext* deviceContext);
	void BindTextures(ID3D11DeviceContext* deviceContext);
	
	void UseVertexShader(ShaderID vertexShader)
	{
		_vertexShader = vertexShader;
	}
	void UsePixelShader(ShaderID pixelShader)
	{
		_pixelShader = pixelShader;
	}
	void UseGeometryShader(ShaderID geometryShader)
	{
		_geometryShader = geometryShader;
	}
	void UsePixelShaderTexture(TextureID ID, int shaderLocation)
	{
		if (shaderLocation < reservedPixelShaderResourceLocations)
		{
			assert(false); // reserved shaderLocation
		}
		_texturesPixelShader.push_back(ShaderTexture(ID, shaderLocation));
	}
	void UseVertexShaderTexture(TextureID ID, int shaderLocation)
	{
		if (shaderLocation < reservedVertexShaderResourceLocations)
		{
			assert(false); // reserved shaderLocation
		}
		_texturesVertexShader.push_back(ShaderTexture(ID, shaderLocation));
	}

	ShaderID GetVertexShader()
	{
		return _vertexShader;
	}
	ShaderID GetPixelShader()
	{
		return _pixelShader;
	}
	ShaderID GetGeometryShader()
	{
		return _geometryShader;
	}

	int GetParticlesCount() const
	{
		return _particlesCount;
	}
};
