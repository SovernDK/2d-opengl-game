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

class IUIService;

enum class BookmarkScene
{
	CHARACTER_SHEET, MAP
};

class GameScene : public IScene
{
private:
	BookmarkScene currBookmark = BookmarkScene::CHARACTER_SHEET;
	IUIService* ui = nullptr;
public:
	GameScene(core::IContext* ctx, std::string id) : IScene(ctx, id) {}

	void start() override;
	void update(float dt) override;
	void draw() override;
	void unload() override;
	void quit() override;
};