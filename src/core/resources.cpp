#pragma once
#include "utility/file_util.h"
#include "core/resources.h"
#include "core/config.h"
#include "debug/logging.h"

#include "services/service_locator.h"
#include "services/file_service.h"
#include "services/log_service.h"
#include "services/audio_service.h"

using namespace std;
using namespace core;

std::unordered_map<std::string, Font>					   Resources::fonts;
														   
std::unordered_map<std::string, shared_ptr<ShaderProgram>> Resources::shaders;
std::unordered_map<std::string, unique_ptr<Texture2D>>	   Resources::textures;
std::unordered_map<std::string, shared_ptr<Material>>	   Resources::sharedMaterials;
														   
std::unordered_map<std::string, MIX_Audio*>				   Resources::sfx;
std::unordered_map<std::string, MIX_Audio*>				   Resources::music;

std::string Resources::defaultTexture = "default";
std::string Resources::defaultMaterial = "default";

std::weak_ptr<ShaderProgram> Resources::loadShader(const string& vertex, const string& fragment, const std::string& name)
{
	std::shared_ptr<ShaderProgram> shaderProgram = std::make_shared<ShaderProgram>();
	Shader fShader, vShader;

	#ifdef _DEBUG
	const std::string vertexPath = std::format("{}shaders/{}", PROJECT_ROOT_DIR, vertex);
	const std::string fragmentPath = std::format("{}shaders/{}", PROJECT_ROOT_DIR, fragment);
	#else
	const std::string vertexPath = file_util::getPath(GConfig.getShadersPath(), vertex).string();
	const std::string fragmentPath = file_util::getPath(GConfig.getShadersPath(), fragment).string();
	#endif

	fShader.FromFile(fragmentPath,  GL_FRAGMENT_SHADER);
	vShader.FromFile(vertexPath,    GL_VERTEX_SHADER);

	shaderProgram->init();
	shaderProgram->attach(fShader);
	shaderProgram->attach(vShader);
	shaderProgram->link();

	fShader.destroy();
	vShader.destroy();

	shaders[name] = move(shaderProgram);
	return std::weak_ptr<ShaderProgram>(shaders[name]);
}

std::weak_ptr<ShaderProgram> Resources::loadShader(const string& name)
{
	if (shaders.contains(name))
	{
		return shaders[name];
	}

	ErrorLog("Resources", "Couldn't find shader with handle %s", name.c_str());
	return std::weak_ptr<ShaderProgram>();
}

std::shared_ptr<ShaderProgram> Resources::getStrPtrShader(const string& name)
{
	if (shaders.contains(name))
	{
		return shaders[name];
	}

	ErrorLog("Resources", "Couldn't find shader with handle %s", name.c_str());
	return std::shared_ptr<ShaderProgram>();
}

Texture2D* Resources::saveTexture(Texture2D&& texture, std::string name)
{
	textures[name] = std::make_unique<Texture2D>(std::move(texture));
	return textures[name].get();
}

Texture2D* Resources::loadTexture(const fs::path& path, const std::string& name)
{
	StbiImage img = ServiceLocator::get<IFileService>()->loadFile(path.string(), (int) STBI_rgb_alpha);

	if (!img.check)
	{
		return nullptr;
	}

	Texture2D texture = TextureBuilder()
		.setFiltering(GL_NEAREST)
		.setWrapping(GL_REPEAT)
		.setWrapAxis(true, true)
		.setBorderColor(0, 0, 0)
		.build(img.width, img.height, img.data);

	InfoLog("Resources", "Loaded texture with handle %s", name.c_str());
	textures[name] = std::make_unique<Texture2D>(std::move(texture));
	return textures[name].get();
}

Texture2D* Resources::loadTexture(const fs::path& path, const std::string& name, TextureBuilder& builder)
{
	StbiImage img = ServiceLocator::get<IFileService>()->loadFile(path.string(), (int)STBI_rgb_alpha);

	if (!img.check)
	{
		return nullptr;
	}

	Texture2D texture = builder.build(img.width, img.height, img.data);

	InfoLog("Resources", "Loaded texture with handle %s", name.c_str());
	textures[name] = std::make_unique<Texture2D>(std::move(texture));
	return textures[name].get();
}

Texture2D* Resources::texture(const string& name)
{
	if (textures.contains(name))
	{
		return textures[name].get();
	}

	ErrorLog("Resources", "Couldn't find texture with handle %s", name.c_str());
	return textures[defaultTexture].get();
}

Texture2D* Resources::texture(TexID id)
{
	std::find_if(textures.begin(), textures.end(), [&](auto& p)
	{
		return p.second->id.id == id.id;
	});

	ErrorLog("Resources", "Couldn't find texture with id %s", id.id);
	return textures[defaultTexture].get();
}

bool Resources::textureExist(const std::string& name)
{
	return textures.contains(name);
}

void Resources::releaseTexture(const std::string& name)
{
	textures.erase(name);
}

