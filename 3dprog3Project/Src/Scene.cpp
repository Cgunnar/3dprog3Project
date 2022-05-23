#include "pch.h"
#include "Scene.h"
#include "CommonComponents.h"
#include "AssetManager.h"
#include "Window.h"
#include "CameraControllerScript.h"
#include "AssimpLoader.h"

using namespace rfe;

struct ModelPart
{
	uint64_t meshID;
	uint64_t materialID;
	uint32_t indexStart;
	uint32_t indexCount;
	uint32_t vertexStart;
	uint32_t vertexCount;
};

void BuildModel(SubMeshTree& node, std::vector<ModelPart>& modelPartsOut)
{
	for (auto& p : node.subMeshes)
	{
		ModelPart part;
		part.indexStart = p.indexStart;
		part.indexCount = p.indexCount;
		part.vertexStart = p.vertexStart;
		part.vertexCount = p.vertexCount;
		part.materialID = AssetManager::Get().AddMaterial(p.pbrMaterial);
		AssetManager::Get().MoveMaterialToGPU(part.materialID);
		modelPartsOut.push_back(part);
	}

	for (auto& n : node.nodes)
	{
		BuildModel(n, modelPartsOut);
	}
}

Scene::Scene()
{
	//AssimpLoader loader;
	//auto sponza = loader.loadStaticModel("Assets/Arrows/DXRefSys.obj");
	////auto sponza = loader.loadStaticModel("Assets/nanosuit/nanosuit.obj");
	////auto sponza = loader.loadStaticModel("Assets/UV_Sphere/sphere.obj");
	////auto sponza = loader.loadStaticModel("Assets/Sponza_gltf/glTF/Sponza.gltf");
	//std::vector<uint32_t> sponzaIndexBuffer;
	//sponzaIndexBuffer.resize(sponza.getIndicesCount());
	//memcpy(sponzaIndexBuffer.data(), sponza.getIndicesData(), sizeof(uint32_t) * sponzaIndexBuffer.size());
	//Mesh newSponzaMesh = Mesh((const float*)sponza.getVertextBuffer(Geometry::VertexFormat::POS_NOR_UV),
	//	sponza.getVertexSize(Geometry::VertexFormat::POS_NOR_UV) * sponza.getVertexCount(Geometry::VertexFormat::POS_NOR_UV),
	//	sponzaIndexBuffer, MeshType::POS_NOR_UV);

	//AssimpLoader loader;
	////auto sponza = loader.loadStaticModel("Assets/Arrows/DXRefSys.obj");
	//auto sponza = loader.loadStaticModel("Assets/nanosuit/nanosuit.obj");
	////auto sponza = loader.loadStaticModel("Assets/UV_Sphere/sphere.obj");
	////auto sponza = loader.loadStaticModel("Assets/Sponza_gltf/glTF/Sponza.gltf");
	//std::vector<uint32_t> sponzaIndexBuffer;
	//sponzaIndexBuffer.resize(sponza.getIndicesCount());
	//memcpy(sponzaIndexBuffer.data(), sponza.getIndicesData(), sizeof(uint32_t) * sponzaIndexBuffer.size());
	//Mesh newSponzaMesh = Mesh((const float*)sponza.getVertextBuffer(Geometry::VertexFormat::POS_NOR_UV),
	//	sponza.getVertexSize(Geometry::VertexFormat::POS_NOR_UV) * sponza.getVertexCount(Geometry::VertexFormat::POS_NOR_UV),
	//	sponzaIndexBuffer, MeshType::POS_NOR_UV);
	//
	//m_sponzaMesh = AssetManager::Get().AddMesh(newSponzaMesh);
	//AssetManager::Get().MoveMeshToGPU(m_sponzaMesh);




	AssimpLoader loader;
	//auto sponza = loader.loadStaticModel("Assets/Arrows/DXRefSys.obj");
	auto sponza = loader.loadStaticModel("Assets/nanosuit/nanosuit.obj");
	//auto sponza = loader.loadStaticModel("Assets/UV_Sphere/sphere.obj");
	//auto sponza = loader.loadStaticModel("Assets/Sponza_gltf/glTF/Sponza.gltf");
	std::vector<uint32_t> sponzaIndexBuffer;
	sponzaIndexBuffer.resize(sponza.getIndicesCount());
	memcpy(sponzaIndexBuffer.data(), sponza.getIndicesData(), sizeof(uint32_t) * sponzaIndexBuffer.size());
	Mesh newSponzaMesh = Mesh((const float*)sponza.getVertextBuffer(Geometry::VertexFormat::POS_NOR_UV),
		sponza.getVertexSize(Geometry::VertexFormat::POS_NOR_UV) * sponza.getVertexCount(Geometry::VertexFormat::POS_NOR_UV),
		sponzaIndexBuffer, MeshType::POS_NOR_UV);

	m_sponzaMesh = AssetManager::Get().AddMesh(newSponzaMesh);
	AssetManager::Get().MoveMeshToGPU(m_sponzaMesh);

	std::vector<ModelPart> model;
	BuildModel(sponza.subsetsInfo, model);

	for (auto& part : model)
	{
		part.meshID = m_sponzaMesh;
	}


	for (auto& part : model)
	{
		Entity newEntity = m_entities.emplace_back(EntityReg::CreateEntity());
		auto& tr = newEntity.AddComponent<TransformComp>()->transform;
		tr.setScale(0.2);
		tr.setTranslation(4, 0, 0);
		newEntity.AddComponent<MaterialComp>()->materialID = part.materialID;
		MeshComp* mesh = newEntity.AddComponent<MeshComp>();
		mesh->meshID = part.meshID;
		mesh->indexStart = part.indexStart;
		mesh->indexCount = part.indexCount;
		mesh->vertexStart = part.vertexStart;
		mesh->vertexCount = part.vertexCount;
		mesh->useStartAndCount = true;
	}

	for (auto& part : model)
	{
		Entity newEntity = m_entities.emplace_back(EntityReg::CreateEntity());
		auto& tr = newEntity.AddComponent<TransformComp>()->transform;
		tr.setScale(0.2);
		tr.setTranslation(-4, 0, 0);
		newEntity.AddComponent<MaterialComp>()->materialID = part.materialID;
		MeshComp* mesh = newEntity.AddComponent<MeshComp>();
		mesh->meshID = part.meshID;
		mesh->indexStart = part.indexStart;
		mesh->indexCount = part.indexCount;
		mesh->vertexStart = part.vertexStart;
		mesh->vertexCount = part.vertexCount;
		mesh->useStartAndCount = true;
	}


	//---------------------------------------------
	
	Geometry::Sphere_POS_NOR_UV sphere = Geometry::Sphere_POS_NOR_UV(32, 0.5f);
	Geometry::AABB_POS_NOR_UV box = Geometry::AABB_POS_NOR_UV({ -0.5f, -0.5f, -0.5f }, { 0.5f, 0.5f, 0.5f });
	Mesh newSphereMesh = Mesh(reinterpret_cast<const float*>(sphere.VertexData().data()), sphere.ArraySize(), sphere.IndexData(), MeshType::POS_NOR_UV);
	Mesh newBoxMesh = Mesh(reinterpret_cast<const float*>(box.VertexData().data()), box.ArraySize(), box.IndexData(), MeshType::POS_NOR_UV);

	m_sphereMesh = AssetManager::Get().AddMesh(newSphereMesh);
	AssetManager::Get().MoveMeshToGPU(m_sphereMesh);

	m_boxMesh = AssetManager::Get().AddMesh(newBoxMesh);
	AssetManager::Get().MoveMeshToGPU(m_boxMesh);

	Material whiteEmissiveMat;
	whiteEmissiveMat.albedoFactor = { 0, 0, 0, 1 };
	whiteEmissiveMat.emissiveFactor = { 1, 1, 1};
	Material matRed;
	matRed.albedoFactor = { 1, 0, 0, 1 };
	Material matGreen;
	matGreen.albedoFactor = { 0, 1, 0, 1 };
	Material matBlue;
	matBlue.albedoFactor = { 0, 0, 1, 1 };
	Material texturedMaterial;
	texturedMaterial.albedoFactor = { 1, 1, 1, 1 };
	texturedMaterial.albedoID = AssetManager::Get().AddTextureFromFile("Assets/Hej.png", false, false);
	Material rustedIron;
	rustedIron.albedoFactor = { 1, 1, 1, 1 };
	rustedIron.albedoID = AssetManager::Get().AddTextureFromFile("Assets/rusted_iron_albedo.png", false, false);

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

	Entity newEntity = m_entities.emplace_back(EntityReg::CreateEntity());
	auto& transform = newEntity.AddComponent<TransformComp>()->transform;
	transform.setTranslation(0,0,-14);
	transform.setScale(0.1f);
	//newEntity.AddComponent<MeshComp>()->meshID = m_sponzaMesh;
	//newEntity.AddComponent<MeshComp>()->meshID = m_sphereMesh;
	//newEntity.AddComponent<MaterialComp>()->materialID = m_greenMaterial;

	std::default_random_engine eng(4);
	std::uniform_int_distribution<> dist1(0, 4);
	std::uniform_int_distribution<> dist2(0, 1);
	
	int l = 16;
#ifdef _DEBUG
	l = 0;
#endif // _DEBUG

	for (int i = 0; i < l; i++)
	{
		for (int j = 0; j < l; j++)
		{
			for (int k = 0; k < l; k++)
			{
				rfm::Vector3 pos = rfm::Vector3( 2*i, 2*j, 2*k ) - rfm::Vector3(l);
				Entity newEntity = m_entities.emplace_back(EntityReg::CreateEntity());
				auto& transform = newEntity.AddComponent<TransformComp>()->transform;
				transform.setTranslation(pos);
				transform.setScale(0.5f);
				int randMat = dist1(eng);
				int randMesh = dist2(eng);

				if (randMesh == 0)
				{
					newEntity.AddComponent<MeshComp>()->meshID = m_boxMesh;
					newEntity.AddComponent<SpinnComp>()->rotSpeed = pos;
				}
				else
					newEntity.AddComponent<MeshComp>()->meshID = m_sphereMesh;

				if(randMat == 0)
					newEntity.AddComponent<MaterialComp>()->materialID = m_redMaterial;
				else if(randMat == 1)
					newEntity.AddComponent<MaterialComp>()->materialID = m_blueMaterial;
				else if(randMat == 2)
					newEntity.AddComponent<MaterialComp>()->materialID = m_greenMaterial;
				else if (randMat == 3)
					newEntity.AddComponent<MaterialComp>()->materialID = m_rustedIronMaterial;
				else
					newEntity.AddComponent<MaterialComp>()->materialID = m_hejMaterial;
			}
		}
	}

	int numLight = 8;
#ifdef _DEBUG
	numLight = 1;
#endif // _DEBUG
	for (int i = 0; i < numLight; i++)
	{
		std::mt19937 eng0(12 + i);
		std::mt19937 eng1(313235 + i);
		std::mt19937 eng2(52121 + i);
		std::uniform_real_distribution<> posDist(-15, 15);
		std::uniform_real_distribution<> lightDist(0, 1);
		rfm::Vector3 pos(posDist(eng0), posDist(eng1), posDist(eng2));
		rfm::Vector3 light(lightDist(eng0), lightDist(eng1), lightDist(eng2));


		Entity newEntity = m_entities.emplace_back(EntityReg::CreateEntity());
		auto& tr0 = newEntity.AddComponent<TransformComp>()->transform;
		tr0.setScale(0.2);
		tr0.setTranslation(pos);
		auto& pl0 = newEntity.AddComponent<PointLightComp>()->pointLight;
		pl0.color = light;
		pl0.strength = 40.0f / numLight;
		pl0.position = pos;

		Material lightMat;
		lightMat.albedoFactor = { 0, 0, 0, 1 };
		lightMat.emissiveFactor = rfm::Vector4(light, 1);

		uint64_t matId = AssetManager::Get().AddMaterial(lightMat);
		AssetManager::Get().MoveMaterialToGPU(matId);

		//newEntity.AddComponent<MeshComp>()->meshID = m_sphereMesh;
		//newEntity.AddComponent<MaterialComp>()->materialID = matId;
	}


	m_camera = EntityReg::CreateEntity();
	m_camera.AddComponent<TransformComp>()->transform.setTranslation(0, 0, -20);
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
		TransformComp* transformComp = rfe::EntityReg::GetComponent<TransformComp>(plComp.GetEntityID());
		if (transformComp)
		{
			rfm::Transform& tr = transformComp->transform;
			if ((orbitPoint - tr.getTranslation()).length() == 0) orbitPoint += {0.1, 0, 0};
			rfm::Vector3 rotNormal = rfm::cross(tr.right(), tr.getTranslation() - orbitPoint);
			if (rotNormal.length() == 0) rotNormal = rfm::cross(tr.up(), tr.getTranslation() - orbitPoint);
			tr.translateW(-orbitPoint);
			tr = rfm::rotationMatrixFromNormal(rfm::normalize(rotNormal), rfm::DegToRad(30.0f * dt)) * tr;
			tr.translateW(orbitPoint);
			plComp.pointLight.position = tr.getTranslation();
		}
	}
}