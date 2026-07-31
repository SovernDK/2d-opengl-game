#pragma once
#include "services/scene.h"
#include <generation/map_generation.h>

class WorldMapScene : public IScene
{
private:
	std::future<MapGeneration> mapFuture;
	std::atomic<bool> mapReady{ false };
public:
	WorldMapScene(core::IContext* ctx, std::string id) : IScene(ctx, id) {}

	void start() override;
	void update(float dt) override;
	void draw() override;
	void unload() override;
	void quit() override;
};