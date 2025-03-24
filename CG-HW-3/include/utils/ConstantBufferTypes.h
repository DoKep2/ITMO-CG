#pragma once

#include <DirectXMath.h>

struct CB_VS_vertexshader
{
	float xOffset = 0;
	float yOffset = 0;
	DirectX::XMMATRIX mat;
};