// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "GetLinearGradientBrush.hpp"

#include <print>
#include <ranges>

#include "ResolveColorReference.hpp"

void GetLinearGradientBrush(
  std::back_insert_iterator<std::vector<Resource>> back,
  const TiXmlElement& it) {
  std::unordered_set<std::string> dependencies;
  std::vector<std::string> stops;

  const auto stopsXML
    = it.FirstChildElement("LinearGradientBrush.GradientStops");
  for (auto stop = stopsXML->FirstChildElement("GradientStop"); stop;
       stop = stop->NextSiblingElement("GradientStop")) {
    const auto value = ResolveColorReference(stop->Attribute("Color"));
    dependencies.emplace(value);

    stops.push_back(
      std::format("{{ {}, {} }}", stop->Attribute("Offset"), value));
  }

  std::vector<std::string> args {
    std::format(
      "LinearGradientBrush::MappingMode::{}", it.Attribute("MappingMode")),
    std::format("/* start = */ SkPoint {{ {} }}", it.Attribute("StartPoint")),
    std::format("/* end = */ SkPoint {{ {} }}", it.Attribute("EndPoint")),
    std::format(
      "/* stops = */ {{ {} }}",
      std::ranges::to<std::string>(std::views::join_with(stops, ','))),
  };

  const auto transform
    = it.FirstChildElement("LinearGradientBrush.RelativeTransform");
  if (transform) {
    if (const auto s = transform->FirstChildElement("ScaleTransform")) {
      const auto centerX = s->Attribute("CenterX");
      const auto centerY = s->Attribute("CenterY");
      const auto scaleX = s->Attribute("ScaleX");
      const auto scaleY = s->Attribute("ScaleY");
      args.push_back(
        std::format(
          "/* scale = */ {{ "
          ".mOrigin = {{ {}, {} }},"
          ".mScaleX = {},"
          ".mScaleY = {}"
          "}}",
          centerX ? centerX : "0",
          centerY ? centerY : "0",
          scaleX ? scaleX : "1",
          scaleY ? scaleY : "1"));
    }
  }

  back = {
    .mName = it.Attribute("x:Key"),
    .mValue = std::format(
      "StaticThemedLinearGradientBrush {{ {} }}",
      std::ranges::to<std::string>(std::views::join_with(args, ','))),
    .mType = "Brush",
    .mDependencies = dependencies,
  };
}
