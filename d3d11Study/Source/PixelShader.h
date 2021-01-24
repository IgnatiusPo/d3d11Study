#pragma once
#include <string>
#include <d3dcompiler.h>
struct ID3D11PixelShader;
struct ID3D11Device;
class PixelShader
{
private:
	ID3DBlob* _blob_ptr;
	ID3D11PixelShader* _d3d11_shader;
	std::wstring _filename;
	std::string _entryPoint;
public:
	PixelShader() = delete;
	PixelShader(const std::string& filename, const std::string& entryPoint);

	// compile from _filename and _entryPoint, store blob in _blob_ptr
	void CompileShader();

	// create vertex shader and store in _d3d11_shader
	void CreatePixelShader(ID3D11Device* device);
	ID3D11PixelShader* GetD3D11PixelShader() const { return _d3d11_shader; };
	ID3DBlob* GetD3D11Blob() const { return _blob_ptr; };
};
