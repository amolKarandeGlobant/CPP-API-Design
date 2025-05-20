#include "LoggerPlugin.h"

void LoggerPlugin::onRequest(const std::string& url) {
    std::lock_guard<std::mutex> lock(logMutex_);
    std::cout << "[LoggerPlugin] Requesting: " << url << std::endl;
}

void LoggerPlugin::onResponse(const std::string& url, const HttpResponse& response) {
    std::lock_guard<std::mutex> lock(logMutex_);
    std::cout << "[LoggerPlugin] Response from " << url << ": " << response.statusCode << std::endl;
}