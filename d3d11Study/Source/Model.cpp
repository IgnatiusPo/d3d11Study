#define _CRT_SECURE_NO_WARNINGS 1
#include "Model.h"
#include "Renderer.h"
#define  HEADER_BUFF_SIZE 80


void Model::AddMesh(const Mesh& mesh)
{
	_meshes.push_back(mesh);
}

void Model::SetModelMatrix(const glm::mat4& ModelMatrix)
{
	_modelMatrix = ModelMatrix;
	for (auto& mesh : _meshes)
	{
		mesh._modelMatrix = ModelMatrix;
	}
}

//void Model::SetMaterial(const MaterialID& ID)
//{
//}

template<>
Model LoadModel<ModelType::STL>(std::string path, Renderer* const renderer )
{
	FILE* file = fopen(path.c_str(), "rb");
	char  aBuff[HEADER_BUFF_SIZE];
	fread(aBuff, sizeof(char), HEADER_BUFF_SIZE, file);
	uint32_t NumberOfTriangles = 0;
	fread(reinterpret_cast<void*>(&NumberOfTriangles), sizeof(NumberOfTriangles), 1, file);

	uint32_t offset = 0;

	char* PositionAndNormals = new char[NumberOfTriangles * sizeof(Vertex<ModelType::STL>) * 3];

	std::vector<Vertex<ModelType::STL>> vc;
	vc.reserve(NumberOfTriangles * 3);

	for (uint32_t i = 0; i < NumberOfTriangles; ++i)
	{
		//todo maybe store these normals differently?
		float normalBuff[3];
		fread(&normalBuff, sizeof(float) * 3, 1, file);
		fread(PositionAndNormals + offset, sizeof(float) * 3, 1, file);
		offset += sizeof(float) * 3;
		memcpy(PositionAndNormals + offset, normalBuff, sizeof(float) * 3);
		offset += sizeof(float) * 3;

		fread(PositionAndNormals + offset, sizeof(float) * 3, 1, file);
		offset += sizeof(float) * 3;
		memcpy(PositionAndNormals + offset, normalBuff, sizeof(float) * 3);
		offset += sizeof(float) * 3;

		fread(PositionAndNormals + offset, sizeof(float) * 3, 1, file);
		offset += sizeof(float) * 3;
		memcpy(PositionAndNormals + offset, normalBuff, sizeof(float) * 3);
		offset += sizeof(float) * 3;

		fseek(file, sizeof(uint16_t), SEEK_CUR);
	}

	//VertexArray vArray;
	//vArray.Init();
	//VertexBufferLayout vLayout;
	//vLayout.Push<float>(3); //position
	//vLayout.Push<float>(3); //normal
	VertexBuffer vBuffer;
	//vBuffer.Init(PositionAndNormals, NumberOfTriangles * sizeof(Vertex<ModelType::STL>) * 3, NumberOfTriangles * 3);
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = NumberOfTriangles * sizeof(Vertex<ModelType::STL>) * 3;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	unsigned int stride = 6 * sizeof(float);
	unsigned int count = NumberOfTriangles * 3;

	vBuffer.CreateBuffer(renderer->_device, desc, PositionAndNormals, stride, count);
	//todo delete PositionAndNormals?

	Mesh mesh(ModelType::STL, std::move(vBuffer));
	Model model;
	model.AddMesh(mesh);
	return model;
};

