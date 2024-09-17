#include "RequestManager.h"
#include <Helpers/Logger.h>


RequestManager::~RequestManager() {
    LOG_FUNCTION_ENTER("~RequestManager");
    this->Cancel();
}

//void RequestManager::SetRequestHandler(std::function<void(QNetworkRequest&, Http::AuthorizationScheme, const QString&, Http::Method)> handler) {
//    this->requestHandler = handler;
//}


void RequestManager::Get(RequestInfo requestInfo, std::function<void()> responseHandler) {
    //Http::RequestType requestType = Http::RequestType::Mixed;

    //QNetworkRequest request = this->ConstructRequest(requestType, requestInfo, Http::Method::GET);
    //if (LOG_ASSERT(!request.url().isEmpty())) {
    //    return;
    //}
    //auto requestItem = std::make_unique<RequestItem>(requestType, Http::Method::GET, request, QByteArray{}, responseHandler);

    //if (requestLocker.is_locked()) {
    //    std::unique_lock lk{ mx };
    //    pendingRequests.push(std::move(requestItem));
    //}
    //else {
    //    this->StartRequest(std::move(requestItem));
    //}
}

//void RequestManager::Post(RequestInfo requestInfo, const QByteArray& body, std::function<void()> responseHandler) {
//    Http::RequestType requestType = Http::RequestType::Mixed;
//
//    QNetworkRequest request = this->ConstructRequest(requestType, requestInfo, Http::Method::POST);
//    if (LOG_ASSERT(!request.url().isEmpty())) {
//        return;
//    }
//    auto requestItem = std::make_unique<RequestItem>(requestType, Http::Method::POST, request, body, responseHandler);
//
//    if (requestLocker.is_locked()) {
//        std::unique_lock lk{ mx };
//        pendingRequests.push(std::move(requestItem));
//    }
//    else {
//        this->StartRequest(std::move(requestItem));
//    }
//}


// [not used]
//void RequestManager::Add(Http::Method method, RequestInfo requestInfo, const QByteArray& body, std::function<void()> responseHandler) {
//    QNetworkRequest request = ConstructRequest(requestInfo, method);
//    if (request.url().isEmpty())
//        return;
//
//    std::unique_lock lk{ mx };
//    pendingRequests.push([this, method, request, body, responseHandler] {
//        StartRequest(method, request, body, responseHandler);
//        });
//}

//void RequestManager::ExecuteAllPendingRequests() {
//    if (auto request = GetNextPendingRequest()) {
//        request();
//    }
//}



const std::vector<char>& RequestManager::GetResponse() {
    //if (responseData.isEmpty()) {
    //    LOG_ERROR_D("Response data is empty !!!");
    //}
    return responseData;
}


void RequestManager::Cancel() {
    LOG_DEBUG_D("Cancel()");
    //if (canceled) {
    //    LOG_DEBUG_D("already canceled");
    //    return;
    //}

    //std::unique_lock lk{ mx };
    //canceled = true;
    //if (reply) {
    //    reply->abort(); // "finished" slot called immediately
    //    reply = nullptr;
    //}
    //pendingRequests = {}; // clear queue
    //requestLocker.unlock();
    //completedCallbacks.clear();

    ////emit publicSignals->Canceled(); // [not used]
}


