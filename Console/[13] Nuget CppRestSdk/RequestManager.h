#pragma once
#include "Common.h"
// https://github.com/microsoft/cpprestsdk
// https://github.com/Microsoft/cpprestsdk/wiki/Getting-Started-Tutorial
#include <cpprest/producerconsumerstream.h>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

#include <Helpers/MultithreadMutex.h>
#include <Helpers/Container.hpp>
#include <Helpers/Logger.h>
#include "HttpModel.h"
#include <list>
#include <map>

namespace requests {
	//enum Type {
	//	Independent,
	//};

	enum Request {
		None,
		RequestA,
		RequestB,
		RequestC,
		RequestD,
		RequestE,
	};

	enum class Status {
		NotStarted,
		Pending,
		Completed
	};
}



template <requests::Request... RequestsTypes>
struct Parents {
	static constexpr const requests::Request listParentRequests[]{ requests::Request::None, RequestsTypes... };
};

template <requests::Request... RequestsTypes>
struct Childs {
	static constexpr const requests::Request listChildRequests[]{ requests::Request::None, RequestsTypes... };
};



struct IRequest {
	virtual ~IRequest() = default;
};

//struct Global {
	inline std::map<requests::Request, IRequest*> g_RequestsMap;
//};



struct RequestBase : IRequest {
protected:
	RequestBase(requests::Request requestId) {
		g_RequestsMap[requestId] = this;
	}

	Concurrency::task<web::http::http_response> StartRequest() {
		if (this->CanStartRequest()) {
			return this->ConstructRequest();
		}
	}

protected:
	virtual bool CanStartRequest() { return false; };
	virtual Concurrency::task<web::http::http_response> ConstructRequest() { return {}; };

protected:
	requests::Status status = requests::Status::NotStarted;
};


template <requests::Request RequestT>
struct Request : RequestBase, /* [opt] */ Parents<>, /* [opt] */ Childs<> {
	static_assert(HELPERS_NS::dependent_false<RequestT>::value, "You must use specialization");
};


template<>
struct Request<requests::RequestA> : RequestBase {
	static constexpr const requests::Request requestId = requests::RequestA;
	
	Request()
		: RequestBase(requestId)
	{}

	bool CanStartRequest() override { 
		return true;
	}
	Concurrency::task<web::http::http_response> ConstructRequest() override {
		return {};
	}
};

template<>
struct Request<requests::RequestB> : RequestBase {
	static constexpr const requests::Request requestId = requests::RequestA;

	Request()
		: RequestBase(requestId)
	{}

	bool CanStartRequest() override {
		return true;
	}
	Concurrency::task<web::http::http_response> ConstructRequest() override {
		return {};
	}
};

template<>
struct Request<requests::RequestE> : RequestBase, Parents<requests::RequestA, requests::RequestB>  {
	static constexpr const requests::Request requestId = requests::RequestA;

	Request()
		: RequestBase(requestId)
	{}

	bool CanStartRequest() override {
		return true;
	}
	Concurrency::task<web::http::http_response> ConstructRequest() override {
		return {};
	}
};



class RequestManager {
public:
	RequestManager() = default;
	~RequestManager();

	struct RequestInfo {
		//std::variant<QUrl, QNetworkRequest> variantData;
		std::string url;
		Http::AuthorizationScheme authScheme = Http::AuthorizationScheme::Bearer;
		std::string authToken;
	};

public:
	//void SetRequestHandler(std::function<void(QNetworkRequest&, Http::AuthorizationScheme, const QString&, Http::Method)> handler);

	void Get(RequestInfo requestInfo, std::function<void()> responseHandler);
	//void Post(RequestInfo requestInfo, const QByteArray& body, std::function<void()> responseHandler);

	const std::vector<char>& GetResponse();

	void Cancel();

private:
	//struct RequestItem {
	//	RequestItem(Http::RequestType requestType, Http::Method method,
	//		QNetworkRequest request, const QByteArray& body, std::function<void()> responseHandler)
	//		: requestType{ requestType }
	//		, method{ method }
	//		, request{ request }
	//		, body{ body }
	//		, responseHandler{ responseHandler }
	//	{}

	//	const Http::RequestType requestType;
	//	const Http::Method method;
	//	const QNetworkRequest request;
	//	const QByteArray body; // not ref, body need copy
	//	const std::function<void()> responseHandler;
	//};

	//QNetworkRequest ConstructRequest(Http::RequestType requestType, RequestInfo requestInfo, Http::Method mehtod);
	//std::unique_ptr<RequestItem> GetNextPendingRequest();

	//void StartRequest(std::unique_ptr<RequestItem> requestItem);

private:
	//void ReadyRead();
	//bool StatusCodeValidation();

private:
	std::mutex mx;
	std::vector<char> responseData;
	//QNetworkReply* reply = nullptr; // pointer life is managed by QNetworkAccessManager
	//QNetworkAccessManager networkManager;

	H::MultithreadMutex requestLocker;
	std::atomic<bool> canceled = false;
	std::atomic<bool> ignoreMixedRequests = false;
	//std::function<void(QNetworkRequest&, Http::AuthorizationScheme, const QString&, Http::Method)> requestHandler;
	//H::iterable_queue<std::unique_ptr<RequestItem>> pendingRequests;

	// TODO: add guard for Dtor call when not all requests handled
};