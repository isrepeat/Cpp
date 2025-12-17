#include "LogHelpers.h"
//#pragma message(PREPROCESSOR_MSG("Build LogHelpers.cpp with LOGGER_API = '" PP_STRINGIFY(LOGGER_API) "'"))
//#pragma message(PREPROCESSOR_MSG("Build LogHelpers.cpp with LOGGER_NS = '" PP_STRINGIFY(LOGGER_NS) "'"))
#include <Helpers/FileSystem_inline.h>
#include <Helpers/PackageProvider.h> // need link with Helpers.lib
#include <Helpers/TokenSingleton.hpp>
#include <Helpers/Macros.h>
#include <Helpers/Helpers.h>
#include <ComAPI/ComAPI.h>
#include <set>


namespace LOGGER_NS {
    // Free letter flags: 'G', 'h', 'J''j', 'K''k', 'N', 'Q', 'U', 'V', 'W''w', 'Z', '*', "?"

    // TODO: Use universal formatter to format source_location with paddings to make it more readable 
    class FunctionNameFormatter : public spdlog::custom_flag_formatter {
    public:
        static const char flag = '?';
        static const int maxFuncLenght = 30;

        explicit FunctionNameFormatter() = default;

        void format(const spdlog::details::log_msg& logMsg, const std::tm&, spdlog::memory_buf_t& dest) override {
            if (logMsg.source.empty()) {
                return;
            }

            // Make pretty function name
            //std::string_view svFuncName = logMsg.source.funcname;
            //if (svFuncName.size() > maxFuncLenght) {
            //    std::string_view svFuncNameStart = svFuncName.substr(0, maxFuncLenght / 2);
            //    std::string_view svFuncNameEnd = svFuncName.substr(svFuncName.size() - maxFuncLenght / 2);
            //    
            //    spdlog::details::fmt_helper::append_string_view({ svFuncNameStart.data(), svFuncNameStart.size() }, dest);
            //    spdlog::details::fmt_helper::append_string_view("...", dest);
            //    spdlog::details::fmt_helper::append_string_view({ svFuncNameEnd.data(), svFuncNameEnd.size() }, dest);
            //    return;
            //}

            spdlog::details::fmt_helper::append_string_view(logMsg.source.funcname, dest);
        }

        std::unique_ptr<custom_flag_formatter> clone() const override {
            return spdlog::details::make_unique<FunctionNameFormatter>();
        }
    };

    class MsgCallbackFormatter : public spdlog::custom_flag_formatter {
    public:
        static const char flag = 'q';

        explicit MsgCallbackFormatter(std::function<std::wstring()> prefixFn, std::function<void(const std::string&)> postfixFn = nullptr)
            : prefixCallback{ prefixFn }
            , postfixCallback{ postfixFn }
        {
        }

        void format(const spdlog::details::log_msg& logMsg, const std::tm&, spdlog::memory_buf_t& dest) override {
            switch (logMsg.level) {
            case spdlog::level::level_enum::err:
                spdlog::details::fmt_helper::append_string_view(" *** [E] =", dest);
                break;
            case spdlog::level::level_enum::warn:
                spdlog::details::fmt_helper::append_string_view(" *** [W] =", dest);
                break;
            }

            if (prefixCallback) {
                std::wstring prefix = std::wstring(padinfo_.width_, ' ') + prefixCallback();
                auto prefixUtf8 = H::WStrToStr(prefix, CP_UTF8);
                dest.append(prefixUtf8.data(), prefixUtf8.data() + prefixUtf8.size());
            }
            if (postfixCallback) {
                postfixCallback(std::string{ logMsg.payload.begin(), logMsg.payload.end() });
            }
        }

        std::unique_ptr<custom_flag_formatter> clone() const override {
            return spdlog::details::make_unique<MsgCallbackFormatter>(prefixCallback, postfixCallback);
        }

    private:
        std::function<std::wstring()> prefixCallback;
        std::function<void(const std::string&)> postfixCallback;
    };


    enum class Pattern {
        Default,
        Raw,
        Time,
        Func,
        Extend,
        Debug,
        DebugFn,
    };

