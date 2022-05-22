#include "pch.h"
#include "ModelLoader.h"

#include <assimp/pbrmaterial.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Importer.hpp>

#ifdef _DEBUG
#pragma comment(lib, "assimp-vc142-mtd.lib")
#else
#pragma comment(lib, "assimp-vc142-mt.lib")
#endif // DEBUG

void ModelLoader::LoadModel(const std::string& path)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		path,
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_PreTransformVertices |
		aiProcess_GenBoundingBoxes
		//aiProcess_DropNormals		// Added 15/03/2021
	);
}
