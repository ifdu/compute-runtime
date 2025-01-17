/*
 * Copyright (C) 2019-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/gen12lp/hw_cmds.h"

#include "opencl/source/command_queue/command_queue.h"
#include "opencl/source/helpers/hardware_commands_helper.h"
#include "opencl/source/helpers/hardware_commands_helper_base.inl"
#include "opencl/source/helpers/hardware_commands_helper_bdw_and_later.inl"

namespace NEO {
using FamilyType = Gen12LpFamily;

template <>
size_t HardwareCommandsHelper<FamilyType>::getSizeRequiredCS() {
    size_t size = 2 * sizeof(typename FamilyType::MEDIA_STATE_FLUSH) +
                  sizeof(typename FamilyType::MEDIA_INTERFACE_DESCRIPTOR_LOAD);
    return size;
}

template struct HardwareCommandsHelper<FamilyType>;
} // namespace NEO
