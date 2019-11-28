#pragma once
#include <unordered_map>

#include "Mesh.h"
#include "ImageTexture.h"

namespace Engine
{
	namespace ResourceManagement
	{
		typedef std::unordered_map<const char*, std::shared_ptr<Texture2D>> FileTextureList;
		typedef std::unordered_map<const char*, std::shared_ptr<Mesh>> FileMeshList;

		// Record already external resources to prevent duplicate loading
		static FileTextureList LoadedImageTextures;
		static FileMeshList LoadedMeshes;
	}
}