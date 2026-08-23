#pragma once
#include "gameplay/states/start_game_state.h"
#include "SDL3/SDL.h"

#include "core/game.h"
#include "scenes/world_map_scene.h"
#include "services/ui_service.h"

void StartGameState::enter(core::IContext& ctx)
{
	SDL_Log("Start Game");

	core::Game& game = dynamic_cast<core::Game&>(ctx);
	auto ui = ServiceLocator::get<IUIService>();

	ui->widget("main_menu_window").lock()->visible = false;

	ServiceLocator::get<ISceneService>()->requestTransition<WorldMapScene>(TransitionMode::Additive);
}

void StartGameState::update(core::IContext& ctx, float dt)
{

}

void StartGameState::exit(core::IContext& ctx)
{

}