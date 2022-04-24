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
	newMaterial.albedo = { 1, 0, 0, 1 };
	m_redMaterial = AssetManager::Get().AddMaterial(newMaterial);
	AssetManager::Get().MoveMaterialToGPU(m_redMaterial);

	Entity newEntity = m_entities.emplace_back(EntityReg::CreateEntity());
	newEntity.AddComponent<TransformComp>()->transform.translateW({ 0.5, 0, 1 });
	newEntity.AddComponent<MeshComp>()->meshID = m_sphereMesh;
	newEntity.AddComponent<MaterialComp>()->materialID = m_greenMaterial;


	newEntity = m_entities.emplace_back(EntityReg::CreateEntity());
	auto& transform = newEntity.AddComponent<TransformComp>()->transform;
	transform.setTranslation({ 0, 0.4f, 1 });
	transform.setScale(0.5f);
	newEntity.AddComponent<MeshComp>()->meshID = m_sphereMesh;
	newEntity.AddComponent<MaterialComp>()->materialID = m_redMaterial;
}

Scene::~Scene()
{

}
