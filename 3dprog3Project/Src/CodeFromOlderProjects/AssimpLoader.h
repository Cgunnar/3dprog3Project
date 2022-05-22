#pragma once
#pragma warning(push, 0)
#include "assimp\Importer.hpp"
#include "assimp\scene.h"
#include "assimp\postprocess.h"
#pragma warning(pop)
#include <string>
#include <vector>
#include "Material.h"
#include "Geometry.h"
#include "boundingVolumes.h"

struct EngineMeshSubset
{
	std::string name;
	Material pbrMaterial;
	AABB aabb;
	unsigned int vertexCount;
	unsigned int vertexStart;
	unsigned int indexStart;
	unsigned int indexCount;
};

struct SubMeshTree
{
	std::vector<SubMeshTree> nodes;
	std::vector<EngineMeshSubset> subMeshes;
};


class AssimpLoader;
struct EngineMeshData
{
	friend AssimpLoader;

private:
	std::vector<uint32_t> indices;
	std::vector<Geometry::Vertex_POS_NOR_UV> vertices;
	std::vector<Geometry::Vertex_POS_NOR_UV_TAN_BITAN> verticesTBN;
	bool hasNormalMap = false;

public:
	SubMeshTree subsetsInfo;

	bool hasNormalMaps() const
	{
		return this->hasNormalMap;
	}

	float* getVertextBuffer() const
	{
		if (hasNormalMap)
			return (float*)this->verticesTBN.data();
		else
			return (float*)this->vertices.data();
	}
	float* getVertextBuffer(Geometry::VertexFormat format) const
	{
		switch (format)
		{
		case Geometry::VertexFormat::POS_NOR_UV:
			return (float*)this->vertices.data();
		case Geometry::VertexFormat::POS_NOR_UV_TAN_BITAN:
			return (float*)this->verticesTBN.data();
		}
		return nullptr;
	}

	uint32_t getVertexSize() const
	{
		if (hasNormalMap)
			return (uint32_t)sizeof(Geometry::Vertex_POS_NOR_UV_TAN_BITAN);
		else
			return (uint32_t)sizeof(Geometry::Vertex_POS_NOR_UV);
	}

	uint32_t getVertexSize(Geometry::VertexFormat format) const
	{
		switch (format)
		{
		case Geometry::VertexFormat::POS_NOR_UV:
			return (uint32_t)sizeof(Geometry::Vertex_POS_NOR_UV);
		case Geometry::VertexFormat::POS_NOR_UV_TAN_BITAN:
			return (uint32_t)sizeof(Geometry::Vertex_POS_NOR_UV_TAN_BITAN);
		}	
		return 0;
	}
	uint32_t getVertexCount() const
	{
		if (hasNormalMap)
			return (uint32_t)verticesTBN.size();
		else
			return (uint32_t)vertices.size();
	}
	uint32_t getVertexCount(Geometry::VertexFormat format) const
	{
		switch (format)
		{
		case Geometry::VertexFormat::POS_NOR_UV:
			return (uint32_t)vertices.size();
		case Geometry::VertexFormat::POS_NOR_UV_TAN_BITAN:
			return (uint32_t)verticesTBN.size();
		}
		return 0;
	}
	uint32_t getIndicesCount() const
	{
		return (uint32_t)indices.size();
	}
	const unsigned int* getIndicesData() const
	{
		return indices.data();
	}
};


class AssimpLoader
{
public:

	AssimpLoader();
	~AssimpLoader();

	EngineMeshData loadStaticModel(const std::string& filePath);

private:
	std::vector<Geometry::Vertex_POS_NOR_UV> m_vertices;
	std::vector<Geometry::Vertex_POS_NOR_UV_TAN_BITAN> m_verticesTBN;
	std::vector<unsigned int> m_indices;

	unsigned int m_meshVertexCount;
	unsigned int m_meshIndexCount;

	bool m_hasNormalMap;

	SubMeshTree processNode(aiNode* node, const aiScene* scene, const std::string& path);
	EngineMeshSubset processMesh(aiMesh* mesh, const aiScene* scene, const std::string& path);
	Material GetPbrMaterials(aiMaterial* aiMat, const std::string& path);
};