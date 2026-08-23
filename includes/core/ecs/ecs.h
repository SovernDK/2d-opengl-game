#pragma once
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <set>
#include <string>
#include <memory>
#include <format>
#include <functional>
#include <iostream>
#include <tuple>

#include "ecs/base_components.h"
#include "utility/id_pool.h"

#include "services/service_locator.h"
#include "debug/logging.h"

namespace ecs
{
#pragma region Definition
	class ECSWorld;
	class EntityBuilder;

	using EntityId = long;

	class Entity
	{
	private:
		bool destroyed = false;
		bool enabled = true;
	public:
		EntityId id = 0;
		ECSWorld* world = nullptr;
	public:
		~Entity() = default;

		Entity() = default;
		Entity(ECSWorld* world, EntityId id) : id(id), world(world) {};

		Entity(const Entity&) = delete;
		Entity& operator=(const Entity&) = delete;
		Entity(Entity&&) = default;
		Entity& operator=(Entity&&) = default;

		template<typename T> Entity& add(T&& copyObj);
		template<typename T> Entity& add(T& copyObj);

		template<typename T> Entity& remove();

		Entity& childOf(EntityId parentId);
		Entity& childOf(Entity& parent);

		template<typename T> const T* const get();
		template<typename T> T* const getMod();

		Entity& enable();
		Entity& disable();

		void destroy();

		const std::vector<EntityId>& children();

		friend class ECSWorld;
	};

	struct ITypeBucket
	{
		virtual ~ITypeBucket() = default;

		virtual size_t size() const = 0;
		virtual std::type_index type() = 0;

		virtual void removeById(EntityId id) = 0;

		virtual void* getRawPtr(EntityId id) = 0;
		virtual void forEach(std::function<void(EntityId, void*)> fn) = 0;
	};

	template<typename T>
	struct TypeBucket : ITypeBucket
	{
		std::unordered_map<EntityId, std::unique_ptr<T>> items;
		size_t size() const override { return items.size(); }
		std::type_index type() override { return typeid(T); }

		void removeById(EntityId id) override;
		void* getRawPtr(EntityId id) override;
		void forEach(std::function<void(EntityId, void*)> fn) override;
	};

	struct ISystem
	{
		virtual ~ISystem() = default;

		virtual void run(ECSWorld& world) = 0;
	};

	template<typename... Ts>
	struct System : ISystem
	{
		using Fn = std::function<void(Entity&, Ts&...)>;
		Fn logic;

		System(Fn logic) : logic(logic) {};

		void run(ECSWorld& world) override;
	};

	class ECSWorld
	{
	private:
		std::unordered_map<EntityId, Entity> entities;
		std::unordered_map<std::string, Entity*> lookupTable;
		mem::flat_map<std::type_index, std::vector<EntityId>> deletedComponents;

		std::unordered_map<std::type_index, std::shared_ptr<ITypeBucket>> components;
		std::unordered_set<EntityId> deletedEntities;

		using ParentId = EntityId; 
		using ChildId = EntityId;
		std::unordered_map<ParentId, std::vector<ChildId>> childrenOf;
		std::unordered_map<ChildId, ParentId> parentOf;

		std::vector<std::unique_ptr<ISystem>> systems;

		mem::flat_map<std::type_index, std::vector<std::function<void()>>> onAddComponent;
		mem::flat_map<std::type_index, std::vector<std::function<void()>>> onRemoveComponent;

		IdPool<EntityId> idPool;
	public:
		ECSWorld() 
		{
			//Emplace empty entity with id 0
			entities.emplace(0, Entity(this, 0));
		};
		~ECSWorld() = default;

		ECSWorld(const ECSWorld&) = delete;
		ECSWorld& operator=(const ECSWorld&) = delete;
		ECSWorld(ECSWorld&&) = delete;
		ECSWorld& operator=(ECSWorld&&) = delete;
	public:
		Entity& create();
		Entity& create(const std::string& handle);

		Entity& entity(EntityId id);
		Entity& entity(std::string handle);

		void destroy(Entity& e);
		void destroy(EntityId id);

		void enable(EntityId id);
		void enable(Entity& e);

		void disable(EntityId id);
		void disable(Entity& e);

		void process(float dt);
		auto types(EntityId id) -> std::vector<std::pair<std::type_index, void*>>;

		void quit();
		void print();
		std::string name(EntityId id);

		template<typename T> void addComponent(EntityId id, T&& copyObj);
		template<typename T> void addComponent(EntityId id, T& obj);
		template<typename T> void addComponent(EntityId id);

