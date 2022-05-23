#pragma once

#include "Geometry.h"

enum class MeshType
{
	NONE,
	POS_UV,
	POS_NOR_UV,
	POS_NOR_UV_TAN_BITAN
};

class Mesh
{
public:
	Mesh() = default;
	Mesh(const float* vertexData, size_t vertexDataBytes, const std::vector<uint32_t>& indices, MeshType type);

	uint32_t GetVertexCount() const;
	uint32_t GetVertexStride() const;
	MeshType GetVertexType() const;
	const std::vector<float>& GetVertexData() const;
	const std::vector<uint32_t>& GetIndices() const;

private:
	MeshType m_type = MeshType::NONE;
	std::vector<float> m_vertexData;
	std::vector<uint32_t> m_indices;
	uint32_t m_vertexStride = 0;
	uint32_t m_vertexCount = 0;
};