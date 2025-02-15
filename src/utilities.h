//
// Created by andre on 13/02/2025.
//

#pragma once
#include <filesystem>

namespace veng {

bool streq(gsl::czstring a, gsl::czstring b);
std::vector<std::uint8_t> ReadFile(std::filesystem::path file);

} // veng

