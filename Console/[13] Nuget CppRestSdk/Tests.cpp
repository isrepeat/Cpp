#include "Tests.h"
// https://github.com/microsoft/cpprestsdk
// https://github.com/Microsoft/cpprestsdk/wiki/Getting-Started-Tutorial
#include <cpprest/producerconsumerstream.h>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

namespace TestStreams {
    void TestWriteRead() {
        concurrency::streams::producer_consumer_buffer<char> resultBuffer;

        concurrency::streams::producer_consumer_buffer<char> internalBuffer;
        auto outStreamInternal = internalBuffer.create_ostream();
        auto task = outStreamInternal.print("Hello, World!")
            .then([&](size_t bytesWritten)
            {
                outStreamInternal.close(); // need close before you can read from buffer.

                auto inStreamInternal = internalBuffer.create_istream();
                auto outStreamResult = resultBuffer.create_ostream();
                return inStreamInternal.read_to_end(outStreamResult.streambuf());
            })
            .then([&](size_t bytesRead)
            {
                std::string resultCopied(bytesRead, '\0');
                resultBuffer.scopy(resultCopied.data(), resultCopied.size());
                LOG_DEBUG_D("resultCopied = {}", resultCopied);
            });

        try {
            task.wait();
        }
        catch (const std::exception& e) {
            LOG_ERROR_D("Error exception: {}", e.what());
        }
    }
}

namespace TestRequests {
    using namespace utility;                // Common utilities like string conversions
    using namespace web;                    // Common features like URIs.
    using namespace web::http;              // Common HTTP functionality
    using namespace web::http::client;      // HTTP client features
    using namespace concurrency::streams;   // Asynchronous streams

    void TestSimpleRequest() {
        concurrency::streams::producer_consumer_buffer<char> requestBody;
        auto requestBodyOutStream = requestBody.create_ostream();

        auto fileStream = std::make_shared<ostream>();

        // Open stream to output file.
        pplx::task<void> requestTask = fstream::open_ostream((g_TestOutputFolder / "results.html"))
            .then([&](ostream outFile)
            {
                *fileStream = outFile;

                // Create http_client to send the request.
                http_client client(U("http://www.bing.com/"));

                // Build request URI and start the request.
                uri_builder builder(U("/search"));
                builder.append_query(U("q"), U("cpprestsdk github"));
                return client.request(methods::GET, builder.to_string());
            })

            // Handle response headers arriving.
            .then([&](http_response response)
            {
                LOG_DEBUG_D("Received response status code: {}", response.status_code());

                // Write response body into the file.
                //return response.body().read_to_end(fileStream->streambuf());

                return response.body().read_to_end(requestBodyOutStream.streambuf());
            })

            // Close the file stream.
            .then([&](size_t bytesRead)
            {
                //return fileStream->close();

                std::string result(bytesRead, '\0');
                requestBody.scopy(result.data(), result.size());
                LOG_DEBUG_D("requestBody:\n{}", result);
                return requestBodyOutStream.close();
            });

        // Wait for all the outstanding I/O to complete and handle any exceptions
        try {
            requestTask.wait();
        }
        catch (const std::exception& e) {
            LOG_ERROR_D("Error exception: {}", e.what());
        }
    }


    void TestRequestsChain() {

        // Create http_client to send the request.
        http_client client(U("http://www.google.com/"));

        auto FnCreateRequest = [&] () -> Concurrency::task<web::http::http_response> {
            // Build request URI and start the request.
            uri_builder builder(U("/search"));
            builder.append_query(U("q"), U("cpprestsdk github"));

            return client.request(methods::GET, builder.to_string());
            };

        auto FnWriteResponseToFile = [&](concurrency::streams::producer_consumer_buffer<uint8_t> requestBody) -> Concurrency::task<void> {
            // NOTE: Lambdas captured by value to keep fileStream shared_ptr.
            auto fileStream = std::make_shared<ostream>();
            auto writeToFileTask = fstream::open_ostream(g_TestOutputFolder / "results.html")
                // Write responseBody to file stream.
                .then([=](ostream outFile)
                {
                    *fileStream = outFile;
                    auto requestBodyInStream = requestBody.create_istream();
                    return requestBodyInStream.read_to_end(fileStream->streambuf());
                })
                // Close the file stream.
                .then([=](size_t bytesRead)
                {
                    return fileStream->close();
                });

            return writeToFileTask;
            };


        try {
            concurrency::streams::producer_consumer_buffer<uint8_t> responseBody;
            auto requestBodyOutStream = responseBody.create_ostream();

            auto requestTask = FnCreateRequest()
                // Read responce body to responseBody buffer.
                .then([&](http_response response)
                {
                    LOG_DEBUG_D("Received response status code: {}", response.status_code());
                    return response.body().read_to_end(requestBodyOutStream.streambuf());
                })
                // Print responseBody.
                .then([&](size_t bytesRead)
                {
                    std::string result(bytesRead, '\0');
                    requestBodyOutStream.streambuf().scopy((uint8_t*)result.data(), result.size());
                    LOG_DEBUG_D("requestBody:\n{}", result);

                    return requestBodyOutStream.close();
                })
                // Optional write responseBody to file.
                .then([&]
                {
                    FnWriteResponseToFile(responseBody).wait();
                })
                .wait();
        }
        catch (const std::exception& e) {
            LOG_ERROR_D("Error exception: {}", e.what());
        }
        return;
    }

