#include <Helpers/Logger.h>
#include <Helpers/Time.h>
#include <Windows.h>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>

void TestWeakAsToken() {
	std::mutex mx;
	std::weak_ptr<std::function<void()>> callbackWeakPtr;

	{
		std::shared_ptr<std::function<void()>> token;
		{
			auto sharedPtr = std::make_shared<std::function<void()>>([] {
				int xx = 9;
				});
			callbackWeakPtr = sharedPtr;
			token = sharedPtr;
		}


		{
			auto locked = callbackWeakPtr.lock();
			if (locked) {
				(*locked)();
				token = nullptr;
			}
		}

		{
			auto lockedAgain = callbackWeakPtr.lock();
			if (lockedAgain) {
				(*lockedAgain)();
			}
		}
	}

	//auto locked = callbackWeakPtr.lock();
	//if (locked) {
	//	(*locked)();
	//}
}

void TestWeakOnEmptySharedOrEmptyPointer() {
	std::shared_ptr<std::function<void()>> shPtr1 = nullptr;
	std::shared_ptr<std::function<void()>> shPtr2 = std::make_shared<std::function<void()>>(nullptr);

	std::weak_ptr<std::function<void()>> weakPtr1 = shPtr1;
	std::weak_ptr<std::function<void()>> weakPtr2 = shPtr2;

	if (!weakPtr1.expired()) {
		printf("weakPtr1 not expired \n"); // not enter
	}
	if (auto ptr1 = weakPtr1.lock()) {
		printf("weakPtr1 not empty \n");  // not enter
	}

	if (!weakPtr2.expired()) {
		printf("weakPtr2 not expired \n"); // enter
	}
	if (auto ptr2 = weakPtr2.lock()) {
		printf("weakPtr2 not empty \n"); // enter
		//if (ptr2) { // will be Access violation below
		if (*ptr2) {
			(*ptr2)();
		}
	}
}

struct Temp {
	Temp() {
		int xx = 9;
	}
	~Temp() {
		int xx = 9;
	}
};

template <typename T>
struct Deleter {
	void operator() (T* ptr) {
		delete ptr;
	}
};

void TestWeakAndDestuctorShared() {
	//std::shared_ptr<Temp> shPtr = std::make_shared<Temp>(Deleter<Temp>{});

	std::weak_ptr<Temp> weakPtr;
	{
		std::shared_ptr<Temp> shPtr = std::shared_ptr<Temp>(new Temp{}, Deleter<Temp>{});
		weakPtr = shPtr;
		Sleep(100);
	}

	if (weakPtr.expired()) {
		printf("weakPtr expired \n");
	}

	Sleep(100);
	return;
}



class MsQuicInitializer {
public:
	MsQuicInitializer(const MsQuicInitializer&) = delete;
	MsQuicInitializer(MsQuicInitializer&&) = delete;

	static MsQuicInitializer& GetInstance() {
		static auto instance = MsQuicInitializer{};
		return instance;
	}

	struct Context {
		void* context = nullptr;
		std::weak_ptr<int> token;
		std::function<void()> shutdownConnectionCallback = nullptr;
	};
	Context* AddContextWithToken(void* context, std::shared_ptr<int> token) {
		LOG_FUNCTION_ENTER("AddContextWithToken(context, token)");
		listContexts.push_back(Context{
			context,
			token,
			});

		auto& pushedItem = listContexts.back();
		LOG_DEBUG_D("pushedItem = {}", static_cast<void*>(&pushedItem));

		pushedItem.shutdownConnectionCallback = [this, currElemIterator = --listContexts.end()]{ // save iterator on current(last) element to erase it later
			// NOTE: Alternatively you can use a timer to remove old expired context if was problem with syncroniously deletion form QUIC connection callback.
			//H::Timer::Once(10'000ms, [this, currElemIterator] {
			LOG_FUNCTION_ENTER("shutdownConnectionCallback()");
			LOG_DEBUG_D("deletionItem = {}", static_cast<void*>(&(*currElemIterator)));
			if (currElemIterator->token.expired()) {
				listContexts.erase(currElemIterator); // remove expired context from list
			}
		//});
		};
		return &pushedItem;
	}

private:
	MsQuicInitializer() {
		LOG_FUNCTION_ENTER("MsQuicInitializer()");
	}
	~MsQuicInitializer() {
		LOG_FUNCTION_ENTER("~MsQuicInitializer()");
		//listContexts.~list();
	}

private:
	LOG_FIELD_DESTRUCTION(MsQuicInitializer);
	std::list<Context> listContexts; // use list to avoid invalidation items when push
};


