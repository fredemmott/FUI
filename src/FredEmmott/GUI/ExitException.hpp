// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <format>
#include <stdexcept>

namespace FredEmmott::GUI {

// If using Win32::WinMain(), this can be used to exit the application
class ExitException final : public std::runtime_error {
 public:
  ExitException() = delete;
  explicit constexpr ExitException(int exitCode)
    : std::runtime_error(std::format("Exit with code {}", exitCode)),
      mExitCode(exitCode) {}

  constexpr int GetExitCode() const {
    return mExitCode;
  }

 private:
  int mExitCode {};
};

}// namespace FredEmmott::GUI