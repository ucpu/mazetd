#include <cage-core/geometry.h>
#include <cage-core/marchingCubes.h>
#include <cage-core/noiseFunction.h>
#include <cage-core/threadPool.h>

#include "generate.h"

namespace
{
	real elevation(const vec2 &pos)
	{
		// todo
		return -max(length(pos) - 50, 0);
	}

	real sdf(const vec3 &pos)
	{
		return elevation(vec2(pos[0], pos[2])) - pos[1];
	}

	using ChunkPtr = ChunkUpload *;

	void makeMaterial(ChunkPtr chunk, const ivec2 &xy, const ivec3 &ids, const vec3 &weights)
	{
		const vec3 &pos = chunk->mesh->positionAt(ids, weights);
		// todo
		const ivec2 tile = ivec2(10 * vec2(pos[0], pos[2]) + 1000) % 10;
		const bool white = tile[0] == 0 || tile[1] == 0;
		chunk->albedo->set(xy, vec3(int(white)));
		chunk->material->set(xy, vec2(0.5, 0));
	}

	void makeChunk(Holder<Mesh> &msh)
	{
		uint32 resolution = 0;
		{
			MeshUnwrapConfig cfg;
#ifdef CAGE_DEBUG
			cfg.texelsPerUnit = 20;
#else
			cfg.texelsPerUnit = 100;
#endif // CAGE_DEBUG
			resolution = meshUnwrap(+msh, cfg);
		}
		ChunkUpload chunk;
		chunk.mesh = msh.share();
		chunk.albedo = newImage();
		chunk.albedo->initialize(ivec2(resolution), 3);
		chunk.albedo->colorConfig.gammaSpace = GammaSpaceEnum::Gamma;
		chunk.material = newImage();
		chunk.material->initialize(ivec2(resolution), 2);
		chunk.material->colorConfig.gammaSpace = GammaSpaceEnum::None;
		chunk.material->colorConfig.alphaMode = AlphaModeEnum::None;
		chunk.material->colorConfig.alphaChannelIndex = m;
		chunk.material->colorConfig.colorChannelsCount = 0;
		{
			MeshGenerateTextureConfig cfg;
			cfg.width = cfg.height = resolution;
			cfg.generator.bind<ChunkPtr, &makeMaterial>(&chunk);
			meshGenerateTexture(+msh, cfg);
		}
		{
#ifdef CAGE_DEBUG
			constexpr uint32 rounds = 2;
#else
			constexpr uint32 rounds = 5;
#endif // CAGE_DEBUG
			imageDilation(+chunk.albedo, rounds);
			imageDilation(+chunk.material, rounds);
		}
		chunksUploadQueue.push(std::move(chunk));
	}

	using MeshesPtr = PointerRange<Holder<Mesh>> *;

	void threadEntry(MeshesPtr ptr, uint32 idx, uint32 cnt)
	{
		uint32 b = 0, e = 0;
		threadPoolTasksSplit(idx, cnt, ptr->size(), b, e);
		for (uint32 i = b; i < e; i++)
			makeChunk((*ptr)[i]);
	}

	void makeMeshes()
	{
		Holder<Mesh> msh;
		{
			MarchingCubesCreateConfig cfg;
			cfg.box = Aabb(vec3(-70, -10, -70), vec3(70, 10, 70));
			cfg.clip = true;
#ifdef CAGE_DEBUG
			cfg.resolution = ivec3(cfg.box.size() * 0.5);
#else
			cfg.resolution = ivec3(cfg.box.size() * 3);
#endif // CAGE_DEBUG
			Holder<MarchingCubes> mc = newMarchingCubes(cfg);
			mc->updateByPosition(Delegate<real(const vec3 &)>().bind<&sdf>());
			msh = mc->makeMesh();
		}
		meshDiscardDisconnected(+msh);
		{
			MeshSimplifyConfig cfg;
			cfg.minEdgeLength = 0.1;
			cfg.maxEdgeLength = 2;
			meshSimplify(+msh, cfg);
		}
		msh->exportObjFile({}, "mesh.obj");
		Holder<PointerRange<Holder<Mesh>>> meshes;
		{
			MeshChunkingConfig cfg;
			cfg.maxSurfaceArea = 500;
			meshes = meshChunking(+msh, cfg);
			msh.clear();
		}
		Holder<ThreadPool> thrs = newThreadPool();
		thrs->function.bind<MeshesPtr, &threadEntry>(+meshes);
		thrs->run();
	}
}

void mapGenerate()
{
	makeMeshes();
	// todo make grid
}
