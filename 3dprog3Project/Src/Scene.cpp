#include "pch.h"
#include "Scene.h"
#include "CommonComponents.h"
#include "AssetManager.h"

using namespace rfe;

Scene::Scene()
{
	Geometry::Sphere_POS_NOR_UV sphere = Geometry::Sphere_POS_NOR_UV(32, 0.5f);
	Mesh newSphereMesh = Mesh(reinterpret_cast<const float*>(sphere.VertexData().data()), sphere.ArraySize(), sphere.IndexData(), MeshType::POS_NOR_UV);

	Material newMaterial;
	newMaterial.albedo = { 0, 1, 0, 1 };

	m_sphereMesh = AssetManager::Get().AddMesh(newSphereMesh);
	AssetManager::Get().MoveMeshToGPU(m_sphereMesh);
	m_greenMaterial = AssetManager::Get().AddMaterial(newMaterial);
	AssetManager::Get().MoveMaterialToGPU(m_greenMaterial);

	Entity& newEntity = m_entities.emplace_back(EntityReg::CreateEntity());
	newEntity.AddComponent<TransformComp>();
	newEntity.AddComponent<MeshComp>()->meshID = m_sphereMesh;
	newEntity.AddComponent<MaterialComp>()->materialID = m_greenMaterial;
}

Scene::~Scene()
{

}
