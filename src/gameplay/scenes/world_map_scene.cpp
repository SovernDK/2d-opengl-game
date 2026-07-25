#pragma once
#include "scenes/world_map_scene.h"

#include "SDL3/SDL.h"
#include "core/game.h"

#include "utility/utils.h"
#include "core/resources.h"
#include "graphics/rendering/canvas_2d.h"
#include "graphics/graphics.h"

#include "graphics/material.h"
#include "graphics/texture.h"

#include <imgui.h>
#include <utility/file_util.h>
#include <services/ui_service.h>

#include "scenes/main_menu_scene.h"
#include <scenes/loading_scene.h>

MapGeneration buildMapData(const ecs::MapGenSettings* settings);
void applyMapToWorld(core::Game& game, MapGeneration& mapGen, int w, int h);
void updateTerrain(core::Game& game);
void initializeWorld(core::Game& game);

using namespace ecs;

void WorldMapScene::start(core::IContext* ctx)
{
	core::Game& game = static_cast<core::Game&>(*ctx);
	auto ui = ServiceLocator::get<IUIService>();

	ecs::EntityId id = game.world->entity("Settings").id;
	assert(id != 0);
	const auto* settings = game.world->get<ecs::MapGenSettings>(id);

	mapFuture = std::async(std::launch::async, buildMapData, settings);
}

void WorldMapScene::update(core::IContext* ctx, float dt)
{
	core::Game& game = static_cast<core::Game&>(*ctx);
	auto ui = ServiceLocator::get<IUIService>();

	if (!mapReady && mapFuture.valid())
	{
		auto status = mapFuture.wait_for(std::chrono::milliseconds(0));
		if (status == std::future_status::ready)
		{
			initializeWorld(game);
			
			MapGeneration result = mapFuture.get();

			EntityId id = game.world->entity("Settings").id;
			auto settings = game.world->get<ecs::MapGenSettings>(id);

			applyMapToWorld(game, result, settings->width, settings->height);
			ServiceLocator::get<ISceneService>()->requestRemoveLast();

			mapReady = true;
		}
	}

	updateTerrain(game);
}

void WorldMapScene::draw(core::IContext* ctx)
{
	
}

void WorldMapScene::unload(core::IContext* ctx)
{

}

void WorldMapScene::quit(core::IContext* ctx)
{
	core::Game& game = static_cast<core::Game&>(*ctx);
	auto ui = ServiceLocator::get<IUIService>();

	EntityId id = game.world->entity("WorldMap").id;

	if (id != 0)
	{
		game.world->destroy(id);
	}

	ui->destroy("topbar");
	mapReady = false;
}

void initializeWorld(core::Game& game)
{
	EntityId id = game.world->entity("Settings").id;
	auto settings = game.world->get<ecs::MapGenSettings>(id);
	auto ui = ServiceLocator::get<IUIService>();

	// =========== Entities =======================
	game.world->create("WorldMap")
		.add<ecs::WorldMap>({})
		.add<ecs::Sprite>({
			.size = glm::vec2(settings->width, settings->height),
			.depth = 1,
			.material{ MaterialInstance(Resources::sharedMat("terrain")) },
			})
		.add<ecs::Transform2D>({});

	MaterialInstance temp = MaterialInstance(Resources::sharedMat(core::GConfig.defaultShader));
	temp.blendMode = BlendMode::Alpha;
	temp.setProperty("mainColor", glm::vec4(1.0f));
	temp.setProperty("useTexture", true);

	Texture2D* tex = Resources::texture("triangle");
	temp.setTexture("image", tex->ID());

	game.world->create()
		.add<ecs::Sprite>(ecs::Sprite{
				.size = glm::vec2(24),
				.material = temp
			})
		.add<ecs::Transform2D>(ecs::Transform2D{
				.position = glm::vec3(850, 550, 1.0)
			});

	game.world->create()
		.add<ecs::Sprite>(ecs::Sprite{
				.size = glm::vec2(24),
				.material = temp
			})
		.add<ecs::Transform2D>(ecs::Transform2D{
				.position = glm::vec3(450, 400, 1.0)
			});
}

