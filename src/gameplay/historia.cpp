#pragma once
#include "gameplay/historia.h"

#include <iostream>
#include <yaml-cpp/yaml.h>
#include <regex>
#include <magic_enum/magic_enum.hpp>

#include "debug/logging.h"
#include "core/texts.h"
#include "core/globals/gdata.h"

using namespace historia;

void Story::load(const std::string& path)
{
	YAML::Node root = YAML::LoadFile(path);

	// Load config
	const auto& config = root["config"];

	// Load rooms
	std::set<StringInterner::Id> duplicates;
	for (const auto& roomNode : root[YAML_ROOMS])
	{
		Room room;
		room.title = roomInterner.intern(roomNode[YAML_ROOM_TITLE].as<std::string>());
		room.header = roomNode[YAML_ROOM_HEADER].as<std::string>();
		room.paragraph = parghInterner.intern(roomNode[YAML_ROOM_PAR].as<std::string>());

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

			room.directions.insert(d.dir, d);
			addEdge(room.title, d.gotoId);
			_graph[room.title].push_back({ d.gotoId , d.dir });

		}

		rooms.insert(room.title, room);
	}

	// Load Paragraphs
	for (const auto& paragraphNode : root[YAML_PARAGRAPHS])
	{
		Paragraph paragraph;
		paragraph.title = parghInterner.intern(paragraphNode[YAML_PAR_TITLE].as<std::string>());
		paragraph.text = paragraphNode[YAML_PAR_TEXT].as<std::string>();

		for (const auto& choiceNode : paragraphNode[YAML_PAR_CHOICES])
		{
			Choice c;
			c.content = choiceNode[YAML_PAR_TEXT].as<std::string>();
			c.gotoId = parghInterner.intern(choiceNode[YAML_PAR_GOTO].as<std::string>());

			if (auto giveItemEff = choiceNode[YAML_PAR_GIVE_ITEM])
			{
				ChoiceEffect effect{ .type = EEffectType::GIVE_ITEM };
				effect.params.insert(YAML_PAR_GIVE_ITEM, giveItemEff.as<std::string>());
				c.effects.push_back(effect);
			}

			paragraph.choices.push_back(c);
		}

		paragraphs.insert(paragraph.title, paragraph);
	}

	currentRoomID = roomInterner.intern(root["config"]["start_room"].as<std::string>());
	currParagraphId = parghInterner.intern(root["config"]["start_par"].as<std::string>());
}

void Story::goTo(const ParagraphId id)
{
	if (paragraphs.find(id) == paragraphs.end())
	{
		FatalErrorLog("Story", "Invalid paragraph's id![%s]", parghInterner.toString(id).c_str());
	}
	currParagraphId = id;
}

void Story::move(const RoomID id)
{
	if (rooms.find(id) == rooms.end())
	{
		FatalErrorLog("Story", "Invalid room id![%s]", roomInterner.toString(id).c_str());
	}
	currentRoomID = id;
	goTo(currentRoom().paragraph);
}

const Room& Story::currentRoom() const
{
	return rooms.at(currentRoomID);
}

const Paragraph& Story::currParagraph() const
{
	return paragraphs.at(currParagraphId);
}

const std::string Story::content(const ParagraphId id)
{
	return parseRoomContent(paragraphs[id].text);
}

const std::string Story::currentContent()
{
	return content(currParagraphId);
}

const std::string Story::currentHeader() const
{
	return core::GTexts.get(currentRoom().header);
}

const std::string Story::parseRoomContent(const std::string& content)
{
	std::string result;
	std::regex textPattern("\\{([^}]*)\\}");
	auto begin = std::sregex_iterator(content.begin(), content.end(), textPattern);
	auto end = std::sregex_iterator();

	auto last = content.cbegin();
	for (auto& it = begin; it != end; ++it)
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