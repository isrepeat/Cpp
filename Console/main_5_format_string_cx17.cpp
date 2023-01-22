//#include <iostream>
//#include <utility>
//#include <string>
//#include <functional>
//#include <vector>
//#include <tuple>
//
//// https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
////template<typename ... Args>
////std::string StringFormat(const std::string& format, Args ... args) {
////	size_t size = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
////	if (size <= 0) { 
////		throw std::runtime_error("Error during formatting."); 
////	}
////	std::unique_ptr<char[]> buf(new char[size]);
////	std::snprintf(buf.get(), size, format.c_str(), args ...);
////	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
////}
//
////template <typename T>
////class TempStr {
////public:
////	TempStr(T data) : data{ data }
////	{}
////
////private:
////	T data;
////};
////
//////TempStr<char> x;
////
//////template<typename T>
//////std::basic_string<T> StringFormat(std::basic_string<T> format) {
//////}
////
////template<typename T>
////void StringFormat(std::vector<T> format) {
////	//return T(12);
////}
////
////template<typename T>
////void StringFormat(T format) {
////	StringFormat(std::vector<T>(std::move(format));
////}
//
//
////std::basic_string<char> StringFormat(const std::basic_string<char>& format) {
////}
//
//
////template<typename T, typename... Args>
////std::basic_string<T> StringFormat(const std::basic_string<T>& format, Args... args) {
////	size_t size = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
////	if (size <= 0) { 
////		throw std::runtime_error("Error during formatting."); 
////	}
////	std::unique_ptr<T[]> buf(new T[size]);
////	std::snprintf(buf.get(), size, format.c_str(), args ...);
////	
////	auto res = std::basic_string<T>(buf.get(), buf.get() + size); // We don't want the '\0' inside
////	return res;
////	//return std::basic_string<T>(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
////}
//
//template<typename T, typename... Args>
//int StringPrintFormat(T* buffer, size_t size, const T* format, Args... args) {
//	if constexpr (std::is_same_v<T, wchar_t>) {
//		return std::swprintf(buffer, size, format, args...);
//	}
//	else {
//		return std::snprintf(buffer, size, format, args...);
//	}
//}
//
//template<typename T, typename... Args>
//std::basic_string<T> StringFormat(const std::basic_string<T>& format, Args... args) {
//	size_t size = StringPrintFormat<T>(nullptr, 0, format.c_str(), std::forward<Args>(args)...) + 1; // Extra space for '\0'
//	if (size <= 0) {
//		throw std::runtime_error("Error during formatting.");
//	}
//	std::unique_ptr<T[]> buf(new T[size]);
//	StringPrintFormat<T>(buf.get(), size, format.c_str(), std::forward<Args>(args)...);
//	return std::basic_string<T>(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
//}
//
//template<typename T, typename... Args>
//std::basic_string<T> StringFormat(const T* format, Args... args) {
//	return StringFormat(std::basic_string<T>(format), std::forward<Args>(args)...);
//}
//
//int main() {
//	printf("Old %d %s %d \n\n", 11, "THE OLD", 22);
//	printf(StringFormat("Formatted %d %s %f \n\n", 666, "THE FORMATTED", 3.14).c_str());
//	wprintf(StringFormat(L"Formatted %d %s %f \n\n", 666, L"THE FORMATTED", 3.14).c_str());
//
//
//	////std::function<int(const char*, va_list)> test2(printf);
//
//	//char buff[100];
//	//StringPrintFormat<int>(buff, 0, "%d", 11);
//	////std::snprintf(buff, 0, "%d", 11);
//
//	system("pause");
//	return 0;
//}