#pragma once
#include <vector>
#include <utility>
#include <functional>


namespace H {
	class Signal { // Functor
	public:
		enum Type {
			Multi,
			Once,
		};

		Signal(Type type = Type::Multi) : type{ type } {
		}
		Signal(std::function<void()> handler, Type type = Type::Multi) : type{ type } {
			handlers.push_back(handler);
		}

		// [Not thread safe]
		Signal& Add(std::function<void()> handler) {
			handlers.push_back(handler);
			return *this;
		}

		Signal& AddFinish(std::function<void()> finishHandler) {
			finishHandlers.push_back(finishHandler);
			return *this;
		}

		void Clear() {
			finishHandlers.clear();
			handlers.clear();
		}

		// Qualify as const to use in lambda captures by value
		void operator() () const {
			for (auto& handler : handlers) {
				if (handler) {
					handler();
				}
			}
			for (auto& finishHandler : finishHandlers) {
				if (finishHandler) {
					finishHandler();
				}
			}

			if (type == Type::Once && (handlers.size() || finishHandlers.size())) {
				finishHandlers.clear();
				handlers.clear();
			}
		}


	private:
		Type type;
		mutable std::vector<std::function<void()>> handlers;
		mutable std::vector<std::function<void()>> finishHandlers;
		//std::mutex mx; // We cannot use mutex as mebmer because its copy constructor removed
	};
	
}