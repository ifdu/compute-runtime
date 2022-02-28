/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "shared/source/os_interface/linux/drm_debug.h"
#include "shared/test/common/libult/linux/drm_query_mock.h"
#include "shared/test/common/test_macros/matchers.h"
#include "shared/test/common/test_macros/test.h"

#include "gtest/gtest.h"

using namespace NEO;

struct DrmDebugPrelimTest : public ::testing::Test {
  public:
    void SetUp() override {
        executionEnvironment = std::make_unique<ExecutionEnvironment>();
        executionEnvironment->prepareRootDeviceEnvironments(1);
        executionEnvironment->rootDeviceEnvironments[0]->setHwInfo(NEO::defaultHwInfo.get());
    }

    void TearDown() override {
    }

  protected:
    std::unique_ptr<ExecutionEnvironment> executionEnvironment;
};

TEST_F(DrmDebugPrelimTest, GivenDrmWhenRegisteringClassesThenHandlesAreStored) {
    DrmQueryMock drm(*executionEnvironment->rootDeviceEnvironments[0]);

    auto handle = drm.context.uuidHandle;

    EXPECT_EQ(0u, drm.classHandles.size());
    auto result = drm.registerResourceClasses();

    EXPECT_TRUE(result);
    EXPECT_EQ(classNamesToUuid.size(), drm.classHandles.size());

    for (size_t i = 0; i < classNamesToUuid.size(); i++) {
        EXPECT_EQ(drm.classHandles[i], handle);
        handle++;
    }
    ASSERT_TRUE(drm.context.receivedRegisterUuid);
    EXPECT_THAT(drm.context.receivedRegisterUuid->uuid, testing::HasSubstr(classNamesToUuid[classNamesToUuid.size() - 1].second));
}

TEST_F(DrmDebugPrelimTest, GivenUnsupportedUUIDRegisterIoctlWhenRegisteringClassesThenErrorIsReturnedAndClassHandlesAreEmpty) {
    DrmQueryMock drm(*executionEnvironment->rootDeviceEnvironments[0]);

    drm.context.uuidControlReturn = -1;

    EXPECT_EQ(0u, drm.classHandles.size());
    auto result = drm.registerResourceClasses();

    EXPECT_FALSE(result);
    EXPECT_EQ(0u, drm.classHandles.size());
}

TEST_F(DrmDebugPrelimTest, GivenNoClassesRegisteredWhenRegisteringResourceThenRegisterUUIDIoctlIsNotCalledAndZeroHandleReturned) {
    DrmQueryMock drm(*executionEnvironment->rootDeviceEnvironments[0]);

    auto registeredHandle = drm.registerResource(Drm::ResourceClass::Isa, nullptr, 0);
    EXPECT_EQ(0u, registeredHandle);
    EXPECT_EQ(0u, drm.ioctlCallsCount);
}

TEST_F(DrmDebugPrelimTest, GivenDrmWhenRegisteringResourceWithoutDataThenRegisterUUIDIoctlIsCalled) {
    DrmQueryMock drm(*executionEnvironment->rootDeviceEnvironments[0]);

    const auto result = drm.registerResourceClasses();
    EXPECT_TRUE(result);

    const auto handle = drm.context.uuidHandle;
    auto registeredHandle = drm.registerResource(Drm::ResourceClass::Isa, nullptr, 0);

    EXPECT_EQ(handle + 1, drm.context.uuidHandle);
    EXPECT_EQ(handle, registeredHandle);

    const auto &receivedUuid = drm.context.receivedRegisterUuid;
    ASSERT_TRUE(receivedUuid);

    EXPECT_EQ(nullptr, receivedUuid->ptr);
    EXPECT_EQ(0u, receivedUuid->size);
    EXPECT_THAT(receivedUuid->uuid, testing::HasSubstr(std::string("00000000-0000-0000")));
    EXPECT_EQ(drm.classHandles[static_cast<uint32_t>(Drm::ResourceClass::Isa)], receivedUuid->uuidClass);
}