		template<typename T> void removeComponent(EntityId id, std::type_index type);

		void setChildOf(EntityId parent, EntityId child);
		const std::vector<EntityId>& getChildrenOf(EntityId parentId);

		template<typename T> const T* const get(EntityId id);
		template<typename T> T* const getMod(EntityId id);

		void each(std::function<void(Entity&)> fn);
		void each(bool hasParent, std::function<void(Entity&)> fn);

		template<typename... Ts, typename Fn> void view(Fn fn);
		template<typename... Ts, typename Fn> void system(Fn fn);
		template<typename T> T* isType(const std::pair<std::type_index, void*>& entry);
		template<typename T> TypeBucket<T>* getType();
	
		template<typename T> void onAdded(std::function<void()> func);
		template<typename T> void onRemoved(std::function<void()> func);
	private:
		void processDestroy(EntityId id);
	};
#pragma endregion

#pragma region Implementation
	template<typename T>
	inline void TypeBucket<T>::removeById(EntityId id)
	{
		items.erase(id);
	}

	template<typename T>
	inline void* TypeBucket<T>::getRawPtr(EntityId id)
	{
		auto it = items.find(id);
		return it != items.end() ? it->second.get() : nullptr;
	}

	template<typename T>
	inline void TypeBucket<T>::forEach(std::function<void(EntityId, void*)> fn)
	{
		for (auto& [id, ptr] : items)
			fn(id, ptr.get());
	}

	#pragma region Entity
	template<typename T>
	inline Entity& Entity::add(T&& obj)
	{
		world->addComponent<T>(id, std::forward<T>(obj));
		return *this;
	}

	template<typename T>
	inline Entity& Entity::add(T& obj)
	{
		world->addComponent<T>(id, obj);
		return *this;
	}

	template<typename T> 
	inline Entity& Entity::remove()
	{
		world->removeComponent<T>(id, typeid(T));
		return *this;
	}

	inline Entity& Entity::childOf(EntityId parentId)
	{
		world->setChildOf(parentId, id);
		return *this;
	}

	inline Entity& Entity::childOf(Entity& parent)
	{
		world->setChildOf(parent.id, id);
		return *this;
	}

	template<typename T>
	const T* const Entity::get()
	{
		return world->get<T>(id);
	}

	template<typename T>
	T* const Entity::getMod()
	{
		return world->getMod<T>(id);
	}

	inline Entity& Entity::enable()
	{
		world->enable(*this);
		return *this;
	}

	inline Entity& Entity::disable()
	{
		world->disable(*this);
		return *this;
	}

	inline void Entity::destroy()
	{
		world->destroy(*this);
	}

	inline const std::vector<EntityId>& Entity::children() 
	{
		return world->getChildrenOf(id);
	}
	#pragma endregion

	#pragma region System
	template<typename... Ts>
	inline void System<Ts...>::run(ECSWorld& world)
	{
		world.view<Ts...>(logic);
	}
	#pragma endregion
	
	#pragma region ECS World
	inline Entity& ECSWorld::create()
	{
		EntityId id = idPool.next();
		auto [it, emplaced] = entities.emplace(id, Entity(this, id));

		return it->second;
	}

	inline Entity& ECSWorld::create(const std::string& handle)
	{
		EntityId id = idPool.next();

		auto [it, inserted] = entities.emplace(id, Entity(this, id));
		lookupTable.emplace(handle, &it->second);

		return it->second;
	}

	inline Entity& ECSWorld::entity(EntityId id)
	{
		if (entities.contains(id))
		{
			return entities.at(id);
		}

		WarnLog("ECS", "Tried to extract entity by non-existent id %d!", id);
		return entities[0];
	}

	inline Entity& ECSWorld::entity(std::string handle)
	{
		auto it = lookupTable.find(handle);
		if (it != lookupTable.end())
		{
			return entities.at(it->second->id);
		}

		WarnLog("ECS", "Tried to extract entity by non-existent handle %s!", handle.c_str());
		return entities[0];
	}

	inline void ECSWorld::destroy(Entity& e)
	{
		e.destroyed = true;
		deletedEntities.insert(e.id);

		for (auto childId : e.children())
		{
			destroy(childId);
		}
	}

	inline void ECSWorld::destroy(EntityId id)
	{
		auto& e = entity(id);
		destroy(e);
	}

	inline void ECSWorld::enable(EntityId id)
	{
		enable(entity(id));
	}

	inline void ECSWorld::enable(Entity& e)
	{
		e.enabled = true;
		for (auto childId : e.children())
		{
			enable(childId);
		}
	}