    // %l - log level
    // %t - thread id
    // %d.%m.%Y %H:%M:%S:%e - time
    // %q - optional prefix/postfix callback
    // %v - log msg
    // %^/%$ - start/end color range
    const std::string GetPattern(Pattern value) {
        static const std::unordered_map<Pattern, std::string> patterns = {
            {Pattern::Default, "[%L] [%t] %d.%m.%Y %H:%M:%S:%e {%s:%# %?}%q %v"},
            {Pattern::Raw, "%v"},
            {Pattern::Time, "[%L] %d.%m.%Y %H:%M:%S:%e  %v"},
            {Pattern::Func, "[%L] [%t] %d.%m.%Y %H:%M:%S:%e {%s:%# %?}%q @@@ %v"},
            {Pattern::Extend, "[%L] [%t] %d.%m.%Y %H:%M:%S:%e {%s:%# %?}%q %v"}, // use postfixCallback
            {Pattern::Debug, "[%L] [%t] %H:%M:%S:%e {%s:%# %?}%^%q %v%$"},
            {Pattern::DebugFn, "[%L] [%t] %H:%M:%S:%e {%s:%# %?}%^%q @@@ %v%$"},
        };
        return patterns.at(value);
    }



	struct DefaultLoggers::UnscopedData {
		UnscopedData()
			: defaultSink{ std::make_shared<spdlog::sinks::msvc_sink_mt>() }
			, defaultFnSink{ std::make_shared<spdlog::sinks::msvc_sink_mt>() }
			, defaultLogger{ std::make_shared<spdlog::logger>(
				"default_logger",
				spdlog::sinks_init_list{ defaultSink }
			) }
			, defaultFuncLogger{ std::make_shared<spdlog::logger>(
				"default_func_logger",
				spdlog::sinks_init_list{ defaultFnSink }
			) } {
			// обычный (Debug)
			{
				auto fmt = std::make_unique<spdlog::pattern_formatter>();
				fmt->add_flag<FunctionNameFormatter>(FunctionNameFormatter::flag)
					.set_pattern(GetPattern(Pattern::Debug));

				fmt->add_flag<MsgCallbackFormatter>(MsgCallbackFormatter::flag, nullptr)
					.set_pattern(GetPattern(Pattern::Debug));

				this->defaultSink->set_formatter(std::move(fmt));
				this->defaultSink->set_level(spdlog::level::trace);

				this->defaultLogger->flush_on(spdlog::level::trace);
				this->defaultLogger->set_level(spdlog::level::trace);
			}

			// функциональный (DebugFn)
			{
				auto fmtFn = std::make_unique<spdlog::pattern_formatter>();
				fmtFn->add_flag<FunctionNameFormatter>(FunctionNameFormatter::flag)
					.set_pattern(GetPattern(Pattern::DebugFn));

				fmtFn->add_flag<MsgCallbackFormatter>(MsgCallbackFormatter::flag, nullptr)
					.set_pattern(GetPattern(Pattern::DebugFn));

				this->defaultFnSink->set_formatter(std::move(fmtFn));
				this->defaultFnSink->set_level(spdlog::level::trace);

				this->defaultFuncLogger->flush_on(spdlog::level::trace);
				this->defaultFuncLogger->set_level(spdlog::level::trace);
			}
		}
		~UnscopedData() {
			assertm(false, "Unexpected destructor call");
		}

		const std::shared_ptr<spdlog::logger> DefaultLogger() const {
			return this->defaultLogger;
		}

		const std::shared_ptr<spdlog::logger> DefaultFuncLogger() const {
			return this->defaultFuncLogger;
		}

	private:
		const std::shared_ptr<spdlog::sinks::msvc_sink_mt> defaultSink;
		const std::shared_ptr<spdlog::sinks::msvc_sink_mt> defaultFnSink;

		const std::shared_ptr<spdlog::logger> defaultLogger;
		const std::shared_ptr<spdlog::logger> defaultFuncLogger;
	};



