#pragma once
#include "Helpers/common.h"
#include "Helpers/TokenContext.hpp"
#include "IEvent.h"

#include <unordered_map>
#include <unordered_set>
#include <type_traits>
#include <functional>
#include <cstdint>
#include <vector>
#include <mutex>

namespace HELPERS_NS {
	namespace Event {
		template<typename TSignature>
		class Signal;

		template<typename TRet, typename... TArgs>
		class Signal<TRet(TArgs...)> : public IEvent<void(TArgs...)> {
		public:
			static_assert(std::is_void_v<TRet>, "Signal currently supports only void return type");

			using MyBase_t = IEvent<void(TArgs...)>;
			using Signal_t = Signal<TRet(TArgs...)>;
			using HandlerFunc_t = typename MyBase_t::HandlerFunc_t;

			// Обьявляем Connection тут же, т.к. dynamic_cast требует чтоб тип был уже известен.
			class Connection: public ISubscriptionToken{
			public:
				Connection(typename HELPERS_NS::TokenContext<Signal_t>::Weak ctx)
					: ctx{ ctx }
					, isConnected{ true } {
				}

				void Disconnect() {
					if (this->ctx.token.expired()) {
						return;
					}
					this->ctx.data->RemoveHandler(this);
					this->isConnected = false;
				}

				bool IsConnected() const {
					if (this->ctx.token.expired()) {
						return false;
					}
					return this->isConnected;
				}

			private:
				typename HELPERS_NS::TokenContext<Signal_t>::Weak ctx;
				bool isConnected;
			};


			class ScopedConnection;

		public:
			Signal() 
				: ctx{ this } {
			}

			~Signal() {
				this->DisconnectAll();
			}

			//
			// IEvent
			//
			std::shared_ptr<ISubscriptionToken> Subscribe(const HandlerFunc_t& handlerFunction) override {
				return this->AddHandler(handlerFunction);
			}

			void Unsubscribe(std::shared_ptr<ISubscriptionToken> subscriptionToken) override {
				if (!subscriptionToken) {
					return;
				}

				if (auto connection = std::dynamic_pointer_cast<Connection>(subscriptionToken)) {
					connection->Disconnect();
				}
			}

			// Invoke — чистим «мертвые» токены и выполняем снапшот
			void Invoke(const TArgs&... args) override {
				std::vector<HandlerFunc_t> handlersSnapshot{};
				std::vector<ISubscriptionToken*> subscriptionTokensToErase{};

				{
					std::scoped_lock lock{ this->mx };
					handlersSnapshot.reserve(this->mapTokenToHandlerFunc.size());

					for (auto& [subscriptionTokenPtr, handler] : this->mapTokenToHandlerFunc) {
						Connection* connection = dynamic_cast<Connection*>(subscriptionTokenPtr);

						if (connection == nullptr || !connection->IsConnected()) {
							subscriptionTokensToErase.push_back(connection);
							continue;
						}

						handlersSnapshot.push_back(handler);
					}

					for (auto* subscriptionTokenPtr : subscriptionTokensToErase) {
						this->mapTokenToHandlerFunc.erase(subscriptionTokenPtr);
					}
				}

				for (auto& fn : handlersSnapshot) {
					fn(args...);
				}
			}

			//
			// Api
			//
			template<typename TCallable>
			std::shared_ptr<Connection> Subscribe(TCallable&& callable) {
				HandlerFunc_t handlerFunction = std::forward<TCallable>(callable);
				return this->AddHandler(handlerFunction);
			}

			template<typename TClass>
			std::shared_ptr<Connection> Subscribe(TClass* objectPtr, void (TClass::* method)(TArgs...)) {
				HandlerFunc_t handlerFunction = std::bind_front(method, objectPtr);
				return this->AddHandler(handlerFunction);
			}

			template<typename TClass>
			std::shared_ptr<Connection> Subscribe(const TClass* objectPtr, void (TClass::* method)(TArgs...) const) {
				HandlerFunc_t handlerFunction = std::bind_front(method, objectPtr);
				return this->AddHandler(handlerFunction);
			};

			void DisconnectAll() {
				// To avoid deadlock first collect tokens, then disconnect them.
				std::vector<ISubscriptionToken*> subscriptionTokens{};
				{
					std::scoped_lock lock{ this->mx };
					for (auto& [subscriptionTokenPtr, handler] : this->mapTokenToHandlerFunc) {
						subscriptionTokens.push_back(subscriptionTokenPtr);
					}
				}

				for (auto subscriptionTokenPtr : subscriptionTokens) {
					if (auto connection = dynamic_cast<Connection*>(subscriptionTokenPtr)) {
						connection->Disconnect();
					}
				}
			}

		private:
			std::shared_ptr<Connection> AddHandler(HandlerFunc_t handlerFunction) {
				std::scoped_lock lock{ this->mx };
				auto connection = std::make_shared<Connection>(this->ctx.GetWeak());
				this->mapTokenToHandlerFunc.emplace(connection.get(), std::move(handlerFunction));
				return connection;
			}

			void RemoveHandler(ISubscriptionToken* subscriptionTokenPtr) {
				std::scoped_lock lock{ this->mx };
				this->mapTokenToHandlerFunc.erase(subscriptionTokenPtr);
			}

		private:
			std::mutex mx;
			std::unordered_map<ISubscriptionToken*, HandlerFunc_t> mapTokenToHandlerFunc;
			const HELPERS_NS::TokenContext<Signal_t> ctx;
		};




		template<typename TRet, typename... TArgs>
		class Signal<TRet(TArgs...)>::ScopedConnection {
		public:
			using Signal_t = Signal<TRet(TArgs...)>;
			using Connection_t = typename Signal_t::Connection;

			ScopedConnection()
				: connection{ nullptr } {
			}

			ScopedConnection(std::shared_ptr<Connection_t> connection)
				: connection{ std::move(connection) } {
			}

			~ScopedConnection() {
				this->Disconnect();
			}

			ScopedConnection(ScopedConnection&& other) 
				: connection{ std::move(other.connection) } {
				other.connection.reset();
			}

			ScopedConnection& operator=(ScopedConnection&& other) {
				if (this != &other) {
					this->Disconnect();
					this->connection = std::move(other.connection);
					other.connection.reset();
				}
				return *this;
			}

			void Disconnect() {
				if (!this->connection) {
					return;
				}
				this->connection->Disconnect();
				this->connection.reset();
			}

			bool IsConnected() const {
				if (!this->connection) {
					return false;
				}
				return this->connection->IsConnected();
			}

		private:
			std::shared_ptr<Connection_t> connection;
		};
	}
}