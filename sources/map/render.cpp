#include <cage-core/assetManager.h>
#include <cage-core/serialization.h>
#include <cage-core/entities.h>
#include <cage-engine/engine.h>
#include <cage-engine/graphics.h>
#include <cage-engine/assetStructs.h>
#include <cage-engine/opengl.h>

#include "generate.h"

#include <vector>

ConcurrentQueue<ChunkUpload> chunksUploadQueue;

namespace
{
	void graphicsDispatch();
	void engineUpdate();
	void engineFinish();
	void engineUnloading();

	struct Callbacks
	{
		EventListener<void()> graphicsDispatchListener;
		EventListener<void()> engineUpdateListener;
		EventListener<void()> engineFinishListener;
		EventListener<void()> engineUnloadingListener;

		Callbacks()
		{
			graphicsDispatchListener.attach(graphicsDispatchThread().dispatch);
			graphicsDispatchListener.bind<&graphicsDispatch>();
			engineUpdateListener.attach(controlThread().update);
			engineUpdateListener.bind<&engineUpdate>();
			engineFinishListener.attach(controlThread().finalize);
			engineFinishListener.bind<&engineFinish>();
			engineUnloadingListener.attach(controlThread().unload);
			engineUnloadingListener.bind<&engineUnloading>();
		}
	} callbacksInstance;

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

	void graphicsDispatch()
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
			Holder<Texture> tex = newTexture();
			tex->importImage(+up.albedo);
			tex->filters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, 100);
			tex->wraps(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			tex->generateMipmaps();
			re.albedo = engineAssets()->generateUniqueName();
			engineAssets()->fabricate<AssetSchemeIndexTexture, Texture>(re.albedo, std::move(tex), "chunkAlbedo");
		}

		{
			Holder<Texture> tex = newTexture();
			tex->importImage(+up.material);
			tex->filters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, 100);
			tex->wraps(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			tex->generateMipmaps();
			re.material = engineAssets()->generateUniqueName();
			engineAssets()->fabricate<AssetSchemeIndexTexture, Texture>(re.material, std::move(tex), "chunkMaterial");
		}

		{
			Holder<Model> model = newModel();
			ModelHeader::MaterialData mat;
			model->importMesh(+up.mesh, bufferView(mat));
			model->setTextureName(0, re.albedo);
			model->setTextureName(1, re.material);
			re.model = engineAssets()->generateUniqueName();
			engineAssets()->fabricate<AssetSchemeIndexModel, Model>(re.model, std::move(model), "chunkModel");
		}

		chunksRenderQueue.push(std::move(re));
	}

	std::vector<ChunkRender> renderingChunks;

	void removeAllAssets()
	{
		for (auto &it : renderingChunks)
			it.remove();
		renderingChunks.clear();
	}

	void engineUpdate()
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
		CAGE_COMPONENT_ENGINE(Transform, t, e);
		CAGE_COMPONENT_ENGINE(Render, r, e);
		r.object = re.model;

		renderingChunks.push_back(std::move(re));
	}

	void engineFinish()
	{
		removeAllAssets();
	}

	void engineUnloading()
	{
		ChunkRender re;
		if (chunksRenderQueue.tryPop(re))
			re.remove();
	}
}
