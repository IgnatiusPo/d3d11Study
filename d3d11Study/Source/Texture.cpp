#include "Texture.h"
#include "stb_image.h"
#include <assert.h>

Texture::Texture() : _d3d11_texture(nullptr), _SRV(nullptr), _format(DXGI_FORMAT_R8G8B8A8_UNORM), _height(0), _width(0)
{

}

void Texture::Init(ID3D11Device* device, const std::string& path)
{
	//stbi_set_flip_vertically_on_load(1);
	unsigned char* localBuffer = stbi_load(path.c_str(), &_width, &_height, &_nChannels, STBI_rgb_alpha);
	if (!localBuffer)
	{
		assert(false);//
	}
	//todo format
	
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = _width;
	textureDesc.Height = _height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = _format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	//textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = localBuffer;
	data.SysMemPitch = _width * 4;
	//data.SysMemPitch = 0;


	HRESULT hr = device->CreateTexture2D(&textureDesc, &data, &_d3d11_texture);

	assert(SUCCEEDED(hr));
}

ID3D11ShaderResourceView* Texture::CreateShaderResourceView(ID3D11Device* device)
{
	if (_SRV)
	{
		assert(false); // create was called twice! 
		return _SRV;
	}
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	srvDesc.Format = _format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	HRESULT hr = device->CreateShaderResourceView(_d3d11_texture, &srvDesc, &_SRV);

	assert(SUCCEEDED(hr));

	return _SRV;
}
