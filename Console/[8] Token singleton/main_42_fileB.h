//#pragma once
//#include <memory>
//#include "main_42_TokenSingleton.h"
//
////template<class T>
////struct IToken {
////	using outer_class = T;
////	struct Data;
////};
//
//class Logger {
//private:
//	Logger();
//	static Logger& GetInstance();
//public:
//	~Logger();
//
//	static void Init();
//	static void Log();
//
//	/*class UnscopedData {
//	public:
//		UnscopedData() {
//			someData = 123;
//		}
//		~UnscopedData() {
//			int xx = 9;
//		}
//
//	private:
//		int someData;
//	};*/
//
//	class UnscopedData;
//
//private:
//	int* data1;
//	int* data2;
//	int* data3;
//	//std::shared_ptr<int> token = std::make_shared<int>();
//
//	/*class MyToken : ITokenSingleton {
//	public:
//		MyToken() {
//			someData = 123;
//		}
//		~MyToken() = delete;
//
//		bool IsExpired() override {
//			return token.expired();
//		}
//	private:
//		int someData;
//		std::weak_ptr<int> token = std::make_shared<int>();
//	};
//
//	MyToken* myToken;*/
//
//
//	
//
//
//
//
//	std::shared_ptr<UnscopedData> token = std::make_shared<UnscopedData>();
//};