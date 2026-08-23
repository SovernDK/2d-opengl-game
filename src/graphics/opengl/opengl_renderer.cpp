#pragma once
#include "graphics/opengl/opengl_renderer.h"

#include "graphics/mesh.h"
#include "graphics/material.h"
#include "core/profiler.h"

#include <graphics/primitive_factory.h>
#include <resources.h>
#include "utility/utils.h"

OpenGlRenderer::~OpenGlRenderer()
{
	delete commandBuffer;
	delete screenMaterial;
}

void OpenGlRenderer::init(int screenWidth, int screenHeight)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	m_internalWidth = screenWidth;
	m_internalHeight = screenHeight;

	commandBuffer = new RenderQueue();
	framebuffer.create();
	framebuffer.attachmentColor(0, m_internalWidth, m_internalHeight);

	auto mesh = PrimitiveFactory::clipSpaceQuad();

	screenBuffer.initBuffer(mesh.vertices.size(), mesh.indices.size());
	screenBuffer.uploadDynamicData(mesh.vertices, mesh.indices);
}

void OpenGlRenderer::render(ICamera& camera)
{
	if (screenMaterial == nullptr)
	{
		screenMaterial = new MaterialInstance(Resources::sharedMat("screen"));
		screenMaterial->setTexture(M_TEX_RENDER, framebuffer.colorBuffer());
	}

	framebuffer.bind();
	setViewport(0, 0, m_internalWidth, m_internalHeight);
	clear(camera.bgColor());

	commandBuffer->sort();
	renderToBuffer(camera);
	framebuffer.unbind();

	//Render to texture
	setViewport(camera.viewport());
	clear(0.05f, 0.05f, 0.07f, 1);
	renderToTexture(*screenMaterial);

	core::Profiler::instance().setDrawCalls(drawCalls);
	commandBuffer->clear();
	drawCalls = 0;
}

/* Used when screen dimensions change */
void OpenGlRenderer::resize(int width, int height)
{
	framebuffer.resize(width, height);
}

void OpenGlRenderer::clear(glm::vec4 color)
{
	clear(color.x, color.y, color.z, color.w);
}

