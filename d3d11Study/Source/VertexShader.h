#pragma once
#include <string>
#include <d3dcompiler.h>
struct ID3D11VertexShader;
struct ID3D11Device;
struct ID3D11InputLayout;
struct D3D11_INPUT_ELEMENT_DESC;
class VertexShader
{
private:
	ID3DBlob* _blob_ptr;
	ID3D11VertexShader* _d3d11_shader;
	std::wstring _filename;
	std::string _entryPoint;
	ID3D11InputLayout* _input_layout;
public:
	VertexShader() = delete;
	VertexShader(const std::string& filename, const std::string& entryPoint);

	// compile from _filename and _entryPoint, store blob in _blob_ptr
	void CompileShader();

	// create vertex shader and store in _d3d11_shader
	void CreateVertexShader( ID3D11Device* device);
	void CreateInputLayout( ID3D11Device* device, const D3D11_INPUT_ELEMENT_DESC* desc, unsigned int numElements);
	ID3D11VertexShader* GetD3D11VertexShader() const { return _d3d11_shader; };
	ID3DBlob* GetD3D11Blob() const { return _blob_ptr; };
	ID3D11InputLayout* GetInputLayout() const { return _input_layout; };
};

