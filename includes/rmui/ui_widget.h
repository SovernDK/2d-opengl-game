#pragma once
#include "rmui/ui.h"
#include "rmui/ui_anim.h"

#include "graphics/material.h"
#include "ecs/base_components.h"
#include "services/service.h"

#include <memory>
#include <canvas_2d.h>
#include <nlohmann/json.hpp>

class IUIService;

namespace rmui
{
	class UIButtonFactory;
	class UIWindowFactory;
	class UIImageFactory;
	class UILabelFactory;
	class UIMultiLabelFactory;

	class UIInteraction;
	class ILayoutStrategy;

	struct UIComponent;

	struct StyleBackground
	{
		bool keepAspectRatio = false;
		TexID texture = 0;
		SDL_Color color = WHITE;
		SDL_Color hoverColor = WHITE;
	};

	struct StyleText
	{
		int size = 24;
		std::string font = "default";
		SDL_Color color = WHITE;
		SDL_Color hoverColor = RED;
		TextAlign align = TextAlign::Left;
		TextVertAlign valign = TextVertAlign::Top;
		TextOverflow overflow = TextOverflow::Ellipsis;
	};

	struct StyleComponent
	{
		std::string name = core::GConfig.defaultShader;
		uint8_t alpha = 255;
		std::unique_ptr<ILayoutStrategy> layoutStrategy = nullptr;

		StyleBackground back{};
		StyleText text{};

		bool dropShadow = false;
		glm::vec2 shadowOffset = glm::vec2(0);
	};

	class UIWidget
	{
	public:
		WidgetID id = 0;
		bool dirtyUpdate = false;

		UIRect rect{ 0 };
		UIRect localRect{ 1 };

		glm::vec2 offset{ 0.0f };
		glm::vec2 pivot = glm::vec2(0.0f);

		uint8_t m_alpha = 255;

		bool visible = true;
		bool blocking = true;
		bool interactive = false;
		bool clipping = true;

		std::string style = "default";
		std::string handle = "";

		std::weak_ptr<UIWidget> parent;
		std::unique_ptr<UIInteraction> interaction;
		std::vector<std::pair<std::type_index, std::unique_ptr<UIComponent>>> components;
	protected:
		std::vector<std::shared_ptr<UIWidget>> m_children;

		int animationCount = 0;

		bool uiPtrSet = false;
		IUIService* m_uiService = nullptr;
	public:
		UIWidget(WidgetID id) : id(id)
		{
			interaction = std::make_unique<UIInteraction>();
		};

		UIWidget(const UIWidget& other) = delete;
		UIWidget& operator=(const UIWidget& other) = delete;

		virtual ~UIWidget()
		{
			m_children.clear();
			parent.reset();
			components.clear();
		}

		const auto& children() const { return m_children; }
		void clearChildren() { m_children.clear(); }
		void addChild(std::shared_ptr<UIWidget> child) { m_children.push_back(child); }
		void eraseChild(std::shared_ptr<UIWidget> child)
		{
			auto it = std::find(m_children.begin(), m_children.end(), child);
			if (it != m_children.end())
			{
				m_children.erase(it);
			}
		}

		void setUI(IUIService* uiService) 
		{ 
			assert(!uiPtrSet && "IUIService pointer already assigned!");

			m_uiService = uiService;
			uiPtrSet = true;
		}

		void setLocalRect(UIRect rect) { localRect = rect; setDirty(); }

		void setLocalPosition(glm::vec2 pos) { localRect.pos = pos; setDirty(); }
		void setLocalPosition(float x, float y) { localRect.pos = glm::vec2(x, y); setDirty(); }
		void setLocalPosition(UIAnchor pos) { localRect.pos = uiAnchorToVec2(pos); setDirty(); }

		void setLocalSize(glm::vec2 size) { localRect.size = size; setDirty(); }
		void setLocalSize(float width, float height) { localRect.size = glm::vec2(width, height); setDirty(); }
		void setLocalSize(float size) { localRect.size = glm::vec2(size, size); setDirty(); }

		void setPivot(glm::vec2 p) { pivot = p; /*setDirty();*/ }
		void setPivot(UIAnchor p) { pivot = uiAnchorToVec2(p);/* setDirty();*/ }

		void setOffset(float x, float y) { this->offset = glm::vec2(x, y); setDirty(); }
		void setOffset(glm::vec2 offset) { this->offset = offset; setDirty(); }

		void setDirty()
		{
			dirtyUpdate = true;
			if (!parent.expired())
			{
				parent.lock()->setDirty();
			}
		}

		void setAlpha(uint8_t alpha)
		{
			m_alpha = alpha;
		}

		template<typename T, typename... TArgs>
		T* play(TArgs&&... args);

