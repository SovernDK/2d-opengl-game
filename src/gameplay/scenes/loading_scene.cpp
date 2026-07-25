#pragma once
#include "scenes/main_menu_scene.h"

#include "SDL3/SDL.h"
#include "core/game.h"
#include "services/ui_service.h"

#include "imgui.h"
#include "scenes/loading_scene.h"
#include "graphics/rendering/canvas_2d.h"

using namespace ecs;

float rot = 0;
Sprite* loader;
Transform2D* loaderTrans;

LoadingScene::~LoadingScene() {}

void LoadingScene::start(core::IContext* ctx)
{
	core::Game& game = static_cast<core::Game&>(*ctx);
	auto ui = ServiceLocator::get<IUIService>();

	loader = new Sprite
	{
		.texture = Resources::texture("loader")->id,
		.blend = BlendMode::Alpha,
		.size = glm::vec2(128)
	};

	//glm::vec2 rightBottomScreen{ game.mainCam->viewport().z - loader->size.x, game.mainCam->viewport().w - loader->size.y };
	glm::vec2 rightBottomScreen{ 0, 0 };
	loaderTrans = new Transform2D
	{
		.position = glm::vec3(rightBottomScreen, 0.0f)
	};
}

void LoadingScene::update(core::IContext* ctx, float dt)
{
	rot += 360.0f * dt;
	core::Game& game = static_cast<core::Game&>(*ctx);

	loaderTrans->rotation = rot;
}

void LoadingScene::draw(core::IContext* ctx)
{
	Canvas2D::drawSprite(*loader, *loaderTrans);
}

void LoadingScene::unload(core::IContext* ctx)
{

}

void LoadingScene::quit(core::IContext* ctx)
{
	core::Game& game = static_cast<core::Game&>(*ctx);
	auto ui = ServiceLocator::get<IUIService>();

	if(loader)		delete loader;
	if(loaderTrans) delete loaderTrans;
}