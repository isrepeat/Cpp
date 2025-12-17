#pragma once
#pragma message(PREPROCESSOR_MSG("Deprecated 'Signal.h'"))
#include "common.h"
#include <functional>
#include <utility>
#include <vector>

namespace HELPERS_NS {
    enum class SignalType {
        Multi,
        Once,
    };

    namespace details {
        template <SignalType _SignalType, typename _Fn>
        class SignalBase {
        };


        template <SignalType _SignalType, typename _Ret, typename... _Args>
        class SignalBase<_SignalType, _Ret(_Args...)> {
        public:
            SignalBase() {}
            SignalBase(std::function<_Ret(_Args...)> handler) {
                handlers.push_back(handler);
            }

            // [Not thread safe]
            SignalBase& Add(std::function<_Ret(_Args...)> handler) {
                handlers.push_back(handler);
                return *this;
            }

            SignalBase& AddFinish(std::function<_Ret(_Args...)> finishHandler) {
                finishHandlers.push_back(finishHandler);
                return *this;
            }

            void Clear() {
                finishHandlers.clear();
                handlers.clear();
            }

            //bool IsTriggeredAtLeastOnce() const {
            //    return triggerCounter;
            //}

            // Qualify as const to use in lambda captures by value
            _Ret operator() (_Args... args) const { // NOTE: _Args must be trivial copyable
                for (auto& handler : handlers) {
                    if (handler) {
                        handler(args...);
                    }
                }
                for (auto& finishHandler : finishHandlers) {
                    if (finishHandler) {
                        finishHandler(args...);
                    }
                }

                //triggerCounter++;

                if (_SignalType == SignalType::Once && (this->handlers.size() || this->finishHandlers.size())) {
                    this->finishHandlers.clear();
                    this->handlers.clear();
                }
            }

            operator bool() const {
                return this->handlers.size() || this->finishHandlers.size();
            }

        private:
            //std::mutex mx; // We cannot use mutex as mebmer because its copy constructor removed
            //mutable std::atomic<int> triggerCounter = 0; // default copy Ctor deleted for std::atomic
            mutable std::vector<std::function<_Ret(_Args...)>> handlers;
            mutable std::vector<std::function<_Ret(_Args...)>> finishHandlers;
        };
    }

    template <typename _Fn, SignalType _SignalType = SignalType::Multi>
    using Signal = details::SignalBase<_SignalType, _Fn>;

    template <typename _Fn>
    using SignalOnce = details::SignalBase<SignalType::Once, _Fn>;
}