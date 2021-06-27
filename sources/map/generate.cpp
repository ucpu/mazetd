#include <cage-core/geometry.h>
#include <cage-core/marchingCubes.h>
#include <cage-core/threadPool.h>

#include "generate.h"

namespace
{
	struct Maker
	{
		Holder<PointerRange<Holder<Mesh>>> meshes;
		Holder<Procedural> procedural;

		struct ChunkMaterial : public ChunkUpload
		{
			Procedural *procedural = nullptr;
		};

		static void makeMaterial(ChunkMaterial *chunk, const ivec2 &xy, const ivec3 &ids, const vec3 &weights)
		{
			chunk->procedural->material(chunk, xy, ids, weights);
		}

		void makeChunk(uint32 meshIndex)
		{
			const Holder<Mesh> &msh = meshes[meshIndex];
			uint32 resolution = 0;
			{
				MeshUnwrapConfig cfg;
#ifdef CAGE_DEBUG
				cfg.texelsPerUnit = 20;
#else
				cfg.texelsPerUnit = 40;
#endif // CAGE_DEBUG
				resolution = meshUnwrap(+msh, cfg);
			}
			ChunkMaterial chunk;
			chunk.procedural = +procedural;
			chunk.mesh = msh.share();
			chunk.albedo = newImage();
			chunk.albedo->initialize(ivec2(resolution), 3);
			chunk.albedo->colorConfig.gammaSpace = GammaSpaceEnum::Gamma;
			chunk.material = newImage();
			chunk.material->initialize(ivec2(resolution), 1);
			chunk.material->colorConfig.gammaSpace = GammaSpaceEnum::None;
			chunk.material->colorConfig.alphaMode = AlphaModeEnum::None;
			chunk.material->colorConfig.alphaChannelIndex = m;
			chunk.material->colorConfig.colorChannelsCount = 0;
			{
				MeshGenerateTextureConfig cfg;
				cfg.width = cfg.height = resolution;
				cfg.generator.bind<ChunkMaterial *, &Maker::makeMaterial>(&chunk);
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

		void chunksThreadEntry(uint32 idx, uint32 cnt)
		{
			uint32 b = 0, e = 0;
			threadPoolTasksSplit(idx, cnt, meshes.size(), b, e);
			for (uint32 i = b; i < e; i++)
				makeChunk(i);
		}

		void makeMeshes()
		{
			Holder<Mesh> msh;
			{
				MarchingCubesCreateConfig cfg;
				cfg.box = Aabb(vec3(-80, -15, -80), vec3(80, 10, 80));
				cfg.clip = true;
#ifdef CAGE_DEBUG
				cfg.resolution = ivec3(cfg.box.size() * 0.3);
#else
				cfg.resolution = ivec3(cfg.box.size() * 2);
#endif // CAGE_DEBUG
				Holder<MarchingCubes> mc = newMarchingCubes(cfg);
				mc->updateByPosition(Delegate<real(const vec3 &)>().bind<Procedural, &Procedural::sdf>(+procedural));
				msh = mc->makeMesh();
			}
			meshDiscardDisconnected(+msh);
			{
				MeshSimplifyConfig cfg;
				cfg.minEdgeLength = 0.1;
				cfg.maxEdgeLength = 2;
				meshSimplify(+msh, cfg);
			}
			CAGE_LOG(SeverityEnum::Info, "mapgen", stringizer() + "mesh faces: " + msh->facesCount());
			msh->exportObjFile({}, "mesh.obj");
			{
				MeshChunkingConfig cfg;
				cfg.maxSurfaceArea = 300;
				meshes = meshChunking(+msh, cfg);
				msh.clear();
			}
			CAGE_LOG(SeverityEnum::Info, "mapgen", stringizer() + "mesh chunks: " + meshes.size());
			Holder<ThreadPool> thrs = newThreadPool();
			thrs->function.bind<Maker, &Maker::chunksThreadEntry>(this);
			thrs->run();
		}
	};
}

void mapGenerate()
{
	CAGE_LOG(SeverityEnum::Info, "mapgen", "generating new map");
	{
		Holder<Procedural> procedural = newProcedural();

		// todo generate grid

		{
			Maker maker;
			maker.procedural = procedural.share();
			maker.makeMeshes();
		}
	}
	CAGE_LOG(SeverityEnum::Info, "mapgen", "map generation done");
}
