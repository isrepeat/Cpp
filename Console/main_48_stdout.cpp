//#include <iostream>
//#include <functional>
//
//class callback_streambuf : public std::streambuf {
//public:
//	callback_streambuf(std::function<void(char const*, std::streamsize)> callback) : callback(callback) {}
//
//protected:
//	std::streamsize xsputn(char_type const* s, std::streamsize count) {
//		callback(s, count);
//		return count;
//	}
//
//private:
//	std::function<void(char const*, std::streamsize)> callback;
//};
//
//struct cout_redirect {
//	cout_redirect(std::streambuf* new_buffer)
//		: old(std::cout.rdbuf(new_buffer))
//	{}
//
//	~cout_redirect() {
//		std::cout.rdbuf(old);
//	}
//
//private:
//	std::streambuf* old;
//};
//
//
//void MyHandler(char const* data, std::streamsize count) {
//	int xx = 9;
//}
//
//
//
//int main() {
//	auto buf = callback_streambuf(MyHandler);
//	auto pold_buffer = std::cout.rdbuf(&buf);
//
//	//std::cout << "Print to custom stream " << std::endl;
//	printf("Print to custom stream\n");
//
//	// Restore original buffer:
//	std::cout.rdbuf(pold_buffer);
//
//	//std::cout << "Print to stdout stream " << std::endl;
//	printf("Print to stdout stream\n");
//
//	return 0;
//}