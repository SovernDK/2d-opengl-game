#pragma once
#include <glad/glad.h>

class Framebuffer
{
private:
	GLuint fbo = 0;
	GLuint renderTexture = 0;

	int m_colorIndex = 0;
	int m_width = 0;
	int m_height = 0;
public:
	Framebuffer() = default;

	Framebuffer(const Framebuffer&) = delete;
	Framebuffer& operator=(const Framebuffer&) = delete;
	Framebuffer(Framebuffer&&) = delete;
	Framebuffer& operator=(Framebuffer&&) = delete;

	~Framebuffer();

	void create();
	void attachmentColor(int index, int width, int height);
	void resize(int newWidth, int newHeight);
	void bind() const;
	void unbind();

	GLuint colorBuffer() const { return renderTexture; }
};