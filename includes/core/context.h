#pragma once
#include <functional>

namespace historia
{
	class Story;
}

namespace ecs
{
	class ECSWorld;
	class Entity;
}

namespace editor
{
	class Editor;
}

class ICamera;

namespace core
{
	class IContext
	{
	public:
		virtual ~IContext() = default;

		virtual historia::Story* story()    = 0;
		virtual ICamera* mainCamera()       = 0;
		virtual editor::Editor* editor()    = 0;
		virtual ecs::ECSWorld* ecsWorld()   = 0;
		virtual ecs::Entity& playerEntity() = 0;

		virtual void callback(float delay, std::function<void()> func) = 0;
	};
}
