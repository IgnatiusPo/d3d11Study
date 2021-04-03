#pragma once
#include "Model.h"
#include <vector>
class Scene
{
public:
	std::vector<Model> _sceneObjects;
public:
	Scene();

	void AddObjectToScene(const Model& mesh);
};

