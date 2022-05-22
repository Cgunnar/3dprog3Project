#include "pch.h"
#include "AssimpLoader.h"
#include <assimp/pbrmaterial.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Importer.hpp>

#ifdef _DEBUG
#pragma comment(lib, "assimp-vc142-mtd.lib")
#else
#pragma comment(lib, "assimp-vc142-mt.lib")
#endif // DEBUG

using namespace Geometry;

AssimpLoader::AssimpLoader() :
	m_meshVertexCount(0),
	m_meshIndexCount(0),
	m_hasNormalMap(false)
{
}

AssimpLoader::~AssimpLoader()
{
}

EngineMeshData AssimpLoader::loadStaticModel(const std::string& filePath)
{
	m_hasNormalMap = false;

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		filePath,
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_PreTransformVertices |
		aiProcess_GenBoundingBoxes
		//aiProcess_DropNormals		// Added 15/03/2021
	);

	if (scene == nullptr)
	{
		OutputDebugStringW(L"Assimp: File not found!");
		assert(false);
	}

	std::string path = "";
	if (filePath.rfind('/') != std::string::npos)
	{
		size_t pos = filePath.rfind('/');
		path = filePath.substr(0, pos + 1);
	}

	unsigned int totalVertexCount = 0;
	unsigned int totalSubsetCount = 0;
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		totalVertexCount += scene->mMeshes[i]->mNumVertices;
		++totalSubsetCount;
	}
	m_vertices.reserve(totalVertexCount);
	m_verticesTBN.reserve(totalVertexCount);
	m_indices.reserve(totalVertexCount);
	//m_subsets.reserve(totalSubsetCount);

	SubMeshTree modelGraph;

	modelGraph = processNode(scene->mRootNode, scene, path);
	//assert(modelGraph.subMeshes.empty()); // i want to know if this can be filled or if the root always is empty



	EngineMeshData data;
	data.indices = m_indices;
	data.vertices = m_vertices;
	data.verticesTBN = m_verticesTBN;
	//data.subMeshesVector = m_subsets;
	data.subsetsInfo = modelGraph;

	m_indices.clear();
	m_vertices.clear();
	m_verticesTBN.clear();
	//m_subsets.clear();
	m_meshVertexCount = 0;
	m_meshIndexCount = 0;



	data.hasNormalMap = m_hasNormalMap;

	return data;
}

SubMeshTree AssimpLoader::processNode(aiNode* node, const aiScene* scene, const std::string& path)
{
	SubMeshTree meshTree;

	// For each mesh in the node, process it!
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		EngineMeshSubset subMesh = processMesh(mesh, scene, path);
		meshTree.subMeshes.push_back(subMesh);
		//m_subsets.push_back(subMesh);
		//meshTree.subMeshesIndex.push_back(m_subsets.size() - 1);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		meshTree.nodes.push_back(processNode(node->mChildren[i], scene, path));
	}
	return meshTree;
}

// Subset of Mesh
EngineMeshSubset AssimpLoader::processMesh(aiMesh* mesh, const aiScene* scene, const std::string& path)
{

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		Vertex_POS_NOR_UV_TAN_BITAN vertTBN = { };
		vertTBN.biTangent.x = mesh->mBitangents[i].x;
		vertTBN.biTangent.y = mesh->mBitangents[i].y;
		vertTBN.biTangent.z = mesh->mBitangents[i].z;

		vertTBN.tangent.x = mesh->mTangents[i].x;
		vertTBN.tangent.y = mesh->mTangents[i].y;
		vertTBN.tangent.z = mesh->mTangents[i].z;

		vertTBN.position.x = mesh->mVertices[i].x;
		vertTBN.position.y = mesh->mVertices[i].y;
		vertTBN.position.z = mesh->mVertices[i].z;

		vertTBN.normal.x = mesh->mNormals[i].x;
		vertTBN.normal.y = mesh->mNormals[i].y;
		vertTBN.normal.z = mesh->mNormals[i].z;

		Vertex_POS_NOR_UV vert = { };
		vert.position.x = mesh->mVertices[i].x;
		vert.position.y = mesh->mVertices[i].y;
		vert.position.z = mesh->mVertices[i].z;

		vert.normal.x = mesh->mNormals[i].x;
		vert.normal.y = mesh->mNormals[i].y;
		vert.normal.z = mesh->mNormals[i].z;

		if (mesh->mTextureCoords[0])
		{
			vert.uv.x = mesh->mTextureCoords[0][i].x;
			vert.uv.y = mesh->mTextureCoords[0][i].y;

			vertTBN.uv.x = mesh->mTextureCoords[0][i].x;
			vertTBN.uv.y = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			vert.uv.x = 0.0f;
			vert.uv.y = 0.0f;

			vertTBN.uv.x = 0.0f;
			vertTBN.uv.y = 0.0f;
		}

		m_vertices.push_back(vert);
		m_verticesTBN.push_back(vertTBN);
	}

	unsigned int indicesThisMesh = 0;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		const aiFace& face = mesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			m_indices.push_back(face.mIndices[j]);
			++indicesThisMesh;
		}

	}

	// Get material
	auto mtl = scene->mMaterials[mesh->mMaterialIndex];

	Material pbrMaterial = GetPbrMaterials(mtl, path);
	// Subset data
	EngineMeshSubset subsetData = { };
	subsetData.aabb.min = { mesh->mAABB.mMin.x, mesh->mAABB.mMin.y, mesh->mAABB.mMin.z };
	subsetData.aabb.max = { mesh->mAABB.mMax.x, mesh->mAABB.mMax.y, mesh->mAABB.mMax.z };
	subsetData.name = pbrMaterial.name;
	subsetData.pbrMaterial = pbrMaterial;

	subsetData.vertexCount = mesh->mNumVertices;
	subsetData.vertexStart = m_meshVertexCount;
	m_meshVertexCount += mesh->mNumVertices;

	subsetData.indexCount = indicesThisMesh;
	subsetData.indexStart = m_meshIndexCount;
	m_meshIndexCount += indicesThisMesh;

	return subsetData;
}

