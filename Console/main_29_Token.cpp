//#include <Windows.h>
//#include <thread>
//#include <mutex>
//#include <memory>
//#include <functional>
//
//// https://stackoverflow.com/questions/45507041/how-to-check-if-weak-ptr-is-empty-non-assigned
//template <typename T>
//bool IsWeakPtrUninitialized(std::weak_ptr<T> const& weak) {
//	using wt = std::weak_ptr<T>;
//	return !weak.owner_before(wt{}) && !wt{}.owner_before(weak);
//}
//
////class TaskToken {
////public:
////	TaskToken() = default;
////	TaskToken(std::function<void()> task)
////		: token{ std::make_shared<std::function<void()>>(task) }
////	{
////	}
////
////	~TaskToken() {
////		std::unique_lock lk{ mx };
////		int xxx = 9;
////	}
////
////	//TaskToken(const TaskToken& other) {
////	//	// no need to lock this objec because no other thread
////	//	// will be using it until after construction
////	//	// but we DO need to lock the other object
////	//	std::unique_lock<std::mutex> lock_other{ other.mx };
////
////	//	token = other.token;
////	//}
////
////	//TaskToken& operator=(const TaskToken& other) {
////	//	if (&other != this) {
////	//		// lock both objects
////	//		std::unique_lock<std::mutex> lock_this(mx, std::defer_lock);
////	//		std::unique_lock<std::mutex> lock_other(other.mx, std::defer_lock);
////
////	//		// ensure no deadlock
////	//		std::lock(lock_this, lock_other);
////
////	//		// safely copy the data
////	//		token = other.token;
////	//	}
////	//	return *this;
////	//}
////
////
////	TaskToken& operator=(std::function<void()>&& task) {
////		token = std::make_shared<std::function<void()>>(task);
////		return *this;
////	}
////
////	explicit operator bool() const {
////		return token != nullptr;
////	}
////
////	//TaskToken(TaskToken&& other) = default;
////	//TaskToken& operator=(TaskToken&& other) = default;
////
////private:
////	//mutable std::mutex mx;
////	std::mutex mx;
////	std::shared_ptr<std::function<void()>> token;
////};
//
//
//
////bool Once(std::shared_ptr<std::function<void()>> tokenTask, int timeout, std::function<void()> callback) {
////	// mutex lock
////	if (tokenTask)
////		return false; // cannot rewrite token callback while it not empty
////
////	tokenTask = std::make_shared<std::function<void()>>(callback);
////	std::weak_ptr<std::function<void()>> callbackWeakPtr = tokenTask;
////	// mutex unlock
////
////	std::thread([tokenTask, timeout, callbackWeakPtr]() mutable {
////		Sleep(timeout);
////		if (auto lockedCallback = callbackWeakPtr.lock()) {
////			// mutex lock
////			(*lockedCallback)();
////			tokenTask = nullptr;
////			// mutex unlock
////		}
////		}).detach();
////
////		return true;
////}
//
//
////struct TaskToken {
////	std::mutex mx;
////	std::shared_ptr<std::function<void()>> token;
////
////	~TaskToken() {
////		std::unique_lock lk{ mx };
////		int xx = 9;
////	}
////};
//
////bool Once(TaskToken& taskToken, int timeout, std::function<void()> callback) {
////	{
////		std::unique_lock lk{ taskToken.mx };
////		if (taskToken.token)
////			return false; // cannot rewrite token callback while it not empty
////
////		taskToken.token = std::make_shared<std::function<void()>>(callback);
////	}
////
////	std::weak_ptr<std::function<void()>> callbackWeakPtr = taskToken.token;
////	std::thread([&taskToken, timeout, callbackWeakPtr] {
////		Sleep(timeout);
////		if (auto lockedCallback = callbackWeakPtr.lock()) {
////			std::unique_lock lk{ taskToken.mx };
////			(*lockedCallback)();
////			taskToken.token = nullptr;
////		}
////		}).detach();
////
////		return true;
////}
//
//class TaskWrapper {
//public:
//	TaskWrapper() = default;
//	~TaskWrapper() {
//		std::unique_lock lk{ mx };
//		int xx = 9;
//	}
//
//	bool SetTask(std::function<void()> task) {
//		std::unique_lock lk{ mx };
//		if (this->task)
//			return false; // task alreay set, you need perform it and then set
//
//		this->task = task;
//		return true;
//	}
//
//	void PerformAndRelease() {
//		std::unique_lock lk{ mx };
//		if (task) {
//			task();
//			task = nullptr;
//		}
//	}
//
//private:
//	std::mutex mx;
//	std::function<void()> task = nullptr;
//};
//
////bool Once(std::shared_ptr<TaskWrapper>& taskToken, int timeout, std::function<void()> callback) {
////	{
////		std::unique_lock lk{ taskToken->mx };
////		if (taskToken->task)
////			return false; // cannot rewrite token callback while it not empty
////
////		taskToken->task = callback;
////	}
////
////	std::weak_ptr<MxTask> taskWeakPtr = taskToken;
////	std::thread([timeout, taskWeakPtr] {
////		Sleep(timeout);
////		if (auto lockedTask = taskWeakPtr.lock()) {
////			std::unique_lock lk{ lockedTask->mx };
////			if (lockedTask->task) {
////				lockedTask->task();
////				lockedTask->task = nullptr;
////			}
////		}
////		}).detach();
////
////		return true;
////}
//
//bool Once(std::shared_ptr<TaskWrapper>& taskToken, int timeout, std::function<void()> callback) {
//	if (!taskToken->SetTask(callback))
//		return false;
//
//	std::weak_ptr<TaskWrapper> weakTaskToken = taskToken;
//	std::thread([timeout, weakTaskToken] {
//		Sleep(timeout);
//		if (auto taskToken = weakTaskToken.lock()) { // here was copy shared pointer
//			taskToken->PerformAndRelease(); 
//			// TaskWrapper will live here even if owner was destoyed
//		}
//		}).detach();
//
//	return true;
//}
//
//
//
//
////
////
////class TaskToken {
////public:
////	TaskToken() = default;
////	//TaskToken(std::function<void()> task)
////	//	: token{ std::make_shared<std::function<void()>>(task) }
////	//{
////	//}
////
////	std::weak_ptr<std::function<void()>> SetTask(std::function<void()> callback) {
////		std::unique_lock lk{ mx };
////		if (token)
////			return std::weak_ptr<std::function<void()>>{}; // token is not empty, ignore
////		
////		token = std::make_shared<std::function<void()>>(callback);
////		return token;
////	}
////
////	void PerformTask() {
////		std::unique_lock lk{ mx };
////		if (token) {
////			(*token)();
////			token = nullptr;
////		}
////	}
////
////	//std::weak_ptr<std::function<void()>> GetWeak() {
////	//	return token;
////	//}
////
////	~TaskToken() {
////		std::unique_lock lk{ mx };
////		int xx = 9;
////	}
////
////private:
////	std::mutex mx;
////	std::shared_ptr<std::function<void()>> token;
////};
//
//
//
//
//
//
////class TaskWrapper {
////public:
////	TaskWrapper() = default;
////	TaskWrapper(std::function<void()> task)
////		: task{ task }
////	{
////	}
////
////	~TaskWrapper() {
////		std::unique_lock lk{ mx };
////		int xx = 9;
////	}
////
////	bool SetTask(std::function<void()> task) {
////		std::unique_lock lk{ mx };
////		if (this->task)
////			return false;
////
////		this->task = task;
////		return true;
////	}
////
////	void PerformTask() {
////		std::unique_lock lk{ mx };
////		if (task) {
////			task();
////			task = nullptr;
////		}
////	}
////
////
////private:
////	std::mutex mx;
////	std::function<void()> task;
////};
////
////
////class TaskToken {
////public:
////	TaskToken() = default;
////	//TaskToken(std::function<void()> task)
////	//	: token{ std::make_shared<std::function<void()>>(task) }
////	//{
////	//}
////
////	std::weak_ptr<TaskWrapper> SetToken(std::function<void()> callback) {
////		std::unique_lock lk{ mxToken };
////		if (token)
////			return std::weak_ptr<TaskWrapper>{}; // token is not empty, ignore
////
////		token = std::make_shared<TaskWrapper>(callback);
////		return token;
////	}
////
////	void PerformTask() {
////		std::unique_lock lk{ mxToken };
////		if (token) {
////			(*token)();
////			token = nullptr;
////		}
////	}
////
////	//std::weak_ptr<std::function<void()>> GetWeak() {
////	//	return token;
////	//}
////
////	~TaskToken() {
////		std::unique_lock lk{ mxToken };
////		int xx = 9;
////	}
////
////private:
////	std::mutex mxToken;
////	std::shared_ptr<TaskWrapper> token;
////};
////
////bool Once(TaskToken& token, int timeout, std::function<void()> callback) {
////	auto weakToken = token.SetToken(callback);
////	if (!weakToken.lock())
////		return false;
////
////	//if (IsWeakPtrUninitialized(weakToken))
////	//	return false;
////
////	//auto tokenWeakPtr = token.GetWeak();
////	std::thread([&token, timeout, weakToken] {
////		Sleep(timeout);
////		if (auto lockedCallback = tokenWeakPtr.lock()) {
////			std::unique_lock lk{ taskToken.mx };
////			(*lockedCallback)();
////			taskToken.token = nullptr;
////		}
////		}).detach();
////
////	return true;
////}
//
//
//
//struct SomeValue {
//	SomeValue() {
//	}
//	~SomeValue() {
//		int xxx = 9;
//	}
//};
//
//class Owner {
//public:
//	Owner() {
//	}
//	~Owner() {
//		int xx = 9;
//	}
//
//	void SetTimeoutHandler(std::function<void()> handler) {
//		timeoutHandler = handler;
//	}
//
//
//	void CallAsync(int afterMs) {
//		Once(taskToken, afterMs, [this] {
//			timeoutHandler();
//			privateData = 5678;
//			int xxx = 9;
//			});
//	}
//
//private:
//	SomeValue someValue; // destruct at last (if Dtor SomeValue called this mean Owner also destucted)
//	std::shared_ptr<TaskWrapper> taskToken = std::make_shared<TaskWrapper>();
//	//TaskToken token;
//	std::function<void()> timeoutHandler;
//	int privateData = 1234;
//};
//
//
//
//
//void main() {
//	//TaskToken token;
//	//token = TaskToken();
//	//if (token) {
//	//	token = [] {
//	//		int xxx = 9;
//	//	};
//	//}
//	//else {
//	//	token = TaskToken([] {
//	//		int yyy = 9;
//	//		});
//	//}
//
//	//std::weak_ptr<std::function<void()>> weak;
//	//if (IsWeakPtrUninitialized(weak)) {
//	//	auto lk = weak.lock();
//	//	int xxx = 9;
//	//}
//	//{
//	//	auto sh1 = std::make_shared<std::function<void()>>([] {});
//	//	weak = sh1;
//	//}
//	//if (IsWeakPtrUninitialized(weak)) {
//	//	int xxx = 9;
//	//}
//
//	//weak = std::weak_ptr<std::function<void()>>{ };
//	//if (IsWeakPtrUninitialized(weak)) {
//	//	int xxx = 9;
//	//}
//	//auto sh2 = std::make_shared<std::function<void()>>(nullptr);
//	//weak = sh2;
//
//	{
//		Owner owner;
//
//		owner.SetTimeoutHandler([] {
//			Sleep(200);
//			Sleep(200);
//			Sleep(200);
//			Sleep(200);
//			Sleep(200);
//			});
//
//		owner.CallAsync(1000);
//
//		Sleep(1500);
//		int end = 99;
//	}
//
//
//	Sleep(10'000);
//
//	return;
//}