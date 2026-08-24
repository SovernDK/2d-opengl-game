#pragma once
#include "rmui/ui.h"
#include "rmui/ui_anim.h"
#include "rmui/ui_widget.h"

#include "utility/utils.h"

using namespace rmui;

#pragma region IAnimation
IAnimation::IAnimation(float time)
{
	animLength = time;
}

void IAnimation::start(UIWidget* widget)
{
	m_timer = Timer(animLength);
	isPlaying = true;
	widget->changeAnimCount(1);

	if (!onAnimationStartCalls.empty())
	{
		for (auto& fn : onAnimationStartCalls) fn();
	}
}

void IAnimation::update(UIWidget* widget, float dt)
{
	if (!isPlaying) return; // Animation stopped, dont update time;
	if (!onAnimationUpdateCalls.empty())
	{
		for (auto& fn : onAnimationUpdateCalls) fn();
	}

	m_timer.step(dt);
	ntime = normalizedTime();

	if (m_timer.isTimeout())
	{
		isPlaying = false;
		done = true;
	}

	if (done) ntime = 1.0f; //Animation done keep time as 1.0 until its not removed
}

void IAnimation::end(UIWidget* widget)
{
	if (!onAnimationExitCalls.empty())
	{
		for (auto& fn : onAnimationExitCalls) fn();
	}

	widget->changeAnimCount(-1);
}

IAnimation* IAnimation::addListener(Listener listener, std::function<void()> func)
{
	switch (listener)
	{
	case Listener::START:
		onAnimationStartCalls.push_back(func); break;
	case Listener::UPDATE:
		onAnimationUpdateCalls.push_back(func); break;
	case Listener::EXIT:
		onAnimationExitCalls.push_back(func); break;
	}
	return this;
}

void IAnimation::setSpeed(float speed)
{
	m_speed = speed;
}

float IAnimation::normalizedTime()
{
	float t = m_timer.getTime() / animLength;
	if (m_curve)
		t = m_curve->sample(t);

	return std::clamp(t, 0.0f, 1.0f);
}
#pragma endregion IAnimation

#pragma region Fade In
void FadeIn::start(UIWidget* widget)
{
	IAnimation::start(widget);
	startingAlpha = widget->alpha();
	/*SDL_Log("FadeIn START");
	SDL_Log("FadeIn animLength: %f", animLength);

	SDL_Log("FadeIn: %f, done = %s", ntime, done ? "true" : "false");*/
}

void FadeIn::update(UIWidget* widget, float dt)
{
	IAnimation::update(widget, dt);

	float value = (float) MAX_ALPHA * ntime;
	value = std::clamp(value, 0.0f, 255.0f);
	uint8_t ivalue = (uint8_t)std::lround(value);
	widget->setAlpha(ivalue);
	//SDL_Log("FadeIn: %f, done = %s, setAlpha: %d", ntime, done ? "true" : "false", ivalue);
}

void FadeIn::end(UIWidget* widget)
{
	//SDL_Log("FadeIn END");
	IAnimation::end(widget);
}
#pragma endregion Fade In

#pragma region Fade Out
void FadeOut::start(UIWidget* widget)
{
	IAnimation::start(widget);
}

void FadeOut::update(UIWidget* widget, float dt)
{
	IAnimation::update(widget, dt);

	float value = (float) MAX_ALPHA - (float) (MAX_ALPHA * ntime);
	value = std::clamp(value, 0.0f, 255.0f);
	uint8_t ichange = (uint8_t)std::lround(value);
	widget->setAlpha(ichange);
}

void FadeOut::end(UIWidget* widget)
{
	IAnimation::end(widget);
}
#pragma endregion Fade Out

#pragma region Move From
void MoveFrom::start(UIWidget* widget)
{
	IAnimation::start(widget);
}

void MoveFrom::update(UIWidget* widget, float dt)
{
	IAnimation::update(widget, dt);

	glm::vec2 change = util_vec::lerp(m_from, glm::vec2(0.0f), ntime);
	widget->setOffset(change);
}

void MoveFrom::end(UIWidget* widget)
{
	IAnimation::end(widget);
}
#pragma endregion Move From

#pragma region Move To
void MoveTo::start(UIWidget* widget)
{
	IAnimation::start(widget);
	widget->clipping = false;
}

void MoveTo::update(UIWidget* widget, float dt)
{
	IAnimation::update(widget, dt);

	glm::vec2 change = util_vec::lerp(glm::vec2(0.0f), m_to, ntime);
	widget->setOffset(change);
}

void MoveTo::end(UIWidget* widget)
{
	IAnimation::end(widget);
}
#pragma endregion Move To