#pragma once
#include <string>
#include "core/context.h"
#include <future>

class IScene
{
private:
	std::string m_name;
	bool started = false;
public:
	IScene(std::string name)
	{
		m_name = name;
	}
	virtual ~IScene() = default;
	IScene(IScene& other) = delete;
	IScene& operator=(IScene& other) = delete;

	virtual void start(core::IContext* ctx) { started = true; }
	virtual void update(core::IContext* ctx, float dt) { if (started) return; }
	virtual void draw(core::IContext* ctx) { if (started) return; }
	virtual void unload(core::IContext* ctx) { started = false; }
	virtual void quit(core::IContext* ctx) { started = false; }

	std::string name() const { return m_name; };
};