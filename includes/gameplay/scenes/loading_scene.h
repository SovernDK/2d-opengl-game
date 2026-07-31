#pragma once
#include "services/scene.h"

class LoadingScene : public IScene
{
public:
	LoadingScene(core::IContext* ctx, std::string id) : IScene(ctx, id) {}
	~LoadingScene() override;

	void start() override;
	void update(float dt) override;
	void draw() override;
	void unload() override;
	void quit() override;
};