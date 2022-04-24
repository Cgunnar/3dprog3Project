#pragma once

#include "rfEntity.hpp"

class Scene
{
public:
	Scene();
	~Scene();
	Scene(const Scene& other) = delete;
	Scene& operator=(const Scene& other) = delete;

private:
	std::vector<rfe::Entity> m_entities;

	uint64_t m_sphereMesh;
	uint64_t m_greenMaterial;
	uint64_t m_redMaterial;
};