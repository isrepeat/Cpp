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
//
//template <typename ...T>
//std::enable_if_t<(std::is_same<T, std::unique_ptr<T>>::value && ...), bool>
//IsPointer(T... ptrs) {
//	return true;
//}
//
//
//template <typename... T>
//bool IsEmptyVariantOfUniquePointers(const std::variant<std::unique_ptr<T>...>& variant) {
//
//	bool emptyVariant = true;
//	std::visit([&emptyVariant](auto&& arg) {
//		emptyVariant = arg == nullptr;
//		}, variant);
//
//	return emptyVariant;
//}
//
//
////template <typename... T>
////std::enable_if_t<???, bool>
////IsEmptyVariantOfPointers(const std::variant<T...>& variant) {
////
////	bool emptyVariant = true;
////	std::visit([&emptyVariant](auto&& arg) {
////		emptyVariant = arg == nullptr;
////		}, variant);
////
////	return emptyVariant;
////}
//
//
//std::condition_variable cvVncClientCreation;
//
//int main() {
//	std::variant<std::unique_ptr<A>, std::unique_ptr<B>> variant;
//
//	auto th = std::thread([&variant] {
//		//Sleep(3000);
//		if (IsEmptyVariantOfUniquePointers(variant)) {
//			std::mutex m;
//			std::unique_lock lk{ m };
//			cvVncClientCreation.wait(lk);
//		}
//		auto& client = std::get<std::unique_ptr<A>>(variant);
//		int xxx = 9;
//		});
//
//
//	Sleep(2000);
//	variant = std::make_unique<A>();
//	cvVncClientCreation.notify_one();
//	//std::get<std::unique_ptr<A>>(variant).reset();
//
//	th.join();
//	return 0;
//}
//
////int main()
////{
////    class A {};
////
////    static_assert(
////        not std::is_pointer<A>::value
////        && not std::is_pointer_v<A>   // same thing as above, but in C++17!
////        && not std::is_pointer<A>()   // same as above, using inherited operator bool
////        && not std::is_pointer<A>{}   // ditto
////    && not std::is_pointer<A>()() // same as above, using inherited operator()
////        && not std::is_pointer<A>{}() // ditto
////        && std::is_pointer<A*>::value
////        && std::is_pointer<A const* volatile>()
////        && not std::is_pointer<A&>::value
////        && not std::is_pointer<int>::value
////        && std::is_pointer<int*>::value
////        && std::is_pointer<int**>::value
////        && not std::is_pointer<int[10]>::value
////        && not std::is_pointer<std::nullptr_t>::value
////        );
////
////    return 0;
////}