#pragma once
#include "services/service.h"
#include "SDL3/SDL.h"
#include "debug/logging.h"

#include <typeindex>
#include <unordered_map>
#include <memory>
#include <stdexcept>

class ServiceLocator
{
private:
	static std::unordered_map<std::type_index, std::shared_ptr<void>>& services()
	{
		static std::unordered_map<std::type_index, std::shared_ptr<void>> instance;
		return instance;
	}
public:
	template<typename TInterface, typename TImpl>
	static void registerService(std::shared_ptr<TImpl> service)
	{
		services()[typeid(TInterface)] = std::move(service);
	}

	template<typename TInterface, typename TImpl, typename... Args>
	static std::shared_ptr<TImpl> registerService(Args&&... args)
	{
		auto service = std::make_shared<TImpl>(std::forward<Args>(args)...);
		services()[typeid(TInterface)] = service;
		return service;
	}

	template<typename T>
	static T* get()
	{
		auto it = services().find(typeid(T));

		if (it == services().end())
		{
			ErrorLog("Service", "Tried to access service[%s] but it doesn't exist!", typeid(T).name());
			throw std::runtime_error("Tried to access non-existent service");
			return nullptr;
		}

		return static_cast<T*>(it->second.get());
	}

	static void quit()
	{
		services().clear();
	}
};