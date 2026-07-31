#pragma once
#include "editor/world_map_edit.h"
#include <imgui.h>
#include <game.h>
#include <scenes/world_map_scene.h>
#include <scenes/loading_scene.h>

void editor::WorldMapEditor::render()
{
	core::Game& game = dynamic_cast<core::Game&>(ctx);

	ImGui::Indent();
	if (ImGui::CollapsingHeader("Height map"))
	{
		ImGui::InputInt("width", &screenWidth);
		ImGui::InputInt("height", &screenHeight);

		ImGui::InputFloat("frequency", &frequency, 0.001f);
		ImGui::InputInt("octaves", &octaves);
		ImGui::InputFloat("gain", &gain, 0.05f);

		ImGui::InputInt("seed", &seed);
	}

	if (ImGui::CollapsingHeader("Lightning"))
	{
		ImGui::InputFloat("Sun X", &sunX, 0.1f, 0.25f);
		ImGui::InputFloat("Sun Y", &sunY, 0.1f, 0.25f);
		ImGui::InputFloat("Sun Z", &sunZ, 0.1f, 0.25f);

		ImGui::SeparatorText("Shadows");

		ImGui::InputInt("Steps", &steps);
		ImGui::InputFloat("StepSize", &stepSize, 0.0001f, 0.001f, "%.4f");
		ImGui::InputFloat("Diffuse Ambient", &diffuseAmbient, 0.01f, 0.1f);
		ImGui::InputFloat("Shadow Length", &shadowLength, 0.001f, 0.1f);
		ImGui::InputFloat("Shadow strength", &shadowStr, 0.01f, 0.1f);
		ImGui::InputFloat("Water shadow strength", &waterShadowStr, 0.01f, 0.1f);

		ImGui::SeparatorText("Ambient");

		float color[3] = { ambientColor.r, ambientColor.g, ambientColor.b };
		if (ImGui::ColorEdit3("Ambient Color", color))
		{
			ambientColor.r = color[0];
			ambientColor.g = color[1];
			ambientColor.b = color[2];
		}
		ImGui::InputFloat("Ambient strength", &ambientStrength, 0.01f);
	}

	if (ImGui::CollapsingHeader("Terrain"))
	{
		ImGui::InputFloat("Specular strength ##", &terrainSpecStr, 0.01f);
		ImGui::InputFloat("Specular spread ##", &terrainSpecSpred, 2.0f);
		ImGui::InputFloat("Normal strength ##", &terrainNormStr, 0.01f);

		ImGui::SeparatorText("Forest");
		ImGui::InputFloat("Forest bump ##", &forestBumpStr, 0.01f);
		ImGui::InputFloat("Forest normal ##", &forestNormStr, 0.01f);
	}

	if (ImGui::CollapsingHeader("Water"))
	{
		float color[3] = { waterColor.r, waterColor.g, waterColor.b };
		if (ImGui::ColorEdit3("Shallow Color", color))
		{
			waterColor.r = color[0];
			waterColor.g = color[1];
			waterColor.b = color[2];
		}

		float deepColor[3] = { deepWaterColor.r, deepWaterColor.g, deepWaterColor.b };
		if (ImGui::ColorEdit3("Deep Color", deepColor))
		{
			deepWaterColor.r = deepColor[0];
			deepWaterColor.g = deepColor[1];
			deepWaterColor.b = deepColor[2];
		}

		ImGui::InputFloat("Specular strength ##x", &waterSpecStr, 0.01f);
		ImGui::InputFloat("Specular spread ##x", &waterSpecSpread, 2.0f);
		ImGui::InputFloat("Normal strength ##x", &waterNormalStr, 0.01f);
		ImGui::InputFloat("Water level", &waterLevel, 0.01f);
	}

	if (ImGui::Button("Regenerate"))
	{
		//Load world map scene
		ServiceLocator::get<ISceneService>()->requestRemoveLast();
		ServiceLocator::get<ISceneService>()->requestTransition<WorldMapScene>(TransitionMode::Additive);
		ServiceLocator::get<ISceneService>()->requestTransition<LoadingScene>(TransitionMode::Additive);
	};
	ImGui::Unindent();

	ecs::EntityId id = game.ecsWorld()->entity("Settings").id;

	if (id != 0)
	{
		auto* settings = game.ecsWorld()->getMod<ecs::MapGenSettings>(id);

		settings->frequency = frequency;
		settings->octaves = octaves;
		settings->gain = gain;
		settings->seed = seed;

		settings->width = screenWidth;
		settings->height = screenHeight;

		settings->sunX = sunX;
		settings->sunY = sunY;
		settings->sunZ = sunZ;
		settings->ambientColor = ambientColor;
		settings->ambientStrength = ambientStrength;
		settings->diffuseAmbient = diffuseAmbient;

		settings->steps = steps;
		settings->stepSize = stepSize;
		settings->shadowLength = shadowLength;
		settings->shadowStr = shadowStr;
		settings->waterShadowStr = waterShadowStr;

		settings->terrainSpecStr = terrainSpecStr;
		settings->terrainSpecSpred = terrainSpecSpred;
		settings->terrainNormStr = terrainNormStr;

		settings->forestNormStr = forestNormStr;
		settings->forestBumpStr = forestBumpStr;

		settings->waterColor = waterColor;
		settings->deepWaterColor = deepWaterColor;
		settings->waterSpecStr = waterSpecStr;
		settings->waterNormalStr = waterNormalStr;
		settings->waterSpecSpread = waterSpecSpread;
		settings->waterLevel = waterLevel;
	}
}