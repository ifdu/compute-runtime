/*
 * Copyright (C) 2018-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

namespace NEO {

enum class MemoryPool {
    MemoryNull,
    System4KBPages,
    System64KBPages,
    System4KBPagesWith32BitGpuAddressing,
    System64KBPagesWith32BitGpuAddressing,
    SystemCpuInaccessible,
    LocalMemory,
};

namespace MemoryPoolHelper {

template <typename... Args>
inline bool isSystemMemoryPool(Args... pool) {
    return ((pool == MemoryPool::System4KBPages ||
             pool == MemoryPool::System64KBPages ||
             pool == MemoryPool::System4KBPagesWith32BitGpuAddressing ||
             pool == MemoryPool::System64KBPagesWith32BitGpuAddressing) &&
            ...);
}

} // namespace MemoryPoolHelper
} // namespace NEO