	DefaultLoggers::DefaultLoggers()
		: initializedLoggersById{}
	{
		this->prefixCallback = [this] {
			return this->className;
			};
		this->postfixCallback = [this](const std::string& logMsg) {
			this->lastMessage = logMsg;
			};

#ifdef _DEBUG
		this->debugSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
		{
			auto formatterDebug = std::make_unique<spdlog::pattern_formatter>();
			formatterDebug->add_flag<FunctionNameFormatter>(FunctionNameFormatter::flag)
				.set_pattern(GetPattern(Pattern::Debug));

			formatterDebug->add_flag<MsgCallbackFormatter>(MsgCallbackFormatter::flag, this->prefixCallback)
				.set_pattern(GetPattern(Pattern::Debug));

			this->debugSink->set_formatter(std::move(formatterDebug));
			this->debugSink->set_level(spdlog::level::trace);
		}

		this->debugFnSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
		{
			auto formatterDebugFn = std::make_unique<spdlog::pattern_formatter>();
			formatterDebugFn->add_flag<FunctionNameFormatter>(FunctionNameFormatter::flag)
				.set_pattern(GetPattern(Pattern::DebugFn));

			formatterDebugFn->add_flag<MsgCallbackFormatter>(MsgCallbackFormatter::flag, this->prefixCallback)
				.set_pattern(GetPattern(Pattern::DebugFn));

			this->debugFnSink->set_formatter(std::move(formatterDebugFn));
			this->debugFnSink->set_level(spdlog::level::trace);
		}
#endif

		// DefaultLoggers::UnscopedData is created inside singleton
		H::TokenSingleton<DefaultLoggers>::SetToken(Passkey<DefaultLoggers>{}, this->token);
	}

    DefaultLoggers::~DefaultLoggers() {
        this->standardLoggersList = {}; // clear loggers to release sinks
    }


    void DefaultLoggers::Init(
        std::filesystem::path logFilePath,
        H::Flags<InitFlags> initFlags,
        LoggingMode loggingMode,
        uintmax_t maxSizeLogFile
    ) {
		DefaultLoggers::GetInstance().InitForId(0, logFilePath, initFlags, loggingMode, maxSizeLogFile);
    }

    void DefaultLoggers::InitForId(
        uint8_t loggerId,
        std::filesystem::path logFilePath,
        H::Flags<InitFlags> initFlags,
        LoggingMode loggingMode,
        uintmax_t maxSizeLogFile
    ) {
        assertm(loggerId < maxLoggers, "loggerId out of bound");
        // TODO: Use H::Bimap<"loggerPath", loggerId> and move to class member (combine with initializedLoggersById)
        static std::set<std::wstring> initializedLoggersByPath;

        if (H::PackageProvider::IsRunningUnderPackage()) {
            if (initFlags.Has(InitFlags::CreateInPackageFolder)) {
                logFilePath = ComApi::GetPackageFolder() / logFilePath.relative_path();
            }
        }
        else {
            if (initFlags.Has(InitFlags::CreateInExeFolderForDesktop)) {
                // dont use exe path when IsRunningUnderPackage() == true because uwp package exe folder protected for writing
                logFilePath = H::ExePath() / logFilePath.relative_path();
            }
        }

        if (initializedLoggersByPath.count(logFilePath) > 0) {
			DefaultLoggers::TimeLogger(loggerId)->warn("the logger on this path has already been initialized");
            return;
        }
        else {
            initializedLoggersByPath.insert(logFilePath);
        }

        if (!std::filesystem::exists(logFilePath)) {
            initFlags &= ~InitFlags::AppendNewSessionMsg; // don't append new session message at first created log file
            std::filesystem::create_directories(logFilePath.parent_path());
        }
        else {
            auto logSize = std::filesystem::file_size(logFilePath);
            if (!initFlags.Has(InitFlags::Truncate) && logSize > maxSizeLogFile) {
                auto truncatedBytes = logSize - maxSizeLogFile / 2;
                H::FS::RemoveBytesFromStart(logFilePath, truncatedBytes, [](std::ofstream& file) {
                    file.write(logTruncationMessage.data(), logTruncationMessage.size());
                    });
            }
        }
      
        auto& _this = DefaultLoggers::GetInstance();
        if (_this.initializedLoggersById.count(loggerId) > 0) {
			DefaultLoggers::TimeLogger(loggerId)->warn("the logger on this id = {} has already been initialized, continue reinitalize ...", loggerId);
        } else {
            _this.initializedLoggersById.insert(loggerId);
        }

        _this.standardLoggersList[loggerId].maxSizeLogFile = maxSizeLogFile;

        _this.standardLoggersList[loggerId].fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, initFlags.Has(InitFlags::Truncate));
        auto formatterDefault = std::make_unique<spdlog::pattern_formatter>();
        formatterDefault->add_flag<FunctionNameFormatter>(FunctionNameFormatter::flag).set_pattern(GetPattern(Pattern::Default));
        formatterDefault->add_flag<MsgCallbackFormatter>(MsgCallbackFormatter::flag, _this.prefixCallback).set_pattern(GetPattern(Pattern::Default));
        _this.standardLoggersList[loggerId].fileSink->set_formatter(std::move(formatterDefault));

