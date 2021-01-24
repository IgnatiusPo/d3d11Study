#include "VertexBuffer.h"
#include <assert.h>

VertexBuffer::VertexBuffer() : _buffer(nullptr), _desc{}
{
}

//VertexBuffer::VertexBuffer(const D3D11_BUFFER_DESC& desc) : _buffer(nullptr), _desc(desc)
//{
//}

void VertexBuffer::CreateBuffer(ID3D11Device* device, const D3D11_BUFFER_DESC& desc, const void* vertex_data,unsigned int stride, unsigned int count, unsigned int offset)
{
	_stride = stride;
	_count = count;
	_offset = offset;
	_desc = desc;
	D3D11_SUBRESOURCE_DATA sr_data = { 0 };
	sr_data.pSysMem = vertex_data;

	HRESULT hr = device->CreateBuffer(&_desc, &sr_data, &_buffer);
	assert(SUCCEEDED(hr));
}
