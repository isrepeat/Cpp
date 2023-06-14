#include <mutex>
#include <iostream>
#include <Windows.h>
#include <functional>

template <typename T>
bool IsWeakPtrUninitialized(const std::weak_ptr<T>& weak) {
	using wt = std::weak_ptr<T>;
	//bool cmp_1 = weak.owner_before(wt{});
	//bool cmp_2 = wt{}.owner_before(weak);
	return !weak.owner_before(wt{}) && !wt{}.owner_before(weak);
}

template <typename T>
bool IsWeakPtrInitialized(const std::weak_ptr<T>& weak) {
	using wt = std::weak_ptr<T>;;
	return weak.owner_before(wt{}) || wt{}.owner_before(weak);
}


class Callback {
public:
	Callback(std::function<void(int)> handler)
		: handler{ handler }
	{}

	//Callback& operator=(const Callback& other) {
	//	if (&other != this) {
	//		//// lock both objects
	//		//std::unique_lock<std::mutex> lock_this(mx, std::defer_lock);
	//		//std::unique_lock<std::mutex> lock_other(other.mx, std::defer_lock);

	//		//// ensure no deadlock
	//		//std::lock(lock_this, lock_other);

	//		// safely copy the data
	//		handler = other.handler;
	//		useToken = other.useToken;
	//	}
	//	return *this;
	//}

	~Callback() {
		int xx = 9;
	}

	void SetToken(std::shared_ptr<int> callerToken) {
		this->callerToken = callerToken;
	}

	void SetCv(std::weak_ptr<std::condition_variable> callerCv) {
		this->callerCv = callerCv;
	}

	void operator() (int arg) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		// guard based on token
		if (IsWeakPtrInitialized(callerToken)) {
			if (callerToken.expired())
				return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (this->handler) {
			this->handler(arg);
		}

		// guard based on cv wait
		if (auto cv = callerCv.lock()) {
			cv->notify_all(); // signal about finish callback
		}
	}

private:
	//std::mutex mx;
	std::weak_ptr<int> callerToken;
	std::weak_ptr<std::condition_variable> callerCv;
	std::function<void(int)> handler;
};



struct ServerCallbacks {
	Callback connect = std::function([](int) {});
	Callback getUserInfo = std::function([](int) {});

	ServerCallbacks() = default;
	~ServerCallbacks() {
		int xx = 9;
	}


	// TODO: mb use variant to set either token or cv?
	// Set caller token to skip operator() when token expired
	void SetToken(std::shared_ptr<int>& callerToken) {
		connect.SetToken(callerToken);
		getUserInfo.SetToken(callerToken);
		useTokenGuard = true;
	}

	// Set caller cv to nitify caller when operaotr() finished
	void SetCv(std::shared_ptr<std::condition_variable>& callerCv) {
		connect.SetCv(callerCv);
		getUserInfo.SetCv(callerCv);
		useCvGuard = true;
	}

	bool IsUseTokenGuard() {
		return useTokenGuard;
	}

	bool IsUseCvGuard() {
		return useCvGuard;
	}

private:
	std::atomic<bool> useTokenGuard = false;
	std::atomic<bool> useCvGuard = false;
};








struct SomeValue {
	~SomeValue() {
		int xxx = 9;
	}
};

class Server {
public:
	Server() {
		callbacks = std::make_shared<ServerCallbacks>();
		callbacks->connect = std::function([](int num) {
			Sleep(100);
			Sleep(100);
			Sleep(100);
			Sleep(100);
			});
		callbacks->getUserInfo = std::function([](int num) {
			Sleep(100);
			Sleep(100);
			Sleep(100);
			Sleep(100);
			});

		callbacks->SetToken(tokenCallbacks);
		//callbacks->SetCv(cvCallbacks);
	}

	~Server() {
		if (callbacks->IsUseCvGuard()) {
			std::unique_lock lk{ mxCallbacks };
			cvCallbacks->wait(lk, [this] {
				int count = callbacks.use_count();
				return count <= 1; // wait if count > 1
				});
		}

		int xxx = 9;
	}


	std::shared_ptr<ServerCallbacks> GetSharedCallbacks() {
		return callbacks;
	}

private:
	SomeValue someValue;
	std::mutex mxCallbacks;
	std::shared_ptr<int> tokenCallbacks = std::make_shared<int>();
	std::shared_ptr<ServerCallbacks> callbacks;
	std::shared_ptr<std::condition_variable> cvCallbacks = std::make_shared<std::condition_variable>();
};



void main() {
	std::thread thInvoker;
	std::thread thDestroyer;
	std::unique_ptr<Server> server = std::make_unique<Server>();

	thInvoker = std::thread([&server, shCallbacks = server->GetSharedCallbacks()]{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		shCallbacks->connect(11);
		});

	thDestroyer = std::thread([&server] {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		server.reset();
		});

	while (true) {
		Sleep(10);
	}

	if (thInvoker.joinable())
		thInvoker.join();

	if (thDestroyer.joinable())
		thDestroyer.join();


	return;
}