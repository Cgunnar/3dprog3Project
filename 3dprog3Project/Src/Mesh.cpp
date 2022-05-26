#include "pch.h"

#include "Mesh.h"

Mesh::Mesh(const float* vertexData, size_t vertexDataBytes, const std::vector<uint32_t>& indices, MeshType type)
{
	m_type = type;
	m_indices = indices;
	m_vertexCount = static_cast<uint32_t>(vertexDataBytes / sizeof(float));
	m_vertexData.resize(m_vertexCount);
	std::memcpy(m_vertexData.data(), vertexData, vertexDataBytes);
	switch (type)
	{
	case MeshType::POS_UV:
		m_vertexStride = sizeof(float) * (3 + 2);
		break;
	case MeshType::POS_NOR_UV:
		m_vertexStride = sizeof(float) * (3 + 3 + 2);
		break;
	case MeshType::POS_NOR_UV_TAN_BITAN:
		m_vertexStride = sizeof(float) * (3 + 3 + 2 + 3 + 3);
		break;
	default:
		m_vertexStride = sizeof(float);
		break;
	}
}

uint32_t Mesh::GetVertexCount() const { return static_cast<uint32_t>(m_vertexData.size() * sizeof(float) / m_vertexStride); }
uint32_t Mesh::GetVertexStride() const { return m_vertexStride; }
MeshType Mesh::GetVertexType() const { return m_type; }
const std::vector<float>& Mesh::GetVertexData() const { return m_vertexData; }
const std::vector<uint32_t>& Mesh::GetIndices() const { return m_indices; }