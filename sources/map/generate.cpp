#include <cage-core/geometry.h>
#include <cage-core/marchingCubes.h>
#include <cage-core/noiseFunction.h>
#include <cage-core/threadPool.h>
#include <cage-core/random.h>

#include "generate.h"

namespace
{
	vec2 elevationOffset = randomChance2() * 1000;

	Holder<NoiseFunction> elevationNoise = [](){
		NoiseFunctionCreateConfig cfg;
		cfg.type = NoiseTypeEnum::Simplex;
		cfg.fractalType = NoiseFractalTypeEnum::Ridged;
		cfg.octaves = 4;
		cfg.gain = 0.55;
		cfg.lacunarity = 2.1;
		cfg.frequency = 0.003;
		cfg.seed = detail::globalRandomGenerator().next();
		return newNoiseFunction(cfg);
	}();

	real elevation(const vec2 &pos)
	{
		const real base = -sqr(max(length(pos) - 50, 0) * 0.2);
		const real noise = elevationNoise->evaluate(pos + elevationOffset) * 10;
		return base + noise;
	}

	real slope(const vec2 &pos, real off)
	{
		const real a = elevation(pos + vec2(-off, 0)) - elevation(pos + vec2(+off, 0));
		const real b = elevation(pos + vec2(0, -off)) - elevation(pos + vec2(0, +off));
		return max(abs(a), abs(b)) / off;
	}

	real find(real value, real lower, real upper)
	{
		return (value - lower) / (upper - lower);
	}

	using ChunkPtr = ChunkUpload *;

	void makeMaterial(ChunkPtr chunk, const ivec2 &xy, const ivec3 &ids, const vec3 &weights)
	{
		const vec3 pos3 = chunk->mesh->positionAt(ids, weights);
		const vec2 pos2 = vec2(pos3[0], pos3[2]);
		const real elev = elevation(pos2);
		const real spikiness = abs(slope(pos2, 0.3) - slope(pos2, 1));
		vec3 albedo = vec3(0, 0, 1);
		real roughness = 0.5;
		{
			const vec3 grass = vec3(0, 1, 0);
			const real factor = saturate(find(elev, -8, -6));
			albedo = interpolate(albedo, grass, factor);
		}
		{
			const vec3 dirt = vec3(1, 0, 0);
			const real factor = saturate(find(elev, 1, 3));
			albedo = interpolate(albedo, dirt, factor);
		}
		{
			const vec3 snow = vec3(1);
			const real factor = saturate(find(elev, 5, 7));
			albedo = interpolate(albedo, snow, factor);
		}
		{
			const vec3 rock = vec3(0.3);
			const real factor = saturate(find(spikiness, 0.2, 0.3));
			albedo = interpolate(albedo, rock, factor);
		}
		{
			const ivec2 tile = ivec2(20 * pos2 + 1000) % 20;
			if (tile[0] == 0 || tile[1] == 0)
				albedo = interpolate(albedo, vec3(0.5), 0.3);
		}
		chunk->albedo->set(xy, albedo);
		chunk->material->set(xy, roughness);
	}

	void makeChunk(Holder<Mesh> &msh)
	{
		uint32 resolution = 0;
		{
			MeshUnwrapConfig cfg;
#ifdef CAGE_DEBUG
			cfg.texelsPerUnit = 20;
#else
			cfg.texelsPerUnit = 50;
#endif // CAGE_DEBUG
			resolution = meshUnwrap(+msh, cfg);
		}
		CAGE_LOG(SeverityEnum::Info, "mapgen", stringizer() + "chunk unwrapped, resolution: " + resolution);
		ChunkUpload chunk;
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

	real sdf(const vec3 &pos)
	{
		return elevation(vec2(pos[0], pos[2])) - pos[1];
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
			mc->updateByPosition(Delegate<real(const vec3 &)>().bind<&sdf>());
			msh = mc->makeMesh();
		}
		CAGE_LOG(SeverityEnum::Info, "mapgen", stringizer() + "base mesh: faces: " + msh->facesCount());
		meshDiscardDisconnected(+msh);
		{
			MeshSimplifyConfig cfg;
			cfg.minEdgeLength = 0.1;
			cfg.maxEdgeLength = 2;
			meshSimplify(+msh, cfg);
		}
		CAGE_LOG(SeverityEnum::Info, "mapgen", stringizer() + "simplified mesh: faces: " + msh->facesCount());
		msh->exportObjFile({}, "mesh.obj");
		Holder<PointerRange<Holder<Mesh>>> meshes;
		{
			MeshChunkingConfig cfg;
			cfg.maxSurfaceArea = 300;
			meshes = meshChunking(+msh, cfg);
			msh.clear();
		}
		CAGE_LOG(SeverityEnum::Info, "mapgen", stringizer() + "chunked mesh: parts: " + meshes.size());
		Holder<ThreadPool> thrs = newThreadPool();
		thrs->function.bind<MeshesPtr, &threadEntry>(+meshes);
		thrs->run();
	}
}

void mapGenerate()
{
	CAGE_LOG(SeverityEnum::Info, "mapgen", "generating new map");
	makeMeshes();
	// todo make grid
	CAGE_LOG(SeverityEnum::Info, "mapgen", "map generation done");
}
