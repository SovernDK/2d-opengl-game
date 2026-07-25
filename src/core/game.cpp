#pragma once
#include "core/game.h"
#include "core/memory/arena.h"
#include "core/ecs/core_systems.h"

#include "ecs/base_components.h"
#include "utility/file_util.h"
#include "core/resources.h"
#include "graphics/camera/camera_ortho.h"

#include "graphics/rendering/canvas_2d.h"

#include "services/service_locator.h"
#include "services/ui_service.h"
#include "services/file_service.h"
#include "services/input_service.h"
#include "services/log_service.h"
#include "services/audio_service.h"

#include "glm/glm.hpp"

#include <glad/glad.h>
#include <nlohmann/json.hpp>

#include "scenes/world_map_scene.h"
#include "scenes/main_menu_scene.h"
#include "scenes/loading_scene.h"
#include "config.h"
#include "settings.h"
#include <scenes/narrative_event_scene.h>

using namespace glm;
using namespace core;
using namespace rmui;

using json = nlohmann::json;
using ECSWorld = ecs::ECSWorld;

bool m_editor = false;

ecs::CoreSystems* coreSystems = nullptr;

void Game::init(AppState& state)
{
	initialized = true;
#pragma region register services
	ServiceLocator::registerService<ILogger, LoggerService>();
	ServiceLocator::registerService<IInputService, InputService>();
	ServiceLocator::registerService<IFileService, FileService>();
	ServiceLocator::registerService<ISceneService, SceneService>();
	ServiceLocator::registerService<IUIService, UIService>();
	ServiceLocator::registerService<IAudioService, AudioService>();
#pragma endregion

#pragma region load assets
	Resources::loadShader("def_vert.glsl", "def_frag.glsl", GConfig.defaultShader);
	Resources::loadShader("ttf_vert.glsl", "ttf_frag.glsl", GConfig.fontShader);
	Resources::loadShader("primitive_vert.glsl", "primitive_frag.glsl", "primitive");
	Resources::loadShader("terrain_vert.glsl", "terrain_frag.glsl", "terrain");
	Resources::loadShader("ui_vert.glsl", "ui_frag.glsl", GConfig.uiShader);
	Resources::loadShader("screen_vert.glsl", "screen_frag.glsl", "screen");
	Resources::loadShader("grad_vert.glsl", "grad_frag.glsl", "grad");

	Material def(Resources::getStrPtrShader(GConfig.defaultShader));
	Material ttf(Resources::getStrPtrShader(GConfig.fontShader));
	Material ui(Resources::getStrPtrShader(GConfig.uiShader));
	Material terrain(Resources::getStrPtrShader("terrain"));
	Material primitive(Resources::getStrPtrShader("primitive"));
	Material screen(Resources::getStrPtrShader("screen"));
	Material grad(Resources::getStrPtrShader("grad"));

	Resources::addSharedMat(GConfig.defaultShader, std::make_shared<Material>(def));
	Resources::addSharedMat(GConfig.fontShader, std::make_shared<Material>(ttf));
	Resources::addSharedMat(GConfig.uiShader, std::make_shared<Material>(ui));
	Resources::addSharedMat("terrain", std::make_shared<Material>(terrain));
	Resources::addSharedMat("primitive", std::make_shared<Material>(primitive));
	Resources::addSharedMat("screen", std::make_shared<Material>(screen));
	Resources::addSharedMat("grad", std::make_shared<Material>(grad));
#pragma endregion
	world = std::make_unique<ECSWorld>();
	world->create("Settings").add<ecs::MapGenSettings>({});

	mainCam = std::make_unique<OrthoCamera>();
	mainCam->screenRes.x = screenWidth;
	mainCam->screenRes.y = screenHeight;

	mainCam->resize(screenWidth, screenHeight);

	editor = std::make_unique<editor::Editor>(*this);

	Resources::loadTexture(file_util::createPath("assets", "default.png").string(),	  "default");
	Resources::loadTexture(file_util::createPath("assets", "heightmap.png").string(), "heightmap");
	Resources::loadTexture(file_util::createPath("assets", "voronoi.png").string(),	  "voronoi");
	Resources::loadTexture(file_util::createPath("assets", "loading.png").string(),	  "loader");

	Resources::loadTexture(file_util::createPath("assets", "triangle.png").string(), "triangle");
	Resources::loadTexture(file_util::createPath("assets", "circle.png").string(), "circle");

	Resources::loadTexture(file_util::createPath("assets", "water_normal.png").string(),  "water_normal");
	Resources::loadTexture(file_util::createPath("assets", "water_normal2.png").string(), "water_normal2");
	Resources::loadTexture(file_util::createPath("assets", "water_normal3.png").string(), "water_normal3");
	Resources::loadTexture(file_util::createPath("assets", "forestNormal.png").string(),  "forest_normal");

	ServiceLocator::get<IAudioService>()->createAudioDevice();
	Resources::loadClip(file_util::createPath("assets", "sfx", "ui_move.wav").string(), "clip");
	Resources::loadMusic(file_util::createPath("assets", "music", "music.wav").string(), "music");

#pragma region Initialize data
	json uiData	= ServiceLocator::get<IFileService>()->loadJsonFile(GConfig.data("style.json"));

	ServiceLocator::get<IInputService>()->load(GSettings.content);

	ServiceLocator::get<IUIService>()->loadStyles(uiData);
	ServiceLocator::get<IUIService>()->init(screenWidth, screenHeight);

	ServiceLocator::get<ISceneService>()->registerContext(this);

	ServiceLocator::get<ISceneService>()->registerScene<WorldMapScene>("WorldMapScene");
	ServiceLocator::get<ISceneService>()->registerScene<MainMenuScene>("MainMenuScene");
	ServiceLocator::get<ISceneService>()->registerScene<NarrativeEventScene>("NarrativeEventScene");
	ServiceLocator::get<ISceneService>()->registerScene<LoadingScene>("LoadingScene");

	ServiceLocator::get<ISceneService>()->requestTransition<MainMenuScene>(TransitionMode::Additive);
#pragma region

	coreSystems = new ecs::CoreSystems(*world.get());
	coreSystems->init();
}

