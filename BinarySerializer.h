#pragma once
#include "ISerializer.h"
#include <sstream>
#include <vector>
#include <cstring>

class BinarySerializer : public ISerializer {
public:
    std::string serialize(const std::any& data) override;
    std::any deserialize(const std::any& str) override;
};