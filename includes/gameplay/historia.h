#pragma once
#include <vector>
#include <map>
#include <list>
#include <string>

#include "memory/flat_map.h"
#include "utility/id_pool.h"
#include "utility/string_interner.h"

namespace historia
{
	using RoomID = StringInterner::Id;

	constexpr const char* YAML_ENTITIES        = "entities";
	constexpr const char* YAML_ENT_NAME        = "name";
	constexpr const char* YAML_ENT_ACTIONS     = "actions";
	constexpr const char* YAML_ACTION_LABEL    = "label";
	constexpr const char* YAML_ACTION_EFFECT   = "effect";
	constexpr const char* YAML_ACT_EFFECT_TYPE = "type";

	constexpr const char* YAML_ROOMS           = "rooms";
	constexpr const char* YAML_ROOM_TITLE      = "title";
	constexpr const char* YAML_ROOM_CONTENT    = "content";
	constexpr const char* YAML_ROOM_DIRECTIONS = "directions";

	enum class EEffectType
	{
		ADD_ITEM, SHOW_TEXT, DIALOGUE, COMBAT
	};

	enum class EDirection
	{
		NORTH, SOUTH, WEST, EAST, UP, DOWN
	};

	struct Effect
	{
		EEffectType type;
		mem::flat_map<StringInterner::Id, StringInterner::Id> params;
	};

	struct Action
	{
		StringInterner::Id label;
		Effect effect;
	};

	struct Entity
	{
		StringInterner::Id name;
		std::vector<Action> actions;
	};

	struct Direction
	{
		EDirection dir;
		StringInterner::Id gotoId;
	};

	struct Room
	{
		StringInterner::Id title;
		std::string content;
		std::vector<Direction> directions;
	};

	class Story
	{
	private:
		// Split to ensure there is no accidental duplicating of string between entities and rooms
		StringInterner entityInterner;
		StringInterner roomInterner;
		IdPool<RoomID> idPool{ {.startingId = 0, .enableRecycle = false} };

		RoomID currentRoomID;
	public:
		std::map<RoomID, std::list<RoomID>> graph;
		std::vector<Entity> entities;
		mem::flat_map<RoomID, Room> rooms;
	private:
		void addEdge(RoomID from, RoomID to);
	public:
		void load(const std::string& path);
		void goTo(const RoomID id);
		const Room& currentRoom() const;
		const std::string content(const RoomID id);
		const std::string currentContent();

		const std::string parseRoomContent(const std::string& content);
	};
}