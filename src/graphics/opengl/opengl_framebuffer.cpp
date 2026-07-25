#pragma once
#include "graphics/opengl/opengl_framebuffer.h"

#include <SDL3/SDL_log.h>

Framebuffer::~Framebuffer()
{
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &renderTexture);
}

void Framebuffer::create() {
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::attachmentColor(int index, int width, int height) 
{
	if (fbo == 0)
		return;

	m_colorIndex = index;
	m_width = width;
	m_height = height;

	if(renderTexture != 0)
	{
		glDeleteTextures(1, &renderTexture);
		renderTexture = 0;
	}

	glGenTextures(1, &renderTexture);
	glBindTexture(GL_TEXTURE_2D, renderTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, renderTexture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::resize(int newWidth, int newHeight)
{
	if (newWidth <= 0 || newHeight <= 0)
		return;

	if (newWidth == m_width && newHeight == m_height)
		return;

	attachmentColor(m_colorIndex, newWidth, newHeight);
}

void Framebuffer::bind() const {
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		SDL_Log("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}