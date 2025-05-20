#pragma once
#include "IApiPlugin.h"
#include "IHttpClient.h"
#include <iostream>
#include <mutex>

class LoggerPlugin : public IApiPlugin {
private:
    std::mutex logMutex_;
public:
    void onRequest(const std::string& url) override;
    void onResponse(const std::string& url, const HttpResponse& response) override;
};