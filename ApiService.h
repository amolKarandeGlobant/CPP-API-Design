#pragma once
#include "IHttpClient.h"
#include "IApiPlugin.h"
#include "ISerializer.h"
#include "ThreadPool.h"
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <any>
#include <future>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <queue>
#include <atomic>
#include "JsonSerializer.h"
#include "BinarySerializer.h"

class ApiService {
public:
    enum class SerializerType {
        JSON,
        BINARY
    };

private:
    std::shared_ptr<IHttpClient> httpClient_;
    std::vector<std::shared_ptr<IApiPlugin>> plugins_;
    std::unique_ptr<ISerializer> jsonSerializer_;
    std::unique_ptr<ISerializer> binarySerializer_;
    std::shared_mutex serializerMutex_;
    ThreadPool threadPool_{ 4 }; // 4 worker threads
    std::atomic<size_t> activeRequests_{ 0 };

    // Producer-consumer queue for HTTP tasks
    struct HttpTask {
        std::function<std::optional<HttpResponse>()> task;
        std::promise<std::optional<HttpResponse>> promise;
    };
    std::queue<HttpTask> requestQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCondition_;
    std::vector<std::thread> requestWorkers_;
    bool stopWorkers_ = false;

    ISerializer* getSerializer(SerializerType type);
    void startWorkers(size_t numWorkers);
    void stopWorkers();

public:
    explicit ApiService(std::shared_ptr<IHttpClient> httpClient);
    ~ApiService();
    ApiService(const ApiService&) = delete;
    ApiService& operator=(const ApiService&) = delete;

    void addPlugin(std::shared_ptr<IApiPlugin> plugin);
    size_t getActiveRequests() const { return activeRequests_.load(); }

    std::optional<HttpResponse> get(const std::string& url);
    std::optional<HttpResponse> post(
        const std::string& url,
        const std::any& data,
        SerializerType serializerType);

    std::future<std::optional<HttpResponse>> getAsync(const std::string& url);
    std::future<std::optional<HttpResponse>> postAsync(
        const std::string& url,
        const std::any& data,
        SerializerType serializerType);

    std::vector<std::future<std::optional<HttpResponse>>> batchGetAsync(const std::vector<std::string>& urls);
};