TEST_F(DrmDebugPrelimTest, GivenDrmWhenRegisteringResourceWithDataThenRegisterUUIDIoctlIsCalledWithCorrectData) {
    DrmQueryMock drm(*executionEnvironment->rootDeviceEnvironments[0]);

    auto result = drm.registerResourceClasses();
    EXPECT_TRUE(result);

    auto handle = drm.context.uuidHandle;
    uint64_t data = 0x12345678;

    auto registeredHandle = drm.registerResource(Drm::ResourceClass::Isa, &data, sizeof(uint64_t));

    EXPECT_EQ(handle + 1, drm.context.uuidHandle);
    EXPECT_EQ(handle, registeredHandle);

    const auto &receivedUuid = drm.context.receivedRegisterUuid;
    ASSERT_TRUE(receivedUuid);

    EXPECT_EQ(&data, receivedUuid->ptr);
    EXPECT_EQ(sizeof(uint64_t), receivedUuid->size);
    EXPECT_THAT(receivedUuid->uuid, testing::HasSubstr(std::string("00000000-0000-0000")));
    EXPECT_EQ(drm.classHandles[static_cast<uint32_t>(Drm::ResourceClass::Isa)], receivedUuid->uuidClass);
    EXPECT_EQ(0u, receivedUuid->flags);
    EXPECT_EQ(0u, receivedUuid->extensions);
}

TEST_F(DrmDebugPrelimTest, GivenDrmWhenUnregisteringResourceThenUnregisterUUIDIoctlIsCalled) {
    DrmQueryMock drm(*executionEnvironment->rootDeviceEnvironments[0]);

    auto result = drm.registerResourceClasses();
    EXPECT_TRUE(result);

    uint64_t data = 0x12345678;
    auto registeredHandle = drm.registerResource(Drm::ResourceClass::Isa, &data, sizeof(uint64_t));

    drm.unregisterResource(registeredHandle);

    const auto &receivedUuid = drm.context.receivedUnregisterUuid;
    ASSERT_TRUE(receivedUuid);

    EXPECT_EQ(registeredHandle, receivedUuid->handle);
    EXPECT_EQ(nullptr, receivedUuid->ptr);
    EXPECT_EQ(0u, receivedUuid->size);
    EXPECT_EQ(0u, receivedUuid->uuidClass);
    EXPECT_EQ(0u, receivedUuid->flags);
    EXPECT_EQ(0u, receivedUuid->extensions);
}

TEST_F(DrmDebugPrelimTest, GivenDrmWhenNotifyFirstCommandQueueCreatedCalledThenCorrectUuidIsRegisteredWithCorrectData) {
    DrmQueryMock drm(*executionEnvironment->rootDeviceEnvironments[0]);

    auto handle = drm.context.uuidHandle;
    auto registeredHandle = drm.notifyFirstCommandQueueCreated();

    EXPECT_EQ(handle + 1, drm.context.uuidHandle);
    EXPECT_EQ(handle, registeredHandle);

    const auto &receivedUuid = drm.context.receivedRegisterUuid;
    ASSERT_TRUE(receivedUuid);
    EXPECT_EQ(DrmPrelimHelper::getStringUuidClass(), receivedUuid->uuidClass);
    EXPECT_EQ(receivedUuid->size, strlen(uuidL0CommandQueueName));
    EXPECT_EQ(0, memcmp(reinterpret_cast<const char *>(receivedUuid->ptr), uuidL0CommandQueueName, receivedUuid->size));
    EXPECT_EQ(0, memcmp(receivedUuid->uuid, uuidL0CommandQueueHash, sizeof(receivedUuid->uuid)));
}

TEST_F(DrmDebugPrelimTest, GivenDrmWhenNotifyLastCommandQueueDestroyedCalledThenCorrectUuidIsUnregistered) {
    DrmQueryMock drm(*executionEnvironment->rootDeviceEnvironments[0]);

    drm.notifyLastCommandQueueDestroyed(1234u);
    EXPECT_EQ(1234u, drm.context.receivedUnregisterUuid->handle);
}

