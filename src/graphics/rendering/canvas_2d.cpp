#pragma once
#include "graphics/rendering/canvas_2d.h"
#include "graphics/material.h"
#include "graphics/mesh.h"

#include "graphics/primitive_factory.h"
#include "core/ecs/base_components.h"
#include "core/resources.h"

#include "utility/utils.h"
#include "core/profiler.h"

#include <algorithm>
#include <config.h>

std::shared_ptr<IRenderer> Canvas2D::renderer;
mem::Arena* Canvas2D::arena;

glm::vec4 Canvas2D::m_clip		= glm::vec4(0.0f);
bool Canvas2D::isClipping		= false;
SDL_Color Canvas2D::m_color		= { 255,255,255,255 };
Depth Canvas2D::m_depth			= 1.0f;
BlendMode Canvas2D::m_blendMode = BlendMode::None;
bool Canvas2D::m_isDynamic		= false;

#pragma region Public functions
void Canvas2D::init(std::shared_ptr<IRenderer> rendererPtr, mem::Arena* frameArena)
{
	renderer = rendererPtr;
	arena = frameArena;
}

void Canvas2D::setInternalResolution(int width, int height)
{
	renderer->m_internalWidth = width;
	renderer->m_internalHeight = height;

	renderer->resize(width, height);
}

void Canvas2D::drawTest()
{

}

void Canvas2D::resize(int width, int height)
{
	renderer->resize(width, height);
}

#pragma region Draw Primitives
void Canvas2D::drawLine(glm::vec2 pointA, glm::vec2 pointB, float lineWidth)
{
	Primitive prim = PrimitiveFactory::createLine(pointA, pointB);
	Mesh* mesh = loadToArena<Mesh>();
	mesh->setVerticies(prim.vertices, prim.indices);

	drawPrimitive(CommandType::Line, mesh, lineWidth);
}

void Canvas2D::drawWireRect(glm::vec4 rect, float lineWidth)
{
	Primitive prim = PrimitiveFactory::createRect(rect);
	Mesh* mesh = loadToArena<Mesh>();
	mesh->setVerticies(prim.vertices, prim.indices);

	drawPrimitive(CommandType::Rect, mesh, lineWidth);
}

void Canvas2D::drawWireRect(glm::vec2 origin, glm::vec2 size, float lineWidth)
{
	Primitive prim = PrimitiveFactory::createRect(origin, size);
	Mesh* mesh = loadToArena<Mesh>();
	mesh->setVerticies(prim.vertices, prim.indices);

	drawPrimitive(CommandType::Rect, mesh, lineWidth);
}

void Canvas2D::drawWirePoly(std::vector<glm::vec2> points, float lineWidth)
{
	Primitive prim = PrimitiveFactory::createPoly(points);
	Mesh* mesh = loadToArena<Mesh>();
	mesh->setVerticies(prim.vertices, prim.indices);

	drawPrimitive(CommandType::Polygon, mesh, lineWidth);
}
#pragma endregion

void Canvas2D::drawSprite(ecs::Sprite& sprite, ecs::Transform2D& transform)
{
	Primitive prim = PrimitiveFactory::createQuad(sprite.uv);
	Mesh* mesh = loadToArena<Mesh>();
	mesh->setVerticies(prim.vertices, prim.indices);

	glm::mat4 model = transform.model(glm::vec2(sprite.size));

	if (sprite.blend != BlendMode::None)
		sprite.material.blendMode = sprite.blend;
	if (sprite.texture.id != 0)
		sprite.material.setTexture(M_TEX_MAIN, sprite.texture.id);

	sprite.material.setProperty(M_PROP_USE_TEX,	   sprite.texture.id != 0);
	sprite.material.setProperty(M_PROP_MAIN_COLOR, color::SDLColorToVec4(sprite.color));
	sprite.material.setProperty(M_PROP_MODEL,	   model);
	sprite.material.setProperty(M_PROP_TIME,	   core::Profiler::instance().getElapsedTime());

	if (m_depth < BACKGROUND_Z) SDL_Log("HERE: %d, %f, %f", sprite.depth, sprite.size.x, sprite.size.y);

	drawMeshWithMaterial(mesh, &sprite.material);
}

void Canvas2D::drawImage(const TexID& texture, glm::vec4 rect, gpu::UVRect& uv)
{
	Primitive prim = PrimitiveFactory::createQuad(uv);
	Mesh* mesh = loadToArena<Mesh>();
	mesh->setVerticies(prim.vertices, prim.indices);

	MaterialInstance* material = loadToArena<MaterialInstance>(Resources::sharedMat(core::GConfig.shaders.def));
	material->blendMode = m_blendMode;
	material->setProperty(M_PROP_MAIN_COLOR, color::SDLColorToVec4(m_color));
	material->setProperty(M_PROP_USE_TEX, texture.id != 0);
	material->setTexture(M_TEX_MAIN, texture);

	ecs::Transform2D transform{ .position = glm::vec3(rect.x, rect.y, 0.0f) };
	glm::mat4 model = transform.model(glm::vec2(rect.z, rect.w));
	material->setProperty(M_PROP_MODEL, model);

	material->setProperty(M_PROP_TIME, core::Profiler::instance().getElapsedTime());

	drawMeshWithMaterial(mesh, material);
}

