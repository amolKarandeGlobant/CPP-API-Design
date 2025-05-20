#pragma once
#include "IHttpClient.h"
#include <thread>

class MockHttpClient : public IHttpClient {
public:
    std::optional<HttpResponse> get(std::string_view url, const std::map<std::string, std::string>& headers) override {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return HttpResponse{ 200, "{\"mock\": true}", {} };
    }

    std::optional<HttpResponse> post(std::string_view url, std::string_view body,
        const std::map<std::string, std::string>& headers) override {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return HttpResponse{ 201, "{\"mock\": true, \"body\": " + std::string(body) + "}", {} };
    }
};