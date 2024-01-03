#include "main_42_fileB.h"
#include "main_42_TokenSingleton.h"

class Logger::UnscopedData {
public:
	UnscopedData() {
		someData = 123;
	}
	~UnscopedData() {
		int xx = 9;
	}

private:
	int someData;
};


Logger& Logger::GetInstance() {
	static Logger instance;
	return instance;
}

Logger::Logger()
	: data1{ new int{111} }
	, data2{ new int{222} }
	, data3{ new int{333} }
{
}

Logger::~Logger() {
	bool expired = TokenSingleton<Logger>::IsExpired();
	auto& _this = GetInstance();
	_this.data1 = nullptr;
	_this.data2 = nullptr;
	_this.data3 = nullptr;
	int xxx = 9;
}

void Logger::Init() {
	auto& _this = GetInstance();
	TokenSingleton<Logger>::SetToken(Passkey<Logger>{}, _this.token);
}

void Logger::Log() {
	bool expired = TokenSingleton<Logger>::IsExpired();
	if (expired) {
		return;
	}
	auto& _this = GetInstance();
	*_this.data1 = 666;
	*_this.data2 = 777;
	*_this.data3 = 888;
	int x = 9;
}