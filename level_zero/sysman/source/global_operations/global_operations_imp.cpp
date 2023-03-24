/*
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "level_zero/sysman/source/global_operations/global_operations_imp.h"

#include "shared/source/helpers/debug_helpers.h"
#include "shared/source/helpers/string.h"

#include "level_zero/sysman/source/sysman_const.h"

#include <algorithm>

namespace L0 {
namespace Sysman {

ze_result_t GlobalOperationsImp::processesGetState(uint32_t *pCount, zes_process_state_t *pProcesses) {
    initGlobalOperations();
    std::vector<zes_process_state_t> pProcessList;
    ze_result_t result = pOsGlobalOperations->scanProcessesState(pProcessList);
    if (result != ZE_RESULT_SUCCESS) {
        return result;
    }

    if ((*pCount > 0) && (*pCount < pProcessList.size())) {
        result = ZE_RESULT_ERROR_INVALID_SIZE;
    }
    if (pProcesses != nullptr) {
        uint32_t limit = std::min(*pCount, static_cast<uint32_t>(pProcessList.size()));
        for (uint32_t i = 0; i < limit; i++) {
            pProcesses[i].processId = pProcessList[i].processId;
            pProcesses[i].engines = pProcessList[i].engines;
            pProcesses[i].memSize = pProcessList[i].memSize;
            pProcesses[i].sharedSize = pProcessList[i].sharedSize;
        }
    }
    *pCount = static_cast<uint32_t>(pProcessList.size());

    return result;
}

ze_result_t GlobalOperationsImp::deviceGetProperties(zes_device_properties_t *pProperties) {
    initGlobalOperations();
    sysmanProperties.numSubdevices = pOsSysman->getSubDeviceCount();
    *pProperties = sysmanProperties;
    return ZE_RESULT_SUCCESS;
}

ze_result_t GlobalOperationsImp::reset(ze_bool_t force) {
    initGlobalOperations();
    return pOsGlobalOperations->reset(force);
}

ze_result_t GlobalOperationsImp::deviceGetState(zes_device_state_t *pState) {
    initGlobalOperations();
    return pOsGlobalOperations->deviceGetState(pState);
}

void GlobalOperationsImp::init() {
    if (pOsGlobalOperations == nullptr) {
        pOsGlobalOperations = OsGlobalOperations::create(pOsSysman);
    }
    UNRECOVERABLE_IF(nullptr == pOsGlobalOperations);
    pOsGlobalOperations->getVendorName(sysmanProperties.vendorName);
    pOsGlobalOperations->getDriverVersion(sysmanProperties.driverVersion);
    pOsGlobalOperations->getModelName(sysmanProperties.modelName);
    pOsGlobalOperations->getBrandName(sysmanProperties.brandName);
    memset(sysmanProperties.boardNumber, 0, ZES_STRING_PROPERTY_SIZE);
    if (!pOsGlobalOperations->getBoardNumber(sysmanProperties.boardNumber)) {
        memcpy_s(sysmanProperties.boardNumber, ZES_STRING_PROPERTY_SIZE, unknown.c_str(), unknown.length() + 1);
    }
    memset(sysmanProperties.serialNumber, 0, ZES_STRING_PROPERTY_SIZE);
    if (!pOsGlobalOperations->getSerialNumber(sysmanProperties.serialNumber)) {
        memcpy_s(sysmanProperties.serialNumber, ZES_STRING_PROPERTY_SIZE, unknown.c_str(), unknown.length() + 1);
    }
}
void GlobalOperationsImp::initGlobalOperations() {
    std::call_once(initGlobalOpOnce, [this]() {
        this->init();
    });
}
GlobalOperationsImp::~GlobalOperationsImp() {
    if (nullptr != pOsGlobalOperations) {
        delete pOsGlobalOperations;
    }
}

} // namespace Sysman
} // namespace L0
