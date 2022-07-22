#include <cage-core/tasks.h>
#include <cage-core/geometry.h>
#include <cage-core/marchingCubes.h>
#include <cage-core/mesh.h>
#include <cage-core/image.h>
#include <cage-core/collider.h>
#include <cage-core/profiling.h>
#include <cage-simple/engine.h>

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

			void makeMaterial(const Vec2i &xy, const Vec3i &ids, const Vec3 &weights)
			{
				const Vec3 pos3 = mesh->positionAt(ids, weights);
				const uint32 index = maker->grid->index(pos3);
				const TileFlags flags = index == m ? TileFlags::Invalid : maker->grid->flags[index];
				Vec3 color;
				Real roughness;
				maker->procedural->material(pos3, flags, color, roughness);
				albedo->set(xy, color);
				material->set(xy, roughness);
			}
		};

		void makeChunk(const Holder<Mesh> &msh)
		{
			ProfilingScope profiling("make chunk");

			uint32 resolution = 0;
			{
				ProfilingScope profiling("chunk unwrap");
				MeshUnwrapConfig cfg;
#ifdef CAGE_DEBUG
				cfg.texelsPerUnit = 10;
#else
				cfg.texelsPerUnit = 20;
#endif // CAGE_DEBUG
				resolution = meshUnwrap(+msh, cfg);
			}
			ChunkMaterial chunk;
			chunk.maker = this;
			chunk.mesh = msh.share();
			chunk.albedo = newImage();
			chunk.albedo->initialize(Vec2i(resolution), 3);
			chunk.albedo->colorConfig.gammaSpace = GammaSpaceEnum::Gamma;
			chunk.material = newImage();
			chunk.material->initialize(Vec2i(resolution), 1);
			chunk.material->colorConfig.gammaSpace = GammaSpaceEnum::None;
			{
				ProfilingScope profiling("chunk generate texture");
				MeshGenerateTextureConfig cfg;
				cfg.width = cfg.height = resolution;
				cfg.generator.bind<ChunkMaterial, &ChunkMaterial::makeMaterial>(&chunk);
				meshGenerateTexture(+msh, cfg);
			}
			{
				ProfilingScope profiling("chunk image dilation");
#ifdef CAGE_DEBUG
				static constexpr uint32 rounds = 2;
#else
				static constexpr uint32 rounds = 4;
#endif // CAGE_DEBUG
				imageDilation(+chunk.albedo, rounds);
				imageDilation(+chunk.material, rounds);
			}
			chunksUploadQueue.push(std::move(chunk));
		}

		Real sdf(const Vec3 &pos)
		{
			Real d = procedural->sdf(pos);
			const uint32 i = grid->index(pos);
			if (i != m)
			{
				const TileFlags f = grid->flags[i];
				if (any(f & TileFlags::Invalid) && none(f & TileFlags::Water))
					d += saturate(find(pos[1], -7.5, -6.5));
			}
			return d;
		}

		void makeMeshes()
		{
			ProfilingScope profiling("make meshes");

			{ // reset rendering queues and assets
				ChunkUpload chunk;
				chunksUploadQueue.push(std::move(chunk));
			}
			{
				ProfilingScope profiling("marching cubes");
				MarchingCubesCreateConfig cfg;
				cfg.box = Aabb(Vec3(-55, -15, -55), Vec3(55, 15, 55));
				cfg.clip = true;
#ifdef CAGE_DEBUG
				cfg.resolution = Vec3i(cfg.box.size() * 0.5);
#else
				cfg.resolution = Vec3i(cfg.box.size() * 2);
#endif // CAGE_DEBUG
				Holder<MarchingCubes> mc = newMarchingCubes(cfg);
				{
					ProfilingScope profiling("sdf");
					mc->updateByPosition(Delegate<Real(const Vec3 &)>().bind<Maker, &Maker::sdf>(this));
				}
				{
					ProfilingScope profiling("mesh");
					msh = mc->makeMesh();
				}
			}
			{
				ProfilingScope profiling("discard disconnected");
				meshDiscardDisconnected(+msh);
			}
			{
				ProfilingScope profiling("mesh simplification");
				MeshSimplifyConfig cfg;
				cfg.approximateError = 0.1;
				cfg.minEdgeLength = 0.2;
				cfg.maxEdgeLength = 2;
#ifdef CAGE_DEBUG
				cfg.iterations = 1;
#else
				cfg.iterations = 3;
#endif // CAGE_DEBUG
				meshSimplify(+msh, cfg);
			}
			CAGE_LOG(SeverityEnum::Info, "mapgen", Stringizer() + "mesh faces: " + msh->facesCount());
			{
				ProfilingScope profiling("mesh chunking");
				MeshChunkingConfig cfg;
				cfg.maxSurfaceArea = 500;
				meshes = meshChunking(+msh, cfg);
			}
			CAGE_LOG(SeverityEnum::Info, "mapgen", Stringizer() + "mesh chunks: " + meshes.size());
			tasksRunBlocking<const Holder<Mesh>>("make chunk", Delegate<void(const Holder<Mesh> &)>().bind<Maker, &Maker::makeChunk>(this), *meshes, tasksCurrentPriority());
		}
	};

	void mapGenTaskEntry(uint32)
	{
		CAGE_LOG(SeverityEnum::Info, "mapgen", "generating new map");
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
		globalGrid = std::move(grid); // this is last as it is frequently used to detect whether the map is loaded
		CAGE_LOG(SeverityEnum::Info, "mapgen", "map generation done");
	}

	Holder<AsyncTask> mapGenTask;

	void engineFinish()
	{
		if (mapGenTask)
		{
			mapGenTask->wait();
			mapGenTask.clear();
		}
	}

	void gameReset()
	{
		globalGrid.clear();
		globalWaypoints.clear();
		globalCollider.clear();
		mapGenTask = tasksRunAsync("map gen task", Delegate<void(uint32)>().bind<&mapGenTaskEntry>());
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
