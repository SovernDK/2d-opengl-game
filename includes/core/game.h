#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_keyboard.h>

#include <memory>
#include <functional>

#include "core/fsm/game_flow_fsm.h"
#include "core/app_state.h"
#include "core/context.h"

#include "graphics/camera/camera.h"
#include "services/scene_service.h"
#include "services/scene.h"
#include "gameplay/historia.h"

#include "ecs/ecs.h"
#include "editor/editor.h"

namespace mem { class Arena; }

namespace core
{
	class Game : public IContext
	{
	private:
		bool initialized = false;

		std::unique_ptr<ecs::ECSWorld> world     = nullptr;
		std::unique_ptr<ICamera> mainCam         = nullptr;
		std::unique_ptr<editor::Editor> m_editor = nullptr;
		std::unique_ptr<historia::Story> m_story = nullptr;

		struct Callback
		{
			std::function<void()> fn;
			Timer timer{ 0.0f };
		};

		std::vector<Callback> callbacks;
	public:
		int screenWidth    = 0,
			screenHeight   = 0;

		int viewportWidth  = 0,
			viewportHeight = 0;

		mem::Arena* arena;
	public:
		Game() = default;
		Game(int width, int height)
		{
			this->viewportWidth  = width;
			this->viewportHeight = height;

			this->screenWidth    = width;
			this->screenHeight   = height;
		}

		void init(AppState& state);
		void update(float dt);
		void input(SDL_Event& e, float dt) const;
		void draw(float dt) const;
		void quit() const;

		void resize(int viewportWidth, int viewportHeight) const;
		void callback(float delay, std::function<void()> func) override
		{
			callbacks.push_back({ func, Timer{ delay } });
		};
		void processCallbacks(const float dt);

		ecs::ECSWorld* ecsWorld() override { return world.get(); }
		ICamera* mainCamera() override     { return mainCam.get(); }
		editor::Editor* editor() override  { return m_editor.get(); }
		historia::Story* story() override  { return m_story.get(); }

		glm::vec2 screenDim() const { return glm::vec2(this->screenWidth, this->screenHeight); }
		glm::vec2 viewport() const { return glm::vec2(this->viewportWidth, this->viewportHeight); }
	};
}