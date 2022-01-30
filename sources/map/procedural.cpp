#include <cage-core/noiseFunction.h>
#include <cage-core/random.h>
#include <cage-core/color.h>

#include "generate.h"

namespace
{
	struct ProceduralImpl : public Procedural
	{
		Vec2 elevationOffset = randomChance2() * 1000;

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
				const Real noise = waterNoise->evaluate(pos3) * 0.07;
				albedo = colorHsvToRgb(Vec3(0.644 + noise, 0.52, 0.75));
				roughness = roughnessNoise->evaluate(pos3) * 0.1 + 0.2;
			}
			{
				const Real noise = grassNoise->evaluate(pos3) * 0.1;
				const Vec3 grass = colorHsvToRgb(Vec3(0.266, 0.6, 0.87 + noise));
				const Real factor = saturate(find(elev, -7.2, -6.8));
				albedo = interpolate(albedo, grass, factor);
				const Real r = roughnessNoise->evaluate(pos3 + 156) * 0.2 + 0.45;
				roughness = interpolate(roughness, r, factor);
			}
			{
				const Real noise = dirtNoise->evaluate(pos3) * 0.2;
				const Vec3 dirt = colorHsvToRgb(Vec3(0.144, 0.45 + noise, 0.8));
				const Real factor = saturate(find(elev, 0.8, 1.2));
				albedo = interpolate(albedo, dirt, factor);
				const Real r = roughnessNoise->evaluate(pos3 - 564) * 0.3 + 0.6;
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
				const Vec2i tile = Vec2i((pos2 + 1000) * 15) % 15;
				if (tile[0] == 7 || tile[1] == 7)
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

		Real sdf(const Vec3 &pos) override
		{
			return elevation(Vec2(pos[0], pos[2])) - pos[1];
		}
	};
}

Holder<Procedural> newProcedural()
{
	return systemMemory().createImpl<Procedural, ProceduralImpl>();
}
