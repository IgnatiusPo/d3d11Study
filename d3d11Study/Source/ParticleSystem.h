#pragma once
#include <vector>
#include "ParticleEmitter.h"

class ParticleSystem
{
	ID3D11Device* _device = nullptr;
	std::vector<ParticleEmitter> _emitters;
public:
	void Init(ID3D11Device* device);
	static const int DefaultParticleCount = 1000000;
	ParticleEmitter& AddEmitter(glm::vec3 origin, EmitterType type,  int particleCount = DefaultParticleCount);
	ParticleEmitter& GetEmitter(int idx)
	{
		return _emitters[idx];
	}
	int GetNumberOfEmitters() const
	{
		return _emitters.size();
	}
	//void AddEmitter(ParticleEmitter&& emitter);
};
