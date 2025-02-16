#pragma once
#include <unordered_map>

template<typename K, typename V>
struct Hashmap {

	void set(const K& key, const V& value) {
		impl[key] = value;
	}
	bool exists(const K& key) {
		return impl.find(key) != impl.end();
	}

	// Returns a pointer to the value if it exists, otherwise nullptr.
	V* lookup(const K& key) {
		auto i = impl.find(key);
		if (i != impl.end())
			return &(i->second);
		else
			return nullptr;
	}

	// Returns the value at key, or default_value if it doesn't exist.
	V getOr(const K& key, const V& default_value) {
		auto i = impl.find(key);
		if (i != impl.end())
			return i->second;
		else
			return default_value;
	}

	// If key exists, return true and fill out the value_out. Otherwise, returns false and doesn't touch value_out.
	bool get(const K& key, V* value_out) {
		auto i = impl.find(key);
		if (i != impl.end()) {
			*value_out = i->second;
			return true;
		} else {
			return false;
		}
	}

	int keyCount() const {
		return impl.size();
	}
	K key(int i) const {
		// There is no iter+i :facepalm:
		auto iter = impl.begin();
		for (int k = 0; k < i; ++k)
			iter++;
		return iter->first;
	}

private:
	//#TODO make an actual hashmap.
	std::unordered_map<K,V> impl;
};