	inline void ECSWorld::disable(EntityId id)
	{
		disable(entity(id));
	}

	inline void ECSWorld::disable(Entity& e)
	{
		e.enabled = false;
		for (auto childId : e.children())
		{
			enable(childId);
		}
	}

	inline void ECSWorld::each(std::function<void(Entity&)> fn)
	{
		for (auto& [_, entity] : entities)
		{
			fn(entity);
		}
	}

	inline void ECSWorld::each(bool hasParent, std::function<void(Entity&)> fn)
	{
		if (hasParent)
		{
			for (auto& [rootId, children] : childrenOf)
			{
				for (auto& childId : children)
					fn(entities[childId]);
			}
		}
		else
		{
			for (auto& [id, entity] : entities)
			{
				const bool entityHasParent = parentOf.find(id) != parentOf.end();
				if (!entityHasParent)
					fn(entity);
			}
		}
	}

	inline void ECSWorld::process(float dt)
	{
		for (auto& sys : systems)
		{
			sys->run(*this);
		}

		//destroy entities marked 'deleted'
		for (auto& id : deletedEntities)
		{
			processDestroy(id);
		}

		for (auto& [type, ids] : deletedComponents)
		{
			const auto& typeBucket = components[type];
			for (auto& id : ids)
			{
				typeBucket->removeById(id);
			}

			for(auto& func : onRemoveComponent[type])
				func();
					
		}

		deletedEntities.clear();
	}

	inline auto ECSWorld::types(EntityId id) -> std::vector<std::pair<std::type_index, void*>>
	{
		std::vector<std::pair<std::type_index, void*>> out;
		for (auto& [type, bucket] : components)
		{
			auto* ptr = bucket->getRawPtr(id);
			if(ptr)
				out.push_back(std::pair(bucket->type(), ptr));
		}

		return out;
	}

	inline void ECSWorld::quit()
	{
		components.clear();
		lookupTable.clear();
		entities.clear();
	}

	inline void ECSWorld::print()
	{
		SDL_Log("--- Entities ---");
		for (auto& [id, entity] : entities)
		{
			SDL_Log("Entity: %d", id);
		}
		SDL_Log("--- END ---");

		SDL_Log("--- Components ---");
		for (auto& [type, bucket] : components)
		{
			SDL_Log("Type: %s, size: %d", type.name(), bucket->size());
		}

		SDL_Log("--- Systems ---");
		SDL_Log("size: %d", systems.size());
		SDL_Log("--- END ---");
	}

	inline std::string ECSWorld::name(EntityId id)
	{
		for (auto& [name, e] : lookupTable)
		{
			if (e->id == id)
			{
				return name;
			}
		}

		return {};
	}

	template<typename T>
	inline void ECSWorld::addComponent(EntityId id, T&& obj)
	{
		if (id == 0)
		{
			ErrorLog("ECS", "Cannot add component %s to entity with invalid id 0!", typeid(T).name());
			return;
		}

		auto [it, _] = components.try_emplace(typeid(T), std::make_unique<TypeBucket<T>>());
		auto* bucket = static_cast<TypeBucket<T>*>(it->second.get());
		bucket->items[id] = std::make_unique<T>(std::forward<T>(obj));

		for (auto& func : onAddComponent[typeid(T)])
			func();
	};

	template<typename T>
	inline void ECSWorld::addComponent(EntityId id, T& obj)
	{
		if (id == 0)
		{
			ErrorLog("ECS", "Cannot add component %s to entity with invalid id 0!", typeid(T).name());
			return;
		}

		auto [it, _] = components.try_emplace(typeid(T), std::make_unique<TypeBucket<T>>());
		auto* bucket = static_cast<TypeBucket<T>*>(it->second.get());
		bucket->items[id] = std::make_unique<T>(obj);

		for (auto& func : onAddComponent[typeid(T)])
			func();
	};

	template<typename T>
	inline void ECSWorld::addComponent(EntityId id)
	{
		if (id == 0)
		{
			std::string logMsg = std::format("Cannot add component {} to entity with invalid id 0!", typeid(T).name());
			ErrorLog("ECS", logMsg);
			return;
		}

		auto [it, _] = components.try_emplace(typeid(T), std::make_unique<TypeBucket<T>>());
		auto* bucket = static_cast<TypeBucket<T>*>(it->second.get());
		bucket->items[id] = std::make_unique<T>();

		for (auto& func : onAddComponent[typeid(T)])
			func();
	};

