#pragma once
#include <string>
#include "IHttpClient.h"

class IApiPlugin {
public:
    virtual ~IApiPlugin() = default;
    virtual void onRequest(const std::string& url) = 0;
    virtual void onResponse(const std::string& url, const HttpResponse& response) = 0;
};