    void PseusoRequestsTree() {
        // NOTE: In real code no need '.wait()' because it is just structure of order handlers.
        //       Task executing and waiting will be in another place.
        //
        // Attributes:
        // [Independent]: 
        //      Can be called at any time regardless of another requests chains.
        //
        // [Child dependent requests]:
        //      Possible list of child requests chains that can be called at any time 
        //      but only after the current request chain is completed (once or every time).
        //
        // [Depends on]:
        //      List of parent requests chains that must be completed before current
        //      request chain started.
        //   Item attributes:
        //     [always] - marked parent request chain must be executed every time before current request chain started.
        //     [once] - marked parent request chain must be executed at least once before current request chain started.


        //
        // --- Handlers ---
        //
        auto HandlerA = [] {};

        auto HandlerB = [] {};

        auto HandlerC = []() -> Concurrency::task<web::http::http_response> { return {}; };
        auto HandlerC_1 = []() -> Concurrency::task<web::http::http_response> { return {}; };
        auto HandlerC_2 = []() -> Concurrency::task<web::http::http_response> { return {}; };
        auto HandlerC_finish = [] {};

        auto HandlerD = [] {};

        auto HandlerE = [] {};

        //
        // --- Requests ---
        // 
        // [Independent]
        // [Child dependent requests]: requestChainE.
        Concurrency::task<web::http::http_response> RequestA;
        auto requestChainA = RequestA
            .then([&](http_response response) { HandlerA(); })
            .wait();

        // [Independent]
        // [Child dependent requests]: 'requestChainC', 'requestChainD'.
        Concurrency::task<web::http::http_response> RequestB;
        auto requestChainB = RequestB
            .then([&](http_response response) { HandlerB(); })
            .wait();

        // [Depends on]: [always] 'requestChainB'.
        Concurrency::task<web::http::http_response> RequestC;
        auto requestChainC = RequestC
            .then([&](http_response response) { return HandlerC(); })
            .then([&](http_response response) { return HandlerC_1(); })
            .then([&](http_response response) { return HandlerC_2(); })
            .then([&](http_response response) { HandlerC_finish(); })
            .wait();

        // [Depends on]: [once] 'requestChainB'.
        Concurrency::task<web::http::http_response> RequestD;
        auto requestChainD = RequestD
            .then([&](http_response response) { HandlerD(); })
            .wait();

        // [Depends on]: [once] 'requestChainA', [always] 'requestChainB'.
        Concurrency::task<web::http::http_response> RequestE;
        auto requestChainE = RequestE
            .then([&](http_response response) { HandlerD(); })
            .wait();
    }

    void PseusoRequests() {
        //
        // --- Handlers ---
        //
        auto HandlerA = [] {};

        auto HandlerB = [] {};

        auto HandlerC = []() -> Concurrency::task<web::http::http_response> { return {}; };
        auto HandlerC_1 = []() -> Concurrency::task<web::http::http_response> { return {}; };
        auto HandlerC_2 = []() -> Concurrency::task<web::http::http_response> { return {}; };
        auto HandlerC_finish = [] {};

        auto HandlerD = [] {};

        auto HandlerE = [] {};

        //
        // --- Requests ---
        // 
        Concurrency::task<web::http::http_response> RequestC;
        auto requestChainC = RequestC
            .then([&](http_response response) { return HandlerC(); })
            .then([&](http_response response) { return HandlerC_1(); })
            .then([&](http_response response) { return HandlerC_2(); })
            .then([&](http_response response) { HandlerC_finish(); })
            .wait();

        Concurrency::task<web::http::http_response> RequestD;
        auto requestChainD = RequestD
            .then([&](http_response response) { HandlerD(); })
            .wait();

        Concurrency::task<web::http::http_response> RequestA;
        auto requestChainA = RequestA
            .then([&](http_response response) { HandlerA(); })
            .wait();

        Concurrency::task<web::http::http_response> RequestE;
        auto requestChainE = RequestE
            .then([&](http_response response) { HandlerD(); })
            .wait();

        Concurrency::task<web::http::http_response> RequestB;
        auto requestChainB = RequestB
            .then([&](http_response response) { HandlerB(); })
            .wait();
    }
}