        _this.standardLoggersList[loggerId].fileSinkRaw = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, initFlags.Has(InitFlags::Truncate));
        if (initFlags.Has(InitFlags::DisableEOLforRawLogger)) {
            auto formatterRaw = std::make_unique<spdlog::pattern_formatter>(GetPattern(Pattern::Raw), spdlog::pattern_time_type::local, std::string(""));
            _this.standardLoggersList[loggerId].fileSinkRaw->set_formatter(std::move(formatterRaw));
        }
        else {
            _this.standardLoggersList[loggerId].fileSinkRaw->set_pattern(GetPattern(Pattern::Raw));
        }

        _this.standardLoggersList[loggerId].fileSinkTime = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, initFlags.Has(InitFlags::Truncate));
        _this.standardLoggersList[loggerId].fileSinkTime->set_pattern(GetPattern(Pattern::Time));

        _this.standardLoggersList[loggerId].fileSinkFunc = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, initFlags.Has(InitFlags::Truncate));
        auto formatterFunc = std::make_unique<spdlog::pattern_formatter>();
        formatterFunc->add_flag<FunctionNameFormatter>(FunctionNameFormatter::flag).set_pattern(GetPattern(Pattern::Func));
        formatterFunc->add_flag<MsgCallbackFormatter>(MsgCallbackFormatter::flag, _this.prefixCallback).set_pattern(GetPattern(Pattern::Func));
        _this.standardLoggersList[loggerId].fileSinkFunc->set_formatter(std::move(formatterFunc));

        _this.standardLoggersList[loggerId].fileSinkExtend = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, initFlags.Has(InitFlags::Truncate));
        auto formatterExtend = std::make_unique<spdlog::pattern_formatter>();
        formatterExtend->add_flag<FunctionNameFormatter>(FunctionNameFormatter::flag).set_pattern(GetPattern(Pattern::Extend));
        formatterExtend->add_flag<MsgCallbackFormatter>(MsgCallbackFormatter::flag, _this.prefixCallback, _this.postfixCallback).set_pattern(GetPattern(Pattern::Extend));
        _this.standardLoggersList[loggerId].fileSinkExtend->set_formatter(std::move(formatterExtend));

        if (initFlags.Has(InitFlags::EnableLogToStdout)) {
            _this.stdoutDebugColorSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto formatterDebug = std::make_unique<spdlog::pattern_formatter>();
            formatterDebug->add_flag<FunctionNameFormatter>(FunctionNameFormatter::flag).set_pattern(GetPattern(Pattern::Debug));
            formatterDebug->add_flag<MsgCallbackFormatter>(MsgCallbackFormatter::flag, _this.prefixCallback).set_pattern(GetPattern(Pattern::Debug));
            _this.stdoutDebugColorSink->set_formatter(std::move(formatterDebug));
            _this.stdoutDebugColorSink->set_color(spdlog::level::err, FOREGROUND_RED);
            _this.stdoutDebugColorSink->set_color(spdlog::level::warn, FOREGROUND_RED);
            _this.stdoutDebugColorSink->set_color(spdlog::level::debug, FOREGROUND_INTENSITY); // dark-gray
            _this.stdoutDebugColorSink->set_level(spdlog::level::trace);

            _this.stdoutDebugFnColorSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto formatterFunc = std::make_unique<spdlog::pattern_formatter>();
            formatterFunc->add_flag<FunctionNameFormatter>(FunctionNameFormatter::flag).set_pattern(GetPattern(Pattern::DebugFn));
            formatterFunc->add_flag<MsgCallbackFormatter>(MsgCallbackFormatter::flag, _this.prefixCallback).set_pattern(GetPattern(Pattern::DebugFn));
            _this.stdoutDebugFnColorSink->set_formatter(std::move(formatterFunc));
            _this.stdoutDebugFnColorSink->set_color(spdlog::level::debug, FOREGROUND_GREEN | FOREGROUND_BLUE);
            _this.stdoutDebugFnColorSink->set_level(spdlog::level::trace);
        }


		// Формирование sink-листов
		auto& list = _this.standardLoggersList[loggerId];

		std::vector<spdlog::sink_ptr> loggerSinks;
		std::vector<spdlog::sink_ptr> rawLoggerSinks;
		std::vector<spdlog::sink_ptr> timeLoggerSinks;
		std::vector<spdlog::sink_ptr> funcLoggerSinks;
		std::vector<spdlog::sink_ptr> extendLoggerSinks;

		// Базовые логгеры (только файл)
		loggerSinks.push_back(list.fileSink);
		rawLoggerSinks.push_back(list.fileSinkRaw);
		timeLoggerSinks.push_back(list.fileSinkTime);
		funcLoggerSinks.push_back(list.fileSinkFunc);
		extendLoggerSinks.push_back(list.fileSinkExtend);

		// Добавляем stdout (если включено)
		if (initFlags.Has(InitFlags::EnableLogToStdout)) {
			loggerSinks.push_back(_this.stdoutDebugColorSink);
			funcLoggerSinks.push_back(_this.stdoutDebugFnColorSink);
			
			if (initFlags.Has(InitFlags::RedirectRawTimeLogToStdout)) {
				rawLoggerSinks.push_back(_this.stdoutDebugColorSink);
				timeLoggerSinks.push_back(_this.stdoutDebugColorSink);
			}
		}


