// Copyright 2024 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <YGEnums.h>

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

struct StyleProperties {
  template <class T>
  using storage_t
    = utility::unordered_map<style_detail::StyleProperty, StyleProperty<T>>;
#define FUI_DECLARE_STYLE_PROPERTY_STORAGE(TYPE, NAME) \
  storage_t<TYPE> m##NAME##Storage; \
  constexpr storage_t<TYPE>& Storage(utility::type_tag_t<TYPE>) noexcept { \
    return m##NAME##Storage; \
  } \
  constexpr const storage_t<TYPE>& Storage(utility::type_tag_t<TYPE>) \
    const noexcept { \
    return m##NAME##Storage; \
  }
  FUI_ENUM_STYLE_PROPERTY_TYPES(FUI_DECLARE_STYLE_PROPERTY_STORAGE)
#undef FUI_DECLARE_STYLE_PROPERTY_STORAGE
  template <style_detail::StyleProperty P>
  constexpr auto& Storage(this auto&& self, utility::value_tag_t<P>) noexcept {
    using value_type = style_detail::property_value_t<P>;
    return self.Storage(utility::type_tag<value_type>);
  }

  template <class T>
  constexpr const auto& GetProperty(
    const style_detail::StyleProperty P) const noexcept {
    static constexpr StyleProperty<T> Empty {};
    const auto& storage = Storage(utility::type_tag<T>);
    return storage.contains(P) ? storage[P] : Empty;
  }

  template <class T>
  constexpr auto& GetProperty(const style_detail::StyleProperty P) noexcept {
    return Storage(utility::type_tag<T>)[P];
  }

#define FUI_DECLARE_PROPERTY_GETTER(NAME, ...) \
  [[nodiscard]] constexpr decltype(auto) NAME(this auto&& self) noexcept { \
    constexpr auto key = style_detail::StyleProperty::NAME; \
    using value_type = style_detail::property_value_t<key>; \
    return self.template GetProperty<value_type>(key); \
  } \
  [[nodiscard]] constexpr bool Has##NAME() const noexcept { \
    constexpr auto key = style_detail::StyleProperty::NAME; \
    return Storage(utility::value_tag<key>).contains(key); \
  }
#define FUI_DECLARE_PROPERTY_SETTER(NAME, ...) \
  template <class Self, class... Args> \
    requires(sizeof...(Args) > 0) \
  [[nodiscard]] \
  decltype(auto) NAME(this Self&& self, Args&&... args) noexcept { \
    constexpr auto key = style_detail::StyleProperty::NAME; \
    using type = style_detail::property_value_t<key>; \
    Self ret = std::forward<Self>(self); \
    ret.Storage(utility::type_tag<type>) \
      .insert_or_assign( \
        key, StyleProperty<type>(std::forward<Args>(args)...)); \
    return ret; \
  } \
  decltype(auto) Unset##NAME() noexcept { \
    using type \
      = style_detail::property_value_t<style_detail::StyleProperty::NAME>; \
    Storage(utility::type_tag<type>).erase(style_detail::StyleProperty::NAME); \
  }
  FUI_ENUM_STYLE_PROPERTIES(FUI_DECLARE_PROPERTY_GETTER)
  FUI_ENUM_STYLE_PROPERTIES(FUI_DECLARE_PROPERTY_SETTER)
#undef FUI_DECLARE_PROPERTY_GETTER
#undef FUI_DECLARE_PROPERTY_SETTER

  StyleProperties& operator+=(const StyleProperties& other) noexcept;
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

  Style& operator+=(const Style& other) noexcept;
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
    utility::unordered_map<style_detail::StyleProperty, StyleProperty<T>> dest,
    const utility::unordered_map<style_detail::StyleProperty, StyleProperty<T>>&
      source);
};

inline Style operator+(const Style& lhs, const Style& rhs) noexcept {
  Style ret {lhs};
  ret += rhs;
  return ret;
}

}// namespace FredEmmott::GUI
