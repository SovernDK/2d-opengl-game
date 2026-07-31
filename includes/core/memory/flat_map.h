#pragma once
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace mem
{
	template<typename TKey, typename TValue>
	class flat_map
	{
	private:
		using iterator = typename std::vector<std::pair<TKey, TValue>>::iterator;
		using const_iterator = typename std::vector<std::pair<TKey, TValue>>::const_iterator;
		std::vector<std::pair<TKey, TValue>> data;

		auto lower_bound(const TKey& key)
		{
			return std::lower_bound(data.begin(), data.end(), key,
				[](const auto& p, const TKey& k) { return p.first < k; });
		}
		auto lower_bound(const TKey& key) const
		{
			return std::lower_bound(data.cbegin(), data.cend(), key,
				[](const auto& p, const TKey& k) { return p.first < k; });
		}

	public:
		TValue& operator[](const TKey& key)
		{
			auto it = lower_bound(key);
			if (it != data.end() && it->first == key)
				return it->second;

			it = data.insert(it, { key, TValue() });
			return it->second;
		}

		std::pair<iterator, bool> insert(const TKey& key, const TValue& value)
		{
			auto it = lower_bound(key);
			if (it != data.end() && it->first == key)
				return { it, false };

			it = data.insert(it, { key, value });
			return { it, true };
		}

		std::pair<iterator, bool> insert(const TKey& key, TValue&& value)
		{
			auto it = lower_bound(key);
			if (it != data.end() && it->first == key)
				return { it, false };

			it = data.insert(it, { key, std::move(value) });
			return { it, true };
		}

		template<typename... TArgs>
		std::pair<iterator, bool> emplace(const TKey& key, TArgs&&... args)
		{
			auto it = lower_bound(key);
			if (it != data.end() && it->first == key)
				return { it, false };
			it = emplace_at(it, key, std::forward<TArgs>(args)...);
			return { it, true };
		}

		void erase(iterator it)
		{
			data.erase(it);
		}
		void erase(const TKey& key)
		{
			auto it = lower_bound(key);
			if (it != data.end() && it->first == key)
				data.erase(it);
		}

		const TValue& at(const TKey& key) const
		{
			auto it = lower_bound(key);
			if (it != data.end() && it->first == key)
				return it->second;
			throw std::out_of_range("flat_map::at: key not found");
		}

		TValue& at(const TKey& key)
		{
			auto it = lower_bound(key);
			if (it != data.end() && it->first == key)
				return it->second;
			throw std::out_of_range("flat_map::at: key not found");
		}

		iterator find(const TKey& key)
		{
			auto it = lower_bound(key);
			return (it != data.end() && it->first == key) ? it : data.end();
		}

		bool contains(const TKey& key) const
		{
			auto it = lower_bound(key);
			return it != data.end() && it->first == key;
		}

		void clear()
		{
			data.clear();
		}

		void reserve(size_t newCapacity)
		{
			data.reserve(newCapacity);
		}

		iterator begin() { return data.begin(); }
		iterator end() { return data.end(); }
		const_iterator begin() const { return data.begin(); }
		const_iterator end() const { return data.end(); }
		size_t size() const { return data.size(); }
	};
}