#ifdef _DEBUG
		funcLoggerSinks.push_back(_this.debugFnSink); // чтобы LOG_FUNCTION_SCOPE_* летели в Output (паттерн DebugFn)
		extendLoggerSinks.push_back(_this.debugSink); // extend тоже в Output

		std::vector<spdlog::sink_ptr> debugLoggerSinks = { list.fileSink, _this.debugSink };
		if (initFlags.Has(InitFlags::EnableLogToStdout)) {
			debugLoggerSinks.push_back(_this.stdoutDebugColorSink);
		}
#endif

        std::string nameId = loggerId > 0 ? "_" + std::to_string(loggerId) : "";

		_this.standardLoggersList[loggerId].logger =
			std::make_shared<spdlog::logger>(
				"logger" + nameId,
				loggerSinks.begin(),
				loggerSinks.end()
			);
		_this.standardLoggersList[loggerId].logger->flush_on(spdlog::level::trace);


		_this.standardLoggersList[loggerId].rawLogger =
			std::make_shared<spdlog::logger>(
				"raw_logger" + nameId,
				rawLoggerSinks.begin(),
				rawLoggerSinks.end()
			);
		_this.standardLoggersList[loggerId].rawLogger->flush_on(spdlog::level::trace);


		_this.standardLoggersList[loggerId].timeLogger =
			std::make_shared<spdlog::logger>(
				"time_logger" + nameId,
				timeLoggerSinks.begin(),
				timeLoggerSinks.end()
			);
		_this.standardLoggersList[loggerId].timeLogger->flush_on(spdlog::level::trace);


		_this.standardLoggersList[loggerId].funcLogger =
			std::make_shared<spdlog::logger>(
				"func_logger" + nameId,
				funcLoggerSinks.begin(),
				funcLoggerSinks.end()
			);
		_this.standardLoggersList[loggerId].funcLogger->flush_on(spdlog::level::trace);


		_this.standardLoggersList[loggerId].extendLogger =
			std::make_shared<spdlog::logger>(
				"extend_logger" + nameId,
				extendLoggerSinks.begin(),
				extendLoggerSinks.end()
			);
		_this.standardLoggersList[loggerId].extendLogger->flush_on(spdlog::level::trace);


#ifdef _DEBUG
		_this.standardLoggersList[loggerId].debugLogger =
			std::make_shared<spdlog::logger>(
				"debug_logger" + nameId,
				debugLoggerSinks.begin(),
				debugLoggerSinks.end()
			);
		_this.standardLoggersList[loggerId].debugLogger->flush_on(spdlog::level::trace);