// Async
MapGeneration buildMapData(const ecs::MapGenSettings* settings)
{
	MapGeneration mapGen{};

	mapGen.mountainsSetting.SetSeed(settings->seed);
	mapGen.mountainsSetting.SetFrequency(settings->frequency);
	mapGen.mountainsSetting.SetFractalOctaves(settings->octaves);
	mapGen.mountainsSetting.SetFractalGain(settings->gain);

	mapGen.moistureSetting.SetSeed(settings->seed);

	mapGen.generate(settings->width, settings->height, settings->seed);

	return mapGen;
}

void applyMapToWorld(core::Game& game, MapGeneration& mapGen, int w, int h)
{
	Texture2D mapTexture = TextureBuilder()
		.setInternalFormat(GL_RGBA8)
		.setFiltering(GL_NEAREST)
		.build(w, h, mapGen.heightPixels.data());

	Texture2D normalTexture = TextureBuilder()
		.setFormat(GL_RGBA)
		.build(w, h, mapGen.normalPixels.data());

	Texture2D moistureTexture = TextureBuilder()
		.setFormat(GL_RGBA)
		.build(w, h, mapGen.moisturePixels.data());

	Texture2D* newTex = Resources::saveTexture(std::move(mapTexture), "map");
	Texture2D* newNormal = Resources::saveTexture(std::move(normalTexture), "n_map");
	Texture2D* newMoisture = Resources::saveTexture(std::move(moistureTexture), "m_map");

	Texture2D* waterNormal = Resources::texture("water_normal3");
	Texture2D* forestNormal = Resources::texture("forest_normal");

	Entity& worldMapEntity = game.world->entity("WorldMap");
	assert(worldMapEntity.id != 0);

	auto& worldMaterial = worldMapEntity.getMod<ecs::Sprite>()->material;

	worldMaterial.blendMode = BlendMode::None;

	updateTerrain(game);

	worldMaterial.setTexture("image", newTex->ID());
	worldMaterial.setTexture("normal", newNormal->ID());
	worldMaterial.setTexture("waterNormal", waterNormal->ID());
	worldMaterial.setTexture("moistureMap", newMoisture->ID());
	worldMaterial.setTexture("forestNormal", forestNormal->ID());
}

void updateTerrain(core::Game& game)
{
	Entity& worldMapEntity = game.world->entity("WorldMap");
	Entity& settingsEntity = game.world->entity("Settings");

	if (worldMapEntity.id == 0 || settingsEntity.id == 0)
		return;

	const auto settings = settingsEntity.get<ecs::MapGenSettings>();

	worldMapEntity.getMod<ecs::Sprite>()->size = glm::vec2(settings->width, settings->height);
	auto& worldMaterial = worldMapEntity.getMod<ecs::Sprite>()->material;

	float aspectRatio = settings->width / settings->height;
	worldMaterial.setProperty("aspectRatio", aspectRatio);
	worldMaterial.setProperty("sunPos", glm::vec3(settings->sunX, settings->sunY, settings->sunZ));
	worldMaterial.setProperty("ambientColor", settings->ambientColor);
	worldMaterial.setProperty("ambientStrength", settings->ambientStrength);
	worldMaterial.setProperty("specularStrength", settings->specularStrength);
	worldMaterial.setProperty("numSteps", settings->steps);
	worldMaterial.setProperty("stepSize", settings->stepSize);
	worldMaterial.setProperty("shadowLength", settings->shadowLength);
	worldMaterial.setProperty("terrainSpecStr", settings->terrainSpecStr);
	worldMaterial.setProperty("terrainSpecSpred", settings->terrainSpecSpred);
	worldMaterial.setProperty("terrainNormStr", settings->terrainNormStr);
	worldMaterial.setProperty("waterColor", settings->waterColor);
	worldMaterial.setProperty("diffuseAmbient", settings->diffuseAmbient);
	worldMaterial.setProperty("waterSpecStr", settings->waterSpecStr);
	worldMaterial.setProperty("waterNormalStr", settings->waterNormalStr);
	worldMaterial.setProperty("waterLevel", settings->waterLevel);
}