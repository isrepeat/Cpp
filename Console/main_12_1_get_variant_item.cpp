//#include <Windows.h>
//#include <type_traits>
//#include <variant>
//#include <memory>
//#include <thread>
//#include <mutex>
//
//
//// NOTE: "T" must have a common(base) type(interface)
//template <typename... Types>
//bool IsEmptyVariantOfPointers(const std::variant<std::unique_ptr<Types>...>& variant) {
//	bool emptyVariant = true;
//	std::visit([&emptyVariant](auto&& arg) {
//		emptyVariant = arg == nullptr;
//		}, variant);
//
//	return emptyVariant;
//}
//
//
//// Helper to get pointer type of variant or nullptr if variant has bad access
//template <typename T, typename... Types>
//std::unique_ptr<T>& GetVariantItem(std::variant<std::unique_ptr<Types>...>& variant) {
//	static const std::unique_ptr<T> emptyPoiner = nullptr;
//
//	try {
//		return std::get<std::unique_ptr<T>>(variant);
//	}
//	catch (const std::bad_variant_access& ex) {
//		return const_cast<std::unique_ptr<T>&>(emptyPoiner); // remove const
//	}
//}
//
//namespace H {
//	struct EmptyStruct {};
//	using NullVariantArg = std::shared_ptr<EmptyStruct>;
//	const NullVariantArg nullVariantArg = nullptr;
//}
//
//
//
//
//struct IVncServer {
//	virtual void StartListening() = 0;
//	virtual ~IVncServer() = default;
//};
//
//
//struct IVncClient {
//	virtual void Render() = 0;
//	virtual ~IVncClient() = default;
//};
//
//
//struct VncClient : IVncClient {
//	VncClient() = default;
//	~VncClient() {
//		int xx = 9;
//	};
//
//	void Render() override {
//	}
//};
//
//
//struct VncServer : IVncServer {
//	VncServer() = default;
//	~VncServer() {
//		int xx = 9;
//	};
//
//	void StartListening() override {
//	}
//};
//
//
////std::variant<H::NullVariantArg, std::unique_ptr<IVncClient>, std::unique_ptr<IVncServer>> vncComponent;
//std::variant<std::unique_ptr<IVncClient>, std::unique_ptr<IVncServer>> vncComponent;
//
//
//enum class VncType {
//	None,
//	Client,
//	Server,
//};
//
//template <typename... T>
//bool IsEmptyVariantOfPointers(const std::variant<std::unique_ptr<T>...>& variant) {
//	bool emptyVariant = true;
//	std::visit([&emptyVariant](auto&& arg) {
//		emptyVariant = arg == nullptr;
//		}, variant);
//
//	return emptyVariant;
//}
//
//
//VncType GetVncComponentType() {
//	if (IsEmptyVariantOfPointers(vncComponent))
//		return VncType::None;
//
//	if (std::holds_alternative<std::unique_ptr<IVncClient>>(vncComponent)) {
//		return VncType::Client;
//	}
//	else if (std::holds_alternative<std::unique_ptr<IVncServer>>(vncComponent)) {
//		return VncType::Server;
//	}
//}
//
//
//int main() {
//	{
//		//if (std::holds_alternative<H::NullVariantArg>(vncComponent)) {
//		//	int xxx = 9;
//		//}
//		//else 
//		if (std::holds_alternative<std::unique_ptr<IVncClient>>(vncComponent)) {
//			int xxx = 9;
//		}
//		else if (std::holds_alternative<std::unique_ptr<IVncServer>>(vncComponent)) {
//			int xxx = 9;
//		}
//		else {
//			int xxx = 9;
//		}
//		vncComponent.index();
//		//std::get<>
//
//		auto vncType = GetVncComponentType();
//		//auto isEmpty = IsEmptyVariantOfPointers(vncComponent);
//		int xxx = 9;
//	}
//
//	vncComponent = std::make_unique<VncServer>();
//
//	{
//		//if (std::holds_alternative<H::NullVariantArg>(vncComponent)) {
//		//	int xxx = 9;
//		//}
//		//else 
//		if (std::holds_alternative<std::unique_ptr<IVncClient>>(vncComponent)) {
//			int xxx = 9;
//		}
//		else if (std::holds_alternative<std::unique_ptr<IVncServer>>(vncComponent)) {
//			int xxx = 9;
//		}
//		else {
//			int xxx = 9;
//		}
//		auto vncType = GetVncComponentType();
//		//auto isEmpty = IsEmptyVariantOfPointers(vncComponent);
//		int xxx = 9;
//	}
//
//	std::visit([](auto&& arg) {
//		arg.reset();
//		}, vncComponent);
//	//vncComponent = H::nullVariantArg;
//
//	{
//		//if (std::holds_alternative<H::NullVariantArg>(vncComponent)) {
//		//	int xxx = 9;
//		//}
//		//else 
//		if (std::holds_alternative<std::unique_ptr<IVncClient>>(vncComponent)) {
//			int xxx = 9;
//		}
//		else if (std::holds_alternative<std::unique_ptr<IVncServer>>(vncComponent)) {
//			int xxx = 9;
//		}
//		else {
//			int xxx = 9;
//		}
//		auto vncType = GetVncComponentType();
//		//auto isEmpty = IsEmptyVariantOfPointers(vncComponent);
//		int xxx = 9;
//	}
//
//
//	return 0;
//}