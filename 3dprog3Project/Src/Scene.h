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

	rfe::Entity m_camera;
private:
	std::vector<rfe::Entity> m_entities;

	uint64_t m_sponzaMesh;
	uint64_t m_nanosuitMesh;
	uint64_t m_sphereMesh;
	uint64_t m_sphereMeshT;
	uint64_t m_boxMesh;
	uint64_t m_redMaterial;
	uint64_t m_greenMaterial;
	uint64_t m_blueMaterial;
	uint64_t m_hejMaterial;
	uint64_t m_rustedIronMaterial;


	
};