#pragma once
#include "editor/editor.h"
#include "lib/imgui/imgui.h"
#include "SDL3/SDL_timer.h"
#include "debug/logging.h"

#include "core/game.h"
#include "core/profiler.h"

#include "services/service_locator.h"
#include "services/ui_service.h"

#include "rmui/ui_widget.h"
#include "graphics/rendering/canvas_2d.h"
#include "graphics/graphics.h"

#include <memory>

using namespace editor;

void Editor::update(float dt, const ICamera& camera)
{
	core::Game& game = dynamic_cast<core::Game&>(m_ctx);

#pragma region Right Bar
	float rightBarWidth = (game.screenWidth - camera.viewport().z) / 2;
	float rightBarHeight = game.screenHeight;
	glm::vec2 rightBarPos = glm::vec2(game.screenWidth - rightBarWidth, 0);

	ImGuiWindowFlags toolbar_flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		//ImGuiWindowFlags_NoScrollbar |
		//ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoCollapse;

	ImGui::SetNextWindowPos(ImVec2(rightBarPos.x, rightBarPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(rightBarWidth, rightBarHeight), ImGuiCond_Always);
	ImGui::Begin("RightBar", nullptr, toolbar_flags);

	ImGui::BeginTabBar("Editor#Right Tabs");
	if (ImGui::BeginTabItem("ECS"))
	{
		worldTree(*game.world.get());
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("UI"))
	{
		uiTree();
		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();

	ImGui::End();
#pragma endregion Right Bar

#pragma region Left Bar
	float leftBarWidth = (game.screenWidth - camera.viewport().z) / 2;
	float leftBarHeight = game.screenHeight;
	glm::vec2 leftBarPos = glm::vec2(0, 0);

	ImGui::SetNextWindowPos(ImVec2(leftBarPos.x, leftBarPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(leftBarWidth, leftBarHeight), ImGuiCond_Always);
	ImGui::Begin("LeftBar", nullptr, toolbar_flags);

	ImGui::BeginTabBar("Editor#Left Tabs");
	if (ImGui::BeginTabItem("ECS"))
	{
		entityInspector(*game.world.get());
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("UI"))
	{
		uiInspector();
		ImGui::EndTabItem();
	}

	if (ImGui::BeginTabItem("Camera"))
	{
		cameraInspector(*game.mainCam.get());
		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();
	ImGui::End();
#pragma endregion Left Bar

#pragma region Top Bar
	float topBarWidth = game.screenWidth - leftBarWidth + rightBarWidth;
	float topBarHeight = (game.screenHeight - camera.viewport().w) /2;
	glm::vec2 topBarPos = glm::vec2(leftBarWidth, 0);

	ImGui::SetNextWindowPos(ImVec2(topBarPos.x, topBarPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(topBarWidth, topBarHeight), ImGuiCond_Always);
	ImGui::Begin("TopBar", nullptr, toolbar_flags);

	generalDebugWindow(core::Profiler::instance().getFPS(),
		core::Profiler::instance().getDeltaTime(),
		core::Profiler::instance().getDrawCalls(),
		core::Profiler::instance().getMemoryStats().workingSetMB,
		core::Profiler::instance().getMemoryStats().privateBytesMB,
		core::Profiler::instance().frameBufferUsed,
		core::Profiler::instance().frameBufferCap);

	ImGui::End();
#pragma endregion Left Bar

#pragma region Bottom Bar
	float botBarWidth = game.screenWidth - leftBarWidth + rightBarWidth;
	float botBarHeight = (game.screenHeight - camera.viewport().w) / 2;
	glm::vec2 botBarPos = glm::vec2(leftBarWidth, game.screenHeight - topBarHeight);

	ImGui::SetNextWindowPos(ImVec2(botBarPos.x, botBarPos.y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(botBarWidth, botBarHeight), ImGuiCond_Always);
	ImGui::Begin("BottomBar", nullptr, toolbar_flags);
	uiDock();
	ImGui::End();
#pragma endregion Bottom Bar

#pragma region Center

#pragma endregion
}

void Editor::draw()
{
	if (uiSelectedId != 0)
	{
		auto selectedWidget = ServiceLocator::get<IUIService>()->widget(uiSelectedId);
		if (!selectedWidget)
		{
			uiSelectedId = 0;
			return;
		}
		Canvas2D::setDepth(999.0f);
		Canvas2D::setColor({0, 255, 0, 255});
		Canvas2D::drawWireRect(selectedWidget->rect.pos, selectedWidget->rect.size);
		Canvas2D::reset();
	}
}

void Editor::generalDebugWindow(Uint64 fps, float dt, int drawCalls, float memMBWorking, float memMBPrivate, size_t frameMemUsed, size_t frameMemCap)
{
	if (ImGui::BeginTable("General", 2, ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableNextColumn();
		ImGui::PushID(0);

		ImGui::SeparatorText("General");
		ImGui::Text("Elapsed time: %.2fs", SDL_GetTicks() / 1000.0f);
		ImGui::Text("Delta time: %f", dt);
		ImGui::Text("FPS: %i", fps);
		ImGui::Text("Draw calls: %i", drawCalls);

		ImGui::PopID();

		ImGui::TableNextColumn();
		ImGui::PushID(0);

		ImGui::SeparatorText("Memory");
		ImGui::Text("Working mem: %.2f MB", memMBWorking);
		ImGui::Text("Mem usage: %.2f MB", memMBPrivate);
		ImGui::Text("Frame buffer: %d / %d kB", frameMemUsed / 1024, frameMemCap / 1024);

		ImGui::PopID();

		ImGui::EndTable();
	}
}

void Editor::sceneEditor()
{
	ImGui::Begin("Toolbar");
	if (ImGui::CollapsingHeader("Scenes"))
	{
		ImGui::SeparatorText("Active scenes:");
		for (auto& scene : ServiceLocator::get<ISceneService>()->activeScenes())
		{
			ImGui::Text(scene->name().c_str());
		}
	}
	ImGui::End();
}

void Editor::stateEditor(std::string currentState)
{
	ImGui::Begin("Toolbar");
	if (ImGui::CollapsingHeader("Game Flow FSM"))
	{
		ImGui::SeparatorText("Current state");
		ImGui::Text(currentState.c_str());
	}
	ImGui::End();
}

void Editor::cameraInspector(ICamera& camera)
{
	ImGui::SeparatorText("Background Color");
	SDL_Color& bg = camera.backgroundColor;
	float color[4] = { bg.r / 255.0f, bg.g / 255.0f, bg.b / 255.0f, bg.a / 255.0f };
	if (ImGui::ColorEdit4("Background", color))
	{
		bg.r = (Uint8)(color[0] * 255.0f);
		bg.g = (Uint8)(color[1] * 255.0f);
		bg.b = (Uint8)(color[2] * 255.0f);
		bg.a = (Uint8)(color[3] * 255.0f);
	}

	ImGui::SeparatorText("Read-only");
	glm::vec4 vp = camera.viewport();
	ImGui::Text("Viewport: %.1f, %.1f, %.1f, %.1f", vp.x, vp.y, vp.z, vp.w);

	glm::vec4 b = camera.bounds();
	ImGui::Text("Bounds: %.2f, %.2f, %.2f, %.2f", b.x, b.y, b.z, b.w);

	glm::mat4 view = camera.view();
	ImGui::Text("View Matrix:");
	for (int r = 0; r < 4; ++r)
		ImGui::Text("%.2f %.2f %.2f %.2f", view[0][r], view[1][r], view[2][r], view[3][r]);

	/*ImGui::SeparatorText("Move");
	static glm::vec3 moveDelta(0.0f);
	ImGui::DragFloat3("Move delta", &moveDelta.x, 0.1f);
	if (ImGui::Button("Apply Move"))
		camera.move(moveDelta);

	ImGui::SeparatorText("Zoom");
	static float zoomDelta = 0.0f;
	ImGui::DragFloat("Zoom delta", &zoomDelta, 0.01f);
	if (ImGui::Button("Apply Zoom"))
		camera.changeZoom(zoomDelta);

	ImGui::SeparatorText("Size");
	static float w = 800, h = 600;
	ImGui::DragFloat("Resize width", &w);
	ImGui::DragFloat("Resize height", &h);
	if (ImGui::Button("Apply Resize"))
		camera.resize(w, h);				*/
}


void Editor::worldTree(ecs::ECSWorld& world)
{
	ImGui::SeparatorText("Entities");

	world.each(false, [&](ecs::Entity& e)
	{
		entityTreeNode(world, e);
	});
}

void Editor::entityTreeNode(ecs::ECSWorld& world, ecs::Entity& e)
{
	if (e.id == 0) return;

	std::string label = world.name(e.id);
	if (label.empty())
		label = std::format("Entity({}) ", e.id);

	bool isSelected = (selectedId == e.id);

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_OpenOnDoubleClick |
		ImGuiTreeNodeFlags_SpanAvailWidth;
	if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

	if (ImGui::TreeNodeEx(label.c_str(), flags, label.c_str()))
	{
		if (ImGui::IsItemClicked())
			selectedId = e.id;

		for (auto childId : e.children())
		{
			entityTreeNode(world, world.entity(childId));
		}

		ImGui::TreePop();
	}
}

void Editor::entityInspector(ecs::ECSWorld& world)
{
	if (selectedId <= 0) 
	{
		ImGui::Text("No entity is selected");
		return;
	}

	core::Game& game = dynamic_cast<core::Game&>(m_ctx);
	ecs::Entity& e = game.world->entity(selectedId);
	
	ImGui::Text("ID: %d", e.id);

	std::vector<std::string> otherComponents;
	for (auto& [type, ptr] : world.types(e.id))
	{
		if (auto transform = world.isType<ecs::Transform2D>({ type, ptr }))
		{
			if (ImGui::CollapsingHeader("Transform2D"))
			{
				ImGui::InputFloat("X: ", &transform->position.x, 1.0f, 5.0f);
				ImGui::InputFloat("Y: ", &transform->position.y, 1.0f, 5.0f);
				//ImGui::InputFloat("Z: ", &transform->position.z, 1.0f, 5.0f);
			}
		}
		else if (auto sprite = world.isType<ecs::Sprite>({ type, ptr }))
		{
			if (ImGui::CollapsingHeader("Sprite"))
			{
				//ImGui::InputInt("Texture Id: ", &sprite->textureId.id);

				ImGui::SeparatorText("UVs");
				ImGui::InputFloat("U0: ", &sprite->uv.u0, 0.01f, 0.05f);
				ImGui::InputFloat("U1: ", &sprite->uv.u1, 0.01f, 0.05f);
				ImGui::InputFloat("V0: ", &sprite->uv.v0, 0.01f, 0.05f);
				ImGui::InputFloat("V1: ", &sprite->uv.v1, 0.01f, 0.05f);

				ImGui::SeparatorText("Depth##sep");
				ImGui::InputInt("Depth: ", &sprite->depth, 1, 2);
			}
		}
		else if (auto emitter = world.isType<ecs::ParticleEmitter>({ type, ptr }))
		{
			if (ImGui::CollapsingHeader("Particle Emitter"))
			{
				ImGui::SeparatorText("Time");
				ImGui::Text(std::format("Emitter time: {}", emitter->emiting.getTime()).c_str());
				ImGui::Text(std::format("Interval time: {}", emitter->interval.getTime()).c_str());

				ImGui::SeparatorText("Area");
				ImGui::InputFloat("X", &emitter->emitArea.x, 1.0f, 5.0f);
				ImGui::InputFloat("Y", &emitter->emitArea.y, 1.0f, 5.0f);
				ImGui::InputFloat("Z", &emitter->emitArea.z, 1.0f, 5.0f);
				ImGui::InputFloat("W", &emitter->emitArea.w, 1.0f, 5.0f);

				ImGui::SeparatorText("Life Range");
				ImGui::InputFloat("X ##lr", &emitter->lifeRange.x, 1.0f, 5.0f);
				ImGui::InputFloat("Y ##lr", &emitter->lifeRange.y, 1.0f, 5.0f);

				ImGui::SeparatorText("Velocity Range");
				ImGui::InputFloat("X ##vr", &emitter->startVelocity.x, 1.0f, 5.0f);
				ImGui::InputFloat("Y ##vr", &emitter->startVelocity.y, 1.0f, 5.0f);

				ImGui::SeparatorText("Velocity Dir");
				ImGui::InputFloat("X ##vd", &emitter->direction.x, 1.0f, 5.0f);
				ImGui::InputFloat("Y ##vd", &emitter->direction.y, 1.0f, 5.0f);

				ImGui::SeparatorText("Scale Range");
				ImGui::InputFloat("X ##sr", &emitter->startScale.x, 0.1f, 0.5f);
				ImGui::InputFloat("Y ##sr", &emitter->startScale.y, 0.1f, 0.5f);

				ImGui::SeparatorText("Starting Alpha");
				int alpha = emitter->startAlpha;
				ImGui::InputInt("Alpha ##pe", &alpha, 1, 5);
				emitter->startAlpha = alpha;

				ImGui::SeparatorText("Starting Color");

				float color[3] = {
					emitter->startColor.r / 255.0f,
					emitter->startColor.g / 255.0f,
					emitter->startColor.b / 255.0f
				};

				if (ImGui::ColorEdit3("Ambient Color", color))
				{
					emitter->startColor.r = static_cast<Uint8>(color[0] * 255.0f);
					emitter->startColor.g = static_cast<Uint8>(color[1] * 255.0f);
					emitter->startColor.b = static_cast<Uint8>(color[2] * 255.0f);
				}

				// Draw gizmos
				auto topLeft  = glm::vec2(emitter->emitArea.x, emitter->emitArea.y);
				auto topRight = glm::vec2(emitter->emitArea.z, emitter->emitArea.w);

				Canvas2D::setDepth(999);
				Canvas2D::setColor(GREEN);
				Canvas2D::drawWireRect(topLeft, topRight);
				Canvas2D::reset();
			}
		}
		else
		{
			otherComponents.push_back(type.name());
		}
	}

	ImGui::SeparatorText("Not implemented");
	for (auto c : otherComponents)
	{
		ImGui::Text(c.c_str());
	}

	if (ImGui::BeginTable("ButtonGrid", 3, ImGuiTableFlags_SizingStretchProp))
	{
		ImGui::TableNextColumn();
		ImGui::PushID(0);
		if (ImGui::Button("Destroy"))
		{
			world.destroy(e);
		}
		ImGui::PopID();

		ImGui::TableNextColumn();
		ImGui::PushID(1);
		if (ImGui::Button("Enable"))
		{
			world.enable(e);
		}
		ImGui::PopID();

		ImGui::TableNextColumn();
		ImGui::PushID(2);
		if (ImGui::Button("Disable"))
		{
			world.disable(e);
		}
		ImGui::PopID();
		
		ImGui::EndTable();
	}
}


void Editor::uiDock()
{
	if (ImGui::BeginTable("ButtonGrid", 6, ImGuiTableFlags_SizingFixedFit))
	{
		ImGui::TableNextColumn();
		ImGui::PushID(0);
		if (ImGui::Button("Events"))
		{
			
		}
		ImGui::PopID();
		ImGui::EndTable();
	}
}


void Editor::uiTree()
{
	ImGui::SeparatorText("Widgets");
	uiNode(ServiceLocator::get<IUIService>()->root());
}

void Editor::uiNode(const rmui::UIWidget& widget)
{
	std::string label = std::format("widget_{}", widget.id);
	if (!widget.handle.empty())
		label = std::format("{}_{}", widget.handle, widget.id);

	bool isSelected = (uiSelectedId == widget.id);

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_OpenOnDoubleClick |
		ImGuiTreeNodeFlags_SpanAvailWidth;

	if (widget.children().empty())
		flags |= ImGuiTreeNodeFlags_Leaf;

	bool greyedOut = !widget.visible;
	if (greyedOut)
	{
		ImGui::PushStyleColor(
			ImGuiCol_Text,
			IM_COL32(120, 120, 120, 255)
		);
	}

	if (isSelected) 
		flags |= ImGuiTreeNodeFlags_Selected;

	if (ImGui::TreeNodeEx(label.c_str(), flags, label.c_str()))
	{
		if (greyedOut)
			ImGui::PopStyleColor();

		if (ImGui::IsItemClicked())
			uiSelectedId = widget.id;

		for (auto& child : widget.children())
		{
			uiNode(*child.get());
		}

		ImGui::TreePop();
	}
}

void Editor::uiInspector() const
{
	if (uiSelectedId <= 0)
	{
		ImGui::Text("No widget is selected");
		return;
	}

	auto selectedWidget = ServiceLocator::get<IUIService>()->widget(uiSelectedId);
	ImGui::Text(std::format("Id: {}", selectedWidget->id).c_str());
	ImGui::Text(std::format("Style: {}", selectedWidget->style).c_str());

	int alpha = selectedWidget->alpha();
	ImGui::InputInt("Alpha ", &alpha, 1, 5);
	selectedWidget->setAlpha(alpha);

	ImGui::SeparatorText("Local Rect");
	ImGui::InputFloat("X", &selectedWidget->localRect.pos.x, 0.01f, 0.05f,  "%.3f");
	ImGui::InputFloat("Y", &selectedWidget->localRect.pos.y, 0.01f, 0.05f,  "%.3f");
	ImGui::InputFloat("W", &selectedWidget->localRect.size.x, 0.01f, 0.05f, "%.3f");
	ImGui::InputFloat("H", &selectedWidget->localRect.size.y, 0.01f, 0.05f, "%.3f");

	ImGui::SeparatorText("World Rect");
	ImGui::Text(std::format("X: {}", selectedWidget->rect.pos.x).c_str());
	ImGui::Text(std::format("Y: {}", selectedWidget->rect.pos.y).c_str());
	ImGui::Text(std::format("W: {}", selectedWidget->rect.size.x).c_str());
	ImGui::Text(std::format("H: {}", selectedWidget->rect.size.y).c_str());

	ImGui::SeparatorText("Offset");
	ImGui::InputFloat("X ##offset", &selectedWidget->offset.x, 1.0f, 5.0f, "%.1f");
	ImGui::InputFloat("Y ##offset", &selectedWidget->offset.y, 1.0f, 5.0f, "%.1f");

	ImGui::SeparatorText("State");
	ImGui::Checkbox("Visible", &selectedWidget->visible);
	ImGui::Checkbox("Blocking", &selectedWidget->blocking);
	ImGui::Checkbox("Interactive", &selectedWidget->interactive);
	ImGui::Checkbox("Clipping", &selectedWidget->clipping);

	for(auto& component : selectedWidget->components)
	{
		if (component.first == typeid(rmui::UIBackground))
		{
			auto* back = static_cast<rmui::UIBackground*>(component.second.get());
			ImGui::SeparatorText("Background");
			ImGui::Text(std::format("Keep Aspect Ratio: {}", back->keepAspectRatio).c_str());
		}
		else if (component.first == typeid(rmui::UIText))
		{
			auto* text = static_cast<rmui::UIText*>(component.second.get());
			ImGui::SeparatorText("Text");
			ImGui::Text(std::format("Text: {}", text->text).c_str());
			ImGui::Text(std::format("Text: {}", text->font).c_str());
			ImGui::InputInt("Size: ", &text->size, 1, 5);
		}
	}

	ImGui::SeparatorText("Actions");
	if (ImGui::Button("Set Dirty"))
	{
		selectedWidget->setDirty();
	}
}