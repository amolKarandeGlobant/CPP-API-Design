#pragma once
#include "ISerializer.h"
#include <nlohmann/json.hpp>
#include <stdexcept>

class JsonSerializer : public ISerializer {
public:
    std::string serialize(const std::any& data) override;
    std::any deserialize(const std::any& str) override;
};