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

class CharacterSheetScene : public IScene
{
private:
	std::shared_ptr<rmui::UIWidget> rightColumn = nullptr, inventoryWin = nullptr, equipment = nullptr;
	std::vector<InventoryItemElement> inventoryElements;

	IUIService* ui = nullptr;
public:
	CharacterSheetScene(core::IContext* ctx, std::string id) : IScene(ctx, id) {}

	void start() override;
	void update(float dt) override;
	void draw() override;
	void unload() override;
	void quit() override;
};