#include <cage-core/tasks.h>
#include <cage-core/geometry.h>
#include <cage-core/marchingCubes.h>
#include <cage-core/mesh.h>
#include <cage-core/image.h>
#include <cage-core/collider.h>
#include <cage-engine/engine.h>

#include "../game.h"
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
				const TileFlags flags = index == m ? TileFlags::Invalid : maker->grid->flags[index];
				vec3 color;
				real roughness;
				maker->procedural->material(pos3, flags, color, roughness);
				albedo->set(xy, color);
				material->set(xy, roughness);
			}
		};

		void makeChunk(const Holder<Mesh> &msh)
		{
			uint32 resolution = 0;
			{
				MeshUnwrapConfig cfg;
#ifdef CAGE_DEBUG
				cfg.texelsPerUnit = 15;
#else
				cfg.texelsPerUnit = 30;
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

		real sdf(const vec3 &pos)
		{
			real d = procedural->sdf(pos);
			const uint32 i = grid->index(pos);
			if (i != m)
			{
				const TileFlags f = grid->flags[i];
				if (any(f & TileFlags::Invalid) && none(f & TileFlags::Water))
				{
					const real e = saturate(find(pos[1], -7.5, -6.5));
					const real o = any(f & TileFlags::Border) ? 1 : 1.5;
					d += o * e;
				}
			}
			return d;
		}

		void makeMeshes()
		{
			{ // reset rendering queues and assets
				ChunkUpload chunk;
				chunksUploadQueue.push(std::move(chunk));
			}
			{
				MarchingCubesCreateConfig cfg;
				cfg.box = Aabb(vec3(-80, -15, -80), vec3(80, 15, 80));
				cfg.clip = true;
#ifdef CAGE_DEBUG
				cfg.resolution = ivec3(cfg.box.size() * 0.5);
#else
				cfg.resolution = ivec3(cfg.box.size() * 2.5);
#endif // CAGE_DEBUG
				Holder<MarchingCubes> mc = newMarchingCubes(cfg);
				mc->updateByPosition(Delegate<real(const vec3 &)>().bind<Maker, &Maker::sdf>(this));
				msh = mc->makeMesh();
			}
			meshDiscardDisconnected(+msh);
			{
				MeshSimplifyConfig cfg;
				cfg.approximateError = 0.1;
				cfg.minEdgeLength = 0.1;
				cfg.maxEdgeLength = 3;
#ifdef CAGE_DEBUG
				cfg.iterations = 2;
#endif // CAGE_DEBUG
				meshSimplify(+msh, cfg);
			}
			CAGE_LOG(SeverityEnum::Info, "mapgen", stringizer() + "mesh faces: " + msh->facesCount());
			{
				MeshChunkingConfig cfg;
				cfg.maxSurfaceArea = 300;
				meshes = meshChunking(+msh, cfg);
			}
			CAGE_LOG(SeverityEnum::Info, "mapgen", stringizer() + "mesh chunks: " + meshes.size());
			tasksRunBlocking<const Holder<Mesh>>("make chunk", Delegate<void(const Holder<Mesh> &)>().bind<Maker, &Maker::makeChunk>(this), *meshes, -10);
		}
	};

	void mapGenThrEntry()
	{
		CAGE_LOG(SeverityEnum::Info, "mapgen", "generating new map");
		{
			CAGE_ASSERT(!globalGrid);
			Holder<Procedural> procedural = newProcedural();
			Holder<Grid> grid = newGrid(procedural.share());
			Holder<Waypoints> paths = newWaypoints(grid.share());
			Maker maker;
			maker.procedural = procedural.share();
			maker.grid = grid.share();
			maker.makeMeshes();
			Holder<Collider> collider = newCollider();
			collider->importMesh(+maker.msh);
			collider->rebuild();
			globalCollider = std::move(collider);
			globalWaypoints = std::move(paths);
			globalGrid = std::move(grid); // this is last as it is frequently used to detect whether a map is loaded
		}
		CAGE_LOG(SeverityEnum::Info, "mapgen", "map generation done");
	}

	Holder<Thread> mapGenThread;

	void engineFinish()
	{
		mapGenThread.clear();
	}

	void gameReset()
	{
		globalGrid.clear();
		globalWaypoints.clear();
		globalCollider.clear();
		mapGenThread = newThread(Delegate<void()>().bind<&mapGenThrEntry>(), "map gen");
	}

	struct Callbacks
	{
		EventListener<void()> engineFinishListener;
		EventListener<void()> gameResetListener;

		Callbacks()
		{
			engineFinishListener.attach(controlThread().finalize);
			engineFinishListener.bind<&engineFinish>();
			gameResetListener.attach(eventGameReset(), -80);
			gameResetListener.bind<&gameReset>();
		}
	} callbacksInstance;
}

Holder<Grid> globalGrid;
Holder<Waypoints> globalWaypoints;
Holder<Collider> globalCollider;
