/*
 * Copyright (C) 2021-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/kernel/kernel_properties.h"
#include "shared/test/common/mocks/mock_device.h"
#include "shared/test/common/test_macros/hw_test.h"

#include "level_zero/core/test/unit_tests/fixtures/device_fixture.h"
#include "level_zero/core/test/unit_tests/mocks/mock_module.h"

namespace L0 {
namespace ult {

using KernelPropertyTest = Test<DeviceFixture>;

HWTEST2_F(KernelPropertyTest, givenKernelExtendedPropertiesStructureWhenKernelPropertiesCalledThenPropertiesAreCorrectlySet, IsXeHpgCore) {
    ze_device_module_properties_t kernelProperties = {};
    ze_float_atomic_ext_properties_t kernelExtendedProperties = {};
    kernelExtendedProperties.stype = ZE_STRUCTURE_TYPE_FLOAT_ATOMIC_EXT_PROPERTIES;
    kernelProperties.pNext = &kernelExtendedProperties;
    ze_result_t res = device->getKernelProperties(&kernelProperties);
    EXPECT_EQ(res, ZE_RESULT_SUCCESS);

    const auto &fp16Properties = kernelExtendedProperties.fp16Flags;
    EXPECT_TRUE(fp16Properties & FP_ATOMIC_EXT_FLAG_GLOBAL_LOAD_STORE);
    EXPECT_TRUE(fp16Properties & FP_ATOMIC_EXT_FLAG_LOCAL_LOAD_STORE);
    EXPECT_TRUE(fp16Properties & FP_ATOMIC_EXT_FLAG_GLOBAL_MIN_MAX);
    EXPECT_TRUE(fp16Properties & FP_ATOMIC_EXT_FLAG_LOCAL_MIN_MAX);

    EXPECT_FALSE(fp16Properties & FP_ATOMIC_EXT_FLAG_GLOBAL_ADD);
    EXPECT_FALSE(fp16Properties & FP_ATOMIC_EXT_FLAG_LOCAL_ADD);

    const auto &fp32Properties = kernelExtendedProperties.fp32Flags;
    EXPECT_TRUE(fp32Properties & FP_ATOMIC_EXT_FLAG_GLOBAL_LOAD_STORE);
    EXPECT_TRUE(fp32Properties & FP_ATOMIC_EXT_FLAG_LOCAL_LOAD_STORE);
    EXPECT_TRUE(fp32Properties & FP_ATOMIC_EXT_FLAG_GLOBAL_MIN_MAX);
    EXPECT_TRUE(fp32Properties & FP_ATOMIC_EXT_FLAG_LOCAL_MIN_MAX);
    EXPECT_TRUE(fp32Properties & FP_ATOMIC_EXT_FLAG_GLOBAL_ADD);
    EXPECT_TRUE(fp32Properties & FP_ATOMIC_EXT_FLAG_LOCAL_ADD);

    const auto &fp64Properties = kernelExtendedProperties.fp64Flags;
    EXPECT_TRUE(fp64Properties & FP_ATOMIC_EXT_FLAG_GLOBAL_LOAD_STORE);
    EXPECT_TRUE(fp64Properties & FP_ATOMIC_EXT_FLAG_LOCAL_LOAD_STORE);
    EXPECT_TRUE(fp64Properties & FP_ATOMIC_EXT_FLAG_GLOBAL_MIN_MAX);
    EXPECT_TRUE(fp64Properties & FP_ATOMIC_EXT_FLAG_LOCAL_MIN_MAX);
    EXPECT_TRUE(fp64Properties & FP_ATOMIC_EXT_FLAG_GLOBAL_ADD);
    EXPECT_TRUE(fp64Properties & FP_ATOMIC_EXT_FLAG_LOCAL_ADD);
}

HWTEST2_F(KernelPropertyTest, givenDG2WhenGetInternalOptionsThenWriteBackBuildOptionIsSet, IsDG2) {
    auto pMockCompilerInterface = new MockCompilerInterface;
    auto &rootDeviceEnvironment = this->neoDevice->executionEnvironment->rootDeviceEnvironments[this->neoDevice->getRootDeviceIndex()];
    rootDeviceEnvironment->compilerInterface.reset(pMockCompilerInterface);
    MockModuleTranslationUnit mockTranslationUnit(this->device);
    mockTranslationUnit.processUnpackedBinaryCallBase = false;
    ze_result_t result = ZE_RESULT_ERROR_MODULE_BUILD_FAILURE;
    result = mockTranslationUnit.buildFromSpirV("", 0U, nullptr, "", nullptr);
    EXPECT_EQ(result, ZE_RESULT_SUCCESS);
    EXPECT_EQ(mockTranslationUnit.processUnpackedBinaryCalled, 1u);
    EXPECT_NE(pMockCompilerInterface->inputInternalOptions.find("-cl-store-cache-default=7 -cl-load-cache-default=4"), std::string::npos);
}

} // namespace ult
} // namespace L0
