#pragma once
#include <string>
#include "core/context.h"
#include <future>

class IScene
{
protected:
	std::string m_name;
	bool started		  = false;
	core::IContext* m_ctx = nullptr;
public:
	IScene(core::IContext* ctx, std::string name)
	{
		m_name = name;
		m_ctx = ctx;
	}
	virtual ~IScene() = default;
	IScene(IScene& other) = delete;
	IScene& operator=(IScene& other) = delete;

	virtual void start()
	{
		started = true;
	}
	virtual void update(float dt) { if (started) return; }
	virtual void draw() { if (started) return; }
	virtual void unload() { started = false; }
	virtual void quit() { started = false; }

	std::string name() const { return m_name; };
};