#pragma once

#include <DirectXMath.h>

struct CB_VS_vertexshader
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
};