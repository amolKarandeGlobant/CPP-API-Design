#pragma once
#include "IHttpClient.h"
#include <curl/curl.h>
#include <sstream>

class CurlHttpClient : public IHttpClient {
public:
    std::optional<HttpResponse> get(std::string_view url, const std::map<std::string, std::string>& headers) override;
    std::optional<HttpResponse> post(std::string_view url, std::string_view body,
        const std::map<std::string, std::string>& headers) override;
};