#include "GeometryShader.h"
#include <d3d11.h>

#include <assert.h>
GeometryShader::GeometryShader(const std::string& filename, const std::string& entryPoint) : 
	_blob_ptr(nullptr), _d3d11_shader(nullptr), _filename(filename.begin(), filename.end()), _entryPoint(entryPoint)
{

}

void GeometryShader::CompileShader()
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
        "gs_5_0",
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

void GeometryShader::CreateGeometryShader(ID3D11Device* device)
{
    if (_blob_ptr == nullptr)
    {
        // CompileShader failed or wasn't executed
        assert(false);
    }
    device->CreateGeometryShader(_blob_ptr->GetBufferPointer(), _blob_ptr->GetBufferSize(), NULL, &_d3d11_shader);
}