TEST_F(DrmDebugPrelimTest, GivenDrmWhenRegisteringIsaCookieThenRegisterUUIDIoctlIsCalled) {
    DrmQueryMock drm(*executionEnvironment->rootDeviceEnvironments[0]);

    auto result = drm.registerResourceClasses();
    EXPECT_TRUE(result);

    auto prevIoctls = drm.ioctlCallsCount;
    auto registeredHandle = drm.registerIsaCookie(3);

    EXPECT_EQ(prevIoctls + 1u, drm.ioctlCallsCount);
    EXPECT_EQ(drm.context.uuidHandle - 1, registeredHandle);
    EXPECT_EQ(3u, drm.context.receivedRegisterUuid->uuidClass);
}

TEST_F(DrmDebugPrelimTest, GivenDrmWhenRegisteringElfResourceWithoutDataThenRegisterUUIDIoctlIsCalled) {
    DrmQueryMock drm(*executionEnvironment->rootDeviceEnvironments[0]);

    auto result = drm.registerResourceClasses();
    EXPECT_TRUE(result);

    auto handle = drm.context.uuidHandle;
    auto registeredHandle = drm.registerResource(Drm::ResourceClass::Elf, nullptr, 0);

    EXPECT_EQ(handle + 1, drm.context.uuidHandle);
    EXPECT_EQ(handle, registeredHandle);

    EXPECT_EQ(nullptr, drm.context.receivedRegisterUuid->ptr);
    EXPECT_EQ(0u, drm.context.receivedRegisterUuid->size);
}

TEST(DrmPrelimTest, givenContextDebugAvailableWhenCheckedForSupportThenTrueIsReturned) {
    auto executionEnvironment = std::make_unique<ExecutionEnvironment>();
    executionEnvironment->setDebuggingEnabled();
    executionEnvironment->prepareRootDeviceEnvironments(1);
    executionEnvironment->rootDeviceEnvironments[0]->setHwInfo(defaultHwInfo.get());
    executionEnvironment->calculateMaxOsContextCount();
    executionEnvironment->rootDeviceEnvironments[0]->osInterface = std::make_unique<OSInterface>();

    auto *drm = new DrmQueryMock(*executionEnvironment->rootDeviceEnvironments[0]);
    drm->contextDebugSupported = true;
    executionEnvironment->rootDeviceEnvironments[0]->osInterface->setDriverModel(std::unique_ptr<DriverModel>(drm));

    auto prevIoctls = drm->ioctlCallsCount;

    auto contextParamCallsBefore = drm->receivedContextParamRequestCount;
    drm->checkContextDebugSupport();
    EXPECT_EQ(prevIoctls + 1u, drm->ioctlCallsCount);
    EXPECT_EQ(contextParamCallsBefore + 1u, drm->receivedContextParamRequestCount);

    EXPECT_TRUE(drm->isContextDebugSupported());
    EXPECT_EQ(prevIoctls + 1u, drm->ioctlCallsCount);
}

TEST(DrmPrelimTest, givenContextDebugNotAvailableWhenCheckedForSupportThenTrueIsReturned) {
    auto executionEnvironment = std::make_unique<ExecutionEnvironment>();
    executionEnvironment->setDebuggingEnabled();
    executionEnvironment->prepareRootDeviceEnvironments(1);
    executionEnvironment->rootDeviceEnvironments[0]->setHwInfo(defaultHwInfo.get());
    executionEnvironment->calculateMaxOsContextCount();
    executionEnvironment->rootDeviceEnvironments[0]->osInterface = std::make_unique<OSInterface>();

    auto *drm = new DrmQueryMock(*executionEnvironment->rootDeviceEnvironments[0]);
    drm->contextDebugSupported = false;
    executionEnvironment->rootDeviceEnvironments[0]->osInterface->setDriverModel(std::unique_ptr<DriverModel>(drm));

    auto prevIoctls = drm->ioctlCallsCount;
    auto contextParamCallsBefore = drm->receivedContextParamRequestCount;
    drm->checkContextDebugSupport();
    EXPECT_EQ(prevIoctls + 1u, drm->ioctlCallsCount);
    EXPECT_EQ(contextParamCallsBefore + 1u, drm->receivedContextParamRequestCount);

    EXPECT_FALSE(drm->isContextDebugSupported());
    EXPECT_EQ(prevIoctls + 1u, drm->ioctlCallsCount);
}