void OpenGlRenderer::clear(float r, float g, float b, float a)
{
	glDisable(GL_SCISSOR_TEST);

	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGlRenderer::setViewport(glm::vec4 vp)
{
	setViewport(vp.x, vp.y, vp.z, vp.w);
}

void OpenGlRenderer::setViewport(float x, float y, float z, float w)
{
	glViewport(x, y, z, w);
}

void OpenGlRenderer::renderToBuffer(ICamera& camera)
{
	for (auto& cmd : commandBuffer->buffer)
	{
		blend(cmd);
		clip(cmd);

		if (cmd.type == CommandType::Sprite)
			renderImage(camera, cmd);
		else if (cmd.type == CommandType::Text)
			renderText(camera, cmd);
		else
			renderPrimitive(camera, cmd);

		drawCalls++;
	}
}

void OpenGlRenderer::renderToTexture(MaterialInstance& mat)
{
	screenBuffer.startDraw();

	screenMaterial->bind();

	screenBuffer.draw(PrimitiveType::Triangle);
	screenBuffer.endDraw();

	screenMaterial->unbind();
}

void OpenGlRenderer::blend(RenderCommand cmd)
{
	switch (cmd.instance->blendMode)
	{
	case BlendMode::None:
		glDisable(GL_BLEND);
		break;
	case BlendMode::Alpha:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BlendMode::Additive:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	}
}

void OpenGlRenderer::clip(RenderCommand cmd)
{
	if (cmd.isClipping)
	{
		glEnable(GL_SCISSOR_TEST);

		float sx = (float)m_internalWidth / (float)m_screenWidth;
		float sy = (float)m_internalHeight / (float)m_screenHeight;

		float x = cmd.clip.x * sx;
		float y = cmd.clip.y * sy;
		float w = cmd.clip.z * sx;
		float h = cmd.clip.w * sy;

		glScissor((GLint)x, (GLint)y, (GLsizei)w, (GLsizei)h);
	}
	else
	{
		glDisable(GL_SCISSOR_TEST);
	}
}

void OpenGlRenderer::renderImage(ICamera& camera, RenderCommand cmd)
{
	GPUBuffer buffer;
	buffer.initBuffer(cmd.mesh->vertexCount(), cmd.mesh->indexCount());
	if(cmd.isDynamic)
		buffer.uploadDynamicData(cmd.mesh->vertices, cmd.mesh->indices);
	else
		buffer.uploadStaticData(cmd.mesh->vertices, cmd.mesh->indices);

	buffer.startDraw();

	cmd.instance->setProperty(M_PROP_PROJECTION, camera.projection());
	cmd.instance->setProperty(M_PROP_VIEW, camera.view());
	cmd.instance->bind();

	buffer.draw(PrimitiveType::Triangle);

	cmd.instance->unbind();

	buffer.endDraw();
	buffer.destroy();
}

void OpenGlRenderer::renderPrimitive(ICamera& camera, RenderCommand cmd)
{
	GPUBuffer buffer;
	buffer.initBuffer(cmd.mesh->vertexCount(), cmd.mesh->indexCount());
	if (cmd.isDynamic)
		buffer.uploadDynamicData(cmd.mesh->vertices, cmd.mesh->indices);
	else
		buffer.uploadStaticData(cmd.mesh->vertices, cmd.mesh->indices);

	buffer.startDraw();

	//glLineWidth(cmd.primitive.lineWidth);

	cmd.instance->setProperty(M_PROP_PROJECTION, camera.projection() * camera.view());
	cmd.instance->bind();

	PrimitiveType current = PrimitiveType::Line;
	switch (cmd.type)
	{
		case CommandType::Line:
			current = PrimitiveType::Line; break;
		case CommandType::Rect:
			current = PrimitiveType::LineLoop; break;
		case CommandType::Triangle:
			current = PrimitiveType::LineLoop; break;
		case CommandType::Polygon:
			current = PrimitiveType::LineLoop; break;
		default:
			current = PrimitiveType::Line;
	}

	buffer.draw(current);

	cmd.instance->unbind();

	buffer.endDraw();
	buffer.destroy();
}

void OpenGlRenderer::renderText(ICamera& camera, RenderCommand cmd)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	auto& data = std::get<TextData>(cmd.rdata);
	auto mesh = PrimitiveFactory::createQuad();

	float x = data.origin.x;
	float y = data.origin.y;
	float scale = 1;

	GPUBuffer buffer;
	buffer.initBuffer(mesh.vertices.size(), mesh.indices.size());
	if (cmd.isDynamic)
		buffer.uploadDynamicData(mesh.vertices, mesh.indices);
	else
		buffer.uploadStaticData(mesh.vertices, mesh.indices);

	cmd.instance->setProperty(M_PROP_PROJECTION, camera.projection());
	cmd.instance->bind();

	buffer.startDraw();
	
	auto* font = Resources::font(data.font.data())->size(data.fontSize);

	for (auto c = data.text.begin(); c != data.text.end(); c++)
	{
		Glyph ch = font->glyphs[*c];

		float xpos = x + ch.bearing.x;
		float ypos = y - ch.bearing.y;

		float w = ch.size.x;
		float h = ch.size.y;

		// update VBO for each character
		mesh = PrimitiveFactory::createQuad(
			glm::vec2(xpos, ypos + h),
			glm::vec2(xpos + w, ypos + h),
			glm::vec2(xpos + w, ypos),
			glm::vec2(xpos, ypos),
			ch.uvs
		);

		buffer.updateGeometryData(mesh.vertices, mesh.indices);
		buffer.draw(PrimitiveType::Triangle);

		x += (ch.advance >> 6);// +current.spacing * scale;
	}

	cmd.instance->unbind();

	buffer.endDraw();
	buffer.destroy();
}