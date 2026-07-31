#pragma once

#include "graphics/shader_program.h"
#include "graphics/shader.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/graphics.h"

#include <unordered_map>
#include <memory>
#include <map>
#include <filesystem>
#include <array>
#include <graphics/material.h>
#include "memory/flat_map.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <SDL3_mixer/SDL_mixer.h>

struct Glyph
{
	gpu::UVRect uvs{ 0 };
	glm::vec2 size{ 0 };
	glm::vec2 bearing{ 0 };
	int advance = 0;

	Glyph() = default;
	Glyph(glm::vec2 size, glm::vec2 bearing, int advance)
	{
		this->size = size;
		this->bearing = bearing;
		this->advance = advance;
	}
	Glyph(Glyph&&) = default;
	Glyph& operator=(Glyph&&) = default;
	Glyph(const Glyph&) = default;
	Glyph& operator=(const Glyph&) = default;
};


struct FontSize
{
	TexID atlas{ 0 };
	std::array<Glyph, 128> glyphs;
};

//Rework to std::array (set size at the beggining since fonts used are static)
struct Font
{
	std::vector<std::pair<int, FontSize>> sizes;
	//mem::flat_map<int, FontSize> _sizes;

	void addSize(int size)
	{
		sizes.push_back({ size, FontSize{} });
	}

	TexID atlas(int size)
	{
		for (auto& row : sizes)
		{
			if (row.first == size)
				return row.second.atlas;
		}

		return TexID{ 0 };
	}

	FontSize* size(int size)
	{
		for (auto& row : sizes)
		{
			if (row.first == size)
				return &row.second;
		}

		return nullptr;
	}
};

class Resources
{
private:
	static std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> shaders;
	static std::unordered_map<std::string, std::unique_ptr<Texture2D>> textures;
	static std::unordered_map<std::string, std::shared_ptr<Material>> sharedMaterials;

public:
	static std::string defaultTexture;
	static std::string defaultMaterial;
public:
	~Resources()
	{
		for(auto& clip : sfx)
		{
			if (clip.second)
				delete clip.second;
		}

		for(auto& track : music)
		{
			if (track.second)
				delete track.second;
		}
	};

	static std::unordered_map<std::string, Font> fonts;
	static std::unordered_map<std::string, MIX_Audio*> sfx;
	static std::unordered_map<std::string, MIX_Audio*> music;

	static std::weak_ptr<ShaderProgram> loadShader(const std::string& vertex, const std::string& fragment, const std::string& name);
	static std::weak_ptr<ShaderProgram> loadShader(const std::string& name);
	static std::shared_ptr<ShaderProgram> getStrPtrShader(const std::string& name);

	static Texture2D* saveTexture(Texture2D&& texture, std::string name);
	static Texture2D* loadTexture(const std::filesystem::path& path, const std::string& name);
	static Texture2D* loadTexture(const std::filesystem::path& path, const std::string& name, TextureBuilder& builder);
	static bool textureExist(const std::string& name);
	static void releaseTexture(const std::string& name);

	static Texture2D* texture(const std::string& name);
	static Texture2D* texture(TexID id);

	static bool loadTTFont(const std::filesystem::path path, int fontSize);
	static Font* font(const std::string& fontName);

	static MIX_Audio* loadClip(const std::filesystem::path& path, const std::string& name);
	static MIX_Audio* loadMusic(const std::filesystem::path& path, const std::string& name);

	static void addSharedMat(const std::string& handle, std::shared_ptr<Material> material);
	static std::shared_ptr<Material> sharedMat(const std::string& handle);
	static Material copySharedMat(const std::string& handle);

	static void quit();
};