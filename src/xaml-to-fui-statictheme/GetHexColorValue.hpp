// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <string>

// Convert string like '#123456' or '#12345678' to 'SkColorSetA?RGB(..)'
std::string GetHexColorValue(std::string_view);