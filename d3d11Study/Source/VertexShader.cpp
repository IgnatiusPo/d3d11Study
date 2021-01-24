#include "VertexShader.h"
#include <d3d11.h>

#include <assert.h>


VertexShader::VertexShader(const std::string& filename, const std::string& entryPoint) 
	: _blob_ptr(nullptr), _d3d11_shader(nullptr), _input_layout(nullptr), _filename(filename.begin(), filename.end()), _entryPoint(entryPoint)
{

}

void VertexShader::CompileShader()
{
    UINT flags;
    flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG; // add more debug output
#endif

    ID3DBlob* error_blob = nullptr;

    HRESULT hr = D3DCompileFromFile(
        _filename.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        _entryPoint.c_str(),
        "vs_5_0",
        flags,
        0,
        &_blob_ptr,
        &error_blob);

    if (FAILED(hr)) {
        if (error_blob) {
            OutputDebugStringA((char*)error_blob->GetBufferPointer());
            error_blob->Release();
        }
        if (_blob_ptr) { _blob_ptr->Release(); }
        assert(false);
    }
}

void VertexShader::CreateVertexShader(ID3D11Device* device)
{
    if (_blob_ptr == nullptr)
    {
        // CompileShader failed or wasn't executed
        assert(false);
    }
    device->CreateVertexShader(_blob_ptr->GetBufferPointer(), _blob_ptr->GetBufferSize(), NULL, &_d3d11_shader);
}

void VertexShader::CreateInputLayout( ID3D11Device* device, const D3D11_INPUT_ELEMENT_DESC* desc, unsigned int numElements)
{
    HRESULT hr = device->CreateInputLayout(
        desc,
        numElements,
        _blob_ptr->GetBufferPointer(),
        _blob_ptr->GetBufferSize(),
        &_input_layout);
    assert(SUCCEEDED(hr));

}
