#pragma once

#include <string>
#include <any>

class ISerializer {
public:
    virtual ~ISerializer() = default;
    virtual std::string serialize(const std::any& data) = 0;
    virtual std::any deserialize(const std::any& str) = 0;
};