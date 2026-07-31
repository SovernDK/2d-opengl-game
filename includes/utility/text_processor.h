#pragma once
#include <regex>
#include <string>
#include <vector>
#include <memory>

struct IRule {};

struct ReplaceRule : public IRule
{
	std::string pattern;
	std::string replace;
};

struct SupplyRule : public IRule
{
	std::string pattern;
	std::vector<std::string> replace;
};

class TextProcessor
{
private:
	std::vector<std::unique_ptr<IRule>> rules;
public:
	std::string process(std::string input);
};