		template<typename TComponent, typename... TArgs>
		void addComponent(TArgs&&... args);

		template<typename TComponent>
		TComponent* tryGetComponent();

		uint8_t alpha() const { return m_alpha; }

		void changeAnimCount(int count)
		{
			animationCount += count;
			if (animationCount > 0)
			{
				clipping = false;
				interactive = false;
			}
			else
			{
				clipping = true;
				interactive = true;
			}
		}
	};

	class UIInteraction
	{
	private:
		std::vector<std::function<void(UIWidget*)>> onClickFuncs;
		std::vector<std::function<void(UIWidget*, glm::vec2)>> onPressedFuncs;
		std::vector<std::function<void(UIWidget*)>> onEnterHoverFuncs;
		std::vector<std::function<void(UIWidget*)>> onExitHoverFuncs;
	public:
		bool hovered = false;
	public:
		UIInteraction() = default;
		UIInteraction(const UIInteraction& other) = default;
		UIInteraction& operator=(const UIInteraction& other) = default;

		~UIInteraction() = default;

		void addOnClick(const std::function<void(UIWidget*)>& callback) { onClickFuncs.push_back(callback); }
		void addOnPressed(const std::function<void(UIWidget*, glm::vec2)>& callback) { onPressedFuncs.push_back(callback); }
		void addOnEnterHover(const std::function<void(UIWidget*)>& callback) { onEnterHoverFuncs.push_back(callback); }
		void addOnExitHover(const std::function<void(UIWidget*)>& callback) { onExitHoverFuncs.push_back(callback); }

		void triggerOnClick(UIWidget* widget)
		{
			for (auto& func : onClickFuncs)
			{
				func(widget);
			}
		}

		void triggerOnPressed(UIWidget* widget, glm::vec2 mousePos)
		{
			for (auto& func : onPressedFuncs)
			{
				func(widget, mousePos);
			}
		}

		void triggerOnEnterHover(UIWidget* widget)
		{
			for (auto& func : onEnterHoverFuncs)
			{
				func(widget);
			}

			hovered = true;
		}

		void triggerOnExitHover(UIWidget* widget)
		{
			for (auto& func : onExitHoverFuncs)
			{
				func(widget);
			}

			hovered = false;
		}
	};

#pragma region Components
	struct UIComponent
	{
		int priority = 0;

		UIComponent(int priority = 0) : priority(priority) {};
		UIComponent(const UIComponent&) = delete;
		UIComponent& operator=(const UIComponent&) = delete;
		UIComponent(UIComponent&&) = delete;
		UIComponent& operator=(UIComponent&&) = delete;

		virtual ~UIComponent() = default;

		virtual void realize(UIWidget* widget) = 0;
	};

	struct UIText : public UIComponent
	{
		std::string text;
		std::string font = "default";

		SDL_Color color = BLACK;
		SDL_Color hoverColor = BLACK;

		int size = 24;
		TextOverflow overflow = TextOverflow::Ellipsis;
		TextAlign align = TextAlign::Left;
		TextVertAlign valign = TextVertAlign::Middle;

		glm::vec2 textOrigin{ 0.0f };

		UIText(std::string text) : UIComponent(2)
		{
			this->text = text;
		};

		UIText(std::string text, const StyleText& style) : UIComponent(2)
		{
			this->text = text;

			font = style.font;
			color = style.color;
			hoverColor = style.hoverColor;
			size = style.size;
			overflow = style.overflow;
			align = style.align;
			valign = style.valign;
		};

		void realize(UIWidget* widget) override
		{
			SDL_Color fontColor = color;
			if (widget->interaction->hovered)
				fontColor = hoverColor;

			fontColor.a = widget->alpha();

			auto& _text = text;

			glm::vec2 tSize = Canvas2D::textSize(_text, font, size);
			glm::vec2 tOrigin = Canvas2D::textOrigin(_text, font, size);

			if (widget->rect.size.x < tSize.x)
			{
				float bigger = tSize.x;
				float lesser = widget->rect.size.x;

				if (overflow == TextOverflow::Shrink)
				{
					size *= lesser / bigger;
				}
				else if(overflow == TextOverflow::Ellipsis)
				{
					_text = _text.substr(0, static_cast<size_t>(_text.size() * (lesser / bigger)));
					_text += "...";
				}
			}

			switch (align)
			{
			case TextAlign::Left:
				textOrigin.x = widget->rect.pos.x;
				break;
			case TextAlign::Middle:
				textOrigin.x = widget->rect.pos.x + (widget->rect.size.x - tSize.x) * 0.5f;
				break;
			case TextAlign::Right:
				textOrigin.x = widget->rect.pos.x + (widget->rect.size.x - tSize.x);
				break;
			}

			switch (valign)
			{
			case TextVertAlign::Top:
				textOrigin.y = widget->rect.pos.y + tSize.y;
				break;
			case TextVertAlign::Middle:
				textOrigin.y = widget->rect.pos.y + (widget->rect.size.y + tSize.y) * 0.5f;
				break;
			case TextVertAlign::Bottom:
				textOrigin.y = widget->rect.pos.y + widget->rect.size.y - tOrigin.y;
				break;
			}

			ecs::Transform2D textTransform = transform();
			Canvas2D::setColor(fontColor);
			
			Canvas2D::drawText(_text, textTransform, font, size);
			Canvas2D::reset();
		}

