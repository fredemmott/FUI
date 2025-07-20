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

struct StyleProperties {
  struct PropertyTypes {
    PropertyTypes() = delete;
#define FUI_DECLARE_STYLE_PROPERTY_TYPE(NAME, TYPE, ...) using NAME##_t = TYPE;
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
  ;

  template <class T>
  constexpr const auto& GetProperty(const StylePropertyKey P) const noexcept {
    static constexpr StyleProperty<T> Empty {};
    return mStorage.contains(P) ? std::get<StyleProperty<T>>(mStorage[P])
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
#define FUI_DECLARE_PROPERTY_SETTERS(NAME, TYPE, ...) \
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

  StyleProperties& operator+=(const StyleProperties& other);
  bool operator==(const StyleProperties& other) const noexcept = default;
};// namespace FredEmmott::GUI

struct Style : StyleProperties {
  using Selector
    = std::variant<std::monostate, StyleClass, const Widgets::Widget*>;
  utility::unordered_map<Selector, StyleProperties> mAnd;
  utility::unordered_map<Selector, StyleProperties> mDescendants;

  constexpr Style() = default;
  constexpr Style(const Style& other) noexcept = default;
  constexpr Style(Style&& other) noexcept = default;
  constexpr Style& operator=(const Style& other) noexcept = default;
  constexpr Style& operator=(Style&& other) noexcept = default;

  constexpr Style(const StyleProperties& other) noexcept
    : StyleProperties(other) {}

  [[nodiscard]] Style InheritableValues() const noexcept;
  [[nodiscard]]
  static Style BuiltinBaseline();

  Style& operator+=(const Style& other);
  bool operator==(const Style& other) const noexcept = default;

  template <class Self, std::convertible_to<Selector> T = Selector>
  [[nodiscard]]
  auto And(this Self&& self, T&& selector, const StyleProperties& values) {
    Self copyOrMoved = std::forward<Self>(self);
    copyOrMoved.mAnd.insert_or_assign(std::forward<T>(selector), values);
    return copyOrMoved;
  }

  template <class Self, std::convertible_to<Selector> T = Selector>
  [[nodiscard]]
  auto
  Descendants(this Self&& self, T&& selector, const StyleProperties& values) {
    Self copyOrMoved = std::forward<Self>(self);
    copyOrMoved.mDescendants.insert_or_assign(
      std::forward<T>(selector), values);
    return copyOrMoved;
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

}// namespace FredEmmott::GUI
