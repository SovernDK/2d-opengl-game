#pragma once
#include "services/scene.h"
#include "graphics/texture.h"
#include "graphics/graphics.h"

namespace core
{
	class Game;
};

namespace rmui
{
	class UIWidget;
	class UIButton;
}

class IUIService;

class NarrativeEventScene : public IScene
{
private:
	TexID bgTex{};
	gpu::UVRect uvs{};
	
	IUIService* ui;

	std::shared_ptr<rmui::UIWidget> bookWindow, leftPage, rightPage,
		header, contentUI, choices;
	std::vector<std::shared_ptr<rmui::UIButton>> directionBtns;

	bool updateRoom = false;
public:
	NarrativeEventScene(core::IContext* ctx, std::string id) : IScene(ctx, id) {}

	void start() override;
	void update(float dt) override;
	void draw() override;
	void unload() override;
	void quit() override;
};