//QNetworkRequest RequestManager::ConstructRequest(Http::RequestType requestType, RequestInfo requestInfo, Http::Method method) {
//    std::unique_lock lk{ mx };
//
//    QNetworkRequest request;
//    if (std::holds_alternative<QUrl>(requestInfo.variantData)) {
//        request = QNetworkRequest(std::get<QUrl>(requestInfo.variantData));
//        request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
//        if (requestHandler) {
//            requestHandler(request, requestInfo.authScheme, requestInfo.authToken, method);
//        }
//    }
//    else {
//        request = std::get<QNetworkRequest>(requestInfo.variantData);
//    }
//
//    switch (requestType) {
//    case Http::RequestType::Mixed:
//        LOG_DEBUG_D("request \"{}\"", request.url().url());
//        break;
//    default:
//        LOG_DEBUG_D("request \"{}\" [{}]", request.url().url(), MagicEnum::ToString(requestType));
//    }
//
//    assert(request.url().toString().size() != 0);
//
//    if (ignoreAllRequests) {
//        LOG_WARNING_D("Request ignored!");
//        return QNetworkRequest{};
//    }
//
//    return request;
//}
//
//std::unique_ptr<RequestManager::RequestItem> RequestManager::GetNextPendingRequest() {
//    std::unique_lock lk{ mx };
//    if (pendingRequests.empty())
//        return nullptr;
//
//    auto requestItem = std::move(pendingRequests.front());
//    pendingRequests.pop();
//    return requestItem;
//}
//
//
//void RequestManager::StartRequest(std::unique_ptr<RequestManager::RequestItem> requestItem) {
//    if (ignoreAllRequests) {
//        this->Cancel();
//    }
//
//    requestLocker.lock();
//    canceled = false;
//
//    switch (requestItem->method) {
//    case Http::Method::GET:
//        this->reply = networkManager.get(requestItem->request);
//        break;
//    case Http::Method::POST:
//        this->reply = networkManager.post(requestItem->request, requestItem->body);
//        LOG_DEBUG_D("request body \"{}\":\n"
//            "{}"
//            , requestItem->request.url().url()
//            , requestItem->body.constData()
//        );
//        break;
//    };
//
//    responseData.clear();
//    connect(reply, &QIODevice::readyRead, this, &RequestManager::ReadyRead);
//    connect(reply, &QNetworkReply::finished, this, [this, currentRequestItem = std::move(requestItem)] {
//        if (canceled) {
//            LOG_WARNING_D("reply was canceled");
//            return;
//        }
//
//        bool requestSucceded = this->StatusCodeValidation();
//        if (!requestSucceded) {
//            if (currentRequestItem->requestType == Http::RequestType::Mixed) {
//                LOG_ERROR_D("Mixed request failed, ignore next mixed requests and completed callbacks!");
//                ignoreMixedRequests = true;
//            }
//        }
//
//        try {
//            if (requestSucceded) {
//                currentRequestItem->responseHandler();
//            }
//
//            while (auto requestItem = this->GetNextPendingRequest()) {
//                if (ignoreMixedRequests && requestItem->requestType == Http::RequestType::Mixed) {
//                    LOG_WARNING_D("Request ignored because ignoreMixedRequests = true");
//                    continue; // handle only "Independent" requests
//                }
//                requestLocker.unlock();
//                this->StartRequest(std::move(requestItem)); // recursive call
//                return;
//            }
//
//
//            if (ignoreMixedRequests) {
//                // Ignore complicated callbacks for other type requests to not complicate current handling requests logic and all available cases
//                LOG_WARNING_D("Completed callbacks ignored because ignoreMixedRequests = true");
//                requestLocker.unlock();
//            }
//            else {
//                int completedCallbacksSizeBefore = completedCallbacks.size();
//
//                // NOTE: If completedCallback have nested requests they will be added to pendingRequests queue becauese requestLocker is locked
//                for (auto& completedCallback : completedCallbacks) {
//                    if (completedCallback) {
//                        // CHECK: You need to ensure that have no errors or multi thread side effects 
//                        //        when start new request in completedCallback.            
//                        completedCallback();
//                        completedCallback = nullptr; // explicitly set nullptr to not handle it at next time if completedCallbacks was increased inside
//                    }
//                }
//
//                requestLocker.unlock();
//
//                // Execute new request if it was added inside "completedCallback" above
//                if (auto nestedRequestItem = this->GetNextPendingRequest()) {
//                    LOG_WARNING_D("Have new request after completedCallbacks was handled");
//                    if (completedCallbacks.size() != completedCallbacksSizeBefore) {
//                        LOG_WARNING_D("completedCallbacks was increased inside some callback handler");
//                    }
//                    this->StartRequest(std::move(nestedRequestItem)); // recursive call
//                    return;
//                }
//            }
//
//            completedCallbacks.clear();
//            ignoreMixedRequests = false;
//
//            LOG_DEBUG_D("All requests completed");
//            emit publicSignals->Completed();
//        }
//        catch (...) {
//            Dbreak;
//            LOG_ERROR_D("Was exception in QNetworkReply::finished_slot(). Cancel all requests!");
//            this->Cancel();
//        }
//        });
//}

//void RequestManager::ReadyRead() {
//    responseData.append(reply->readAll());
//    auto jsonDoc = QJsonDocument::fromJson(responseData);
//
//    LOG_DEBUG_D("response \"{}\":\n"
//        "{}"
//        , reply->request().url().url()
//        , jsonDoc.isNull() ? responseData.constData() : jsonDoc.toJson().constData()
//    );
//}
//
//bool RequestManager::StatusCodeValidation() {
//    auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
//    if (200 <= statusCode && statusCode <= 299) {
//        if (statusCode != 200) {
//            LOG_DEBUG_D("Request status [201-299]: {} [{}]", reply->request().url().url(), Http::ErrorCodeToString(statusCode));
//        }
//        return true;
//    }
//    else {
//        LOG_ERROR_D("Request error: {} [{}]", reply->request().url().url(), Http::ErrorCodeToString(statusCode));
//        emit publicSignals->Error(statusCode, reply->request().url(), responseData); // pass responseData bacause signal is queued
//        return false;
//    }
//}