		ecs::Transform2D transform()
		{
			return ecs::Transform2D{ .position = glm::vec3(textOrigin, 1.0f) };
		}
	};

	struct UIMultilineText : public UIComponent
	{
		std::vector<std::string> lines;
		std::vector<std::string_view> views;

		std::string text;
		std::string font = "default";

		SDL_Color color = BLACK;
		SDL_Color hoverColor = BLACK;

		int size = 24;
		TextOverflow overflow = TextOverflow::Ellipsis;
		TextAlign align = TextAlign::Left;
		TextVertAlign valign = TextVertAlign::Middle;

		glm::vec2 textOrigin = glm::vec2(0.0f);

		UIMultilineText(std::string text) : UIComponent(2)
		{
			lines = text::split(text, "\n");
			views.clear();
			views.reserve(lines.size());
			for (auto& line : lines)
			{
				views.push_back(line);
			}
		};

		UIMultilineText(std::string text, const StyleText& style) : UIComponent(2)
		{
			lines = text::split(text, "\n");
			views.clear();
			views.reserve(lines.size());
			for (auto& line : lines)
			{
				views.push_back(line);
			}

			font = style.font;
			color = style.color;
			hoverColor = style.hoverColor;
			size = style.size;
			overflow = style.overflow;
			align = style.align;
			valign = style.valign;
		};

		void realize(UIWidget* widget) override
		{
			// Font
			SDL_Color fontColor = color;
			if (widget->interaction->hovered)
				fontColor = hoverColor;

			fontColor.a = widget->alpha();

			// Calculate longest line
			float longest = 0.0f;
			for (auto& line : views)
			{
				glm::vec2 tSize = Canvas2D::textSize(line, font, size);
				longest = std::max(longest, tSize.x);
			}

			if (widget->rect.size.x < longest)
			{
				float bigger = longest;
				float lesser = widget->rect.size.x;

				size *= lesser / bigger;
			}

			// Draw line by line
			float y = widget->rect.pos.y;
			textOrigin = widget->rect.pos;

			Canvas2D::setColor(fontColor);

			for (auto& line : views)
			{
				glm::vec2 tSize = Canvas2D::textSize(line, font, size);
				ecs::Transform2D textTransform = transform();

				switch (align)
				{
				case TextAlign::Left:
					textTransform.position.x = textOrigin.x; break;
				case TextAlign::Middle:
					textTransform.position.x = textOrigin.x + (widget->rect.size.x - tSize.x) * 0.5f; break;
				case TextAlign::Right:
					textTransform.position.x = textOrigin.x + (widget->rect.size.x - tSize.x); break;
				}

				y += tSize.y;
				textTransform.position.y = y;

				Canvas2D::drawText(line, textTransform, font, size);
			}

			Canvas2D::reset();
		}

		ecs::Transform2D transform()
		{
			return ecs::Transform2D{ .position = glm::vec3(textOrigin, 1.0f) };
		}
	};

	struct UIBackground : public UIComponent
	{
		TexID texture = 0;
		bool keepAspectRatio = false;

		SDL_Color color = WHITE;
		SDL_Color hoverColor = WHITE;

		ecs::Sprite m_sprite{};
		ecs::Transform2D m_transform{};

		UIBackground(TexID _texture, const StyleBackground& style) : UIComponent(1)
		{
			texture = _texture;
			keepAspectRatio = style.keepAspectRatio;

			color = style.color;
			hoverColor = style.hoverColor;

			m_sprite.material = Resources::sharedMat(core::GConfig.uiShader);
			m_sprite.material.blendMode = BlendMode::Alpha;
		};

		void realize(UIWidget* widget) override
		{
			SDL_Color c = color;

			if (widget->interaction)
				c = widget->interaction->hovered ? hoverColor : color;

			c.a = widget->alpha();

			m_sprite.size = widget->rect.size;
			m_sprite.texture = texture;
			m_sprite.color = c;

			m_transform.position = glm::vec3(widget->rect.pos, 1.0f);

			Canvas2D::drawSprite(m_sprite, m_transform);
		}
	};

