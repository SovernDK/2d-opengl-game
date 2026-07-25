#pragma once
#include <deque>
#include <variant>
#include <vector>
#include <string>

#include "glm/glm.hpp"

class Mesh;
class Material;
class MaterialInstance;
class Texture2D;

class IRenderer;

struct PrimitiveData
{
	float lineWidth = 1.0f;
};

struct TextData
{
	std::string_view text;
	std::string_view font;
	int fontSize = 0;
	glm::vec2 origin = glm::vec2(0.0f);
};

using RenderData = std::variant<PrimitiveData, TextData>;

enum class CommandType
{
	Sprite, Text, Triangle, Rect, Line, Polygon
};

struct RenderCommand
{
	CommandType type			= CommandType::Sprite;
	int depth					= 0;
	Mesh* mesh					= nullptr;
	MaterialInstance* instance	= nullptr;
	
	bool isClipping				= false;
	glm::vec4 clip				= glm::vec4(0.0f);

	RenderData rdata;
};

class RenderQueue
{
public:
	std::deque<RenderCommand> buffer;
public:
	void submit(RenderCommand cmd);
	void sort();
	void clear();
};