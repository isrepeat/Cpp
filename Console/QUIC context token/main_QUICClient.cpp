#include "MsQuic.h"
#include <iostream>
#include <memory>


struct SomeValue {
	SomeValue() = default;
	~SomeValue() {
		int aaa = 9;
	}

	int data = 1234;
};


class QUICClient {
public:
	QUICClient(std::string name) {
		printf("QUICClient()  \n");
		ownerName = name;
		//tokenThis = std::shared_ptr<MyQUIC>(this);
	};

	~QUICClient() {
		printf("~QUICClient()  \n");
		std::unique_lock<std::mutex> lk{ mxHandleEvent };
		cv.wait(lk);
		MsQuicWrapper::GetInstance().GetApiTable()->ListenerStop();
		MsQuicWrapper::GetInstance().GetApiTable()->ListenerClose();
	};


	void StartServer(bool startSendEventsSimmulation = true) {
		auto context = MsQuicWrapper::GetInstance().AddContextWithToken(this, token);
		MsQuicWrapper::GetInstance().GetApiTable()->ListenerOpen(&QUICClient::MyQUICListener, context, &hListener);
		if (startSendEventsSimmulation) {
			MsQuicWrapper::GetInstance().GetApiTable()->ListenerStart();
		}
	}

	static HRESULT MyQUICListener(void* context, MsQuic::Event event) {
		auto _context = static_cast<MsQuicWrapper::Context*>(context);
		if (_context->token.expired()) {
			if (event == MsQuic::Event::Shutdown) {
				_context->shutdownCallback();
			}
			return E_FAIL;
		}

		auto _this = static_cast<QUICClient*>(_context->context);
		if (IsBadReadPtr(_this, sizeof(decltype(*_this)))) {
			return E_FAIL;
		}

		switch (event) {
		case MsQuic::Event::NewConnection: {
			std::unique_lock<std::mutex> lk{ _this->mxHandleEvent };
			_this->Log("NewConnection");
			break;
		}
		case MsQuic::Event::UserData_1: {
			std::unique_lock<std::mutex> lk{ _this->mxHandleEvent };
			_this->Log("UserData_1");
			break;
		}
		case MsQuic::Event::UserData_2: {
			Sleep(500);
			std::unique_lock<std::mutex> lk{ _this->mxHandleEvent };
			_this->Log("UserData_2");
		}
			break;
		case MsQuic::Event::Shutdown: // must be handled in normal case when QUICClient of this context not expired
			_this->Log("Shutdown");
			break;

		case MsQuic::Event::Unknown:
			_this->Log("Unknown");
			break;
		}

		return S_OK;
	}

	void Log(std::string eventName) {
		printf("[%s] %s \n", ownerName.c_str(), eventName.c_str());
	}

private:
	HANDLE hListener;
	SomeValue someValue;
	std::mutex mxHandleEvent;
	std::string ownerName;
	std::shared_ptr<int> token = std::make_shared<int>();
	std::condition_variable cv;
	//std::shared_ptr<MyQUIC> tokenThis;
};



int main() {
	//std::list<int> list;
	//list.push_back(111);
	//list.push_back(222);
	//list.push_back(333);
	//list.push_back(444);
	//list.push_back(555);
	//auto it = --list.end();
	//auto it2 = &*it;
	//list.push_back(666);
	//list.push_back(777);
	//list.push_back(888);

	//list.erase(it2);

	{
		QUICClient* quicClient = new QUICClient("client_1");
		quicClient->StartServer();
		Sleep(3'000);
		delete quicClient;
		quicClient = nullptr;
	}
	Sleep(100);

	//QUICClient* quicClient = new QUICClient("client_2");
	//quicClient->StartServer(false);
	
	Sleep(10);

	while (true) {
		Sleep(1);
	}
	return 0;
}
