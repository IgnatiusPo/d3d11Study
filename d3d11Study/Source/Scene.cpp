#include "Scene.h"

Scene::Scene()
{
}

void Scene::AddObjectToScene(const Model& mesh)
{
	_sceneObjects.push_back(mesh);
}
