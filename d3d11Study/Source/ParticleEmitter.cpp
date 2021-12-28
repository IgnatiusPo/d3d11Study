#include "ParticleEmitter.h"
#include <random>
#include "Renderer.h"


ParticleEmitter::ParticleEmitter(int particlesCount, EmitterType type) : _particlesCount(particlesCount), _GPUDataBuffer(nullptr), _SRVGPUData(nullptr), _type(type), _PerParticleBuffer(nullptr), _blendState(nullptr)
{
	_PerParticleData._origin = glm::vec3(0.f, 0.f, 0.f);
}

void ParticleEmitter::Init(ID3D11Device* device, glm::vec3 origin)
{
	_PerParticleData._origin = origin;
	std::uniform_real_distribution<float> randomFloats(-10.f, 10.f); //  [-10, 10]
	std::default_random_engine randomGenerator;
	for (int i = 0; i < _particlesCount; ++i)
	{
		GPUParticleData data;

		glm::vec3 position(
			randomFloats(randomGenerator),
			randomFloats(randomGenerator),
			randomFloats(randomGenerator));

		data._position = position;
		data._velocity = glm::vec3(0.f, 0.0f, 0.f); // no velocity for now
		_GPUData.push_back(data);
	}
	CreateGPUDataBuffer( device); // shall i do that here? new srv for every new particle? seems reasonable, need to investigate
	CreateSRV(device);
	CreatePerParticleBuffer(device);

	D3D11_BLEND_DESC blendDesc;
	if (_type & EmitterType::Points)
	{
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		for (int i = 0; i < 8; ++i)
		{
			blendDesc.RenderTarget[i].BlendEnable = true; // no blending
			blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}
		device->CreateBlendState(&blendDesc, &_blendState);
	}
	else if(_type & EmitterType::Quads)
	{
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		for (int i = 0; i < 8; ++i)
		{
			blendDesc.RenderTarget[i].BlendEnable = true; // no blending
			//blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
			//blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

			blendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
			blendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}
		device->CreateBlendState(&blendDesc, &_blendState);

	}
	else
	{
		assert(false); // not implemented 
	}
}

void ParticleEmitter::CreateGPUDataBuffer(ID3D11Device* device)
{
	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(GPUParticleData) * _particlesCount;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.CPUAccessFlags = 0; // NO Cpu Access!
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(GPUParticleData);

	D3D11_SUBRESOURCE_DATA SubData;
	ZeroMemory(&SubData, sizeof(SubData));
	SubData.pSysMem = _GPUData.data();
	SubData.SysMemPitch = 0; // todo: 0?

	HRESULT hr = device->CreateBuffer(&desc, &SubData, &_GPUDataBuffer);

	assert(SUCCEEDED(hr));


}

void ParticleEmitter::CreatePerParticleBuffer(ID3D11Device* device)
{
	// create constant buffer
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = sizeof(_PerParticleData);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HRESULT hr = device->CreateBuffer(&desc, nullptr, &_PerParticleBuffer);

	assert(SUCCEEDED(hr));
}

void ParticleEmitter::CreateSRV(ID3D11Device* device)
{

	HRESULT hr = device->CreateShaderResourceView(_GPUDataBuffer, nullptr, &_SRVGPUData);
	assert(SUCCEEDED(hr));
}

void ParticleEmitter::PreDraw(Renderer* renderer)
{
	ID3D11DeviceContext* deviceContext = renderer->_deviceContext;
	ShaderID vertexShaderID = GetVertexShader();
	VertexShader* vertexShader = renderer->GetVertexShaderByID(vertexShaderID);
	renderer->SetVertexShader(vertexShader);

	ShaderID pixelShaderID = GetPixelShader();
	PixelShader* pixelShader = renderer->GetPixelShaderByID(pixelShaderID);
	renderer->SetPixelShader(pixelShader);

	if (_type & EmitterType::Quads) 
	{
		ShaderID geometryShaderID = GetGeometryShader();
		GeometryShader* geometryShader = renderer->GetGeometryShaderByID(geometryShaderID);
		renderer->SetGeometryShader(geometryShader);
		renderer->BindGeometryViewBuffer();
	}

	renderer->BindVertexViewBuffer();

	renderer->EnableDepthDisableWrite();

	BindSRVGPUData(deviceContext);
	renderer->SetClampSampler(); // todo: own samplers
	BindTextures(deviceContext);
	
	if (_PerParticleDataDirty)
	{
		UpdatePerParticleBuffer(deviceContext); 
		_PerParticleDataDirty = false;
	}
	BindOriginBuffer(deviceContext);
}

