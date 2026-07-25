#pragma once
#include "services/scene.h"
#include "graphics/texture.h"
#include "graphics/graphics.h"

namespace core
{
	class Game;
};

class NarrativeEventScene : public IScene
{
private:
	TexID bgTex{};
	gpu::UVRect uvs{};
public:
	NarrativeEventScene(std::string id) : IScene(id) {}
	~NarrativeEventScene() override;

	void start(core::IContext* ctx) override;
	void update(core::IContext* ctx, float dt) override;
	void draw(core::IContext* ctx) override;
	void unload(core::IContext* ctx) override;
	void quit(core::IContext* ctx) override;
};