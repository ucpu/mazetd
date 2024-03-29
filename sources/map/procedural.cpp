#include "generate.h"

#include <cage-core/color.h>
#include <cage-core/noiseFunction.h>
#include <cage-core/random.h>

namespace mazetd
{
	namespace
	{
		struct ProceduralImpl : public Procedural
		{
			const Vec2 elevationOffset = randomChance2() * 1000;

			Holder<NoiseFunction> elevationNoise = []()
			{
				NoiseFunctionCreateConfig cfg;
				cfg.type = NoiseTypeEnum::Simplex;
				cfg.fractalType = NoiseFractalTypeEnum::Ridged;
				cfg.octaves = 4;
				cfg.gain = 0.55;
				cfg.lacunarity = 2.1;
				cfg.frequency = 0.003;
				cfg.seed = detail::randomGenerator().next();
				return newNoiseFunction(cfg);
			}();

			Holder<NoiseFunction> waterNoise = []()
			{
				NoiseFunctionCreateConfig cfg;
				cfg.type = NoiseTypeEnum::Simplex;
				cfg.frequency = 0.03;
				cfg.seed = detail::randomGenerator().next();
				return newNoiseFunction(cfg);
			}();

			Holder<NoiseFunction> grassNoise = []()
			{
				NoiseFunctionCreateConfig cfg;
				cfg.type = NoiseTypeEnum::Perlin;
				cfg.fractalType = NoiseFractalTypeEnum::PingPong;
				cfg.frequency = 0.1;
				cfg.seed = detail::randomGenerator().next();
				return newNoiseFunction(cfg);
			}();

			Holder<NoiseFunction> dirtNoise = []()
			{
				NoiseFunctionCreateConfig cfg;
				cfg.type = NoiseTypeEnum::Perlin;
				cfg.frequency = 0.2;
				cfg.seed = detail::randomGenerator().next();
				return newNoiseFunction(cfg);
			}();

			Holder<NoiseFunction> roughnessNoise = []()
			{
				NoiseFunctionCreateConfig cfg;
				cfg.type = NoiseTypeEnum::Perlin;
				cfg.frequency = 0.5;
				cfg.seed = detail::randomGenerator().next();
				return newNoiseFunction(cfg);
			}();

			Real elevation(const Vec2 &pos) override
			{
				const Real base = -sqr(max(length(pos) - 30, 0) * 0.2);
				const Real noise = elevationNoise->evaluate(pos + elevationOffset) * 10;
				return max(base + noise, -8.5);
			}

			Real slope(const Vec2 &pos, Real off)
			{
				const Real a = elevation(pos + Vec2(-off, 0)) - elevation(pos + Vec2(+off, 0));
				const Real b = elevation(pos + Vec2(0, -off)) - elevation(pos + Vec2(0, +off));
				return max(abs(a), abs(b)) / off;
			}

			void material(const Vec3 &pos3, TileFlags flags, Vec3 &albedo, Real &roughness) override
			{
				const Vec2 pos2 = Vec2(pos3[0], pos3[2]);
				const Real elev = pos3[1];
				{
					const Real noise = waterNoise->evaluate(pos3) * 0.25;
					albedo = colorHsvToRgb(Vec3(0.644, 0.52, 0.75 + noise));
					roughness = roughnessNoise->evaluate(pos3) * 0.1 + 0.2;
				}
				{
					const Real noise = grassNoise->evaluate(pos3) * 0.1;
					const Vec3 grass = colorHsvToRgb(Vec3(0.266 + randomChance() * 0.1, 0.85, 0.8 + noise));
					const Real factor = saturate(find(elev, -7.2, -6.8));
					albedo = interpolate(albedo, grass, factor);
					const Real r = roughnessNoise->evaluate(pos3 * 5 + 156) * 0.2 + 0.45;
					roughness = interpolate(roughness, r, factor);
				}
				{
					const Real noise = dirtNoise->evaluate(pos3) * 0.2;
					const Vec3 dirt = colorHsvToRgb(Vec3(0.135, 0.75 + noise, 0.75 + randomChance() * 0.07));
					const Real factor = saturate(find(elev, 0.8, 1.2));
					albedo = interpolate(albedo, dirt, factor);
					const Real r = roughnessNoise->evaluate(pos3 * 2 - 564) * 0.3 + 0.6;
					roughness = interpolate(roughness, r, factor);
				}
				{
					const Vec3 snow = Vec3(randomChance() * 0.1 + 0.9);
					const Real factor = saturate(find(elev, 5.8, 6.2));
					albedo = interpolate(albedo, snow, factor);
					const Real r = randomChance() * 0.3 + 0.7;
					roughness = interpolate(roughness, r, factor);
				}
				{
					const Vec2i tile = Vec2i((pos2 + 1000) * 10) % 10;
					if (tile[0] == 5 || tile[1] == 5)
					{
						constexpr Real factor = 0.3;
						albedo = interpolate(albedo, Vec3(0.5), factor);
						roughness = interpolate(roughness, 1, factor);
					}
				}
				if (any(flags & TileFlags::Invalid))
				{
					constexpr Real factor = 0.2;
					albedo = interpolate(albedo, Vec3(), factor);
					roughness = interpolate(roughness, 1, factor);
				}
				if (any(flags & TileFlags::Waypoint))
				{
					constexpr Real factor = 0.5;
					albedo = interpolate(albedo, Vec3(1, 0, 1), factor);
					roughness = interpolate(roughness, 1, factor);
				}
			}

			Real sdf(const Vec3 &pos) override { return elevation(Vec2(pos[0], pos[2])) - pos[1]; }
		};
	}

	Holder<Procedural> newProcedural()
	{
		return systemMemory().createImpl<Procedural, ProceduralImpl>();
	}
}
