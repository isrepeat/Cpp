#pragma once
#include <map>

namespace H {
	template<typename Key, typename Value>
	class Bimap {
	public:
		Bimap(std::map<Key, Value> map) : directMap{ map } {
			for (auto& item : map)
				reverseMap.insert(std::pair{ item.second, item.first });
		}
		void Add(std::pair<Key, Value> item) {
			directMap.insert(item);
			reverseMap.insert(std::pair{ item.second, item.first });
		}

		// WARNING: add new value if key not exist
		Value operator[](Key key) {
			return directMap[key];
		}

		Key operator[](Value value) {
			return reverseMap[value];
		}

		// WARNING: throw exception if key not exist
		Value at(Key key) const {
			return directMap.at(key);
		}

		Key at(Value value) const {
			return reverseMap.at(value);
		}

	private:
		std::map<Key, Value> directMap;
		std::map<Value, Key> reverseMap;
	};
}