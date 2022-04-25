#include "pch.h"
#include "Scene.h"
#include "CommonComponents.h"
#include "AssetManager.h"

using namespace rfe;

Scene::Scene()
{
	Geometry::Sphere_POS_NOR_UV sphere = Geometry::Sphere_POS_NOR_UV(32, 0.5f);
	Geometry::AABB_POS_NOR_UV box = Geometry::AABB_POS_NOR_UV({ -1, -1, -1 }, { 1, 1, 1 });
	Mesh newSphereMesh = Mesh(reinterpret_cast<const float*>(sphere.VertexData().data()), sphere.ArraySize(), sphere.IndexData(), MeshType::POS_NOR_UV);
	Mesh newBoxMesh = Mesh(reinterpret_cast<const float*>(box.VertexData().data()), box.ArraySize(), box.IndexData(), MeshType::POS_NOR_UV);

	m_sphereMesh = AssetManager::Get().AddMesh(newSphereMesh);
	AssetManager::Get().MoveMeshToGPU(m_sphereMesh);

	m_boxMesh = AssetManager::Get().AddMesh(newBoxMesh);
	AssetManager::Get().MoveMeshToGPU(m_boxMesh);

	Material matRed;
	matRed.albedo = { 1, 0, 0, 1 };
	Material matGreen;
	matGreen.albedo = { 0, 1, 0, 1 };
	Material matBlue;
	matBlue.albedo = { 0, 0, 1, 1 };
	
	m_redMaterial = AssetManager::Get().AddMaterial(matRed);
	AssetManager::Get().MoveMaterialToGPU(m_redMaterial);
	m_greenMaterial = AssetManager::Get().AddMaterial(matGreen);
	AssetManager::Get().MoveMaterialToGPU(m_greenMaterial);
	m_blueMaterial = AssetManager::Get().AddMaterial(matBlue);
	AssetManager::Get().MoveMaterialToGPU(m_blueMaterial);

	Entity newEntity = m_entities.emplace_back(EntityReg::CreateEntity());
	newEntity.AddComponent<TransformComp>()->transform.translateW({ 0.5, 0, 1 });
	newEntity.AddComponent<MeshComp>()->meshID = m_sphereMesh;
	newEntity.AddComponent<MaterialComp>()->materialID = m_greenMaterial;


	newEntity = m_entities.emplace_back(EntityReg::CreateEntity());
	auto& transform = newEntity.AddComponent<TransformComp>()->transform;
	transform.setTranslation({ 0, 0.4f, 0.6f });
	transform.setRotationDeg(30, 20, 0);
	transform.setScale(0.4f);
	newEntity.AddComponent<MeshComp>()->meshID = m_boxMesh;
	newEntity.AddComponent<MaterialComp>()->materialID = m_redMaterial;


	m_camera = EntityReg::CreateEntity();
	m_camera.AddComponent<TransformComp>()->transform.setTranslation(0, 0, -2);
	m_camera.AddComponent<CameraComp>();
}

Scene::~Scene()
{

}

void Scene::Update(float dt)
{

}