bool Resources::loadTTFont(const fs::path path, int fontSize)
{
	string fontName = file_util::getNameFromPath(path);
	
	if (!fs::exists(path))
	{
		ErrorLog("Font", "Font file does not exist: %s", path.string().c_str());
		return false;
	}

	FT_Library ft;
	if (FT_Init_FreeType(&ft))
	{
		ErrorLog("Font", "ERROR::FREETYPE: Could not init FreeType Library");
		return false;
	}

	FT_Face face;
	FT_Error err = FT_New_Face(ft, path.string().c_str(), 0, &face);
	if (err)
	{
		ErrorLog("Font", "ERROR::FREETYPE: Failed to load font (%s - size: %d) from: %s [FT error: %s]",
			fontName, fontSize, path.string(), err);
		return false;
	}
	FT_Set_Pixel_Sizes(face, 0, fontSize);

	FT_Error err2 = FT_Load_Char(face, 'X', FT_LOAD_RENDER);
	if (err2)
	{
		ErrorLog("Font", "ERROR::FREETYPE: Failed to load Glyph (%s - size: %d) from: %s  [FT error: %s]",
			fontName.c_str(), fontSize, path.string().c_str(), err2);
		return false;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

	int atlasWidth = 0;
	int atlasHeight = 0;
	float padding = 1.0f; // Padding between glyphs in the atlas

	fonts[fontName].addSize(fontSize);

	for (unsigned char c = 0; c < 128; c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			WarnLog("Font", "ERROR::FREETYTPE: Failed to load Glyph");
			continue;
		}

		auto& glyph = face->glyph;
		auto& bmp = face->glyph->bitmap;

		glm::vec2 glyphSize = glm::vec2(bmp.width, bmp.rows);
		glm::vec2 bearing = glm::vec2(glyph->bitmap_left, glyph->bitmap_top);
		int advance = glyph->advance.x;

		fonts[fontName].size(fontSize)->glyphs[c] = Glyph(glyphSize, bearing, advance);

		atlasWidth += bmp.width + padding;
		atlasHeight = std::max(atlasHeight, (int)bmp.rows);
	}

	atlasHeight += padding * 2;

	auto bufferSize = atlasWidth * atlasHeight;
	unsigned char* buffer = new unsigned char[bufferSize]();

	// pack glyphs
	// Rework to grid packing algorithm to make better use of space
	int xOffset = 0;
	for (unsigned char c = 32; c < 128; c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;
		auto& bmp = face->glyph->bitmap;
		
		for (unsigned int row = 0; row < bmp.rows; row++)
		{
			auto dst = (row + (int) padding) * atlasWidth + xOffset;
			auto src = row * bmp.width;

			memcpy(buffer + dst, bmp.buffer + src, bmp.width);
		}

		// Add some debug tool to test different fontSizes and see how the UVs are calculated
		// Reduce by half-pixel to avoid bleeding into neighboring glyphs
		float offset = 0.5f;
		fonts[fontName].size(fontSize)->glyphs[c].uvs.u0 = (float)(xOffset - offset) / (float)atlasWidth;
		fonts[fontName].size(fontSize)->glyphs[c].uvs.u1 = (float)(xOffset + bmp.width + offset) / (float)atlasWidth;
		fonts[fontName].size(fontSize)->glyphs[c].uvs.v0 = 0.0f;
		fonts[fontName].size(fontSize)->glyphs[c].uvs.v1 = (float)(bmp.rows + padding) / (float) (atlasHeight - padding);

		xOffset += bmp.width + padding;
	}

	Texture2D glyphTexture = TextureBuilder()
		.setFormat(GL_RED)
		.setInternalFormat(GL_RED)
		.setFiltering(GL_LINEAR)
		.setWrapping(GL_CLAMP_TO_EDGE)
		.build(atlasWidth, atlasHeight, buffer);

	auto atlasTextureName = fontName + "_" + std::to_string(fontSize);

	fonts[fontName].size(fontSize)->atlas = glyphTexture.id;
	saveTexture(std::move(glyphTexture), atlasTextureName);

	// Save font atlasses to files for testing
	//ServiceLocator::get<IFileService>()->savePng(file_util::createPath("assets", atlasTextureName + ".png").string(), atlasWidth, atlasHeight, buffer);

	InfoLog("Font", "Font %s size: %d loaded succesfully", fontName.c_str(), fontSize);
	delete[] buffer;

	return true;
}

Font* Resources::font(const std::string& fontName)
{
	if (!fonts.contains(fontName))
	{
		WarnLog("Font", "Font \"%s\" doesn't exist!", fontName.c_str());
		return &fonts["default"];
	}
	return &fonts[fontName];
}

MIX_Audio* Resources::loadClip(const std::filesystem::path& path, const std::string& name)
{
	char* _path = nullptr;

	SDL_asprintf(&_path, "%s", path.string().c_str());
	MIX_Audio* audio = MIX_LoadAudio(ServiceLocator::get<IAudioService>()->mixer(), _path, false);
	if (!audio)
	{
		ErrorLog("Audio", "Couldn't open audio file: %s", path.string().c_str());
		SDL_free(_path);
		return nullptr;
	}
	
	SDL_free(_path);
	InfoLog("Audio", "Loaded audio clip with handle %s", name.c_str());

	sfx[name] = audio;
	return audio;
}

MIX_Audio* Resources::loadMusic(const std::filesystem::path& path, const std::string& name)
{
	char* _path = nullptr;

	SDL_asprintf(&_path, "%s", path.string().c_str());
	MIX_Audio* audio = MIX_LoadAudio(ServiceLocator::get<IAudioService>()->mixer(), _path, false);
	if (!audio)
	{
		ErrorLog("Audio", "Couldn't open music file: %s", path.string().c_str());
		SDL_free(_path);
		return nullptr;
	}

	SDL_free(_path);
	InfoLog("Audio", "Loaded music with handle %s", name.c_str());

	music[name] = audio;
	return audio;
}

void Resources::addSharedMat(const std::string& handle, std::shared_ptr<Material> material)
{
	sharedMaterials[handle] = material;
}

std::shared_ptr<Material> Resources::sharedMat(const std::string& handle)
{
	if (sharedMaterials.contains(handle))
	{
		return sharedMaterials[handle];
	}

	return sharedMaterials[defaultMaterial];
}

Material Resources::copySharedMat(const std::string& handle)
{
	return *sharedMaterials.at(handle).get();
}

void Resources::quit() { }