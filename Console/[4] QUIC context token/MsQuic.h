//#pragma once
//#include <Windows.h>
//#include <string>
//#include <vector>
//#include <thread>
//#include <memory>
//#include <mutex>
//#include <queue>
//#include <functional>
//#include <condition_variable>
//
//
//struct Listener {
//	std::string name;
//};
//
//class MsQuic {
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
//	MsQuic() = default;
//	~MsQuic() {
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
//				std::unique_lock lk{ mx };
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
//				Sleep(950);
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
//	void ListenerClose() {
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
//class MsQuicWrapper {
//private:
//	MsQuicWrapper() {
//		msQuic = new MsQuic;
//	}
//
//public:
//	~MsQuicWrapper() {
//	}
//
//	static MsQuicWrapper& GetInstance() {
//		static MsQuicWrapper instance;
//		return instance;
//	}
//
//	MsQuic* GetApiTable() {
//		return msQuic;
//	}
//
//	struct Context {
//		void* context = nullptr;
//		std::weak_ptr<int> token;
//		std::function<void()> shutdownCallback = nullptr;
//	};
//
//	Context* AddContextWithToken(void* context, std::shared_ptr<int> token) {
//		listContexts.push_back(Context{
//			context,
//			token,
//			});
//
//		auto& pushedItem = listContexts.back();
//		pushedItem.shutdownCallback = [this, currElemIterator = --listContexts.end()] {
//			if (currElemIterator->token.expired()) {
//				//printf("context deleted from list, list.size = %d \n", (int)(listContexts.size()-1));
//				listContexts.erase(currElemIterator); // remove expired context from list
//				printf("context deleted from list \n");
//				// NOTE: after call erese capture context of lambda shutdownCallback are invalid
//			}
//		};
//		return &pushedItem;
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
//	MsQuic* msQuic;
//	std::list<Context> listContexts; // Use list to avoid invalidation items when push !!!
//};
