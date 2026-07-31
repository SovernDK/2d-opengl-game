#pragma once
#include "rmui/ui_widget.h"
#include "services/service.h"
#include "utility/id_pool.h"

#include "rmui/factories/ui_factory.h"
#include "graphics/graphics.h"

#include <string>
#include <queue>

#include <nlohmann/json.hpp>
#include <rmui/ui_layout.h>

union SDL_Event;
class Material;

class UIService : public IUIService
{
private:
	std::unordered_map<WidgetID, std::shared_ptr<rmui::UIWidget>> ids;
	std::unordered_map<std::string, std::shared_ptr<rmui::UIWidget>> handles;
	std::unordered_map<std::string, std::unique_ptr<rmui::StyleComponent>> styles;

	std::unordered_map<WidgetID, std::queue<std::unique_ptr<rmui::IAnimation>>> m_animations;

	std::shared_ptr<rmui::UIWidget> m_root = nullptr;
	rmui::UIWidget* focused = nullptr;
	rmui::UIWidget* prevFocused = nullptr;

	int submissionIndex = UI_Z;
	bool isBlocked = false;

	IdPool<WidgetID> idPool;
	int canvasWidth = 0;
	int canvasHeight = 0;

	bool ignoreAnim = false;
public:
	UIService() = default;
	~UIService() = default;

	void loadStyles(nlohmann::json data) override;

	void init(int width, int height) override;
	void resizeCanvas(int width, int height) override;
	void draw(float dt) override;
	void update() override;
	void handleMouse(glm::vec2 mousePos) override;
	void handleInput(SDL_Event& e) override;

	void destroy(const std::string& handle) override;
	void destroy(const std::shared_ptr<rmui::UIWidget>& widget) override;
	void destroy(const rmui::UIWidget* widget) override;

	void progressAnimations(float dt) override;
	void clearAnimations() override { m_animations.clear(); }

	rmui::UIWidget* const widget(int id) const override;
	rmui::UIWidget* const widget(std::string handle) const override;

	template<typename TWidget>
	auto widget(int id) -> const TWidget*
	{
		static_assert(std::is_base_of_v<rmui::UIWidget, TWidget>,
			"widget<T> - T type must be derived from rmui::UIWidget!");

		return static_cast<TWidget*>(widget(id));
	}

	template<typename TWidget>
	auto widget(std::string handle) -> const TWidget*
	{
		static_assert(std::is_base_of_v<rmui::UIWidget, TWidget>,
			"widget<T> - T type must be derived from UIWidget!");

		return static_cast<TWidget*>(widget(handle));
	};

	const rmui::UIWidget& root() override { return *m_root.get(); }
	int nextId() override { return idPool.next(); }

	const rmui::StyleComponent& style(const std::string& handle) override { return *styles[handle].get(); }

	rmui::UIButtonFactory createButton() override { return rmui::UIButtonFactory(*this); }
	rmui::UIWindowFactory createWindow() override { return rmui::UIWindowFactory(*this); }
	rmui::UIImageFactory createImage() override { return rmui::UIImageFactory(*this); }
	rmui::UILabelFactory createLabel() override { return rmui::UILabelFactory(*this); }
	rmui::UIMultiLabelFactory createMultiLabel() override { return rmui::UIMultiLabelFactory(*this); }
private:
	void realizeStyle(rmui::UIWidget& widget, float dt);
	void drawRecursive(rmui::UIWidget* m_root, const glm::vec4 parentClip, float dt);
	void updateRecursive(rmui::UIWidget* m_root, const rmui::UIRect& parentRect, rmui::ILayoutStrategy& strategy, int index);
	void topWidgetAtPos(rmui::UIWidget* widget, glm::vec2 pos);

	void initWidget(std::shared_ptr<rmui::UIWidget> widget, const std::string& handle, const std::string& style, const std::shared_ptr<rmui::UIWidget>& parent) override
	{
		widget->setUI(this);
		widget->style = style;

		if (parent)
		{
			widget->parent = parent;
			parent->addChild(widget);
		}
		else
		{
			widget->parent = m_root;
			m_root->addChild(widget);
		}

		widget->handle = handle;
		handles[handle] = widget;
		ids[widget->id] = widget;
	}

	void playAnimation(WidgetID id, std::unique_ptr<rmui::IAnimation> animation) override;
};