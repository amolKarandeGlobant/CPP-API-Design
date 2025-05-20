#include "BinarySerializer.h"
#include <cstring>

std::string BinarySerializer::serialize(const std::any& data) {
    const auto& vec = std::any_cast<std::vector<int>>(data);
    std::ostringstream oss;
    for (int v : vec) oss.write(reinterpret_cast<const char*>(&v), sizeof(int));
    return oss.str();
}

std::any BinarySerializer::deserialize(const std::any& str) {
    std::vector<int> result;
    auto str1 = std::any_cast<std::string>(str);
    for (size_t i = 0; i < str1.size(); i += sizeof(int)) {
        int v;
        std::memcpy(&v, str1.data() + i, sizeof(int));
        result.push_back(v);
    }
    return result;
}