void ParticleEmitter::Draw(Renderer* renderer)
{
	ID3D11DeviceContext* deviceContext = renderer->_deviceContext;
	if (_type & EmitterType::Points)
	{

		D3D_PRIMITIVE_TOPOLOGY currentTopology;
		deviceContext->IAGetPrimitiveTopology(&currentTopology);
		ID3D11BlendState* currentBlendState = nullptr;
		FLOAT BlendFactor[4] = {0.f, 0.f, 0.f, 0.f};
		UINT sampleMask;
		deviceContext->OMGetBlendState(&currentBlendState, BlendFactor, &sampleMask);


		FLOAT NullBlendFactor[4] = {0.f, 0.f, 0.f, 0.f};
		deviceContext->OMSetBlendState(_blendState, NullBlendFactor, 0xffffffff);

		deviceContext->IASetPrimitiveTopology(
			D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		deviceContext->Draw(_particlesCount, 0);

		deviceContext->IASetPrimitiveTopology(currentTopology); // restore topology
		deviceContext->OMSetBlendState(currentBlendState, BlendFactor, sampleMask); // restore blend state

	}
	else if (_type & EmitterType::Quads)
	{
		D3D_PRIMITIVE_TOPOLOGY currentTopology;
		deviceContext->IAGetPrimitiveTopology(&currentTopology);
		ID3D11BlendState* currentBlendState = nullptr;
		FLOAT BlendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
		UINT sampleMask;
		deviceContext->OMGetBlendState(&currentBlendState, BlendFactor, &sampleMask);


		FLOAT NullBlendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
		deviceContext->OMSetBlendState(_blendState, NullBlendFactor, 0xffffffff);

		deviceContext->IASetPrimitiveTopology(
			D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		deviceContext->Draw(_particlesCount, 0);

		deviceContext->IASetPrimitiveTopology(currentTopology); // restore topology
		deviceContext->OMSetBlendState(currentBlendState, BlendFactor, sampleMask); // restore blend state
	}
	else
	{
		assert(false); //not implemented
	}
}

void ParticleEmitter::PostDraw(Renderer* renderer)
{
	ID3D11DeviceContext* deviceContext = renderer->_deviceContext;
	ID3D11ShaderResourceView* nullSRV = nullptr;
	ID3D11Buffer* nullBuffer = nullptr;
	deviceContext->VSSetShaderResources(0, 1, &nullSRV); // unbind
	deviceContext->VSSetConstantBuffers(1, 1, &nullBuffer);

	if (_type & EmitterType::Quads)
	{
		renderer->SetGeometryShader();
	}
}

void ParticleEmitter::UpdateOrigin(glm::vec3 newOrigin)
{
	if (_PerParticleData._origin != newOrigin)
	{
		_PerParticleData._origin = newOrigin;
		_PerParticleDataDirty = true;
	}
}

void ParticleEmitter::UpdatePerParticleBuffer(ID3D11DeviceContext* deviceContext)
{
	deviceContext->UpdateSubresource(_PerParticleBuffer, 0, 0, &_PerParticleData, 0, 0);
}

void ParticleEmitter::BindOriginBuffer(ID3D11DeviceContext* deviceContext)
{
	deviceContext->VSSetConstantBuffers(1, 1, &_PerParticleBuffer);
}

void ParticleEmitter::BindSRVGPUData(ID3D11DeviceContext* deviceContext)
{
	deviceContext->VSSetShaderResources(0, 1, &_SRVGPUData); // should always be with index 0
}

void ParticleEmitter::BindTextures(ID3D11DeviceContext* deviceContext)
{
	for (ShaderTexture& shaderTexture : _texturesVertexShader)
	{
		Texture* texture = Renderer::GetTextureByID(shaderTexture.first);
		ID3D11ShaderResourceView* srv = texture->GetShaderResourceView();
		deviceContext->VSSetShaderResources(shaderTexture.second, 1, &srv); // 
	}

	for (ShaderTexture& shaderTexture : _texturesPixelShader)
	{
		Texture* texture = Renderer::GetTextureByID(shaderTexture.first);
		ID3D11ShaderResourceView* srv = texture->GetShaderResourceView();
		deviceContext->PSSetShaderResources(shaderTexture.second, 1, &srv); // 
	}

}
