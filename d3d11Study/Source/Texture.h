#pragma once
#include <string>
#include "d3d11.h"
using TextureID = int;
const TextureID InvalidTextureID = -1;


class Texture
{
	ID3D11Texture2D* _d3d11_texture;
	ID3D11ShaderResourceView* _SRV;
	DXGI_FORMAT _format;

	std::string		_path;
	int _width, _height, _nChannels = 0;
public:
	Texture();
	void Init(ID3D11Device* device, const std::string& path);

	ID3D11ShaderResourceView* CreateShaderResourceView(ID3D11Device* device);
	// create RTV
	
	ID3D11ShaderResourceView* GetShaderResourceView() const
	{
		return _SRV;
	}

};

