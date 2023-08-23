#include <iostream>
#include <optional>
#include <cassert>
#include <string>


class TRDEndpoints {
private:
	static TRDEndpoints& GetInstance();
	TRDEndpoints();
public:
	~TRDEndpoints() = default;

	struct Data {
		const std::string serverUrl;
		const std::string queueUrl;
		const std::string stunServerIp;
		const std::string queueUserName;
		const std::string queuePassword;
		const std::string queueVirtualHost;
		const uint16_t queuePort;
		const uint16_t stunServerPort;
	};

	static const Data& GetData();
	static void LoadFromJSON(const std::string& json);
	
private:
	std::optional<Data> data;
};

TRDEndpoints& TRDEndpoints::GetInstance() {
	static TRDEndpoints instance;
	return instance;
}

TRDEndpoints::TRDEndpoints()  {
	data.emplace(Data{
			"http://teamrd.net:5240",
			"teamrd.net",
			"teamrd.net",
			"client_rdm",
			"8c9f6788c54a410a98c1d09c6e821c37",
			"rdmvhost",
			5243,
			53488,
		});
}

void TRDEndpoints::LoadFromJSON(const std::string& json) {
	static bool loaded = false;
	if (loaded) {
		return;
	}

	try {
		GetInstance().data.emplace(Data{
			"111",
			"222",
			"333",
			"444",
			"555",
			"666",
			0,
			0,
			});

		loaded = true;
	}
	catch (...) {
	}
}

const TRDEndpoints::Data& TRDEndpoints::GetData() {
	assert(GetInstance().data.has_value());
	return GetInstance().data.value();
}


void main() {
	auto& trdEndpoints = TRDEndpoints::GetData();

	auto var1 = TRDEndpoints::GetData().queuePassword;
	auto var2 = TRDEndpoints::GetData().queuePort;
	TRDEndpoints::LoadFromJSON("one");
	auto var3 = TRDEndpoints::GetData().queuePassword;
	auto var4 = TRDEndpoints::GetData().queuePort;
	TRDEndpoints::LoadFromJSON("two");
	auto var5 = TRDEndpoints::GetData().queuePassword;
	auto var6 = TRDEndpoints::GetData().queuePort;

	return;
}

