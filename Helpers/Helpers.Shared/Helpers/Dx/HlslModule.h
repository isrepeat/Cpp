#pragma once
#include "Helpers/common.h"
#include "DxConstantBuffer.h"
#include <d3dcompiler.h>
#include <string>
#include <map>

namespace HELPERS_NS {
	namespace Dx {
		enum class Slot {
			_Return = D3D_RETURN_PARAMETER_INDEX,
			_0 = 0,
			_1,
			_2,
			_3,
		};

		struct HlslFunction {
			struct ParamMatch {
				Slot from;
				Slot to;
			};
			std::string functionName;
			std::vector<ParamMatch> paramsMatch;
		};

		struct HlslModule {
			friend class DxLinkingGraph;

			struct BindResource {
				uint32_t srcSlot = 0;
				uint32_t dstSlot = 0;
				uint32_t count = 0;
			};
			struct BindSampler {
				uint32_t srcSlot = 0;
				uint32_t dstSlot = 0;
				uint32_t count = 0;
			};
			struct BindConstantBuffer {
				uint32_t srcSlot = 0;
				uint32_t dstSlot = 0;
				uint32_t cbDstOffset = 0;
				std::shared_ptr<DxConstantBufferBase> dxConstantBuffer;
			};

			std::filesystem::path hlslFile;
			std::vector<HlslFunction> hlslFunctions;
			std::optional<BindResource> bindedResource;
			std::optional<BindSampler> bindedSampler;

			template <typename T, typename... Args>
			void AddConstantBuffer(std::string name, uint32_t srcSlot, uint32_t dstSlot, uint32_t cbDstOffset, Args&&... args) {
				if (this->bindedConstantBuffers.count(name)) {
					LOG_ERROR_D("'{}' key already exist, ignore.", name);
					return;
				}
				this->bindedConstantBuffers[name] = BindConstantBuffer{
					srcSlot,
					dstSlot,
					cbDstOffset,
					std::make_shared<DxConstantBuffer<T>>(std::forward<Args&&>(args)...)
				};
			}

			template <typename T>
			std::shared_ptr<DxConstantBuffer<T>> GetConstantBuffer(std::string name) {
				try {
					auto bindedConstantBuffer = this->bindedConstantBuffers.at(name);

					if (auto casted = std::dynamic_pointer_cast<DxConstantBuffer<T>>(bindedConstantBuffer.dxConstantBuffer)) {
						return casted;
					}
					else {
						LOG_ERROR_D("bad casted");
						throw std::bad_cast();
					}
				}
				catch (const std::out_of_range& ex) {
					LOG_ERROR_D("key not exist");
					throw;
				}
			}

		private:
			std::map<std::string, BindConstantBuffer> bindedConstantBuffers;
		};
	}
}