#pragma once
#include "glm/glm.hpp"

template<typename T>
struct Rect
{
	T x = 0, y = 0, w = 0, h = 0;

	Rect(T v)
	{
		x, y, w, h = v;
	}

	Rect(T x, T y, T w, T h)
	{
		x = x;
		y = y;
		w = w;
		h = h;
	}

	glm::vec4 vector() const { return glm::vec4(x, y, w, h); };
};

struct Rectf : Rect<float> { };

struct Recti : Rect<int> { };