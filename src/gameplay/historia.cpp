#pragma once
#include "gameplay/historia.h"
#include <iostream>
#include <yaml-cpp/yaml.h>
#include <regex>
#include <magic_enum/magic_enum.hpp>

#include "debug/logging.h"
#include "core/texts.h"

using namespace historia;

void Story::load(const std::string& path)
{
	YAML::Node root = YAML::LoadFile(path);

	// Load config
	const auto& config = root["config"];

	// Load entities
	for (const auto& entityNode : root[YAML_ENTITIES])
	{
		Entity e;
		e.name = entityInterner.intern(entityNode[YAML_ENT_NAME].as<std::string>());

		for (const auto& actionNode : entityNode[YAML_ENT_ACTIONS])
		{
			Action a;
			a.label = entityInterner.intern(actionNode[YAML_ACTION_LABEL].as<std::string>());

			Effect ef;
			auto type = actionNode[YAML_ACTION_EFFECT][YAML_ACT_EFFECT_TYPE].as<std::string>();
			auto efType = magic_enum::enum_cast<EEffectType>(type, magic_enum::case_insensitive);
			if (efType.has_value())
				ef.type = efType.value();
			else
			{
				ErrorLog("Story", "Invalid Effect type (%s) in entity - %s", type.c_str(), entityInterner.toString(e.name).c_str());
				continue;
			}

			for (const auto& param : actionNode[YAML_ACTION_EFFECT])
			{
				auto key = entityInterner.intern(param.first.as<std::string>());
				auto value = entityInterner.intern(param.second.as<std::string>());
				ef.params.insert(key, value);
			}

			a.effect = ef;
			e.actions.push_back(a);
		}

		entities.push_back(e);
	}

	// Load rooms
	std::set<StringInterner::Id> duplicates;
	for (const auto& roomNode : root[YAML_ROOMS])
	{
		Room room;
		room.title = roomInterner.intern(roomNode[YAML_ROOM_TITLE].as<std::string>());
		room.content = roomNode[YAML_ROOM_CONTENT].as<std::string>();

		//Check if all contents have valid entities
		if (!duplicates.insert(room.title).second)
			FatalErrorLog("Story", "Found duplicate event title - %s!", roomInterner.toString(room.title).c_str());

		for (const auto& dirNode : roomNode[YAML_ROOM_DIRECTIONS])
		{
			Direction d;
			for (const auto& kv : dirNode)
			{
				auto key = kv.first.as<std::string>();
				auto value = kv.second.as<std::string>();

				auto eDir = magic_enum::enum_cast<EDirection>(key, magic_enum::case_insensitive);
				if (eDir.has_value())
					d.dir = eDir.value();
				else
				{
					ErrorLog("Story", "Invalid direction (%s) in event - %s", key.c_str(), roomInterner.toString(room.title).c_str());
					continue;
				}

				d.gotoId = roomInterner.intern(value);
			}

			room.directions.push_back(d);
			addEdge(room.title, d.gotoId);
		}

		rooms.insert(room.title, room);
	}

	currentRoomID = 1;
}

void Story::goTo(const RoomID id)
{
	if (rooms.find(id) == rooms.end())
	{
		FatalErrorLog("Story", "Invalid room id![%s]", roomInterner.toString(id).c_str());
	}
	currentRoomID = id;
}

const Room& Story::currentRoom() const
{
	return rooms.at(currentRoomID);
}

const std::string Story::content(const RoomID id)
{
	return parseRoomContent(rooms[id].content);
}

const std::string Story::currentContent()
{
	return parseRoomContent(rooms[currentRoomID].content);
}

const std::string Story::parseRoomContent(const std::string& content)
{
	std::string result;
	std::regex textPattern("\\{([^}]*)\\}");
	auto begin = std::sregex_iterator(content.begin(), content.end(), textPattern);
	auto end = std::sregex_iterator();

	auto last = content.cbegin();
	for (auto it = begin; it != end; ++it)
	{
		std::smatch match = *it;
		std::string key = match[1].str();

		result.append(last, content.cbegin() + match.position());
		result += core::GTexts.tryGet(key, match[0].str());

		last = content.cbegin() + match.position() + match.length();
	}
	result.append(last, content.cend());

	std::regex pattern("\\[br\\]");
	result = std::regex_replace(result, pattern, "\n");

	return result;
}

void Story::addEdge(RoomID from, RoomID to)
{
	graph[from].push_back(to);
}