class MyClass {
public:
	static int instanceCounter;

	MyClass(int callStaticFunctionAfterMs = 0) {
		LOG_FUNCTION_ENTER("MyClass() [ptr = {}]", static_cast<void*>(this));
		auto currentInstanceIdx = instanceCounter++;

		auto contextPtr = MsQuicInitializer::GetInstance().AddContextWithToken(this, token);
		
		if (callStaticFunctionAfterMs > 0) {
			H::Timer::Once(std::chrono::milliseconds{ callStaticFunctionAfterMs }, [contextPtr, currentInstanceIdx] {
				LOG_FUNCTION_ENTER("Timer_lambda() [{}]", currentInstanceIdx);
				MyClass::StaticFunction(contextPtr);
				});
		}
	}
	~MyClass() {
		LOG_FUNCTION_ENTER("~MyClass()");
	}

	static void StaticFunction(void* context) {
		LOG_FUNCTION_ENTER("StaticFunction(context)");
		auto _context = static_cast<MsQuicInitializer::Context*>(context);
		if (_context->token.expired()) {
			_context->shutdownConnectionCallback();
		}
	}

private:
	std::shared_ptr<int> token = std::make_shared<int>();
};

int MyClass::instanceCounter = 0;

void TestWeakDestroyInSingleton() {
	//{
		//MyClass* myClass1 = new MyClass;
		//MyClass* myClass2 = new MyClass;
		//MyClass* myClass3 = new MyClass;
	//}
	{
		for (int i = 0; i < 5; i++) {
			if (i % 3 == 0) {
				//MyClass myClass(2190);
				MyClass myClass(1000);
			}
			else {
				MyClass myClass;
			}
		}
		//MyClass myClass1(1000);
		//MyClass myClass2;
		//MyClass myClass3(2190);
		//MyClass myClass4;
		//MyClass myClass6;
		//MyClass myClass7(500);
		//MyClass myClass8;
		//MyClass myClass1(3000ms);
	}
	//{ 
	//	MyClass myClass2; 
	//}
	//{
		//MyClass myClass3;
	//}
	Sleep(2200);
	return;
}


void TestRemoveExpiredWeakFromVector() {
	std::vector<std::weak_ptr<int>> vecWeaks;
	vecWeaks.reserve(5);
	auto shPtr1 = std::make_shared<int>(11);
	auto shPtr2 = std::make_shared<int>(22);
	auto shPtr3 = std::make_shared<int>(33);

	vecWeaks.push_back(shPtr1);
	vecWeaks.push_back(shPtr2);
	{
		auto shPtrTmp1 = std::make_shared<int>(33);
		auto shPtrTmp2 = std::make_shared<int>(44);
		vecWeaks.push_back(shPtrTmp1);
		vecWeaks.push_back(shPtrTmp2);
		vecWeaks.push_back(shPtr3);
		Sleep(10);
	}

	Sleep(10);

	auto predicate = [](auto element) {
		return element.expired();
	};

	auto newEnd = std::remove_if(vecWeaks.begin(), vecWeaks.end(), predicate);
	auto shPtr6 = std::make_shared<int>(66);
	vecWeaks.push_back(shPtr6);
	vecWeaks.erase(newEnd, vecWeaks.end());

	Sleep(10);
}


void main() {
	lg::DefaultLoggers::Init("D:\\main_26_weak_ptr.log", lg::InitFlags::DefaultFlags);
	//TestWeakAsToken();
	//TestWeakOnEmptySharedOrEmptyPointer();
	//TestWeakAndDestuctorShared();
	TestWeakDestroyInSingleton();
	//TestRemoveExpiredWeakFromVector();

	return;
}