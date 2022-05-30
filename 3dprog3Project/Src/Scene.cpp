#include "pch.h"
#include "Scene.h"
#include "CommonComponents.h"
#include "AssetManager.h"
#include "Window.h"
#include "CameraControllerScript.h"
#include "AssimpLoader.h"

using namespace rfe;

Scene::Scene()
{
	m_sponzaMesh = AssetManager::Get().LoadModel("Assets/Sponza_gltf/glTF/Sponza.gltf");
	m_nanosuitMesh = AssetManager::Get().LoadModel("Assets/nanosuit/nanosuit.obj");

	Entity modelEntity = m_entities.emplace_back(EntityReg::CreateEntity());
	modelEntity.AddComponent<TransformComp>()->transform.setScale(0.3f);
	modelEntity.AddComponent<ModelComp>(m_sponzaMesh);

	modelEntity = m_entities.emplace_back(EntityReg::CreateEntity());
	modelEntity.AddComponent<TransformComp>()->transform.setScale(0.7f);
	modelEntity.GetComponent<TransformComp>()->transform.setTranslation(10, 20, 15);
	modelEntity.GetComponent<TransformComp>()->transform.setRotationDeg(-110, 0, 30);
	modelEntity.AddComponent<ModelComp>(m_sponzaMesh);

	for (int i = 0; i < 10; i++)
	{
		modelEntity = m_entities.emplace_back(EntityReg::CreateEntity());
		modelEntity.AddComponent<TransformComp>()->transform.setScale((10-i) * 0.06f);
		modelEntity.GetComponent<TransformComp>()->transform.setTranslation(-8, 0, 4-i*3);
		modelEntity.AddComponent<ModelComp>(m_nanosuitMesh);

		modelEntity = m_entities.emplace_back(EntityReg::CreateEntity());
		modelEntity.AddComponent<TransformComp>()->transform.setScale((10 - i) * 0.06f);
		modelEntity.GetComponent<TransformComp>()->transform.setTranslation(-8, -10, 4 - i * 3);
		modelEntity.AddComponent<ModelComp>(m_nanosuitMesh);
	}

	Entity modelEntityLight = m_entities.emplace_back(EntityReg::CreateEntity());
	auto& lTransform = modelEntityLight.AddComponent<TransformComp>()->transform;
	lTransform.setTranslation(1, 0.6, 0);
	auto& light = modelEntityLight.AddComponent<PointLightComp>()->pointLight;
	light.color = { 0.7f, 1, 0.5f };
	light.strength = 5;
	light.position = lTransform.getTranslation();
	

	Geometry::Sphere_POS_NOR_UV* sphere = new Geometry::Sphere_POS_NOR_UV(32, 0.5f);
	Mesh* newSphereMesh = new Mesh(reinterpret_cast<const float*>(sphere->VertexData().data()), sphere->ArraySize(), sphere->IndexData(), MeshType::POS_NOR_UV);
	delete sphere;
	m_sphereMesh = AssetManager::Get().AddMesh(*newSphereMesh);
	delete newSphereMesh;
	AssetManager::Get().MoveMeshToGPU(m_sphereMesh);

	Geometry::Sphere_POS_NOR_UV_TAN_BITAN* sphereT = new Geometry::Sphere_POS_NOR_UV_TAN_BITAN(32, 0.5f);
	Mesh* newSphereMeshT = new Mesh(reinterpret_cast<const float*>(sphereT->VertexData().data()), sphereT->ArraySize(), sphereT->IndexData(), MeshType::POS_NOR_UV_TAN_BITAN);
	delete sphereT;
	m_sphereMeshT = AssetManager::Get().AddMesh(*newSphereMeshT);
	delete newSphereMeshT;
	AssetManager::Get().MoveMeshToGPU(m_sphereMeshT);

	Geometry::AABB_POS_NOR_UV* box = new Geometry::AABB_POS_NOR_UV({ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f });
	Mesh* newBoxMesh = new Mesh(reinterpret_cast<const float*>(box->VertexData().data()), box->ArraySize(), box->IndexData(), MeshType::POS_NOR_UV);
	delete box;
	m_boxMesh = AssetManager::Get().AddMesh(*newBoxMesh);
	delete newBoxMesh;
	AssetManager::Get().MoveMeshToGPU(m_boxMesh);


	Material whiteEmissiveMat;
	whiteEmissiveMat.albedoFactor = { 0, 0, 0, 1 };
	whiteEmissiveMat.emissiveFactor = { 1, 1, 1};
	Material matRed;
	matRed.albedoFactor = { 1, 0, 0, 1 };
	matRed.roughnessFactor = 0.5f;
	Material matGreen;
	matGreen.albedoFactor = { 0, 1, 0, 1 };
	matGreen.roughnessFactor = 0.5f;
	Material matBlue;
	matBlue.albedoFactor = { 0, 0, 1, 1 };
	matBlue.roughnessFactor = 0.5f;
	Material texturedMaterial;
	texturedMaterial.albedoFactor = { 1, 1, 1, 1 };
	texturedMaterial.albedoID = AssetManager::Get().AddTextureFromFile("Assets/Hej.png", TextureType::albedo, false, false);
	Material rustedIron;
	rustedIron.albedoFactor = { 1, 1, 1, 1 };
	rustedIron.metallicFactor = 1;
	rustedIron.albedoID = AssetManager::Get().AddTextureFromFile("Assets/rustediron/basecolor.png", TextureType::albedo, true, false);
	rustedIron.normalID = AssetManager::Get().AddTextureFromFile("Assets/rustediron/normal.png", TextureType::normalMap, true, true);

	m_redMaterial = AssetManager::Get().AddMaterial(matRed);
	AssetManager::Get().MoveMaterialToGPU(m_redMaterial);
	m_greenMaterial = AssetManager::Get().AddMaterial(matGreen);
	AssetManager::Get().MoveMaterialToGPU(m_greenMaterial);
	m_blueMaterial = AssetManager::Get().AddMaterial(matBlue);
	AssetManager::Get().MoveMaterialToGPU(m_blueMaterial);
	m_hejMaterial = AssetManager::Get().AddMaterial(texturedMaterial);
	AssetManager::Get().MoveMaterialToGPU(m_hejMaterial);
	m_rustedIronMaterial = AssetManager::Get().AddMaterial(rustedIron);
	AssetManager::Get().MoveMaterialToGPU(m_rustedIronMaterial);


	Geometry::Sphere_POS_NOR_UV* smallSphere = new Geometry::Sphere_POS_NOR_UV(16, 0.2f);
	Mesh* smallSphereMesh = new Mesh(reinterpret_cast<const float*>(smallSphere->VertexData().data()), smallSphere->ArraySize(), smallSphere->IndexData(), MeshType::POS_NOR_UV);
	uint64_t smallSphereMeshID = AssetManager::Get().AddMesh(*smallSphereMesh, false);
	AssetManager::Get().MoveMeshToGPU(smallSphereMeshID);
	delete smallSphere;
	delete smallSphereMesh;


	std::default_random_engine eng(4);
	std::uniform_int_distribution<> distMat(0, 4);
	std::uniform_int_distribution<> distMesh(0, 3);

	int l = 10;
#ifdef _DEBUG
	l = 4;
#endif // _DEBUG
	l = std::min(l, 100000);
	for (int i = 0; i < l; i++)
	{
		for (int j = 0; j < l; j++)
		{
			for (int k = 0; k < l; k++)
			{
				int randMat = distMat(eng);
				int randMesh = distMesh(eng);
				rfm::Vector3 pos = rfm::Vector3( 2.0f*i, 2.0f*j, 2.0f*k ) - rfm::Vector3(static_cast<float>(l));

				Entity newEntity = m_entities.emplace_back(EntityReg::CreateEntity());
				auto& transform = newEntity.AddComponent<TransformComp>()->transform;
				transform.setTranslation(pos);
				transform.setScale(0.5f);

				if (randMesh == 0)
				{
					newEntity.AddComponent<MeshComp>()->meshID = m_boxMesh;
					newEntity.AddComponent<SpinnComp>()->rotSpeed = pos;
				}
				else if (randMesh == 1)
				{
					newEntity.AddComponent<MeshComp>()->meshID = m_sphereMeshT;
					newEntity.AddComponent<SpinnComp>()->rotSpeed = pos;
					randMat = 10;
				}
				else
				{
					newEntity.AddComponent<MeshComp>()->meshID = m_sphereMesh;
				}

				if (randMat == 0)
					newEntity.AddComponent<MaterialComp>()->materialID = m_redMaterial;
				else if (randMat == 1)
					newEntity.AddComponent<MaterialComp>()->materialID = m_blueMaterial;
				else if (randMat == 2)
					newEntity.AddComponent<MaterialComp>()->materialID = m_greenMaterial;
				else if (randMat == 10)
					newEntity.AddComponent<MaterialComp>()->materialID = m_rustedIronMaterial;
				else
					newEntity.AddComponent<MaterialComp>()->materialID = m_hejMaterial;
			}
		}
	}

	int numLight = 5;
#ifdef _DEBUG
	numLight = 1;
#endif // _DEBUG
	numLight = std::min(numLight, 10);
	for (int i = 0; i < numLight; i++)
	{
		std::mt19937 eng0(12 + i);
		std::mt19937 eng1(313235 + i);
		std::mt19937 eng2(52121 + i);
		std::uniform_real_distribution<> posDist(-15, 15);
		std::uniform_real_distribution<> lightDist(0, 1);
		rfm::Vector3 pos(static_cast<float>(posDist(eng0)), static_cast<float>(posDist(eng1)), static_cast<float>(posDist(eng2)));
		rfm::Vector3 light(static_cast<float>(lightDist(eng0)), static_cast<float>(lightDist(eng1)), static_cast<float>(lightDist(eng2)));


		Entity newEntity = m_entities.emplace_back(EntityReg::CreateEntity());
		auto& tr0 = newEntity.AddComponent<TransformComp>()->transform;
		tr0.setTranslation(pos);
		auto& pl0 = newEntity.AddComponent<PointLightComp>()->pointLight;
		pl0.color = light;
		pl0.strength = 40.0f / numLight;
		pl0.position = pos;

		Material lightMat;
		lightMat.albedoFactor = { 0, 0, 0, 1 };
		lightMat.emissiveFactor = pl0.strength * rfm::Vector4(light, 1);

		uint64_t matId = AssetManager::Get().AddMaterial(lightMat);
		AssetManager::Get().MoveMaterialToGPU(matId);

		newEntity.AddComponent<MeshComp>()->meshID = smallSphereMeshID;
		newEntity.AddComponent<MaterialComp>()->materialID = matId;
		newEntity.AddComponent<OrbitComp>();
	}


	m_camera = EntityReg::CreateEntity();
	m_camera.AddComponent<TransformComp>()->transform.setTranslation(0, 0, -10);
	m_camera.AddComponent<CameraComp>();
	auto controller = m_camera.AddComponent<CameraControllerScript>();
	controller->ToggleCameraLock();
}

