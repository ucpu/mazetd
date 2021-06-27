#include <cage-core/noiseFunction.h>
#include <cage-core/random.h>
#include <cage-core/color.h>

#include "generate.h"

namespace
{
	struct ProceduralImpl : public Procedural
	{
		vec2 elevationOffset = randomChance2() * 1000;

		Holder<NoiseFunction> elevationNoise = []() {
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

		Holder<NoiseFunction> waterNoise = []() {
			NoiseFunctionCreateConfig cfg;
			cfg.type = NoiseTypeEnum::Simplex;
			cfg.frequency = 0.08;
			cfg.seed = detail::globalRandomGenerator().next();
			return newNoiseFunction(cfg);
		}();

		Holder<NoiseFunction> grassNoise = []() {
			NoiseFunctionCreateConfig cfg;
			cfg.type = NoiseTypeEnum::Perlin;
			cfg.fractalType = NoiseFractalTypeEnum::PingPong;
			cfg.frequency = 0.1;
			cfg.seed = detail::globalRandomGenerator().next();
			return newNoiseFunction(cfg);
		}();

		Holder<NoiseFunction> dirtNoise = []() {
			NoiseFunctionCreateConfig cfg;
			cfg.type = NoiseTypeEnum::Perlin;
			cfg.frequency = 0.2;
			cfg.seed = detail::globalRandomGenerator().next();
			return newNoiseFunction(cfg);
		}();

		Holder<NoiseFunction> roughnessNoise = []() {
			NoiseFunctionCreateConfig cfg;
			cfg.type = NoiseTypeEnum::Perlin;
			cfg.frequency = 0.5;
			cfg.seed = detail::globalRandomGenerator().next();
			return newNoiseFunction(cfg);
		}();

		real elevation(const vec2 &pos) override
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

		void material(const vec3 &pos3, TileFlags flags, vec3 &albedo, real &roughness) override
		{
			const vec2 pos2 = vec2(pos3[0], pos3[2]);
			const real elev = elevation(pos2);
			const real spikiness = abs(slope(pos2, 0.3) - slope(pos2, 1));
			{
				const real noise = waterNoise->evaluate(pos3) * 0.07;
				albedo = colorHsvToRgb(vec3(0.644 + noise, 0.52, 0.75));
				roughness = roughnessNoise->evaluate(pos3) * 0.1 + 0.2;
			}
			{
				const real noise = grassNoise->evaluate(pos3) * 0.1;
				const vec3 grass = colorHsvToRgb(vec3(0.266, 0.6, 0.87 + noise));
				const real factor = saturate(find(elev, -8, -6));
				albedo = interpolate(albedo, grass, factor);
				const real r = roughnessNoise->evaluate(pos3 + 156) * 0.2 + 0.45;
				roughness = interpolate(roughness, r, factor);
			}
			{
				const real noise = dirtNoise->evaluate(pos3) * 0.2;
				const vec3 dirt = colorHsvToRgb(vec3(0.144, 0.45 + noise, 0.8));
				const real factor = saturate(find(elev, 0, 2));
				albedo = interpolate(albedo, dirt, factor);
				const real r = roughnessNoise->evaluate(pos3 - 564) * 0.3 + 0.6;
				roughness = interpolate(roughness, r, factor);
			}
			{
				const vec3 snow = vec3(randomChance() * 0.1 + 0.9);
				const real factor = saturate(find(elev, 5, 7));
				albedo = interpolate(albedo, snow, factor);
				const real r = randomChance() * 0.3 + 0.7;
				roughness = interpolate(roughness, r, factor);
			}
			{
				const vec3 rock = vec3(0.3);
				const real factor = saturate(find(spikiness, 0.2, 0.3)) * 0.3;
				albedo = interpolate(albedo, rock, factor);
				roughness = interpolate(roughness, 0.8, factor);
			}
			{
				const ivec2 tile = ivec2(20 * pos2 + 1000) % 20;
				if (tile[0] == 0 || tile[1] == 0)
				{
					albedo = interpolate(albedo, vec3(0.5), 0.3);
					roughness = interpolate(roughness, 1, 0.3);
				}
			}
		}

		real sdf(const vec3 &pos) override
		{
			return elevation(vec2(pos[0], pos[2])) - pos[1];
		}
	};
}

Holder<Procedural> newProcedural()
{
	return systemMemory().createImpl<Procedural, ProceduralImpl>();
}
