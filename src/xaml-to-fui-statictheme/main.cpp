// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include <expected>
#include <filesystem>
#include <fstream>
#include <optional>
#include <print>
#include <ranges>
#include <span>
#include <string>
#include <vector>

#include "GetCpp.hpp"
#include "GetHpp.hpp"
#include "GetResources.hpp"
#include "SortResources.hpp"

struct Arguments {
  std::string mComponent;
  std::vector<std::filesystem::path> mInputs;
  std::filesystem::path mCppOutput;
  std::filesystem::path mHppOutput;
  std::string mParent;
};

void ShowUsage(FILE* file, const char* self) {
  std::println(
    file,
    "USAGE: {} [--cpp-output FILE] [--hpp-output FILE] [--parent PARENT] "
    "COMPONENT INPUT [INPUT...]\n"
    "  --parent PARENT: either 'NONE', or the name of a parent component",
    self);
}

enum class ParseArgumentsExitCode : int {
  HelpRequested = EXIT_SUCCESS,
  SyntaxError = EXIT_FAILURE,
};

std::expected<Arguments, ParseArgumentsExitCode> ParseArguments(
  int argc,
  char** argv) {
  using enum ParseArgumentsExitCode;

  const auto args = std::ranges::transform_view(
    std::span {argv, static_cast<size_t>(argc)},
    [](const auto arg) { return std::string_view {arg}; });

  for (auto&& arg: args) {
    if (arg == "--help") {
      return std::unexpected {HelpRequested};
    }
  }

  Arguments ret {};

  for (std::size_t i = 1; i < args.size(); ++i) {
    const auto arg = args[i];

    if (arg == "--parent") {
      if (++i >= args.size()) {
        std::println(stderr, "--parent requires an argument");
        return std::unexpected {SyntaxError};
      }
      std::string value {args[i]};
      if (value != "NONE") {
        ret.mParent = std::move(value);
      }
      continue;
    }

    if (arg == "--cpp-output") {
      if (++i >= args.size()) {
        std::println(stderr, "--cpp-output requires an argument");
        return std::unexpected {SyntaxError};
      }
      const auto value = args[i];
      if (value.empty()) {
        std::println(stderr, "--cpp-output value can not be empty");
        return std::unexpected {SyntaxError};
      }
      ret.mCppOutput = value;
      continue;
    }

    if (arg == "--hpp-output") {
      if (++i >= args.size()) {
        std::println(stderr, "--hpp-output requires an argument");
        return std::unexpected {SyntaxError};
      }
      const auto value = args[i];
      if (value.empty()) {
        std::println(stderr, "--hpp-output value can not be empty");
        return std::unexpected {SyntaxError};
      }
      ret.mHppOutput = value;
      continue;
    }

    if (arg == "--") {
      for (++i; i < args.size(); ++i) {
        ret.mInputs.push_back(args[i]);
      }
      break;
    }

    if (arg.starts_with('-')) {
      std::println(stderr, "Unknown argument: {}", arg);
      return std::unexpected {SyntaxError};
    }

    if (ret.mComponent.empty()) {
      ret.mComponent = std::string {arg};
      continue;
    }

    ret.mInputs.push_back(arg);
  }

  if (ret.mComponent.empty()) {
    std::println(stderr, "COMPONENT must be specified");
    return std::unexpected {SyntaxError};
  }
  if (ret.mInputs.empty()) {
    std::println(stderr, "INPUTS must be specified");
    return std::unexpected {SyntaxError};
  }
  return ret;
}

void WriteOutput(const std::filesystem::path& path, std::string_view content) {
  if (path == "-") {
    std::println("{}", content);
    return;
  }
  std::ofstream file {path};
  file << content;
  file.close();
}

int main(int argc, char** argv) {
  const auto arguments = ParseArguments(argc, argv);
  if (!arguments.has_value()) {
    const auto result = arguments.error();
    ShowUsage(
      result == ParseArgumentsExitCode::HelpRequested ? stdout : stderr,
      argv[0]);
    return std::to_underlying(arguments.error());
  }

  std::vector<Resource> resources;
  try {
    for (auto&& input: arguments->mInputs) {
      GetResources(std::back_inserter(resources), input);
    }
  } catch (const std::exception& e) {
    std::println(stderr, "ERROR: {}", e.what());
    return EXIT_FAILURE;
  }

  if (arguments->mCppOutput.empty() && arguments->mHppOutput.empty()) {
    std::println(
      stderr, "WARNING: Neither --cpp-output nor --hpp-output was specified");
    return EXIT_SUCCESS;
  }

  SortResources(resources);

  const auto header = std::format(
    "// @{} by {}\n\n",
    "generated" /* avoid including the combined token in the generator */,
    std::filesystem::path(argv[0]).filename().string());

  const Metadata metadata {
    .mComponent = arguments->mComponent,
    .mParent = arguments->mParent,
    .mNamespace
    = std::format("FredEmmott::GUI::StaticTheme::{}", arguments->mComponent),
    .mDetailNamespace
    = std::format("detail_StaticTheme_{}", arguments->mComponent),
  };

  if (const auto file = arguments->mHppOutput; !file.empty()) {
    const auto content = GetHpp(metadata, resources);
    WriteOutput(file, std::format("{}\n{}", header, content));
    std::println(stderr, "Generated {}", file.string());
  }

  if (const auto file = arguments->mCppOutput; !file.empty()) {
    const auto content = GetCpp(metadata, resources);
    WriteOutput(file, std::format("{}\n{}", header, content));
    std::println(stderr, "Generated {}", file.string());
  }

  return EXIT_SUCCESS;
}