#pragma once
#include "glm/glm.hpp"
#include "VertexBuffer.h"

enum class ModelType
{
	STL,
	OBJ,
};

template<ModelType T>
struct Vertex
{
	glm::vec3 _position;
};

template<>
struct Vertex<ModelType::STL>
{
	glm::vec3 _position;
	glm::vec3 _normal;
	Vertex(glm::vec3 position, glm::vec3 normal) : _position(position), _normal(normal) {};
	Vertex() = default;
};

template<>
struct Vertex<ModelType::OBJ>
{
	glm::vec3 _position;
	glm::vec3 _normal;
	glm::vec2 _texCoord;
};

class Mesh
{
	VertexBuffer _vertexBuffer;
	ModelType _modelType;

	//MaterialID _material = Material::InvalidMaterialID;


public:
	Mesh() = default;
	Mesh(ModelType modelType, VertexBuffer&& vBuffer) : _modelType(modelType), _vertexBuffer(vBuffer) { } ;
	const VertexBuffer* GetVertexBuffer() const
	{
		return &_vertexBuffer;
	}



	glm::mat4 _modelMatrix = glm::mat4(1.f);

};
