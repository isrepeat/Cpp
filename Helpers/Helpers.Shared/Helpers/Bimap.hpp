#pragma once
#include "common.h"
#include <map>

namespace HELPERS_NS {
	template<class Key, class Value>
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
		
		Value operator[](Key key) const {
			return directMap[key]; // WARNING: if key not exist will be created new item with Value{}
		}

		Key operator[](Value value) const {
			return reverseMap[value];
		}

	private:
		mutable std::map<Key, Value> directMap;
		mutable std::map<Value, Key> reverseMap;
	};
}