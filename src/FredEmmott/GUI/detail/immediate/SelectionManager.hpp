// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once
#include <FredEmmott/GUI/Immediate/selectable_key.hpp>
#include <FredEmmott/GUI/Widgets/Focusable.hpp>
#include <boost/container/flat_map.hpp>

namespace FredEmmott::GUI::Immediate::immediate_detail {

template <selectable_key T>
class SelectionManager : public Widgets::Context {
 public:
  static void BeginContainer(
    Widgets::ISelectionContainer* const container,
    T* state) {
    auto& self
      = *container->GetWidget()->GetOrCreateContext<SelectionManager>();
    self.mNextIndex = 0;

    struct Item {
      std::size_t mIndex {};
      Widgets::ISelectionItem* mItem {};
      ItemContext* mContext {};
      T mKey {};
    };

    const auto items
      = container->GetSelectionItems() | std::views::enumerate
      | std::views::transform([](const auto& pair) {
          const auto [idx, item] = pair;
          const auto ctx
            = item->GetWidget()->template GetContext<ItemContext>();
          FUI_ASSERT(
            ctx,
            "All managed ISelectionItems should be assigned an ItemContext on "
            "their first frame; is SelectionManager::BeginItem() called?");
          return Item {
            static_cast<std::size_t>(idx),
            item,
            ctx,
            ctx->mKey,
          };
        })
      | std::ranges::to<std::vector>();

    // 1. User interaction takes priority
    // 2. Then current selection by key
    // 3. Then min(size, last, index)

    for (auto&& [idx, item, ctx, key]: items) {
      if (item->ConsumeWasSelected()) {
        self.mSelectedIndex = idx;
        self.mSelectedKey = *state = key;
        ctx->mSelectedThisFrame = true;
      } else {
        ctx->mSelectedThisFrame = false;
      }
    }

    // Optimize "no new or removed items"
    if (
      self.mSelectedKey == *state && self.mSelectedIndex < items.size()
      && items.at(self.mSelectedIndex).mKey == *state) {
      return;
    }

    const auto it = std::ranges::find(items, *state, &Item::mKey);
    if (it != items.end()) {
      self.mSelectedIndex = it->mIndex;
      self.mSelectedKey = it->mKey;
      it->mItem->Select();
      it->mContext->mSelectedThisFrame = it->mItem->ConsumeWasSelected();
      return;
    }

    if (items.empty()) {
      *state = T {};
      self.mSelectedIndex = 0;
      self.mSelectedKey.reset();
      return;
    }

    self.mSelectedIndex
      = std::clamp<std::size_t>(self.mSelectedIndex, 0, items.size() - 1);
    auto& item = items.at(self.mSelectedIndex);
    self.mSelectedKey = *state = item.mKey;
    item.mItem->Select();
    it->mContext->mSelectedThisFrame = it->mItem->ConsumeWasSelected();
  }

  [[nodiscard]]
  static bool BeginItem(const T& key, Widgets::ISelectionItem* item) {
    auto& self = *item->GetSelectionContainer()
                    ->GetWidget()
                    ->GetContext<SelectionManager<T>>();
    const auto widget = item->GetWidget();
    const auto ctx = widget->GetOrCreateContext<ItemContext>();
    ctx->mKey = key;
    const auto idx = self.mNextIndex++;
    if (idx != self.mSelectedIndex) {
      return false;
    }

    self.mSelectedKey = key;
    if (item->IsSelected()) {
      return std::exchange(ctx->mSelectedThisFrame, false);
    }
    item->Select();
    std::ignore = item->ConsumeWasSelected();
    return true;
  }

 private:
  struct ItemContext : Widgets::Context {
    ~ItemContext() override = default;
    T mKey {};
    bool mSelectedThisFrame {};
  };

  /// 0 can mean 'first item', but can also mean 'no items' or 'first frame'
  std::size_t mNextIndex {};

  std::size_t mSelectedIndex {};
  std::optional<T> mSelectedKey;
};

}// namespace FredEmmott::GUI::Immediate::immediate_detail