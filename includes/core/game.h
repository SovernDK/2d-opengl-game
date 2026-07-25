#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_keyboard.h>

#include <memory>

#include "core/fsm/game_flow_fsm.h"
#include "core/app_state.h"
#include "core/context.h"

#include "graphics/camera/camera.h"
#include "services/scene_service.h"
#include "services/scene.h"

#include "ecs/ecs.h"
#include "editor/editor.h"

namespace mem { class Arena; }

namespace core
{
	class Game : public IContext
	{
	private:
		bool initialized = false;
	public:
		int screenWidth = 0,
			screenHeight = 0;

		int viewportWidth = 0,
			viewportHeight = 0;

		std::unique_ptr<ecs::ECSWorld>	world = nullptr;
		std::unique_ptr<ICamera>		mainCam = nullptr;
		std::unique_ptr<GameFlowFSM>	gameFlowFsm = nullptr;
		std::unique_ptr<editor::Editor> editor = nullptr;

		mem::Arena* arena;
	public:
		Game() = default;
		Game(int width, int height)
		{
			this->viewportWidth = width;
			this->viewportHeight = height;

			this->screenWidth = width;
			this->screenHeight = height;
		}

		void init(AppState& state);
		void update(float dt) const;
		void input(SDL_Event& e, float dt) const;
		void draw(float dt) const;
		void quit() const;

		void resize(int viewportWidth, int viewportHeight) const;

		const ecs::ECSWorld& getWorld() { return *world; }
		glm::vec2 getScreenDim() const { return glm::vec2(this->screenWidth, this->screenHeight); }
		glm::vec2 getViewport() const { return glm::vec2(this->viewportWidth, this->viewportHeight); }

		ICamera& getMainCam() { return *mainCam.get(); }
	};
}