Scene::~Scene()
{

}

void Scene::Update(float dt)
{
	//system to rotate the cubes
	const auto& spinningEntities = EntityReg::GetComponentArray<SpinnComp>();
	for (const auto& spinnComp : spinningEntities)
	{
		TransformComp* transformComp = rfe::EntityReg::GetComponent<TransformComp>(spinnComp.GetEntityID());
		if (transformComp)
			transformComp->transform.rotateDegL(dt * spinnComp.rotSpeed);
	}


	//system for making the lights orbit
	rfm::Vector3 orbitPoint = { 0,0,0 };
	auto& pointLightsEntities = EntityReg::GetComponentArray<PointLightComp>();
	for (auto& plComp : pointLightsEntities)
	{
		if (rfe::EntityReg::GetComponent<OrbitComp>(plComp.GetEntityID()))
		{
			TransformComp* transformComp = rfe::EntityReg::GetComponent<TransformComp>(plComp.GetEntityID());
			if (transformComp)
			{
				rfm::Transform& tr = transformComp->transform;
				if ((orbitPoint - tr.getTranslation()).length() == 0) orbitPoint += {0.1f, 0, 0};
				rfm::Vector3 rotNormal = rfm::cross(tr.right(), tr.getTranslation() - orbitPoint);
				if (rotNormal.length() == 0) rotNormal = rfm::cross(tr.up(), tr.getTranslation() - orbitPoint);
				tr.translateW(-orbitPoint);
				tr = rfm::rotationMatrixFromNormal(rfm::normalize(rotNormal), rfm::DegToRad(30.0f * dt)) * tr;
				tr.translateW(orbitPoint);
				plComp.pointLight.position = tr.getTranslation();
			}
		}
	}
}
