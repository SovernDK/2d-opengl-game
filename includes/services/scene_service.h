#include "services/scene.h"
#include "services/service.h"

#include <typeindex>

enum class TransitionMode
{
	Replace, Additive, PopBack
};

struct TransitionRequest
{
	std::type_index sceneId;
	TransitionMode mode;
};

class ISceneService : public IService 
{
protected:
	virtual void requestTransition(const std::type_index& id, const TransitionMode mode) = 0;
	virtual void registerScene(const std::type_index& type, std::shared_ptr<IScene> scene) = 0;

	virtual void unloadScene() = 0;
	virtual void addScene(TransitionRequest req) = 0;

	core::IContext* ctx = nullptr;
public:
	virtual std::vector<std::shared_ptr<IScene>> activeScenes() = 0;
public:
	virtual ~ISceneService() = default;

	virtual void registerContext(core::IContext* ctx) = 0;
	virtual void requestRemoveLast() = 0;
	virtual void requestClearAll() = 0;

	virtual void update(float dt) = 0;
	virtual void draw() = 0;
	virtual void quit() = 0;

	template<typename T>
	void registerScene(std::string name)
	{
		registerScene(typeid(T), std::make_shared<T>(ctx, name));
	}

	template<typename T>
	void requestTransition(const TransitionMode mode)
	{
		requestTransition(typeid(T), mode);
	}
};

class SceneService : public ISceneService
{
public:
	std::unordered_map<std::type_index, std::shared_ptr<IScene>> scenes;
	std::deque<std::shared_ptr<IScene>> m_activeScenes;

	std::deque<TransitionRequest> pending;
protected:
	void requestTransition(const std::type_index& id, const TransitionMode mode) override
	{
		pending.push_back(TransitionRequest{ .sceneId = id, .mode = mode });
	}

	void registerScene(const std::type_index& type, std::shared_ptr<IScene> scene) override
	{
		scenes[type] = std::move(scene);
	}

	void unloadScene() override
	{
		// In unlikely case unloading of a scene is called twice (debugging) dont 
		// double the action
		if (m_activeScenes.empty()) 
			return;

		m_activeScenes.back()->unload();
		m_activeScenes.pop_back();
	}

	void addScene(TransitionRequest req) override
	{
		auto& newScene = scenes[req.sceneId];
		assert(newScene != nullptr); // Forgot to register scene

		m_activeScenes.push_back(newScene);
		newScene->start();
	}
public:
	SceneService() {};

	~SceneService()
	{
		for (auto& [_, scene] : scenes)
		{
			scene->quit();
		}

		m_activeScenes.clear();
		scenes.clear();
	}

	void registerContext(core::IContext* ctx) override
	{
		this->ctx = ctx;
	}

	void requestRemoveLast() override
	{
		pending.push_back(TransitionRequest{ .sceneId = typeid(*this), .mode = TransitionMode::PopBack });
	}

	void requestClearAll() override
	{
		for (auto& scene : m_activeScenes)
		{
			requestRemoveLast();
		}
	}

	void update(float dt) override
	{
		for (auto& scene : m_activeScenes)
		{
			scene->update(dt);
		}

		for (auto& request : pending)
		{
			switch (request.mode)
			{
			case TransitionMode::Replace:
				unloadScene();
				addScene(request);
				break;
			case TransitionMode::Additive:
				addScene(request);
				break;
			case TransitionMode::PopBack:
				unloadScene();
				break;
			}
		}

		pending.clear();
	}

	void draw() override
	{
		for (auto& scene : m_activeScenes)
		{
			scene->draw();
		}
	}

	void quit() override
	{
		for (auto& [_, scene] : scenes)
		{
			scene->quit();
		}

		m_activeScenes.clear();
		scenes.clear();
	}

	std::vector<std::shared_ptr<IScene>> activeScenes() override
	{
		std::vector<std::shared_ptr<IScene>> out;

		for (auto& scene : m_activeScenes)
		{
			out.push_back(scene);
		}

		return out;
	}
};