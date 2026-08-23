#pragma once
#include <vector>
#include <map>
#include <list>
#include <string>
#include <memory>

#include "memory/flat_map.h"
#include "utility/id_pool.h"
#include "utility/string_interner.h"

#include "globals/data_defs.h"

namespace historia
{
	using RoomID = StringInterner::Id;
	using ParagraphId = StringInterner::Id;

	constexpr const char* YAML_ROOMS           = "rooms";
	constexpr const char* YAML_ROOM_TITLE      = "title";
	constexpr const char* YAML_ROOM_HEADER	   = "header";
	constexpr const char* YAML_ROOM_PAR		   = "paragraph";
	constexpr const char* YAML_ROOM_DIRECTIONS = "directions";
	constexpr const char* YAML_ROOM_CHOICES    = "choices";

	constexpr const char* YAML_PARAGRAPHS	 = "paragraphs";
	constexpr const char* YAML_PAR_CHOICES	 = "choices";
	constexpr const char* YAML_PAR_TITLE	 = "title";
	constexpr const char* YAML_PAR_TEXT		 = "text";
	constexpr const char* YAML_PAR_GOTO		 = "goto";
	constexpr const char* YAML_PAR_GIVE_ITEM = "give_item";

	enum class EEffectType
	{
		GIVE_ITEM, SHOW_TEXT, DIALOGUE, COMBAT
	};

	enum class EDirection
	{
		NORTH, SOUTH, WEST, EAST, COUNT
	};

	struct ChoiceEffect
	{
		EEffectType type;
		mem::flat_map<std::string, std::string> params;
	};

	struct Choice
	{
		std::string content;
		ParagraphId gotoId;
		std::vector<ChoiceEffect> effects;
		bool visited = false;
	};

	struct Paragraph
	{
		StringInterner::Id title;
		std::string text;
		std::vector<Choice> choices;
	};

	struct Direction
	{
		EDirection dir;
		StringInterner::Id gotoId;
	};

	struct Room
	{
		StringInterner::Id title;
		std::string header;
		ParagraphId paragraph;
		mem::flat_map<EDirection, Direction> directions;
	};

	struct GraphEdge
	{
		RoomID toId;
		EDirection dir;
	};

	class Story
	{
	private:
		StringInterner roomInterner;
		StringInterner parghInterner;
		//IdPool<RoomID> idPool{ {.startingId = 0, .enableRecycle = false} };

		RoomID currentRoomID;
		ParagraphId currParagraphId;
	public:
		std::map<RoomID, std::list<RoomID>> graph;
		std::map<RoomID, std::list<GraphEdge>> _graph;
		mem::flat_map<RoomID, Room> rooms;
		mem::flat_map<ParagraphId, Paragraph> paragraphs;
		std::unordered_map<std::string, bool> flags;
	private:
		void addEdge(RoomID from, RoomID to);
	public:
		void load(const std::string& path);
		void goTo(const ParagraphId id);
		void applyEffect(const ChoiceEffect& effect);
		void move(const RoomID id);
		const Room& currentRoom() const;
		const Paragraph& currParagraph() const;
		const std::string content(const ParagraphId id);
		const std::string currentContent();
		const std::string currentHeader() const;

		const std::string parseRoomContent(const std::string& content);
	};
}