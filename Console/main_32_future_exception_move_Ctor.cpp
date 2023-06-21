//#include <Windows.h>
//#include <functional>
//#include <thread>
//#include <Memory>
//#include <future>
//#include <stdexcept>
//#include <comdef.h>
//#include <Helpers/Helpers.h>
//#include <Helpers/Macros.h>
//#include <Helpers/Scope.h>
//#include <Helpers/Time.h>
//
//
//
//struct TestClass {
//	TestClass() = default;
//	~TestClass() = default;
//};
//
//namespace H {
//	namespace System {
//		class ComException : public std::exception {
//		public:
//			ComException(HRESULT hr, const std::wstring& message) 
//				: std::exception(H::WStrToStr(message).c_str())
//				, errorMessage{ message }
//				, errorCode{ hr }
//			{
//			}
//			~ComException() {
//				auto msg = this->what();
//				int xxx = 9;
//			}
//
//			ComException(const ComException&) = default;
//
//			//ComException(ComException&&) = default;
//			//ComException(ComException&&) {
//			//    int xx = 9;
//			//};
//
//			std::wstring ErrorMessage() const {
//				return errorMessage;
//			}
//			HRESULT ErrorCode() const {
//				return errorCode;
//			}
//
//		private:
//			LOG_FIELD_DESTRUCTION(ComException);
//			std::shared_ptr<TestClass> ptr;
//			std::wstring errorMessage;
//			HRESULT errorCode = S_OK;
//		};
//
//
//		inline void ThrowIfFailed(HRESULT hr) {
//			if (FAILED(hr)) {
//				throw ComException(hr, _com_error(hr).ErrorMessage());
//			}
//		}
//	}
//}
//
//
//std::future<void> future;
//
//int main() {
//	{
//		future = std::async(std::launch::async, [] {
//			auto scoped = H::MakeScope([] {
//				H::Timer::Once(100ms, [] {
//					try {
//						// explicitly wait to ensure that previous future lambda finished
//						if (future.valid())
//							future.get();
//					}
//					catch (const H::System::ComException& ex) {
//						int xxx = 9;
//					}
//					});
//				});
//
//			OutputDebugStringA("111 \n");
//			Sleep(200);
//			try {
//				H::System::ThrowIfFailed(E_ACCESSDENIED);
//			}
//			catch (std::exception& ex) {
//				Sleep(1000);
//				throw;
//			}
//			OutputDebugStringA("222 \n");
//			Sleep(500);
//			});
//	}
//
//
//
//	while (1) {
//		Sleep(100);
//	}
//
//	return 0;
//}
