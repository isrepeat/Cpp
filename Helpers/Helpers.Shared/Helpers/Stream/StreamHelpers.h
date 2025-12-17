#pragma once
#include <Helpers/common.h>
#include <Helpers/Macros.h>

#include <fstream>
#include <format>

namespace HELPERS_NS {
	namespace Stream {
		inline void ThrowIfStreamNotOpened(const std::ios& stream, const std::string& context = "") {
			if (!stream.good()) {
				const std::string prefix = context.empty() ? "" : std::format("[{}] ", context);
				const std::string message = std::format(
					"{}Stream is not open or in bad state",
					prefix
				);
				Dbreak;
				throw ::std::exception(message.c_str());
			}
		}

		inline void ThrowIfStreamFailed(const std::ios& stream, const std::string& context = "") {
			if (stream.fail() || stream.bad()) {
				const std::string prefix = context.empty() ? "" : std::format("[{}] ", context);
				const std::string message = std::format(
					"{}Stream failure (fail={}, bad={}, eof={})",
					prefix,
					stream.fail(),
					stream.bad(),
					stream.eof()
				);
				Dbreak;
				throw std::ios_base::failure(message);
			}
		}
	}
}