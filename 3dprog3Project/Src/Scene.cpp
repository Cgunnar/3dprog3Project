#include "pch.h"
#include "Scene.h"
#include "CommonComponents.h"

using namespace rfe;
using namespace DirectX;

Scene::Scene()
{
	Entity& newEntity = m_entities.emplace_back(EntityReg::CreateEntity());
	auto transform = newEntity.AddComponent<TransformComp>();
	XMStoreFloat4x4(&transform->matrix, DirectX::XMMatrixIdentity());
	auto mesh = newEntity.AddComponent<MeshComp>();
	mesh->meshID = 0;
	auto material = newEntity.AddComponent<MaterialComp>();
	material->materialID = 0;
}

Scene::~Scene()
{

}
