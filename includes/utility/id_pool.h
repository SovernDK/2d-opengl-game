#pragma once
#include <concepts>
#include <unordered_set>

template<std::integral T>
class IdPool
{
private:
	T nextId = 1;
	std::unordered_set<T> freeIds{};
public:
	IdPool() = default;
	IdPool(const IdPool&) = delete;
	IdPool& operator=(const IdPool&) = delete;
	IdPool(IdPool&& other) = delete;
	IdPool& operator=(IdPool&& other) = delete;

	~IdPool() = default;

	T next()
	{
		T id = 0;
		if (!freeIds.empty())
		{
			auto it = freeIds.begin();
			id = *it;
			freeIds.erase(it);
		}
		else id = nextId++;

		return id;
	}

	void releaseId(T id)
	{
		freeIds.insert(id);
	}
};