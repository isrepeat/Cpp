#pragma once
#include "common.h"
#include "Meta/FunctionTraits.h"
#include <system_error>
#include <span>

namespace HELPERS_NS {
	class PipelineResult final {
	public:
		enum class Flow {
			Continue,
			EarlyExit,
		};

		bool IsContinue() const { 
			return !errorCode && flow == Flow::Continue;
		}
		bool IsEarlyExit() const {
			return !errorCode && flow == Flow::EarlyExit;
		}
		bool IsFailed() const {
			return static_cast<bool>(errorCode);
		}
		std::error_code GetErrorCode() const { 
			return errorCode;
		}
		explicit operator bool() const { 
			return !errorCode;
		}

		static PipelineResult Continue() { 
			return { Flow::Continue, {} };
		}
		static PipelineResult EarlyExit() {
			return { Flow::EarlyExit, {} };
		}
		static PipelineResult Failed(std::error_code ec) {
			return { Flow::Continue, ec };
		}

	private:
		PipelineResult(Flow f, std::error_code ec) 
			: flow{ f }
			, errorCode{ ec } {
		}

	private:
		Flow flow;
		std::error_code errorCode;
	};


	template <typename TDerived>
	class Pipeline {
	public:
		using StepFn_t = PipelineResult(TDerived::*)();

		virtual std::span<const StepFn_t> GetPipelineSteps() const = 0;

		std::error_code Run() {
			TDerived* castToDerived = static_cast<TDerived*>(this);
			
			for (StepFn_t stepFn : this->GetPipelineSteps()) {
				const PipelineResult res = (castToDerived->*stepFn)();
				if (res.IsFailed()) {
					return res.GetErrorCode();
				}
				else if (res.IsEarlyExit()) {
					return {};
				}
			}
			return {};
		}
	};
};