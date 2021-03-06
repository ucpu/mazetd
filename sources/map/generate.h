#include <cage-core/concurrentQueue.h>

#include "../grid.h"

struct ChunkUpload : private Noncopyable
{
	Holder<Mesh> mesh;
	Holder<Image> albedo;
	Holder<Image> material;
};

extern ConcurrentQueue<ChunkUpload> chunksUploadQueue;

struct Procedural : private Immovable
{
	virtual ~Procedural() = default;
	virtual Real elevation(const Vec2 &pos) = 0;
	virtual Real sdf(const Vec3 &pos) = 0;
	virtual void material(const Vec3 &position, TileFlags flags, Vec3 &albedo, Real &roughness) = 0;
};

Holder<Procedural> newProcedural();
Holder<Grid> newGrid(Holder<Procedural> procedural);
Holder<Waypoints> newWaypoints(Holder<Grid> grid);

inline Real find(Real value, Real lower, Real upper)
{
	return (value - lower) / (upper - lower);
}
