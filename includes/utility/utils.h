#pragma once
#define NOMINMAX
#include "SDL3/SDL.h"

#include "glm/glm.hpp"

#include <ext/scalar_constants.hpp>
#include <random>
#include <optional>
#include <string>

#define HexToRGBA(hex, alpha) color::HexToRGB(hex, alpha)

namespace util_vec {
    inline bool equal(glm::vec2 a, glm::vec2 b)
    {
        return glm::distance(a, b) < glm::epsilon<float>();
    }

	inline glm::vec2 lerp(glm::vec2 a, glm::vec2 b, float t)
	{
		return glm::vec2{ a.x + t * (b.x - a.x),
						  a.y + t * (b.y - a.y) };
	}
}

namespace random
{
    static std::mt19937 gen;

    inline void Init(std::optional<uint32_t> seed)
    {
        if (seed.has_value())
            gen.seed(seed.value());
        else
            gen.seed(std::random_device{}());
    }

    inline uint32_t Seed()
    {
        return std::uniform_int_distribution<uint32_t>(0, (std::numeric_limits<uint32_t>::max)())(gen);
    }

    inline int Int(int min, int max)
    {
        if (min > max) {
            std::swap(min, max);
            SDL_Log("Warning: min value was greater than max value in Random::Int, swapping values to avoid issues!");
        }
        else if (max == min)
        {
            SDL_Log("Warning: min value was equal to max value in Random::Int, returning the only possible value!");
            return min;
        }

        return std::uniform_int_distribution<>(min, max)(gen);
    }

    inline float Float(float min, float max)
    {
        if (min > max) {
            std::swap(min, max);
            SDL_Log("Warning: min value was greater than max value in Random::Float, swapping values to avoid issues!");
        }
        else if (max == min)
        {
            SDL_Log("Warning: min value was equal to max value in Random::Float, returning the only possible value!");
            return min;
        }

        return std::uniform_real_distribution<>(min, max)(gen);
    }

    inline bool Chance(float probability)
    {
        return std::uniform_real_distribution<>(0.0f, 1.0f)(gen) < probability;
    }
}

namespace text
{
    inline std::vector<std::string> split(const std::string& text, std::string delimeter)
    {
        std::vector<std::string> result;
        size_t start = 0;

        for (size_t found = text.find(delimeter); found != std::string::npos; found = text.find(delimeter, start))
        {
            result.emplace_back(text.begin() + start, text.begin() + found);
            start = found + delimeter.size();
        }
        if (start != text.size())
            result.emplace_back(text.begin() + start, text.end());
        return result;
    }

    inline std::string toLower(std::string& text)
    {
        for (auto& c : text)
        {
            c = std::tolower(c);
        }

        return text;
    }
}

namespace color
{
    inline SDL_Color lerp(const SDL_Color& a, const SDL_Color& b, float t)
    {
        return {
            static_cast<uint8_t>(a.r + t * (b.r - a.r)),
            static_cast<uint8_t>(a.g + t * (b.g - a.g)),
            static_cast<uint8_t>(a.b + t * (b.b - a.b)),
            static_cast<uint8_t>(a.a + t * (b.a - a.a))
        };
    }

    inline SDL_Color HexToRGB(const std::string& hex, uint8_t alpha)
    {
        std::string s = hex;

        if (s[0] == '#')
            s = s.substr(1);

        if (s.length() != 6)
            throw std::runtime_error("Invalid hex color");

        SDL_Color c;
        c.r = static_cast<uint8_t>(std::stoi(s.substr(0, 2), nullptr, 16));
        c.g = static_cast<uint8_t>(std::stoi(s.substr(2, 2), nullptr, 16));
        c.b = static_cast<uint8_t>(std::stoi(s.substr(4, 2), nullptr, 16));
        c.a = alpha;

        return c;
    }

    inline glm::vec4 SDLColorToVec4(SDL_Color color)
    {
        return glm::vec4(
            	color.r / 255.0f,
            	color.g / 255.0f,
            	color.b / 255.0f,
            	color.a / 255.0f
            );
    }

	inline glm::vec3 SDLColorToVec3(SDL_Color color)
	{
		return glm::vec3(
			color.r / 255.0f,
			color.g / 255.0f,
			color.b / 255.0f
		);
	}
}

namespace utils
{
    inline float normalize(float value, float min, float max)
    {
        if (min > max) {
            std::swap(min, max);
            SDL_Log("Warning: min value was greater than max value in utils::normalize, swapping values to avoid issues!");
        }
        else if (max == min)
        {
            SDL_Log("Warning: min value was equal to max value in utils::normalize, returning 0.0f to avoid division by zero!");
            return 0.0f;
        }
        return (value - min) / (max - min);
    }

    inline void packFloatToRGBA(float value, uint8_t* out)
    {
        glm::vec4 enc = glm::vec4(1.0f, 255.0f, 65025.0f, 16581375.0f) * value;
        enc = glm::fract(enc); // keep fractional part of each component
        // subtract bleed from higher channels into lower ones
        enc.x -= enc.y * (1.0f / 255.0f);
        enc.y -= enc.z * (1.0f / 255.0f);
        enc.z -= enc.w * (1.0f / 255.0f);

        out[0] = (uint8_t)std::round(enc.x * 255.0f);
        out[1] = (uint8_t)std::round(enc.y * 255.0f);
        out[2] = (uint8_t)std::round(enc.z * 255.0f);
        out[3] = (uint8_t)std::round(enc.w * 255.0f);
    }
}