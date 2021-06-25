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
