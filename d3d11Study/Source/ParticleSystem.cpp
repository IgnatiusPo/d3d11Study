#include "ParticleSystem.h"

void ParticleSystem::Init(ID3D11Device* device)
{
	_device = device;
}

ParticleEmitter& ParticleSystem::AddEmitter(glm::vec3 origin, EmitterType type, int particleCount)
{
	int newEmitterIdx = _emitters.size();
	_emitters.emplace_back(particleCount, type);
	ParticleEmitter& emitter = _emitters[newEmitterIdx];
	emitter.Init(_device, origin);
	return emitter;
}
