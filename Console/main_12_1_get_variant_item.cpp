//#include <Windows.h>
//#include <type_traits>
//#include <variant>
//#include <memory>
//#include <thread>
//#include <mutex>
//
//
//class Interface {
//public:
//	Interface() = default;
//	virtual ~Interface() = default;
//
//	virtual void Foo() = 0;
//};
//
//class A : public Interface {
//public:
//	A() {};
//
//	void Foo() {}
//};
//
//
//class B : public Interface {
//public:
//	B() {};
//
//	void Foo() {}
//};
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
////std::unique_ptr<int>& foo() {
////	static std::unique_ptr<int> emptyPoiner = nullptr;
////	return emptyPoiner;
////};
//
//std::condition_variable cvVncClientCreation;
//
//int main() {
//	std::variant<std::unique_ptr<A>, std::unique_ptr<B>> variant;
//
//	//auto& varEmpty = std::get<std::unique_ptr<B>>(variant);
//	//if (!varEmpty) {
//	//	int xxx = 9;
//	//}
//
//	
//
//	if (auto& ptr = GetVariantItem<A>(variant)) {
//		int xxx = 9;
//	}
//
//	
//	variant = std::make_unique<B>();
//	
//	auto& var = std::get<std::unique_ptr<B>>(variant);
//	var.reset();
//
//	if (!var) {
//		int xxx = 9;
//	}
//
//	auto& varReset = std::get<std::unique_ptr<B>>(variant);
//
//	if (!varReset) {
//		int xxx = 9;
//	}
//
//
//	return 0;
//}