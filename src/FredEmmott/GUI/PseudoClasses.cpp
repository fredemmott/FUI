// Copyright 2025 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT

#include "PseudoClasses.hpp"

namespace FredEmmott::GUI::PseudoClasses {
const StyleClass Active = StyleClass::Make(":active");
const StyleClass Checked = StyleClass::Make(":checked");
const StyleClass Disabled = StyleClass::Make(":disabled");
const StyleClass Hover = StyleClass::Make(":hover");
const StyleClass LayoutOrphan = StyleClass::Make(":layout-orphan");
}// namespace FredEmmott::GUI::PseudoClasses