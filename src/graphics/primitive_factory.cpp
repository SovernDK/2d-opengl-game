#pragma once
#include "graphics/primitive_factory.h"

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL.h>

using namespace std;
using namespace gpu;
using namespace glm;

Primitive PrimitiveFactory::createTriangle()
{
    vector<Vertex> verts = {
        { { 0.0f,  0.5f, 0.0f }, { 0.5f, 1.0f } },
        { {-0.5f, -0.5f, 0.0f }, { 0.5f, 1.0f } },
        { { 0.5f, -0.5f, 0.0f }, { 0.5f, 1.0f } },
    };

    std::vector<GLuint> indices = {
      0, 1, 2
    };

    return { verts, indices };
}

Primitive PrimitiveFactory::createLine()
{
    vector<Vertex> verts = {
        { { 0.0f,  0.0f, 0.0f }, {0.0f, 0.0f} },
        { { 1.0f,  1.0f, 0.0f }, {0.0f, 0.0f} }, 
    };

    vector<GLuint> indices = {
        0, 1
    };

    return { verts, indices };
}

Primitive PrimitiveFactory::createLine(vec2 start, vec2 end)
{
    vector<Vertex> verts = {
        { { start.x, start.y, 0.0f }, {0.0f, 0.0f} },
        { { end.x,   end.y,   0.0f }, {0.0f, 0.0f} },
    };

    vector<GLuint> indices = {
        0, 1
    };

    return { verts, indices };
}

#pragma region Simple Rects for primitives
Primitive PrimitiveFactory::createRect()
{
    vector<Vertex> verts = {
        { { 0.0f, 1.0f, 0.0f }, {0.0f, 1.0f} }, // top-left
        { { 1.0f, 1.0f, 0.0f }, {1.0f, 1.0f} }, // top-right
        { { 1.0f, 0.0f, 0.0f }, {1.0f, 0.0f} }, // bottom-right
        { { 0.0f, 0.0f, 0.0f }, {0.0f, 0.0f} }, // bottom-left
    };

    vector<GLuint> indices = {
        0, 1, 2, 3
    };

    return { verts, indices };
}

Primitive PrimitiveFactory::createRect(glm::vec2 origin, glm::vec2 size)
{
    float x = origin.x;
    float y = origin.y;
    float w = size.x;
    float h = size.y;

    vector<Vertex> verts = {
        { { x,     y,       0.0f }, {0.0f, 1.0f} }, // top-left
        { { x + w, y,       0.0f }, {1.0f, 1.0f} }, // top-right
        { { x + w, y + h,   0.0f }, {1.0f, 0.0f} }, // bottom-right
        { { x,     y + h,   0.0f }, {0.0f, 0.0f} }, // bottom-left
    };

    vector<GLuint> indices = {
        0, 1, 2, 3
    };

    return { verts, indices };
}

Primitive PrimitiveFactory::createRect(glm::vec4 rect)
{
	float x = rect.x;
	float y = rect.y;
	float w = rect.z;
	float h = rect.w;

	vector<Vertex> verts = {
		{ { x,     y,       0.0f }, {0.0f, 1.0f} }, // top-left
		{ { x + w, y,       0.0f }, {1.0f, 1.0f} }, // top-right
		{ { x + w, y + h,   0.0f }, {1.0f, 0.0f} }, // bottom-right
		{ { x,     y + h,   0.0f }, {0.0f, 0.0f} }, // bottom-left
	};

	vector<GLuint> indices = {
		0, 1, 2, 3
	};

	return { verts, indices };
}
#pragma endregion

Primitive PrimitiveFactory::createPoly(std::vector<glm::vec2> points)
{
	vector<Vertex> verts;
	std::vector<GLuint> indices;

	int index = 0;
	for (int i = 0; i < points.size(); i++)
	{
		verts.push_back({ { points[i].x, points[i].y, 1.0f}, {0.0f, 0.0f} });
	}

	for (int i = 0; i < points.size() - 1; i++)
	{
		indices.push_back(i);
		indices.push_back(i + 1);
	}

	indices.push_back(points.size() - 1);
	indices.push_back(0);

	return { verts, indices };
}

Primitive PrimitiveFactory::clipSpaceQuad()
{
	vector<Vertex> verts = {
		{ { -1.0f,  1.0f, 0.0f }, {0.0f, 1.0f} }, // 0 - top-left
		{ {  1.0f,  1.0f, 0.0f }, {1.0f, 1.0f} }, // 1 - top-right
		{ {  1.0f, -1.0f, 0.0f }, {1.0f, 0.0f} }, // 2 - bottom-right
		{ { -1.0f, -1.0f, 0.0f }, {0.0f, 0.0f} }, // 3 - bottom-left
	};

	vector<GLuint> indices = {
		0, 1, 3,
		1, 2, 3
	};

	return { verts, indices };
}

Primitive PrimitiveFactory::createQuad(gpu::UVRect uvs)
{
	vector<Vertex> verts = {
		{ { 0.0f, 1.0f, 0.0f }, {uvs.u0, uvs.v1} }, // 0 - top-left
		{ { 1.0f, 1.0f, 0.0f }, {uvs.u1, uvs.v1} }, // 1 - top-right
		{ { 1.0f, 0.0f, 0.0f }, {uvs.u1, uvs.v0} }, // 2 - bottom-right
		{ { 0.0f, 0.0f, 0.0f }, {uvs.u0, uvs.v0} }, // 3 - bottom-left
	};

    vector<GLuint> indices = {
		0, 1, 3,
		1, 2, 3
    };

    return { verts, indices };
}

Primitive PrimitiveFactory::createQuad(vec2 topLeft, vec2 topRight, vec2 botRight, vec2 botLeft, gpu::UVRect uvs)
{
	vector<Vertex> verts = {
		{ { topLeft.x,  topLeft.y,  0.0f }, {uvs.u0, uvs.v1} }, // 0 - top-left
		{ { topRight.x, topRight.y, 0.0f }, {uvs.u1, uvs.v1} }, // 1 - top-right
		{ { botRight.x, botRight.y, 0.0f }, {uvs.u1, uvs.v0} }, // 2 - bottom-right
		{ { botLeft.x,  botLeft.y,  0.0f }, {uvs.u0, uvs.v0} }, // 3 - bottom-left
	};

	vector<GLuint> indices = {
		0, 1, 3,
		1, 2, 3 
	};

	return { verts, indices };
}

Primitive PrimitiveFactory::createQuad(glm::vec4 rect, gpu::UVRect uvs)
{
	float x = rect.x;
	float y = rect.y;
	float w = rect.z;
	float h = rect.w;

	vector<Vertex> verts = {
		{ { x,     y,       0.0f }, {uvs.u0, uvs.v1} }, // top-left
		{ { x + w, y,       0.0f }, {uvs.u1, uvs.v1} }, // top-right
		{ { x + w, y + h,   0.0f }, {uvs.u1, uvs.v0} }, // bottom-right
		{ { x,     y + h,   0.0f }, {uvs.u0, uvs.v0} }, // bottom-left
	};

	vector<GLuint> indices = {
		0, 1, 2, 3
	};

	return { verts, indices };
}