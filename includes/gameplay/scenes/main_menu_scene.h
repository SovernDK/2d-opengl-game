#pragma once
#include "services/scene.h"

namespace core { 
	class Game; 
};

class MainMenuScene : public IScene
{
public:
	MainMenuScene(std::string id) : IScene(id) {}
	~MainMenuScene() override;

	void start(core::IContext* ctx) override;
	void update(core::IContext* ctx, float dt) override;
	void draw(core::IContext* ctx) override;
	void unload(core::IContext* ctx) override;
	void quit(core::IContext* ctx) override;
};