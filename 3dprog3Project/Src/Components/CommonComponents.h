#pragma once

#include "rfEntity.hpp"

#include "RimfrostMath.hpp"

struct TransformComp : public rfe::Component<TransformComp>
{
	TransformComp() = default;
	TransformComp(rfm::Vector3 pos) { transform.setTranslation(pos); }
	rfm::Transform transform;
};

struct MeshComp : public rfe::Component<MeshComp>
{
	MeshComp(uint64_t id = 0) : meshID(id) {}
	uint64_t meshID;
};

struct MaterialComp : public rfe::Component<MaterialComp>
{
	MaterialComp(uint64_t id = 0) : materialID(id) {}
	uint64_t materialID;
};