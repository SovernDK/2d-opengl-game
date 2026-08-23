#pragma once
#include "services/scene.h"
#include "graphics/texture.h"
#include "graphics/graphics.h"
#include "memory/flat_map.h"

namespace core
{
	class Game;
};

namespace rmui
{
	class UIWidget;
	class UIButton;
}

namespace historia
{
	enum class EDirection;
	struct ChoiceEffect;
}

class IUIService;

class NarrativeEventScene : public IScene
{
private:
	TexID bgTex{};
	gpu::UVRect uvs{};
	
	IUIService* ui;

	std::shared_ptr<rmui::UIWidget> rightPage, header, contentUI, actions, choices, directions;
	mem::flat_map<historia::EDirection, std::shared_ptr<rmui::UIButton>> directionBtns;
	std::vector<std::shared_ptr<rmui::UIButton>> choiceBtns;

	bool updateRoom = false;
public:
	NarrativeEventScene(core::IContext* ctx, std::string id) : IScene(ctx, id) {}

	void start() override;
	void update(float dt) override;
	void draw() override;
	void unload() override;
	void quit() override;

	void applyEffect(const historia::ChoiceEffect& effect);
};