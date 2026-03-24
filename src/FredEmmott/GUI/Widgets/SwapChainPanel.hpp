// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
#pragma once

#include <FredEmmott/GUI/SwapChain.hpp>

#include "Widget.hpp"

struct ID3D11Device3;
struct ID3D12Device;

namespace FredEmmott::GUI::Widgets {

class SwapChainPanel final : public Widget {
 public:
  explicit SwapChainPanel(Window*);
  ~SwapChainPanel() override;

  SwapChain GetSwapChain() const noexcept;

 protected:
  void PaintOwnContent(Renderer*, const Rect&, const Style&) const override;

 private:
  friend class ::FredEmmott::GUI::SwapChain;
  using Resources = SwapChain::Resources;
  struct Submission {
    ImportedTexture* mTexture {};
    bool mFenceIsNew {};
    HANDLE mFenceHandle {};
    uint64_t mFenceValue {};
  };

  std::shared_ptr<Resources> mResources;

  void Init(Renderer* renderer, const Size& size);

#ifdef FUI_ENABLE_DIRECT2D
  [[nodiscard]]
  bool InitD3D11(ID3D11Device3*, Renderer*, const Size&);
  [[nodiscard]]
  bool InitD3D11(Renderer*, const Size&);
#endif
#ifdef FUI_ENABLE_SKIA
  [[nodiscard]]
  bool InitD3D12(ID3D12Device*, Renderer*, const Size&);
  [[nodiscard]]
  bool InitSkia(Renderer*, const Size&);
#endif
};

}// namespace FredEmmott::GUI::Widgets