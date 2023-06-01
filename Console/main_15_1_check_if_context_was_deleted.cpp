//#include <Windows.h>
//#include <iostream>
//#include <string>
//#include <vector>
//#include <memory>
//#include <thread>
//#include <queue>
//#include <condition_variable>
//
//struct Listener {
//	std::string name;
//};
//
//class ThirdParty {
//public:
//	enum class Event {
//		Unknown,
//		NewConnection,
//		UserData_1,
//		UserData_2,
//		Shutdown,
//	};
//	using LISTENER_CALLBACK = HRESULT(*)(void* context, Event event);
//
//	ThirdParty() = default;
//	~ThirdParty() {
//		ListenerStop();
//		if (eventsThread.joinable())
//			eventsThread.join();
//
//		ListenerClose();
//		if (listenerThread.joinable())
//			listenerThread.join();
//
//		// TODO: manually delete hListeners
//	}
//
//	void ListenerOpen(LISTENER_CALLBACK callback, void* context, HANDLE* listener) {
//		listenerCallback = callback;
//		listenerContext = context;
//		listenerClosed = false;
//
//		this->listener = new Listener{ "Listener_1" };
//		listener = (HANDLE*)(this->listener);
//
//		listenerThread = std::thread([this] {
//			while (true) {
//				//std::mutex m;
//				std::unique_lock lk{mx};
//				cv.wait(lk);
//
//				Event event = Event::Unknown;
//				{
//					//std::unique_lock lk2{ mx };
//					if (!queueEvents.empty()) {
//						event = queueEvents.front();
//						queueEvents.pop();
//					}
//				}
//				auto res = listenerCallback(listenerContext, event);
//
//				if (event == Event::Shutdown && listenerClosed) {
//					break;
//				}
//			}
//
//			listenerCallback = nullptr;
//			listenerContext = nullptr;
//			delete this->listener;
//			this->listener = nullptr;
//			});
//	}
//
//	void ListenerStart() {
//		listenerStopped = false;
//
//		eventsThread = std::thread([this] {
//			Event arrEvents[3] = { Event::NewConnection, Event::UserData_1, Event::UserData_2 };
//			int i = 0;
//			while (!listenerStopped) {
//				Sleep(1000);
//				{
//					std::unique_lock lk{ mx };
//					queueEvents.push(arrEvents[i]);
//				}
//				cv.notify_all();
//				i++; i %= 3;
//			}
//
//			Sleep(1000);
//			{
//				std::unique_lock lk{ mx };
//				queueEvents.push(Event::Shutdown);
//			}
//			cv.notify_all();
//			});
//	}
//
//	void ListenerStop() {
//		listenerStopped = true;
//	}
//
//	void ListenerClose () {
//		listenerClosed = true;
//	}
//
//private:
//	LISTENER_CALLBACK listenerCallback = nullptr;
//	void* listenerContext = nullptr;
//	std::queue<Event> queueEvents;
//
//	std::thread eventsThread;
//	std::thread listenerThread;
//	std::atomic<bool> listenerClosed = false;
//	std::atomic<bool> listenerStopped = false;
//
//	std::mutex mx;
//	std::condition_variable cv;
//
//	Listener* listener;
//};
//
//
//
//
//class ThirdPartyWrapper {
//private:
//	ThirdPartyWrapper() {
//		thirdParty = new ThirdParty;
//		//contextsManagerThread = std::thread([this] {
//		//	while (contextsManagerWorking) {
//		//		Sleep(100);
//		//	}
//		//	});
//	}
//
//public:
//	~ThirdPartyWrapper() {
//		//contextsManagerWorking = false;
//		//if (contextsManagerThread.joinable())
//		//	contextsManagerThread.join();
//
//	}
//	static ThirdPartyWrapper& GetInstance() {
//		static ThirdPartyWrapper instance;
//		return instance;
//	}
//
//	ThirdParty* GetApiTable() {
//		return thirdParty;
//	}
//
//	struct Context {
//		void* context = nullptr;
//		std::weak_ptr<int> token;
//	};
//
//	Context* AddContextWithToken(void* context, std::shared_ptr<int> token) {
//		PurgeExpiredPtr();
//		listContexts.push_back(Context{
//			context,
//			token,
//			});
//
//		return &listContexts.back();
//	}
//
//private:
//	void PurgeExpiredPtr() {
//		auto predicate = [](Context& context) {
//			return context.token.expired();
//		};
//		auto newEnd = std::remove_if(listContexts.begin(), listContexts.end(), predicate);
//		listContexts.erase(newEnd, listContexts.end());
//	}
//
//private:
//	ThirdParty* thirdParty;
//	
//	//std::mutex mx;
//	//std::thread contextsManagerThread;
//	//std::atomic<bool> contextsManagerWorking = true;
//	std::vector<Context> listContexts;
//};
//
//
//
//struct SomeValue {
//	SomeValue() = default;
//	~SomeValue() {
//		int aaa = 9;
//	}
//
//	int data = 1234;
//};
//
//class MyQUIC {
//public:
//	MyQUIC() {
//		printf("MyQUIC()  \n");
//		ownerName = "MyQUIC";
//		//tokenThis = std::shared_ptr<MyQUIC>(this);
//	};
//
//	~MyQUIC() {
//		printf("~MyQUIC()  \n");
//		ThirdPartyWrapper::GetInstance().GetApiTable()->ListenerStop();
//		ThirdPartyWrapper::GetInstance().GetApiTable()->ListenerClose();
//	};
//
//
//	void StartServer() {
//		auto context = ThirdPartyWrapper::GetInstance().AddContextWithToken(this, token);
//		ThirdPartyWrapper::GetInstance().GetApiTable()->ListenerOpen(&MyQUIC::MyQUICListener, context, &hListener);
//		ThirdPartyWrapper::GetInstance().GetApiTable()->ListenerStart();
//	}
//
//	static HRESULT MyQUICListener(void* context, ThirdParty::Event event) {
//		auto _context = static_cast<ThirdPartyWrapper::Context*>(context);
//		if (_context->token.expired()) {
//			return E_FAIL;
//		}
//		auto _this = static_cast<MyQUIC*>(_context->context);
//		if (IsBadReadPtr(_this, sizeof(decltype(*_this)))) {
//			return E_FAIL;
//		}
//
//		switch (event) {
//		case ThirdParty::Event::NewConnection:
//			_this->Log("NewConnection");
//			break;
//		case ThirdParty::Event::UserData_1:
//			_this->Log("UserData_1");
//			break;
//		case ThirdParty::Event::UserData_2:
//			_this->Log("UserData_2");
//			break;
//		case ThirdParty::Event::Shutdown:
//			_this->Log("Shutdown");
//			break;
//
//		case ThirdParty::Event::Unknown:
//			_this->Log("Unknown");
//			break;
//		}
//
//		return S_OK;
//	}
//
//	void Log(std::string eventName) {
//		printf("[%s] %s \n", ownerName.c_str(), eventName.c_str());
//	}
//
//private:
//	HANDLE hListener;
//	SomeValue someValue;
//	std::string ownerName;
//	std::shared_ptr<int> token = std::make_shared<int>();
//	//std::shared_ptr<MyQUIC> tokenThis;
//};
//
//
//
//int main() {
//	//int* rawPtrSave;
//	//std::weak_ptr<int> weakPtr;
//	//{
//	//	auto rawPtr = new int{ 17 };
//	//	std::shared_ptr<int> shPtr = std::shared_ptr<int>(rawPtr);
//	//	weakPtr = shPtr;
//	//	rawPtrSave = rawPtr;
//	//	//delete rawPtr;
//	//	Sleep(10);
//	//}
//	//Sleep(10);
//
//	{
//		MyQUIC* myQUIC = new MyQUIC();
//		myQUIC->StartServer();
//		Sleep(3'000);
//		delete myQUIC;
//		myQUIC = nullptr;
//	}
//	Sleep(10);
//
//	while (true) {
//		Sleep(1);
//	}
//	return 0;
//}
