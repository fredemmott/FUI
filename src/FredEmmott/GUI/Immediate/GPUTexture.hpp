// Copyright 2026 Fred Emmott <fred@fredemmott.com>
// SPDX-License-Identifier: MIT
//
// Platform dispatcher for the Immediate-mode GPUTexture API. Each backend
// has its own shape (Win32 takes shared HANDLEs; a future Linux variant
// will likely take dmabuf-fd) so each lives in its own sibling header
// included conditionally below.
#pragma once

#ifdef _WIN32
#include "GPUTexture_win32.hpp"
#endif
