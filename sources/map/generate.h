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
	virtual real elevation(const vec2 &pos) = 0;
	virtual real sdf(const vec3 &pos) = 0;
	virtual void material(const vec3 &position, TileFlags flags, vec3 &albedo, real &roughness) = 0;
};

Holder<Procedural> newProcedural();

Holder<Grid> newGrid(Holder<Procedural> procedural);