	template<typename T> 
	inline void ECSWorld::removeComponent(EntityId id, std::type_index type)
	{
		auto bucket = std::static_pointer_cast<TypeBucket<T>>(components.at(typeid(T)));
		if (bucket->items.contains(id))
		{
			deletedComponents[type].push_back(id);
		}
	}

	inline void ECSWorld::setChildOf(EntityId parentId, EntityId childId)
	{
		if (childrenOf.contains(parentId))
		{
			childrenOf.at(parentId).push_back(childId);
			parentOf.insert({ childId, parentId });
		}
		else
		{
			std::vector<EntityId> children{ childId };

			childrenOf.insert({ parentId, children });
			parentOf.insert({ childId, parentId });
		}
	}

	inline const std::vector<EntityId>& ECSWorld::getChildrenOf(EntityId parentId)
	{
		if (childrenOf.contains(parentId))
			return childrenOf.at(parentId);
		else
			return {};
	}

	template<typename T>
	inline const T* const ECSWorld::get(EntityId id)
	{
		auto bucket = std::static_pointer_cast<TypeBucket<T>>(components.at(typeid(T)));
		if (bucket->items.contains(id))
		{
			return bucket->items.at(id).get();
		}

		ErrorLog("ECS", "Tried to get component %s (for id(%d) but it doesn't exist!", typeid(bucket).name(), id);
		return nullptr;
	}

	template<typename T>
	inline T* const ECSWorld::getMod(EntityId id)
	{
		auto bucket = std::static_pointer_cast<TypeBucket<T>>(components.at(typeid(T)));
		if (bucket->items.contains(id))
		{
			return bucket->items.at(id).get();
		}

		ErrorLog("ECS", "Tried to get component %s (for id(%d) but it doesn't exist!", typeid(bucket).name(), id);
		return nullptr;
	}

	template<typename... Ts, typename Fn>
	inline void ECSWorld::view(Fn fn)
	{
		static_assert(std::is_invocable_v<Fn, Entity&, Ts&...>,
			"view<Ts...>: lambda signature must match (Entity&, Ts&...)");

		auto buckets = std::make_tuple(getType<Ts>()...);
		if ((... || (std::get<TypeBucket<Ts>*>(buckets) == nullptr)))
			return;

		auto* first = std::get<0>(buckets);

		for (auto& [id, _] : first->items)
		{
			auto& e = entity(id);
			if (e.destroyed || !e.enabled) 
				continue;

			bool hasAll = (std::get<TypeBucket<Ts>*>(buckets)->items.contains(id) && ...);
			if (!hasAll) continue;

			fn(e, *static_cast<Ts*>(std::get<TypeBucket<Ts>*>(buckets)->items.at(id).get())...);
		}
	}

	template<typename... Ts, typename Fn>
	inline void ECSWorld::system(Fn fn)
	{
		static_assert(std::is_invocable_v<Fn, Entity&, Ts&...>,
			"view<Ts...>: lambda signature must match (Entity&, Ts&...)");
		systems.emplace_back(std::make_unique<System<Ts...>>(fn));
	}

	template<typename T>
	inline T* ECSWorld::isType(const std::pair<std::type_index, void*>& entry)
	{
		if (entry.first == std::type_index(typeid(T)))
			return static_cast<T*>(entry.second);
		return nullptr;
	}

	template<typename T>
	inline TypeBucket<T>* ECSWorld::getType()
	{
		auto it = components.find(typeid(T));
		if (it == components.end()) return nullptr;
		return static_cast<TypeBucket<T>*>(it->second.get());
	}

	template<typename T> 
	inline void ECSWorld::onAdded(std::function<void()> func)
	{
		onAddComponent[typeid(T)].push_back(func);
	}

	template<typename T> 
	inline void ECSWorld::onRemoved(std::function<void()> func)
	{
		onRemoveComponent[typeid(T)].push_back(func);
	}

	inline void ECSWorld::processDestroy(EntityId id)
	{
		auto& e = entity(id);

		if(parentOf.contains(id))
		{
			auto& parentsChildren = childrenOf[parentOf[id]];
			auto it = std::find(parentsChildren.begin(), parentsChildren.end(), id);
			if (it != parentsChildren.end())
				parentsChildren.erase(it);

			parentOf.erase(id);
		}

		auto it = std::find_if(lookupTable.begin(), lookupTable.end(), [id](const auto& pair)
		{
			return pair.second->id == id;
		});
		if (it != lookupTable.end()) lookupTable.erase(it);

		entities.erase(id);

		for (auto& [_, bucketPtr] : components)
		{
			bucketPtr->removeById(id);
		}

		idPool.releaseId(id);
	}
	#pragma endregion

#pragma endregion
}