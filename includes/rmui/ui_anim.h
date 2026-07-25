#pragma once
#include "utility/feel.h"
#include "core/time/timer.h"

#include <functional>
#include <memory>

namespace rmui
{
	class UIWidget;

	enum class Listener
	{
		START, UPDATE, EXIT
	};

	class IAnimation
	{
	protected:
		Timer m_timer{ 0.0f };
		float m_speed = 1.0f;
		// Normalized animation time
		float ntime = 0.0f;
		std::unique_ptr<feel::ICurve> m_curve = nullptr;

		std::vector<std::function<void()>> onAnimationStartCalls;
		std::vector<std::function<void()>> onAnimationUpdateCalls;
		std::vector<std::function<void()>> onAnimationExitCalls;
	public:
		float animLength = 0.0f;
		bool isPlaying = false;
		bool done = false;
	public:
		IAnimation(float time);

		IAnimation(const IAnimation&) = default;
		IAnimation& operator=(const IAnimation&) = default;
		IAnimation(IAnimation&&) = default;
		IAnimation& operator=(IAnimation&&) = default;
		virtual ~IAnimation() = default;

		virtual void start(UIWidget* self);
		virtual void update(UIWidget* self, float dt);
		virtual void end(UIWidget* self);

		template<typename TCurve, typename... Args>
		IAnimation* setCurve(Args... args)
		{
			static_assert(std::is_base_of_v<feel::ICurve, TCurve>,
				"setCurve<T> - T type must be derived from feel::ICurve!");

			m_curve = std::make_unique<TCurve>(std::forward<Args>(args)...);
			return this;
		}

		virtual IAnimation* addListener(Listener listener, std::function<void()> func);
		virtual void setSpeed(float speed);
	private:
		virtual float normalizedTime();
	};

	class FadeIn : public IAnimation
	{
	private:
		const uint8_t MAX_ALPHA = 255;
		uint8_t startingAlpha = 0;
	public:
		FadeIn(float time) : IAnimation(time) {};

		void start(UIWidget* self) override;
		void update(UIWidget* self, float dt) override;
		void end(UIWidget* self) override;
	};

	class FadeOut : public IAnimation
	{
	private:
		const uint8_t MAX_ALPHA = 255;
	public:
		FadeOut(float time) : IAnimation(time) {};

		void start(UIWidget* self) override;
		void update(UIWidget* self, float dt) override;
		void end(UIWidget* self) override;
	};

	class MoveFrom : public IAnimation
	{
	private:
		glm::vec2 m_from{ 0.0f };
		bool m_interactive = false;
	public:
		MoveFrom(float time, glm::vec2 from, bool interactive = true) : IAnimation(time)
		{
			m_from = from;
			m_interactive = interactive;
		};

		void start(UIWidget* self) override;
		void update(UIWidget* self, float dt) override;
		void end(UIWidget* self) override;
	};

	class MoveTo : public IAnimation
	{
	private:
		glm::vec2 m_to{ 0.0f };
		bool m_interactive = false;
	public:
		MoveTo(float time, glm::vec2 to, bool interactive = true) : IAnimation(time)
		{
			m_to = to;
			m_interactive = interactive;
		};

		void start(UIWidget* self) override;
		void update(UIWidget* self, float dt) override;
		void end(UIWidget* self) override;
	};
}
