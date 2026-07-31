#pragma once
#include "services/scene.h"

namespace core { 
	class Game; 
};

class MainMenuScene : public IScene
{
public:
	MainMenuScene(core::IContext* ctx, std::string id) : IScene(ctx, id) {}
	~MainMenuScene() override;

	void start() override;
	void update(float dt) override;
	void draw() override;
	void unload() override;
	void quit() override;
};