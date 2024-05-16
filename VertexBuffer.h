#pragma once

class VertexBuffer
{
public:
	VertexBuffer(const void* data, unsigned int size);
	~VertexBuffer();

	void Bind();
	void Unbind();

	unsigned int rendererId;
};