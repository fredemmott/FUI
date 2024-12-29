// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <span>
#include <string>

#include "Metadata.hpp"
#include "Resource.hpp"

std::string GetCpp(const Metadata&, std::span<Resource> resources);