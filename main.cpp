#include "ApiService.h"
#include "CurlHttpClient.h"
#include "LoggerPlugin.h"
#include "JsonSerializer.h"
#include "BinarySerializer.h"
#include <memory>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>
#include <future>
#include <thread>
#include <chrono>

class UrlGenerator {
public:
    static std::string getJsonUrl() { return "http://httpbin.org/post"; }
    static std::string getBinaryUrl() { return "http://httpbin.org/post"; }
    static std::vector<std::string> getBatchUrls() {
        return {
            "http://httpbin.org/get?1",
            "http://httpbin.org/get?2",
            "http://httpbin.org/get?3"
        };
    }
};

void processJsonResponse(std::future<std::optional<HttpResponse>> futureResponse) {
    if (auto response = futureResponse.get()) {
        std::cout << "Async JSON Response: "
            << std::any_cast<nlohmann::json>(response->body).dump()
            << std::endl;
    }
    else {
        std::cerr << "Async JSON request failed." << std::endl;
    }
}

int main() {
    auto httpClient = std::make_shared<CurlHttpClient>();
    auto loggerPlugin = std::make_shared<LoggerPlugin>();

    ApiService apiService(httpClient);
    apiService.addPlugin(loggerPlugin);

    // Asynchronous JSON call
    nlohmann::json jsonData = { {"key", "async_json"} };
    std::future<std::optional<HttpResponse>> jsonFuture =
        apiService.postAsync(UrlGenerator::getJsonUrl(), jsonData,
            ApiService::SerializerType::JSON);

    std::thread jsonThread(processJsonResponse, std::move(jsonFuture));
    jsonThread.detach();

    // Synchronous Binary call
    std::vector<int> binaryData = { 4, 5, 6 };
    auto binaryResponse =
        apiService.post(UrlGenerator::getBinaryUrl(), binaryData,
            ApiService::SerializerType::BINARY);
    if (binaryResponse) {
        std::cout << "Sync Binary Response: ";
        for (int val : std::any_cast<std::vector<int>>(binaryResponse->body)) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

    // Batch GET requests
    auto batchFutures = apiService.batchGetAsync(UrlGenerator::getBatchUrls());
    for (size_t i = 0; i < batchFutures.size(); ++i) {
        if (auto response = batchFutures[i].get()) {
            std::cout << "Batch Response " << i + 1 << ": " << response->statusCode << std::endl;
        }
        else {
            std::cerr << "Batch Request " << i + 1 << " failed." << std::endl;
        }
    }

    // Monitor active requests
    while (apiService.getActiveRequests() > 0) {
        std::cout << "Active Requests: " << apiService.getActiveRequests() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "All requests completed." << std::endl;
    std::cin.get();
    return 0;
}