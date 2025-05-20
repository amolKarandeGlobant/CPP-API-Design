#include "JsonSerializer.h"

std::string JsonSerializer::serialize(const std::any& data) {
    try {
        const auto& obj = std::any_cast<nlohmann::json>(data);
        return obj.dump();
    }
    catch (...) {
        throw std::runtime_error("Invalid JSON input");
    }
}

std::any JsonSerializer::deserialize(const std::any& str) {
    try {
        return nlohmann::json::parse(std::any_cast<std::string>(str));
    }
    catch (...) {
        throw std::runtime_error("Invalid JSON string");
    }
}