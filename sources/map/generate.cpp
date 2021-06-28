#include <cage-core/geometry.h>
#include <cage-core/marchingCubes.h>
#include <cage-core/threadPool.h>
#include <cage-core/mesh.h>
#include <cage-core/image.h>
#include <cage-core/collider.h>

#include "generate.h"

namespace
{
	struct Maker
	{
		Holder<Mesh> msh;
		Holder<PointerRange<Holder<Mesh>>> meshes;
		Holder<Procedural> procedural;
		Holder<Grid> grid;

		struct ChunkMaterial : public ChunkUpload
		{
			Maker *maker = nullptr;
			
			void makeMaterial(const ivec2 &xy, const ivec3 &ids, const vec3 &weights)
			{
				const vec3 pos3 = mesh->positionAt(ids, weights);
				const uint32 index = maker->grid->index(pos3);
				const TileFlags flags = index == m ? TileFlags::Invalid : maker->grid->tiles[index];
				vec3 color;
				real roughness;
				maker->procedural->material(pos3, flags, color, roughness);
				albedo->set(xy, color);
				material->set(xy, roughness);
			}
		};

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
			chunk.maker = this;
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
				cfg.generator.bind<ChunkMaterial, &ChunkMaterial::makeMaterial>(&chunk);
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
			{
				MeshChunkingConfig cfg;
				cfg.maxSurfaceArea = 300;
				meshes = meshChunking(+msh, cfg);
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
		Holder<Grid> grid = newGrid(procedural.share());
		Holder<MultiPaths> paths = newMultiPaths(grid.share());
		Maker maker;
		maker.procedural = procedural.share();
		maker.grid = grid.share();
		maker.makeMeshes();
		Holder<Collider> collider = newCollider();
		collider->importMesh(+maker.msh);
		collider->rebuild();
		globalCollider = std::move(collider);
		globalPaths = std::move(paths);
		globalGrid = std::move(grid); // this is last as it is frequently used to detect whether a map is loaded
	}
	CAGE_LOG(SeverityEnum::Info, "mapgen", "map generation done");
}

Holder<Grid> globalGrid;
Holder<MultiPaths> globalPaths;
Holder<Collider> globalCollider;
