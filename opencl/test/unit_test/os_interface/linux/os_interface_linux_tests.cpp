/*
 * Copyright (C) 2017-2021 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/command_stream/preemption.h"
#include "shared/source/gmm_helper/gmm_lib.h"
#include "shared/source/helpers/hw_helper.h"
#include "shared/source/os_interface/linux/os_context_linux.h"
#include "shared/source/os_interface/linux/os_interface.h"

#include "opencl/test/unit_test/mocks/mock_execution_environment.h"
#include "opencl/test/unit_test/os_interface/linux/drm_mock.h"

#include "gtest/gtest.h"

namespace NEO {

TEST(OsInterfaceTest, GivenLinuxWhenCallingAre64kbPagesEnabledThenReturnFalse) {
    EXPECT_FALSE(OSInterface::are64kbPagesEnabled());
}

TEST(OsInterfaceTest, GivenLinuxOsInterfaceWhenDeviceHandleQueriedThenZeroIsReturned) {
    OSInterface osInterface;
    EXPECT_EQ(0u, osInterface.getDeviceHandle());
}

TEST(OsInterfaceTest, GivenLinuxOsWhenCheckForNewResourceImplicitFlushSupportThenReturnTrue) {
    EXPECT_TRUE(OSInterface::newResourceImplicitFlush);
}

TEST(OsInterfaceTest, GivenLinuxOsWhenCheckForGpuIdleImplicitFlushSupportThenReturnFalse) {
    EXPECT_TRUE(OSInterface::gpuIdleImplicitFlush);
}

TEST(OsInterfaceTest, GivenLinuxOsInterfaceWhenCallingIsDebugAttachAvailableThenFalseIsReturned) {
    OSInterface osInterface;

    auto executionEnvironment = std::make_unique<ExecutionEnvironment>();
    executionEnvironment->prepareRootDeviceEnvironments(1);
    DrmMock *drm = new DrmMock(*executionEnvironment->rootDeviceEnvironments[0]);

    osInterface.get()->setDrm(drm);
    EXPECT_FALSE(osInterface.isDebugAttachAvailable());
}

} // namespace NEO
