#pragma once
#include <unordered_map>
#include <vector>
#include <string>

class StringInterner
{
public:
	using Id = uint32_t;
	static constexpr Id Invalid = 0;
public:
	Id intern(const std::string& s)
	{
		auto it = toId.find(s);
		if (it != toId.end()) return it->second;
		Id id = static_cast<Id>(toStr.size()) + 1;
		toId[s] = id;
		toStr.push_back(s);
		return id;
	}

	const std::string& toString(Id id) const { return toStr[id - 1]; }
private:
	std::unordered_map<std::string, Id> toId;
	std::vector<std::string> toStr;
};