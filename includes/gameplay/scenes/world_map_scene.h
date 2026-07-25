#pragma once
#include "services/scene.h"
#include <generation/map_generation.h>

class WorldMapScene : public IScene
{
private:
	std::future<MapGeneration> mapFuture;
	std::atomic<bool> mapReady{ false };
public:
	WorldMapScene(std::string id) : IScene(id) {}

	void start(core::IContext* ctx) override;
	void update(core::IContext* ctx, float dt) override;
	void draw(core::IContext* ctx) override;
	void unload(core::IContext* ctx) override;
	void quit(core::IContext* ctx) override;
};