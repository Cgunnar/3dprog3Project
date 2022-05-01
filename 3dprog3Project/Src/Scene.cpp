#include "pch.h"
#include "Scene.h"
#include "CommonComponents.h"
#include "AssetManager.h"
#include "Window.h"
#include "CameraControllerScript.h"

using namespace rfe;

Scene::Scene()
{
	Geometry::Sphere_POS_NOR_UV sphere = Geometry::Sphere_POS_NOR_UV(32, 0.5f);
	Geometry::AABB_POS_NOR_UV box = Geometry::AABB_POS_NOR_UV({ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f });
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

	/*Entity newEntity = m_entities.emplace_back(EntityReg::CreateEntity());
	newEntity.AddComponent<TransformComp>()->transform.translateW({ 0.5, 0, 1 });
	newEntity.AddComponent<MeshComp>()->meshID = m_sphereMesh;
	newEntity.AddComponent<MaterialComp>()->materialID = m_greenMaterial;*/


	/*newEntity = m_entities.emplace_back(EntityReg::CreateEntity());
	auto& transform = newEntity.AddComponent<TransformComp>()->transform;
	transform.setTranslation({ 0, 0.4f, 0.6f });
	transform.setRotationDeg(30, 20, 0);
	transform.setScale(0.4f);
	newEntity.AddComponent<MeshComp>()->meshID = m_boxMesh;
	newEntity.AddComponent<MaterialComp>()->materialID = m_redMaterial;*/

	//std::random_device d;
	std::default_random_engine eng(4);
	std::uniform_int_distribution<> dist1(0, 2);
	std::uniform_int_distribution<> dist2(0, 1);
	
	for (int i = 0; i < 30; i++)
	{
		for (int j = 0; j < 30; j++)
		{
			for (int k = 0; k < 30; k++)
			{
				rfm::Vector3 pos = rfm::Vector3( 2*i, 2*j, 2*k );
				Entity newEntity = m_entities.emplace_back(EntityReg::CreateEntity());
				auto& transform = newEntity.AddComponent<TransformComp>()->transform;
				transform.setTranslation(pos);
				transform.setScale(0.5f);
				int r = dist1(eng);
				int r2 = dist2(eng);

				if(r2 == 0)
					newEntity.AddComponent<MeshComp>()->meshID = m_boxMesh;
				else
					newEntity.AddComponent<MeshComp>()->meshID = m_sphereMesh;

				if(r == 0)
					newEntity.AddComponent<MaterialComp>()->materialID = m_redMaterial;
				else if(r == 1)
					newEntity.AddComponent<MaterialComp>()->materialID = m_blueMaterial;
				else
					newEntity.AddComponent<MaterialComp>()->materialID = m_greenMaterial;
			}
		}
	}


	m_camera = EntityReg::CreateEntity();
	m_camera.AddComponent<TransformComp>()->transform.setTranslation(10, 0, -10);
	m_camera.AddComponent<CameraComp>();
	auto controller = m_camera.AddComponent<CameraControllerScript>();
	controller->ToggleCameraLock();
}

Scene::~Scene()
{

}

void Scene::Update(float dt)
{
	rfe::EntityReg::RunScripts<CameraControllerScript>(dt);

	
}