#endif

        // SetLoggingMode requires pauseLoggingEvent to exist
		DefaultLoggers::SetLoggingMode(loggingMode, loggerId);

        if (initFlags.Has(InitFlags::AppendNewSessionMsg)) {
            std::string rawEOL = "";
            if (initFlags.Has(InitFlags::DisableEOLforRawLogger)) {
                rawEOL = "\n";
            }
            // whitespaces are selected by design
            _this.standardLoggersList[loggerId].rawLogger->debug("\n" + rawEOL);
            _this.standardLoggersList[loggerId].rawLogger->debug("==========================================================================================================" + rawEOL);
            _this.standardLoggersList[loggerId].timeLogger->debug("                       New session started");
            _this.standardLoggersList[loggerId].rawLogger->debug("==========================================================================================================" + rawEOL);
        }
    }


    bool DefaultLoggers::IsInitialized(uint8_t id) {
        return DefaultLoggers::GetInstance().initializedLoggersById.count(id) > 0;
    }


    std::string DefaultLoggers::GetLastMessage() {
        std::unique_lock lk{ DefaultLoggers::GetInstance().mxCustomFlagHandlers };
        return DefaultLoggers::GetInstance().lastMessage;
    }


    LoggingMode DefaultLoggers::GetLoggingMode(uint8_t id) {
        auto& _this = DefaultLoggers::GetInstance();

        if (H::TokenSingleton<DefaultLoggers>::IsExpired()) {
            auto defaultLogger = H::TokenSingleton<DefaultLoggers>::GetData().DefaultLogger();
            auto mode = DefaultLoggers::SpdlogLevelToLoggingMode(defaultLogger->level());
            return mode;
        }

        auto& loggers = _this.standardLoggersList[id];
        return loggers.loggingMode;
    }


    void DefaultLoggers::SetLoggingMode(LoggingMode mode, uint8_t id) {
        auto& _this = DefaultLoggers::GetInstance();
        auto logLevel = DefaultLoggers::LoggingModeToSpdlogLevel(mode);

        if (H::TokenSingleton<DefaultLoggers>::IsExpired()) {
            auto defaultLogger = H::TokenSingleton<DefaultLoggers>::GetData().DefaultLogger();
            defaultLogger->set_level(logLevel);
            return;
        }

        auto& loggers = _this.standardLoggersList[id];

        loggers.loggingMode = mode;

        loggers.fileSink->set_level(logLevel);
        loggers.fileSinkRaw->set_level(logLevel);
        loggers.fileSinkTime->set_level(logLevel);
        loggers.fileSinkFunc->set_level(logLevel);
        loggers.fileSinkExtend->set_level(logLevel);

		DefaultLoggers::ForEachLogger(id, [logLevel](auto& logger) {
            logger.set_level(logLevel);
        });
    }


    uintmax_t DefaultLoggers::GetMaxLogFileSize(uint8_t id) {
        auto& _this = DefaultLoggers::GetInstance();

        if (H::TokenSingleton<DefaultLoggers>::IsExpired()) {
            return StandardLoggers::defaultLogSize;
        }

        auto& loggers = _this.standardLoggersList[id];
        return loggers.maxSizeLogFile;
    }


    void DefaultLoggers::SetMaxLogFileSize(uintmax_t size, uint8_t id) {
        auto& _this = DefaultLoggers::GetInstance();

        if (H::TokenSingleton<DefaultLoggers>::IsExpired()) {
            return;
        }

        auto& loggers = _this.standardLoggersList[id];

        // Will have effect after the next dynamic sink filesize check timeout
        loggers.maxSizeLogFile = size;
    }


    void DefaultLoggers::ForEachLogger(uint8_t id, const std::function<void(spdlog::logger&)>& action) {
        auto& _this = DefaultLoggers::GetInstance();

        if (H::TokenSingleton<DefaultLoggers>::IsExpired()) {
            auto defaultLogger = H::TokenSingleton<DefaultLoggers>::GetData().DefaultLogger();
            action(*defaultLogger);
            return;
        }

        auto& loggers = _this.standardLoggersList[id];

        spdlog::logger* allLoggers[] = {
#ifdef _DEBUG
            loggers.debugLogger.get(),
#endif
            loggers.extendLogger.get(),
            loggers.funcLogger.get(),
            loggers.logger.get(),
            loggers.rawLogger.get(),
            loggers.timeLogger.get()
        };

        for (auto logger : allLoggers) {
            if (logger) {
                action(*logger);
            }
        }
    }

    spdlog::level::level_enum DefaultLoggers::LoggingModeToSpdlogLevel(LoggingMode mode) {
        switch (mode) {
        case LoggingMode::Normal:
            return spdlog::level::level_enum::info;

        case LoggingMode::Verbose:
            return spdlog::level::level_enum::trace;

        default:
            assert(false);
            return spdlog::level::level_enum::off;
        }
    }


    LoggingMode DefaultLoggers::SpdlogLevelToLoggingMode(spdlog::level::level_enum level) {
        switch (level) {
        case spdlog::level::level_enum::info:
            return LoggingMode::Normal;

        case spdlog::level::level_enum::trace:
            return LoggingMode::Verbose;

        default:
            return LoggingMode::Normal;
        }
    }


    std::shared_ptr<spdlog::logger> DefaultLoggers::Logger(uint8_t id) {
        auto& _this = DefaultLoggers::GetInstance(); // ensure that token set

        if (H::TokenSingleton<DefaultLoggers>::IsExpired() ||
			!_this.standardLoggersList[id].logger
			) {
            return H::TokenSingleton<DefaultLoggers>::GetData().DefaultLogger();
        }
        return _this.standardLoggersList[id].logger;
    }


    std::shared_ptr<spdlog::logger> DefaultLoggers::RawLogger(uint8_t id) {
        auto& _this = DefaultLoggers::GetInstance();

        if (H::TokenSingleton<DefaultLoggers>::IsExpired() ||
			!_this.standardLoggersList[id].rawLogger
			) {
            return H::TokenSingleton<DefaultLoggers>::GetData().DefaultLogger();
        }
        return _this.standardLoggersList[id].rawLogger;
    }


    std::shared_ptr<spdlog::logger> DefaultLoggers::TimeLogger(uint8_t id) {
        auto& _this = DefaultLoggers::GetInstance();

        if (H::TokenSingleton<DefaultLoggers>::IsExpired() ||
			!_this.standardLoggersList[id].timeLogger
			) {
            return H::TokenSingleton<DefaultLoggers>::GetData().DefaultLogger();
        }
        return _this.standardLoggersList[id].timeLogger;
    }


    std::shared_ptr<spdlog::logger> DefaultLoggers::FuncLogger(uint8_t id) {
        auto& _this = DefaultLoggers::GetInstance();

        if (H::TokenSingleton<DefaultLoggers>::IsExpired() ||
			!_this.standardLoggersList[id].funcLogger
			) {
            return H::TokenSingleton<DefaultLoggers>::GetData().DefaultFuncLogger();
        }
        return _this.standardLoggersList[id].funcLogger;
    }


    std::shared_ptr<spdlog::logger> DefaultLoggers::DebugLogger(uint8_t id) {
#ifdef _DEBUG
        auto& _this = DefaultLoggers::GetInstance();

        if (H::TokenSingleton<DefaultLoggers>::IsExpired() ||
			!_this.standardLoggersList[id].debugLogger
			) {
            return H::TokenSingleton<DefaultLoggers>::GetData().DefaultLogger();
        }
        return _this.standardLoggersList[id].debugLogger;
#else
        return DefaultLoggers::Logger(id);
#endif
    }


    std::shared_ptr<spdlog::logger> DefaultLoggers::ExtendLogger(uint8_t id) {
        auto& _this = DefaultLoggers::GetInstance();

        if (H::TokenSingleton<DefaultLoggers>::IsExpired() ||
			!_this.standardLoggersList[id].extendLogger
			) {
            return H::TokenSingleton<DefaultLoggers>::GetData().DefaultLogger();
        }
        return _this.standardLoggersList[id].extendLogger;
    }
}


LOGGER_API H::meta::nothing* __LgCtx() {
    return nullptr;
}