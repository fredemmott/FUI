// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <YGEnums.h>

#include <FredEmmott/utility/drop_last_t.hpp>
#include <FredEmmott/utility/type_tag.hpp>
#include <FredEmmott/utility/unordered_map.hpp>
#include <unordered_set>

#include "Brush.hpp"
#include "Font.hpp"
#include "PseudoClasses.hpp"
#include "StyleClass.hpp"
#include "StyleProperty.hpp"
#include "StylePropertyTypes.hpp"

namespace FredEmmott::GUI::Widgets {
class Widget;
}

namespace FredEmmott::GUI {

using style_detail::StylePropertyKey;

struct Style {
  static const Style& Empty();
  struct PropertyTypes {
    PropertyTypes() = delete;
#define FUI_DECLARE_STYLE_PROPERTY_TYPE(NAME, TYPE, SCOPE) \
  using NAME##_t = TYPE;
    FUI_ENUM_STYLE_PROPERTIES(FUI_DECLARE_STYLE_PROPERTY_TYPE)
#undef FUI_DECLARE_STYLE_PROPERTY_TYPE
  };
  utility::unordered_map<
    StylePropertyKey,
    utility::drop_last_t<
      std::variant,
#define FUI_DECLARE_STYLE_PROPERTY_STORAGE(TYPE, NAME) StyleProperty<TYPE>,
      FUI_ENUM_STYLE_PROPERTY_TYPES(FUI_DECLARE_STYLE_PROPERTY_STORAGE)
#undef FUI_DECLARE_STYLE_PROPERTY_STORAGE
        void>>
    mStorage;

  template <class T>
  constexpr const auto& GetProperty(const StylePropertyKey P) const noexcept {
    static constexpr StyleProperty<T> Empty {};
    return mStorage.contains(P)
      ? std::get<StyleProperty<T>>(mStorage[P])
      : Empty;
  }

  template <class T>
  constexpr auto& GetProperty(const StylePropertyKey P) noexcept {
    if (!mStorage.contains(P)) {
      mStorage.emplace(P, StyleProperty<T> {});
    }
    return std::get<StyleProperty<T>>(mStorage[P]);
  }

#define FUI_DECLARE_PROPERTY_GETTER(NAME, TYPE, ...) \
  [[nodiscard]] constexpr decltype(auto) NAME(this auto&& self) noexcept { \
    constexpr auto key = StylePropertyKey::NAME; \
    return self.template GetProperty<TYPE>(key); \
  } \
  [[nodiscard]] constexpr bool Has##NAME() const noexcept { \
    constexpr auto key = StylePropertyKey::NAME; \
    return mStorage.contains(key); \
  }
#define FUI_DECLARE_PROPERTY_SETTER(NAME, TYPE, PARAM_TYPE) \
  template <class... Args> \
  [[nodiscard]] \
  decltype(auto) NAME( \
    this auto&& self, PARAM_TYPE value, Args&&... args) noexcept \
    requires std::is_rvalue_reference_v<decltype(self)> \
  { \
    constexpr auto key = StylePropertyKey::NAME; \
    self.mStorage.insert_or_assign( \
      key, StyleProperty<TYPE>(value, std::forward<Args>(args)...)); \
    return std::move(self); \
  }
#define FUI_DECLARE_PROPERTY_SETTERS(NAME, TYPE, SCOPE) \
  FUI_DECLARE_PROPERTY_SETTER(NAME, TYPE, const TYPE&); \
  FUI_DECLARE_PROPERTY_SETTER(NAME, TYPE, const StaticTheme::Resource<TYPE>&); \
  FUI_DECLARE_PROPERTY_SETTER(NAME, TYPE, std::nullopt_t) \
  decltype(auto) Unset##NAME() noexcept { \
    mStorage.erase(StylePropertyKey::NAME); \
  }
  FUI_ENUM_STYLE_PROPERTIES(FUI_DECLARE_PROPERTY_GETTER)
  FUI_ENUM_STYLE_PROPERTIES(FUI_DECLARE_PROPERTY_SETTERS)
#undef FUI_DECLARE_PROPERTY_GETTER
#undef FUI_DECLARE_PROPERTY_SETTER
#undef FUI_DECLARE_PROPERTY_SETTERS

  using Selector = std::variant<
    std::monostate,
    StyleClass,
    NegatedStyleClass,
    const Widgets::Widget*>;
  std::list<std::tuple<Selector, Style>> mAnd;
  utility::unordered_map<Selector, Style> mDescendants;

  constexpr Style() = default;
  constexpr Style(const Style& other) noexcept = default;
  constexpr Style(Style&& other) noexcept = default;
  constexpr Style& operator=(const Style& other) noexcept = default;
  constexpr Style& operator=(Style&& other) noexcept = default;

  [[nodiscard]] Style InheritableValues() const noexcept;
  [[nodiscard]]
  static Style BuiltinBaseline();

  Style& operator+=(const Style& other);
  bool operator==(const Style& other) const noexcept = default;

  [[nodiscard]]
  auto And(this auto&& self, const Selector& selector, const Style& values)
    requires std::is_rvalue_reference_v<decltype(self)>
  {
    const auto it = std::ranges::find(
      self.mAnd, selector, [](const auto& tuple) -> const auto& {
        return std::get<0>(tuple);
      });
    if (it != self.mAnd.end()) {
      self.mAnd.erase(it);
    }
    self.mAnd.emplace_back(selector, values);
    return self;
  }

  template <std::convertible_to<Selector> T = Selector>
  [[nodiscard]]
  auto Descendants(this auto&& self, T&& selector, const Style& values)
    requires std::is_rvalue_reference_v<decltype(self)>
  {
    self.mDescendants.insert_or_assign(std::forward<T>(selector), values);
    return self;
  }

 private:
  template <class T>
  static void CopyInheritableValues(
    decltype(mStorage)& dest,
    const decltype(mStorage)& source);
};

inline Style operator+(const Style& lhs, const Style& rhs) noexcept {
  Style ret {lhs};
  ret += rhs;
  return ret;
}

class ImmutableStyle final {
 public:
  ImmutableStyle() = default;

  explicit ImmutableStyle(Style&& style)
    : mSharedData {std::make_shared<SharedData>(std::move(style))} {
    for (auto&& [key, value]: mSharedData->mStyle.mStorage) {
      style_detail::VisitStyleProperty(
        key,
        [](auto& prop) { prop.mPriority = StylePropertyPriority::UserAgent; },
        value);
    }
  }

  operator bool() const noexcept {
    return mSharedData != nullptr;
  }

  [[nodiscard]]
  const Style& Get() const noexcept {
    return mSharedData ? mSharedData->mStyle : Style::Empty();
  }

  operator const Style&() const noexcept {
    return Get();
  }

  Style const* operator->() const noexcept {
    if (!mSharedData) {
      return nullptr;
    }
    return &mSharedData->mStyle;
  }

  [[nodiscard]]
  std::optional<Style> GetCached(const std::string& key) const {
    if (!mSharedData) {
      return std::nullopt;
    }

    if (mSharedData->mCache.contains(key)) {
      return mSharedData->mCache.at(key);
    }
    return std::nullopt;
  }

  void EmplaceCache(std::string_view key, const Style& value) {
    if (!mSharedData) {
      return;
    }
    mSharedData->mCache.emplace(std::string {key}, value);
  }

 private:
  struct SharedData {
    Style mStyle;
    std::unordered_map<std::string, Style> mCache;
  };
  std::shared_ptr<SharedData> mSharedData {};
};

}// namespace FredEmmott::GUI
