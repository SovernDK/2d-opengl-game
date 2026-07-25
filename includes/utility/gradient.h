#pragma once
#include "SDL3/SDL.h"
#include "utility/utils.h"

#include <vector>
#include <algorithm>

struct ColorStop
{
	float position;
	SDL_Color color;
};

class Gradient
{
private:
	std::vector<ColorStop> stops;
public:
	SDL_Color sampleGradient(float t)
	{
		if (stops.size() <= 1) return stops.empty() ? SDL_Color{ 0, 0, 0, 255 } : stops[0].color;

		t = std::clamp(t, 0.0f, 1.0f);

		ColorStop* current = &stops[0];
		ColorStop* next = nullptr;

		for (auto it = stops.begin(); it != stops.end(); ++it)
		{
			if (t <= it->position)
			{
				next = &(*it);
				break;
			}
			current = &(*it);
		}

		if (next == nullptr)
			return current->color;

		float gradT = utils::normalize(t, current->position, next->position);
		return color::lerp(current->color, next->color, gradT);
	}

	void addColor(float position, SDL_Color color)
	{
		stops.push_back({ position, color });
		std::sort(stops.begin(), stops.end(), [](const ColorStop& a, const ColorStop& b)
		{
			return a.position < b.position;
		});
	}

	void removeColor(float position)
	{
		stops.erase(std::remove_if(stops.begin(), stops.end(), [position](const ColorStop& stop)
		{
			return stop.position == position;
		}), stops.end());

		std::sort(stops.begin(), stops.end(), [](const ColorStop& a, const ColorStop& b)
		{
			return a.position < b.position;
		});
	}

	void clear()
	{
		stops.clear();
	}

	const std::vector<ColorStop>& getStops() const { return stops; }
};