void Game::update(float dt) const
{
	if(m_editor)
		editor->update(dt, *mainCam);

	glm::vec2 mousePos = ServiceLocator::get<IInputService>()->getMousePos();
	glm::vec2 convertedMouse = mainCam->pointToViewport(mousePos);

	ServiceLocator::get<IUIService>()->handleMouse(convertedMouse);
	ServiceLocator::get<IUIService>()->update();

	ServiceLocator::get<ISceneService>()->update(dt);
	ServiceLocator::get<IInputService>()->reset();

	world->process(dt);
}

void Game::input(SDL_Event& e, float dt) const
{
	if (e.type == SDL_EVENT_KEY_UP && e.key.key == SDLK_C)
	{
		ServiceLocator::get<IAudioService>()->playOnce("clip");
		ServiceLocator::get<IAudioService>()->stopMusic(5000);
	}

	if (e.type == SDL_EVENT_KEY_UP && e.key.key == SDLK_F1)
	{
		if (m_editor) 
		{
			resize(screenWidth, screenHeight);
			m_editor = false;
		}
		else 
		{
			resize(screenWidth * 0.7f, screenHeight * 0.7f);
			m_editor = true;
		}
	}

	if (e.type == SDL_EVENT_KEY_UP && e.key.key == SDLK_F2)
	{
		//ServiceLocator::get<ISceneService>()->requestClearAll();
		//ServiceLocator::get<ISceneService>()->requestRemoveLast();
		ServiceLocator::get<ISceneService>()->requestTransition<MainMenuScene>(TransitionMode::Replace);
	}

	ServiceLocator::get<IInputService>()->processEvents(e);
}

void Game::draw(float dt) const
{
	if(m_editor)
		editor->draw();

	world->view<ecs::Transform2D, ecs::Sprite>([&](ecs::Entity& entity, ecs::Transform2D& t, ecs::Sprite& s)
	{
		Canvas2D::setDepth(s.depth);
		Canvas2D::drawSprite(s, t);
		Canvas2D::reset();
	});

	ServiceLocator::get<IUIService>()->draw(dt);
	ServiceLocator::get<ISceneService>()->draw();
}

void Game::quit() const
{
	if (initialized)
	{
		// Logger saves session to log, add periodic saves not only save on ending
		ServiceLocator::get<ILogger>()->quit();
		// Destroy remaining scenes and their dependencies (access to service locator here is still necessary so we clear them)
		ServiceLocator::get<ISceneService>()->quit();

		ServiceLocator::quit();

		delete coreSystems;
		world->quit();
		Resources::quit();
	}
}

void Game::resize(int viewportWidth, int viewportHeight) const
{
	mainCam->resize(viewportWidth, viewportHeight);
}