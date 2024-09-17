#pragma once
#include "Common.h"
#include <Helpers/MultithreadMutex.h>
#include <Helpers/Container.hpp>
#include <Helpers/Logger.h>
#include "HttpModel.h"
#include <list>


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