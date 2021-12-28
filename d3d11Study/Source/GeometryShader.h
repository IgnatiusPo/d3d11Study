#pragma once
#include "Shader.h"
#include <string>
#include <d3dcompiler.h>
struct ID3D11GeometryShader;
struct ID3D11Device;

class GeometryShader
{
private:
	ID3DBlob* _blob_ptr;
	ID3D11GeometryShader* _d3d11_shader;
	std::wstring _filename;
	std::string _entryPoint;
public:
	GeometryShader(const std::string& filename, const std::string& entryPoint);

	// compile from _filename and _entryPoint, store blob in _blob_ptr
	void CompileShader();

	// create geometry shader and store in _d3d11_shader
	void CreateGeometryShader(ID3D11Device* device);
	ID3D11GeometryShader* GetD3D11GeometryShader() const { return _d3d11_shader; };
	ID3DBlob* GetD3D11Blob() const { return _blob_ptr; };
};

