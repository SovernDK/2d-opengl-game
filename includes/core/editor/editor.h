#pragma once
#include "glm/glm.hpp"
#include "context.h"
#include "ecs/ecs.h"

#include "editor/world_map_edit.h"
#include <SDL3/SDL_stdinc.h>

namespace rmui { class UIWidget; }
class ICamera;

namespace editor
{
	class IEditor
	{
	protected:
		core::IContext& m_ctx;
	public:
		IEditor(core::IContext& ctx) : m_ctx(ctx) {};
		~IEditor() = default;
		virtual void update(float dt, const ICamera& camera) = 0;
		virtual void draw() = 0;
	};

	class Editor : public IEditor
	{
	private:
		ecs::EntityId selectedId = 0;
		int uiSelectedId = 0;

		std::unique_ptr<IWorldMapEditor> worldMap;
	public:
		Editor(core::IContext& ctx) : IEditor(ctx) 
		{
			worldMap = std::make_unique<WorldMapEditor>(ctx);
		};
		~Editor() = default;

	public:
		void update(float dt, const ICamera& camera) override; 
		void draw() override;

		void toolbar(int width, int height, glm::vec2 pos);

		void generalDebugWindow(Uint64 fps, float dt, int drawCalls, float memMBWorking, float memMBPrivate, size_t frameMemUsed, size_t frameMemCap);
		void cameraInspector(ICamera& camera);

		void worldTree(ecs::ECSWorld& world);
		void entityTreeNode(ecs::ECSWorld& world, ecs::Entity& entity);
		void entityInspector(ecs::ECSWorld& world);

		void uiDock();
		void showEditableTableWindow(bool* p_open);

		void sceneEditor();
		void stateEditor(std::string currentState);

		void uiTree();
		void uiNode(const rmui::UIWidget& widget);
		void uiInspector() const;
	};
}