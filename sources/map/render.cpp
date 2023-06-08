#include <cage-core/assetManager.h>
#include <cage-core/entities.h>
#include <cage-core/meshImport.h>
#include <cage-core/profiling.h>
#include <cage-core/serialization.h>
#include <cage-engine/assetStructs.h>
#include <cage-engine/model.h>
#include <cage-engine/opengl.h>
#include <cage-engine/scene.h>
#include <cage-engine/texture.h>
#include <cage-simple/engine.h>

#include "generate.h"

#include <vector>

ConcurrentQueue<ChunkUpload> chunksUploadQueue;

namespace
{
	struct ChunkRender : private Noncopyable
	{
		uint32 model = 0;
		uint32 albedo = 0;
		uint32 material = 0;

		void remove()
		{
			if (model)
				engineAssets()->remove(model);
			if (albedo)
				engineAssets()->remove(albedo);
			if (material)
				engineAssets()->remove(material);
			model = albedo = material = 0;
		}
	};

	ConcurrentQueue<ChunkRender> chunksRenderQueue;

	const auto graphicsDispatchListener = graphicsDispatchThread().dispatch.listen(
		[]()
		{
			ProfilingScope profiling("dispatch chunks");

			for (uint32 iter = 0; iter < 5; iter++)
			{
				ChunkUpload up;
				ChunkRender re;
				if (!chunksUploadQueue.tryPop(up))
					return;

				if (!up.mesh)
				{
					chunksRenderQueue.push(std::move(re));
					return;
				}

				{
					ProfilingScope profiling("dispatch albedo");
					Holder<Texture> tex = newTexture();
					tex->importImage(+up.albedo);
					tex->filters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, 100);
					tex->wraps(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
					tex->generateMipmaps();
					re.albedo = engineAssets()->generateUniqueName();
					engineAssets()->fabricate<AssetSchemeIndexTexture, Texture>(re.albedo, std::move(tex), "chunkAlbedo");
				}

				{
					ProfilingScope profiling("dispatch material");
					Holder<Texture> tex = newTexture();
					tex->importImage(+up.material);
					tex->filters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, 100);
					tex->wraps(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
					tex->generateMipmaps();
					re.material = engineAssets()->generateUniqueName();
					engineAssets()->fabricate<AssetSchemeIndexTexture, Texture>(re.material, std::move(tex), "chunkMaterial");
				}

				{
					ProfilingScope profiling("dispatch model");
					Holder<Model> model = newModel();
					MeshImportMaterial mat;
					model->importMesh(+up.mesh, bufferView(mat));
					model->textureNames[0] = re.albedo;
					model->textureNames[1] = re.material;
					model->layer = -100;
					re.model = engineAssets()->generateUniqueName();
					engineAssets()->fabricate<AssetSchemeIndexModel, Model>(re.model, std::move(model), "chunkModel");
				}

				chunksRenderQueue.push(std::move(re));
			}
		});

	std::vector<ChunkRender> renderingChunks;

	void removeAllAssets()
	{
		for (auto &it : renderingChunks)
			it.remove();
		renderingChunks.clear();
	}

	const auto engineUpdateListener = controlThread().update.listen(
		[]()
		{
			ProfilingScope profiling("update chunks");

			for (uint32 iter = 0; iter < 5; iter++)
			{
				ChunkRender re;
				if (!chunksRenderQueue.tryPop(re))
					return;

				if (re.model == 0)
				{
					removeAllAssets();
					return;
				}

				Entity *e = engineEntities()->createAnonymous();
				TransformComponent &t = e->value<TransformComponent>();
				RenderComponent &r = e->value<RenderComponent>();
				r.object = re.model;

				renderingChunks.push_back(std::move(re));
			}
		});

	const auto engineFinishListener = controlThread().finalize.listen([]() { removeAllAssets(); });

	const auto engineUnloadListener = controlThread().unload.listen(
		[]()
		{
			ChunkRender re;
			if (chunksRenderQueue.tryPop(re))
				re.remove();
		});
}
