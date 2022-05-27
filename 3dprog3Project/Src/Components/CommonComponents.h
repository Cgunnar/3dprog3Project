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

struct ModelComp : public rfe::Component<ModelComp>
{
	ModelComp(uint64_t id = 0) : meshID(id) {}
	uint64_t meshID;
};

struct MaterialComp : public rfe::Component<MaterialComp>
{
	MaterialComp(uint64_t id = 0) : materialID(id) {}
	uint64_t materialID;
};

struct CameraComp : public rfe::Component<CameraComp>
{
	rfm::Matrix projectionMatrix = rfm::PerspectiveProjectionMatrix(rfm::PIDIV4, 16.0f / 9.0f, 0.001f, 1000.0f);
};

struct SpinnComp : public rfe::Component<SpinnComp>
{
	rfm::Vector3 rotSpeed{0, 15, 0};
};

struct PointLight
{
	rfm::Vector3 position;
	float strength = 1.0f;
	rfm::Vector3 color = { 1.0f, 1.0f, 1.0f };
	float constAtt = 1.0f;
	float linAtt = 0.1f;
	float expAtt = 0.01f;
};
struct PointLightComp : public rfe::Component<PointLightComp>
{
	bool lit = true;
	PointLight pointLight;
};