#pragma once
#include <concepts>
#include <unordered_set>

template<std::integral T>
class IdPool
{
public:
	struct Options
	{
		T startingId = 1;
		bool enableRecycle = true;
	};

private:
	T nextId;
	std::unordered_set<T> freeIds{};
	bool m_enableRecycle;

public:
	IdPool() : IdPool(Options{}) {}

	explicit IdPool(Options opts)
		: nextId(opts.startingId), m_enableRecycle(opts.enableRecycle)
	{}

	IdPool(const IdPool&) = delete;
	IdPool& operator=(const IdPool&) = delete;
	IdPool(IdPool&&) = delete;
	IdPool& operator=(IdPool&&) = delete;
	~IdPool() = default;

	T next()
	{
		T id = 0;
		if (m_enableRecycle && !freeIds.empty())
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