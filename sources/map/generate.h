#include <cage-core/mesh.h>
#include <cage-core/image.h>
#include <cage-core/concurrentQueue.h>

using namespace cage;

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
	virtual void material(ChunkUpload *chunk, const ivec2 &xy, const ivec3 &ids, const vec3 &weights) = 0;
};

Holder<Procedural> newProcedural();
