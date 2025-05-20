#include "CurlHttpClient.h"
#include <stdexcept>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::optional<HttpResponse> CurlHttpClient::get(std::string_view url, const std::map<std::string, std::string>& headers) {
    CURL* curl = curl_easy_init();
    if (!curl) return std::nullopt;

    std::string readBuffer;
    struct curl_slist* chunk = nullptr;

    for (const auto& [key, value] : headers) {
        std::string h = key + ": " + value;
        chunk = curl_slist_append(chunk, h.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.data());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    long statusCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);

    curl_easy_cleanup(curl);
    if (chunk) curl_slist_free_all(chunk);

    if (res != CURLE_OK) return std::nullopt;
    return HttpResponse{ static_cast<int>(statusCode), readBuffer, {} };
}

std::optional<HttpResponse> CurlHttpClient::post(std::string_view url, std::string_view body, const std::map<std::string, std::string>& headers) {
    CURL* curl = curl_easy_init();
    if (!curl) return std::nullopt;

    std::string readBuffer;
    struct curl_slist* chunk = nullptr;

    for (const auto& [key, value] : headers) {
        std::string h = key + ": " + value;
        chunk = curl_slist_append(chunk,h.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.data());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.data());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    long statusCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);

    curl_easy_cleanup(curl);
    if (chunk) curl_slist_free_all(chunk);

    if (res != CURLE_OK) return std::nullopt;
    return HttpResponse{ static_cast<int>(statusCode), readBuffer, {} };
}