#pragma once

#include <optional>
#include <string_view>
#include <map>
#include <string>
#include <any>

struct HttpResponse {
    int statusCode;
    std::any body;
    std::map<std::string, std::string> headers;
};

class IHttpClient {
public:
    virtual ~IHttpClient() = default;
    virtual std::optional<HttpResponse> get(std::string_view url, const std::map<std::string, std::string>& headers) = 0;
    virtual std::optional<HttpResponse> post(std::string_view url, std::string_view body,
        const std::map<std::string, std::string>& headers) = 0;
};