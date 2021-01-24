#pragma once
#include "d3d11.h"
class VertexBuffer
{
	ID3D11Buffer* _buffer;
	D3D11_BUFFER_DESC _desc;
public:

	VertexBuffer();
	//VertexBuffer(const D3D11_BUFFER_DESC& desc);

	void CreateBuffer(ID3D11Device* device, const D3D11_BUFFER_DESC& desc, const void* vertex_data, unsigned int stride, unsigned int count, unsigned int offset = 0);
	ID3D11Buffer* GetD3DBuffer() const { return _buffer; }

	unsigned int _stride = 0;
	unsigned int _offset = 0;
	unsigned int _count = 0;
};

