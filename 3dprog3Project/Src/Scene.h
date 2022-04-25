#pragma once

#include "rfEntity.hpp"

class Scene
{
public:
	Scene();
	~Scene();
	Scene(const Scene& other) = delete;
	Scene& operator=(const Scene& other) = delete;

	void Update(float dt);

private:
	std::vector<rfe::Entity> m_entities;
	rfe::Entity m_camera;

	uint64_t m_sphereMesh;
	uint64_t m_boxMesh;
	uint64_t m_redMaterial;
	uint64_t m_greenMaterial;
	uint64_t m_blueMaterial;
};