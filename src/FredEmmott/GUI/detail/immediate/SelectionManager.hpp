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
  const auto& GetAllKeys() {
    FUI_ASSERT(
      mNextIndex > mSelectedIndex || (mNextIndex == 0 && mSelectedIndex == 0),
      "GetAllKeys() should be called before BeginContainer()");
    mAllKeys.resize(mNextIndex);
    return mAllKeys;
  }

  void BeginContainer(T* state) {
    mNextIndex = 0;

    struct Item {
      std::size_t mIndex {};
      Widgets::ISelectionItem* mItem {};
      ItemContext* mContext {};
      T mKey {};
    };

    const auto items
      = mContainer->GetSelectionItems() | std::views::enumerate
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
        mSelectedIndex = idx;
        mSelectedKey = *state = key;
        ctx->mSelectedThisFrame = true;
      } else {
        ctx->mSelectedThisFrame = false;
      }
    }

    // Optimize "no new or removed items"
    if (
      mSelectedKey == *state && mSelectedIndex < items.size()
      && items.at(mSelectedIndex).mKey == *state) {
      return;
    }

    if (const auto it = std::ranges::find(items, *state, &Item::mKey);
        it != items.end()) {
      mSelectedIndex = it->mIndex;
      mSelectedKey = it->mKey;
      it->mItem->Select();
      it->mContext->mSelectedThisFrame = it->mItem->ConsumeWasSelected();
      return;
    }

    if (items.empty()) {
      *state = T {};
      mSelectedIndex = 0;
      mSelectedKey.reset();
      return;
    }

    mSelectedIndex
      = std::clamp<std::size_t>(mSelectedIndex, 0, items.size() - 1);
    auto& item = items.at(mSelectedIndex);
    mSelectedKey = *state = item.mKey;
    item.mItem->Select();
    item.mContext->mSelectedThisFrame = item.mItem->ConsumeWasSelected();
  }

  [[nodiscard]]
  static auto& Get(Widgets::ISelectionContainer* const container) {
    auto& self
      = *container->GetWidget()->GetOrCreateContext<SelectionManager>();
    self.mContainer = container;
    return self;
  }

  static void BeginContainer(
    Widgets::ISelectionContainer* const container,
    T* state) {
    Get(container).BeginContainer(state);
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

    if (idx < self.mAllKeys.size()) {
      self.mAllKeys[idx] = key;
    } else {
      FUI_ASSERT(idx == self.mAllKeys.size());
      self.mAllKeys.push_back(key);
    }

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

  Widgets::ISelectionContainer* mContainer {};

  /// 0 can mean 'first item', but can also mean 'no items' or 'first frame'
  std::size_t mNextIndex {};

  std::size_t mSelectedIndex {};
  std::optional<T> mSelectedKey;
  std::vector<T> mAllKeys;
};

}// namespace FredEmmott::GUI::Immediate::immediate_detail