	struct UIDropShadow : public UIComponent
	{
		bool enabled = true;
		glm::vec2 offset{ 0.0f };

		UIDropShadow() : UIComponent(0) {};
		UIDropShadow(const StyleComponent& style) : UIComponent(0) {
			enabled = style.dropShadow;
			offset = style.shadowOffset;
		};

		void realize(UIWidget* widget) override
		{
			if (!enabled) return;

			auto material = Canvas2D::loadToArena<MaterialInstance>(Resources::sharedMat("ui"));
			material->setProperty("mainColor", glm::vec4(glm::vec3(0.0f), 1.0f));
			material->setProperty("useTexture", false);

			glm::vec4 shadowRect = widget->rect.renderRect() + glm::vec4(offset, 0.0f, 0.0f);
			Canvas2D::drawQuad(shadowRect, material);
		}
	};
#pragma endregion

#pragma region Widget concrete classes
	class UIButton : public UIWidget
	{
	public:
		UIWidget* label = nullptr;
	public:
		UIButton(WidgetID id) : UIWidget(id) {};
	};
#pragma endregion
}

class IUIService : public IService
{
protected:
	std::vector<std::pair<WidgetID, std::unique_ptr<rmui::IAnimation>>> m_animations;
public:
	IUIService() = default;
	virtual ~IUIService() = default;

	virtual void loadStyles(nlohmann::json data) = 0;

	virtual void init(int width, int height) = 0;
	virtual void resizeCanvas(int width, int height) = 0;
	virtual void draw(float dt) = 0;
	virtual void update() = 0;

	virtual void handleMouse(glm::vec2 mousePos) = 0;
	virtual void handleInput(SDL_Event& e) = 0;

	virtual rmui::UIWidget* const widget(WidgetID id) const = 0;
	virtual rmui::UIWidget* const widget(std::string handle) const = 0;
	virtual const rmui::UIWidget& root() = 0;
	virtual int nextId() = 0;

	virtual void destroy(const std::string& handle) = 0;
	virtual void destroy(const std::shared_ptr<rmui::UIWidget>& widget) = 0;
	virtual void destroy(const rmui::UIWidget* widget) = 0;

	virtual void progressAnimations(float dt) = 0;
	virtual void clearAnimations() = 0;

	template<typename T, typename... TArgs>
	T* play(WidgetID id, TArgs&&... args)
	{
		static_assert(std::is_base_of_v<rmui::IAnimation, T>,
			"addAnimation<T> - T type must be derived from Animation!");

		std::unique_ptr<rmui::IAnimation> anim = std::make_unique<T>(std::forward<TArgs>(args)...);
		auto* it = static_cast<T*>(anim.get());
		m_animations.push_back({ id, std::move(anim) });
		return it;
	}

	template<typename T>
	std::shared_ptr<T> create(const std::string& handle, const std::string& style, const std::shared_ptr<rmui::UIWidget>& parent)
	{
		std::shared_ptr<rmui::UIWidget> widget = std::make_shared<T>(nextId());
		initWidget(widget, handle, style, parent);
		return std::dynamic_pointer_cast<T>(widget);
	}

	virtual const rmui::StyleComponent& style(const std::string& handle) = 0;

	virtual rmui::UIButtonFactory createButton() = 0;
	virtual rmui::UIWindowFactory createWindow() = 0;
	virtual rmui::UIImageFactory createImage() = 0;
	virtual rmui::UILabelFactory createLabel() = 0;
	virtual rmui::UIMultiLabelFactory createMultiLabel() = 0;
protected:
	virtual void initWidget(std::shared_ptr<rmui::UIWidget> widget, const std::string& handle, const std::string& style, const std::shared_ptr<rmui::UIWidget>& parent) = 0;
};

#pragma region Implementation
template<typename T, typename... TArgs>
T* rmui::UIWidget::play(TArgs&&... args)
{
	return m_uiService->play<T>(id, args...);
}

template<typename TComponent, typename... TArgs>
void rmui::UIWidget::addComponent(TArgs&&... args)
{
	components.emplace_back(
		std::piecewise_construct,
		std::forward_as_tuple(typeid(TComponent)),
		std::forward_as_tuple(std::make_unique<TComponent>(std::forward<TArgs>(args)...)));

	std::sort(components.begin(), components.end(), [](const auto& a, const auto& b)
	{
		return a.second->priority < b.second->priority;
	});
}

template<typename TComponent>
TComponent* rmui::UIWidget::tryGetComponent()
{
	for (auto& comp : components)
	{
		if (comp.first == typeid(TComponent))
			return static_cast<TComponent*>(comp.second.get());
	}

	return nullptr;
}
#pragma endregion