#pragma once
#include "scenes/main_menu_scene.h"

#include "SDL3/SDL.h"
#include "core/game.h"
#include "core/texts.h"
#include "services/ui_service.h"

#include <imgui.h>

#include "rmui/ui_anim.h"
#include "rmui/ui_widget.h"

#include <scenes/world_map_scene.h>
#include <scenes/loading_scene.h>
#include <services/audio_service.h>

#include <utility/file_util.h>
#include <utility/utils.h>
#include "utility/feel.h"

#include <profiler.h>
#include <scenes/narrative_event_scene.h>

using namespace rmui;
using namespace core;

std::shared_ptr<UIWidget> window;
std::shared_ptr<UIButton> newGameBtn, loadGameBtn, settingsGameBtn, exitGameBtn;
ecs::EntityId emitterId;

MainMenuScene::~MainMenuScene() {}

void MainMenuScene::start(core::IContext* ctx)
{
	core::Game& game = static_cast<core::Game&>(*ctx);
	auto ui = ServiceLocator::get<IUIService>();

	ServiceLocator::get<IAudioService>()->playMusic("music");

	window = ui->createWindow()
		.setLocPos(UIAnchor::Center)
		.setLocSize(0.15f, 0.3f)
		.setPivot(UIAnchor::Center)
		.setStyle("mm_window")
		.setAlpha(0)
		.build("main_menu_window");

	newGameBtn = ui->createButton()
		.setLocSize(0.1f, 0.3f)
		.setParent(window)
		.setStyle("mm_button")
		.setAlpha(0)
		.setText(GTexts.content["main_menu.new_game"])
		.build("new_game_btn");

	loadGameBtn = ui->createButton()
		.setLocSize(0.1f, 0.3f)
		.setParent(window)
		.setStyle("mm_button")
		.setAlpha(0)
		.setText(GTexts.content["main_menu.load_game"])
		.build("load_game_btn");

	 /*settingsGameBtn = ui->createButton()
		 .setLocSize(0.1f, 0.3f)
		 .setParent(window)
		 .setStyle("mm_button")
		 .setText("Settings")
		 .build("settings_btn");*/

	exitGameBtn = ui->createButton()
		.setLocSize(0.1f, 0.3f)
		.setParent(window)
		.setStyle("mm_button")
		.setAlpha(0)
		.setText(GTexts.content["main_menu.exit_game"])
		.build("exit_game_btn");

	newGameBtn->label->play<FadeIn>(1.7f)->setSpeed(2.0f);
	loadGameBtn->label->play<FadeIn>(1.7f)->setSpeed(1.5f);
	exitGameBtn->label->play<FadeIn>(1.7f);

	glm::vec2 start{ -50.0f, 0.0f };
	newGameBtn->play<MoveFrom>(1.1f, start)->setCurve<feel::QuadCurve>(feel::EasingType::Out)->setSpeed(2.0f);
	loadGameBtn->play<MoveFrom>(1.1f, start)->setCurve<feel::QuadCurve>(feel::EasingType::Out)->setSpeed(1.5f);
	exitGameBtn->play<MoveFrom>(1.1f, start)->setCurve<feel::QuadCurve>(feel::EasingType::Out);

	const auto& emitterEntity = game.world->create("ParticleEmitter")
		.add<ecs::ParticleEmitter>({
			.emiting{ 10.0f },
			.interval{ 0.2f },
			.texture = Resources::texture("circle")->id,
			.emitArea{ 10.0f, 960.0f, 1900.0f, 100.0f },

			.size = glm::vec2(64.0f),
			.lifeRange{ 5.0f, 7.0f },
			.startVelocity{ 90.0f, 100.0f },
			.startScale{ 0.8f, 1.2f },

			.direction{ 0.0f, -1.0f },

			.scaleCurve = std::make_unique<feel::HillCurve>(),
			.alphaCurve = std::make_unique<feel::HillCurve>()
		});

	emitterId = emitterEntity.id;

	const auto newGameAnimFn = [&](UIWidget* widget)
	{
		newGameBtn->label->play<FadeOut>(1.7f)
			->setCurve<feel::QuadCurve>(feel::EasingType::In)
			->addListener(Listener::EXIT, [&]() {
			ServiceLocator::get<ISceneService>()->requestRemoveLast();
			ServiceLocator::get<ISceneService>()->requestTransition<NarrativeEventScene>(TransitionMode::Additive);
		});

		loadGameBtn->label->play<FadeOut>(1.7f)->setCurve<feel::QuadCurve>(feel::EasingType::In)->setSpeed(1.5f);
		exitGameBtn->label->play<FadeOut>(1.7f)->setCurve<feel::QuadCurve>(feel::EasingType::In)->setSpeed(2.0f);

		glm::vec2 target{ -50.0f, 0.0f };
		newGameBtn->play<MoveTo>(1.7f, target)->setCurve<feel::QuadCurve>(feel::EasingType::In);
		loadGameBtn->play<MoveTo>(1.7f, target)->setCurve<feel::QuadCurve>(feel::EasingType::In)->setSpeed(1.5f);
		exitGameBtn->play<MoveTo>(1.7f, target)->setCurve<feel::QuadCurve>(feel::EasingType::In)->setSpeed(2.0f);
	};

	newGameBtn->interaction->addOnClick(newGameAnimFn);
}

void MainMenuScene::update(core::IContext* ctx, float dt)
{
	core::Game& game = static_cast<core::Game&>(*ctx);
}

void MainMenuScene::draw(core::IContext* ctx)
{
	core::Game& game = static_cast<core::Game&>(*ctx);
	auto ui = ServiceLocator::get<IUIService>();

	glm::vec4 bg = glm::vec4(0, 0, game.screenWidth, game.screenHeight);
	auto* mat = Canvas2D::loadToArena<MaterialInstance>(Resources::sharedMat("grad"));

	mat->setProperty("horizontal", false);
	mat->setProperty("upper_color1", color::SDLColorToVec3(Bg1));
	mat->setProperty("upper_color2", color::SDLColorToVec3(Bg2));

	mat->setProperty("lower_color1", color::SDLColorToVec3(Bg3));
	mat->setProperty("lower_color2", color::SDLColorToVec3(Bg4));
	mat->setProperty("time", core::Profiler::instance().getElapsedTime());

	Canvas2D::drawQuad(bg, mat);
}

void MainMenuScene::unload(core::IContext* ctx)
{
	core::Game& game = static_cast<core::Game&>(*ctx);
	auto ui = ServiceLocator::get<IUIService>();
	
	ui->destroy("main_menu_window");
	game.world->destroy(emitterId);
}

void MainMenuScene::quit(core::IContext* ctx)
{
	window.reset();
	newGameBtn.reset();
	loadGameBtn.reset();
	exitGameBtn.reset();
}