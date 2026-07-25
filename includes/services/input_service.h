#pragma once
#include "services/service.h"

#include "glm/glm.hpp"
#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_events.h"

#include <string>
#include <nlohmann/json.hpp>

struct InputAction
{
	std::string action = "";
	bool enabled = false;

	InputAction() = default;
	InputAction(std::string name)
	{
		action = name;
	}
};

struct InputBindings
{
	SDL_Keycode key;
	SDL_EventType eventType;
	uint8_t button = 0;
	uint8_t device = 0; //0 - keyboard, 1 - mouse, 2 - gamepad

	InputBindings() = default;
	InputBindings(SDL_Keycode key, SDL_EventType eventType, uint8_t button, uint8_t device)
	{
		this->key		= key;
		this->eventType = eventType;
		this->button	= button;
		this->device	= device;
	}
};

class IInputService : public IService
{
protected:
	const bool* keys = SDL_GetKeyboardState(nullptr);
	std::unordered_map<std::string, bool> actions;
	std::unordered_map<std::string, std::vector<InputBindings>> bindings;
public:
	virtual ~IInputService() = default;
	
	virtual void load(nlohmann::json data) = 0;
	virtual glm::vec2 getMousePos() = 0;
	virtual bool getAction(std::string action) = 0;
	virtual void processEvents(SDL_Event& event) = 0;
	virtual void reset() = 0;
};

class InputService : public IInputService
{
public:
	~InputService() = default;

	void load(nlohmann::json data) override;
	glm::vec2 getMousePos() override;
	bool getAction(std::string action) override;
	void processEvents(SDL_Event& event) override;
	void reset() override;
};