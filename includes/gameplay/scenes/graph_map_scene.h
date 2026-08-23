#pragma once
#include "services/scene.h"
#include "graphics/texture.h"
#include "graphics/graphics.h"

#include "gameplay/ui_element.h"

namespace core
{
	class Game;
};

namespace rmui
{
	class UIWidget;
}

namespace historia
{
	struct Room;
}

class IUIService;

class GraphMapScene : public IScene
{
private:
	std::shared_ptr<rmui::UIWidget> leftPageWindow = nullptr;

	IUIService* ui = nullptr;
public:
	GraphMapScene(core::IContext* ctx, std::string id) : IScene(ctx, id) {}

	void start() override;
	void update(float dt) override;
	void draw() override;
	void unload() override;
	void quit() override;

	void drawRoom(float x, float y, const historia::Room& room);
};