void Canvas2D::drawQuad(const glm::vec4& rect, MaterialInstance* const mat)
{
	Primitive prim = PrimitiveFactory::createQuad();
	Mesh* mesh = loadToArena<Mesh>();
	mesh->setVerticies(prim.vertices, prim.indices);

	ecs::Transform2D transform{ .position = glm::vec3(rect.x, rect.y, 0.0f) };
	glm::mat4 model = transform.model(glm::vec2(rect.z, rect.w));
	mat->setProperty(M_PROP_MODEL, model);

	drawMeshWithMaterial(mesh, mat);
}

void Canvas2D::drawText(const std::string_view text, ecs::Transform2D& transform, const std::string_view fontName, int fontSize)
{
	MaterialInstance* material = loadToArena<MaterialInstance>(Resources::sharedMat(core::GConfig.shaders.font));
	material->blendMode = BlendMode::Alpha;
	auto* font = Resources::font(fontName.data());

	if (font->atlas(fontSize).id == 0)
		Resources::loadTTFont(core::GConfig.fromFont(fontName.data()), fontSize);
	auto* rfont = font->size(fontSize);

	material->setTexture(M_TEX_MAIN, font->size(fontSize)->atlas);
	material->setProperty(M_PROP_MAIN_COLOR, color::SDLColorToVec4(m_color));

	renderer->commandBuffer->submit(RenderCommand{
		.type = CommandType::Text,
		.depth = m_depth,
		.instance = material,
		.isClipping = isClipping,
		.clip = m_clip,
		.rdata = TextData {
			.text = text,
			.font = fontName,
			.fontSize = fontSize,
			.origin = transform.position
		},
	});
}

glm::vec2 Canvas2D::textSize(const std::string_view text, const std::string_view fontName, int fontSize)
{
	auto l_fontName = fontName;
	if (l_fontName.empty())
		l_fontName = core::GConfig.defaultFontName;
	std::string path = core::GConfig.fromFont(l_fontName.data()).string();
	auto* font = Resources::font(l_fontName.data());

	if (font->atlas(fontSize).id == 0)
		Resources::loadTTFont(path, fontSize);

	auto* rfont = Resources::font(l_fontName.data())->size(fontSize);

	glm::vec2 textSize = glm::vec2(0);
	int upperHeight = 0;
	int lowerHeight = 0;

	for (auto c = text.begin(); c != text.end(); c++)
	{
		Glyph ch = rfont->glyphs[*c];
		
		int advance = ch.advance;
		textSize.x += (advance >> 6);
		textSize.y = (ch.size.y > textSize.y) ? ch.size.y : textSize.y;
	}

	return textSize;
}

glm::vec2 Canvas2D::textOrigin(const std::string_view text, const std::string_view fontName, int fontSize)
{
	auto l_fontName = fontName;
	if (l_fontName.empty())
		l_fontName = core::GConfig.defaultFontName;

	std::string path = core::GConfig.fromFont(l_fontName.data()).string();
	auto* font = Resources::font(l_fontName.data());

	if (font->atlas(fontSize).id == 0)
		Resources::loadTTFont(path, fontSize);

	auto* rfont = Resources::font(l_fontName.data())->size(fontSize);

	glm::vec2 origin = glm::vec2(0);
	int upperHeight = 0;
	int lowerHeight = 0;
	
	for (auto c = text.begin(); c != text.end(); c++)
	{
		Glyph ch = rfont->glyphs[*c];
		int advance = ch.advance;
		origin.x += (advance >> 6);
		upperHeight = (ch.bearing.y > upperHeight) ? ch.bearing.y : upperHeight;
		lowerHeight = (ch.size.y - ch.bearing.y > lowerHeight) ? ch.size.y - ch.bearing.y : lowerHeight;
	}

	origin.x = upperHeight;
	origin.y = lowerHeight;

	return origin;
}
#pragma endregion

#pragma region Private functions
void Canvas2D::drawMeshWithMaterial(Mesh* mesh, MaterialInstance* instance)
{
	renderer->commandBuffer->submit(RenderCommand{
		.type = CommandType::Sprite,
		.depth = m_depth,
		.mesh = mesh,
		.instance = instance,
		.isClipping = isClipping,
		.clip = m_clip,
		});
}

void Canvas2D::drawPrimitive(CommandType primitiveType, Mesh* mesh, float lineWidth)
{
	MaterialInstance* mat = loadToArena<MaterialInstance>(Resources::sharedMat("primitive"));
	glm::vec4 mainColor = color::SDLColorToVec4(m_color);
	mat->setProperty(M_PROP_MAIN_COLOR, mainColor);

	renderer->commandBuffer->submit(RenderCommand{
			.type = primitiveType,
			.depth = m_depth,
			.mesh = mesh,
			.instance = mat,
			.rdata = PrimitiveData{
				.lineWidth = lineWidth
			}
		});
}
#pragma endregion