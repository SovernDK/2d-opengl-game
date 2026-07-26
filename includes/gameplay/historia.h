#pragma once
#include <yaml-cpp/yaml.h>
#include <vector>
#include <string>
#include <iostream>

namespace historia
{
	struct Choice
	{
		std::string text;
		std::string gotoId;
	};

	struct Event
	{
		std::string title;
		std::string content;
		std::vector<Choice> choices;
	};

	class Story
	{
	public:
		std::vector<Event> events;
	public:
		void Load(const std::string& path)
		{
			YAML::Node root = YAML::LoadFile(path);

			Event event;
			event.title = root["title"].as<std::string>();
			event.content = root["content"].as<std::string>();

			for (const auto& choiceNode : root["choices"])
			{
				Choice c;
				c.text = choiceNode["text"].as<std::string>();
				c.gotoId = choiceNode["goto"].as<std::string>();
				event.choices.push_back(c);
			}

			events.push_back(event);
		}
	};
}