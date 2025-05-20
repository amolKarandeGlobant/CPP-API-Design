#include "ApiService.h"
#include <stdexcept>
#include <future>

ApiService::ApiService(std::shared_ptr<IHttpClient> httpClient)
    : httpClient_(std::move(httpClient)),
    jsonSerializer_(std::make_unique<JsonSerializer>()),
    binarySerializer_(std::make_unique<BinarySerializer>()) {
    startWorkers(4); // Start 4 worker threads
}

ApiService::~ApiService() {
    stopWorkers();
}

void ApiService::startWorkers(size_t numWorkers) {
    for (size_t i = 0; i < numWorkers; ++i) {
        requestWorkers_.emplace_back([this] {
            while (true) {
                HttpTask task;
                {
                    std::unique_lock<std::mutex> lock(queueMutex_);
                    queueCondition_.wait(lock, [this] { return !requestQueue_.empty() || stopWorkers_; });
                    if (stopWorkers_ && requestQueue_.empty()) return;
                    task = std::move(requestQueue_.front());
                    requestQueue_.pop();
                }
                try {
                    auto result = task.task();
                    task.promise.set_value(result);
                }
                catch (...) {
                    task.promise.set_exception(std::current_exception());
                }
            }
            });
    }
}

void ApiService::stopWorkers() {
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        stopWorkers_ = true;
    }
    queueCondition_.notify_all();
    for (auto& worker : requestWorkers_) {
        worker.join();
    }
}

void ApiService::addPlugin(std::shared_ptr<IApiPlugin> plugin) {
    std::unique_lock<std::shared_mutex> lock(serializerMutex_);
    plugins_.push_back(std::move(plugin));
}

ISerializer* ApiService::getSerializer(SerializerType type) {
    std::shared_lock<std::shared_mutex> lock(serializerMutex_);
    switch (type) {
    case SerializerType::JSON:
        return jsonSerializer_.get();
    case SerializerType::BINARY:
        return binarySerializer_.get();
    default:
        throw std::runtime_error("Invalid SerializerType");
    }
}

std::optional<HttpResponse> ApiService::get(const std::string& url) {
    ++activeRequests_;
    std::vector<std::future<void>> pluginFutures;
    {
        std::shared_lock<std::shared_mutex> lock(serializerMutex_);
        for (const auto& plugin : plugins_) {
            pluginFutures.push_back(std::async(std::launch::async, [&plugin, &url] {
                plugin->onRequest(url);
                }));
        }
    }
    for (auto& future : pluginFutures) {
        future.wait();
    }

    auto response = httpClient_->get(url, {});

    if (response) {
        pluginFutures.clear();
        {
            std::shared_lock<std::shared_mutex> lock(serializerMutex_);
            for (const auto& plugin : plugins_) {
                pluginFutures.push_back(std::async(std::launch::async, [&plugin, &url, &response] {
                    plugin->onResponse(url, *response);
                    }));
            }
        }
        for (auto& future : pluginFutures) {
            future.wait();
        }
    }

    --activeRequests_;
    return response;
}

std::optional<HttpResponse> ApiService::post(
    const std::string& url,
    const std::any& data,
    SerializerType serializerType) {
    ++activeRequests_;
    ISerializer* serializer = getSerializer(serializerType);
    std::string serializedData = serializer->serialize(data);
    std::map<std::string, std::string> headers;

    if (serializerType == SerializerType::JSON) {
        headers["Content-Type"] = "application/json";
    }
    else if (serializerType == SerializerType::BINARY) {
        headers["Content-Type"] = "application/octet-stream";
    }

    std::vector<std::future<void>> pluginFutures;
    {
        std::shared_lock<std::shared_mutex> lock(serializerMutex_);
        for (auto& plugin : plugins_) {
            pluginFutures.push_back(std::async(std::launch::async, [&plugin, &url] {
                plugin->onRequest(url);
                }));
        }
    }
    for (auto& future : pluginFutures) {
        future.wait();
    }

    auto response = httpClient_->post(url, serializedData, headers);

    if (response) {
        response->body = serializer->deserialize(response->body);
        pluginFutures.clear();
        {
            std::shared_lock<std::shared_mutex> lock(serializerMutex_);
            for (auto& plugin : plugins_) {
                pluginFutures.push_back(std::async(std::launch::async, [&plugin, &url, &response] {
                    plugin->onResponse(url, *response);
                    }));
            }
        }
        for (auto& future : pluginFutures) {
            future.wait();
        }
    }

    --activeRequests_;
    return response;
}

std::future<std::optional<HttpResponse>> ApiService::getAsync(const std::string& url) {
    HttpTask task;
    task.task = [this, url]() { return this->get(url); };
    std::future<std::optional<HttpResponse>> future = task.promise.get_future();
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        requestQueue_.push(std::move(task));
    }
    queueCondition_.notify_one();
    return future;
}

std::future<std::optional<HttpResponse>> ApiService::postAsync(
    const std::string& url,
    const std::any& data,
    SerializerType serializerType) {
    HttpTask task;
    task.task = [this, url, data, serializerType]() { return this->post(url, data, serializerType); };
    std::future<std::optional<HttpResponse>> future = task.promise.get_future();
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        requestQueue_.push(std::move(task));
    }
    queueCondition_.notify_one();
    return future;
}

std::vector<std::future<std::optional<HttpResponse>>> ApiService::batchGetAsync(const std::vector<std::string>& urls) {
    std::vector<std::future<std::optional<HttpResponse>>> futures;
    for (const auto& url : urls) {
        futures.push_back(threadPool_.enqueue([this, url]() { return this->get(url); }));
    }
    return futures;
}