Material AssimpLoader::GetPbrMaterials(aiMaterial* aiMat, const std::string& path)
{
	Material pbrMat;

	aiString materialName;
	if (!aiMat->Get(AI_MATKEY_NAME, materialName))
	{
		pbrMat.name = materialName.C_Str();
	}

	/*bool twoSided = false;
	if (!aiMat->Get(AI_MATKEY_TWOSIDED, twoSided))
	{
		if(twoSided)
			pbrMat.flags |= RenderFlag::noBackFaceCull;
	}*/

	float metallicFactor;
	if (!aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor))
	{
		pbrMat.metallicFactor = metallicFactor;
	}

	float roughnessFactor;
	if (!aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor))
	{
		pbrMat.roughnessFactor = roughnessFactor;
	}

	/*aiString alphaMode;
	if (!aiMat->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode))
	{
		if (alphaMode == aiString("MASK"))
		{
			pbrMat.flags |= RenderFlag::alphaToCov;
			float cutOf = 1;
			if (!aiMat->Get(AI_MATKEY_GLTF_ALPHACUTOFF, cutOf))
				pbrMat.maskCutOfValue = cutOf;
		}
		else if(alphaMode == aiString("BLEND"))
		{
			pbrMat.flags |= RenderFlag::alphaBlend;
		}
		else if (alphaMode == aiString("OPAQUE"))
		{
		}
	}*/

	
	

	aiColor3D ai_matkey_emissive(0.0f, 0.0f, 0.0f);
	if (!aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, ai_matkey_emissive))
	{
		pbrMat.emissiveFactor = rfm::Vector3(ai_matkey_emissive[0], ai_matkey_emissive[1], ai_matkey_emissive[2]);
	}

	aiString baseColorName, normName, metallicRoughnessName, emissiveName, aoName;
	if (!aiMat->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &baseColorName))
	{
		pbrMat.SetAlbedoTexture(path + baseColorName.C_Str());
	}
	else if (!aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &baseColorName))
	{
		pbrMat.SetAlbedoTexture(path + baseColorName.C_Str());
	}
	
	if (!aiMat->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &metallicRoughnessName))
	{
		pbrMat.SetMetallicRoughnessTexture(path + metallicRoughnessName.C_Str());
	}
	
	if (!aiMat->GetTexture(aiTextureType::aiTextureType_EMISSIVE, 0, &emissiveName))
	{
		pbrMat.SetEmissiveTexture(path + emissiveName.C_Str());
	}

	if (!aiMat->GetTexture(aiTextureType_NORMALS, 0, &normName))
	{
		pbrMat.SetNormalTexture(path + normName.C_Str());
		m_hasNormalMap = true; // to use tangent and bitangent vertexbuffer
	}

	if (!aiMat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &aoName))
	{
		//pbrMat.aoPath = path + aoName.C_Str();
		assert(false);// want to know if this is used
	}

	aiColor4D baseColorFactor(0.0f, 0.0f, 0.0f, 0.0f);
	aiColor3D colorDiff(0.f, 0.f, 0.f); // legacy material
	if (!aiMat->Get(AI_MATKEY_BASE_COLOR, baseColorFactor))
	{
		pbrMat.albedoFactor = rfm::Vector4(baseColorFactor[0], baseColorFactor[1], baseColorFactor[2], baseColorFactor[3]);
	}
	else if (!aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, colorDiff) && !pbrMat.albedoID) // legacy material
	{
		float opacity = 1;
		if (!aiMat->Get(AI_MATKEY_OPACITY, opacity))
		{
			if (opacity < 1)
			{
				//pbrMat.flags |= RenderFlag::alphaBlend;
			}
		}
		pbrMat.albedoFactor = rfm::Vector4(colorDiff[0], colorDiff[1], colorDiff[2], opacity);
	}
	else
		pbrMat.albedoFactor = { 1,1,1,1 };

	return pbrMat;
}
