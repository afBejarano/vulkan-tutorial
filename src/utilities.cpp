//
// Created by andre on 13/02/2025.
//

#include "utilities.h"

#include <cstring>
#include <fstream>

#include "spdlog/spdlog.h"

namespace veng {
bool streq(const gsl::czstring a, const gsl::czstring b) {
    return std::strcmp(a, b) == 0;
}

std::vector<std::uint8_t> ReadFile(std::filesystem::path file) {
    if (!exists(file)) { return {}; }
    if (!is_regular_file(file)) { return {}; }
    std::ifstream fileData(file, std::ios::binary);
    if (!fileData.is_open()) { return {}; }

    std::uint32_t size = file_size(file);
    std::vector<std::uint8_t> data(size);
    fileData.read(reinterpret_cast<char*>(data.data()), size);
    return data;
}
} // veng
