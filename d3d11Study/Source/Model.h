#pragma once
#include "Mesh.h"
#include <vector>
#include <string>
class Renderer;
class Model
{
public:
	std::vector<Mesh> _meshes;
	glm::mat4 _modelMatrix = glm::mat4(1.f);

	void AddMesh(const Mesh& mesh);

	void SetModelMatrix(const glm::mat4& ModelMatrix);
	//void SetMaterial(const MaterialID& ID);

};




template<ModelType T>
Model LoadModel(std::string path, Renderer* const renderer);
