#pragma once

#include "rfEntity.hpp"

#include <DirectXMath.h>

struct TransformComp : public rfe::Component<TransformComp>
{
	DirectX::XMFLOAT4X4 matrix;
};

struct MeshComp : public rfe::Component<MeshComp>
{
	uint64_t meshID;
};

struct MaterialComp : public rfe::Component<MaterialComp>
{
	uint64_t materialID;
};