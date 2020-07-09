/*
 * Copyright (C) 2019-2020 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "level_zero/core/source/cmdlist/cmdlist_imp.h"

#include "shared/source/command_stream/command_stream_receiver.h"
#include "shared/source/command_stream/linear_stream.h"
#include "shared/source/device/device.h"
#include "shared/source/helpers/engine_node_helper.h"
#include "shared/source/indirect_heap/indirect_heap.h"

#include "level_zero/core/source/device/device.h"
#include "level_zero/core/source/device/device_imp.h"
#include "level_zero/tools/source/metrics/metric.h"

#include "igfxfmid.h"

#include <algorithm>

namespace L0 {

CommandListAllocatorFn commandListFactory[IGFX_MAX_PRODUCT] = {};
CommandListAllocatorFn commandListFactoryImmediate[IGFX_MAX_PRODUCT] = {};

ze_result_t CommandListImp::destroy() {
    delete this;
    return ZE_RESULT_SUCCESS;
}

ze_result_t CommandListImp::appendMetricMemoryBarrier() {
    return MetricQuery::appendMemoryBarrier(*this);
}

ze_result_t CommandListImp::appendMetricStreamerMarker(zet_metric_streamer_handle_t hMetricStreamer,
                                                       uint32_t value) {
    return MetricQuery::appendStreamerMarker(*this, hMetricStreamer, value);
}

ze_result_t CommandListImp::appendMetricTracerMarker(zet_metric_tracer_handle_t hMetricTracer,
                                                     uint32_t value) {

    zet_metric_streamer_handle_t hMetricStreamer = reinterpret_cast<zet_metric_streamer_handle_t>(hMetricTracer);

    return MetricQuery::appendStreamerMarker(*this, hMetricStreamer, value);
}

ze_result_t CommandListImp::appendMetricQueryBegin(zet_metric_query_handle_t hMetricQuery) {
    return MetricQuery::fromHandle(hMetricQuery)->appendBegin(*this);
}

ze_result_t CommandListImp::appendMetricQueryEnd(zet_metric_query_handle_t hMetricQuery,
                                                 ze_event_handle_t hCompletionEvent) {
    return MetricQuery::fromHandle(hMetricQuery)->appendEnd(*this, hCompletionEvent);
}

CommandList *CommandList::create(uint32_t productFamily, Device *device, bool isCopyOnly) {
    CommandListAllocatorFn allocator = nullptr;
    if (productFamily < IGFX_MAX_PRODUCT) {
        allocator = commandListFactory[productFamily];
    }

    CommandListImp *commandList = nullptr;
    if (allocator) {
        commandList = static_cast<CommandListImp *>((*allocator)(CommandList::defaultNumIddsPerBlock));

        commandList->initialize(device, isCopyOnly);
    }
    return commandList;
}

CommandList *CommandList::createImmediate(uint32_t productFamily, Device *device,
                                          const ze_command_queue_desc_t *desc,
                                          bool internalUsage, bool isCopyOnly) {

    auto deviceImp = static_cast<DeviceImp *>(device);
    NEO::CommandStreamReceiver *csr = nullptr;
    if (internalUsage) {
        csr = deviceImp->neoDevice->getInternalEngine().commandStreamReceiver;
    } else if (isCopyOnly) {
        auto &selectorCopyEngine = deviceImp->neoDevice->getDeviceById(0)->getSelectorCopyEngine();
        csr = deviceImp->neoDevice->getDeviceById(0)->getEngine(NEO::EngineHelpers::getBcsEngineType(deviceImp->neoDevice->getHardwareInfo(), selectorCopyEngine), false).commandStreamReceiver;
    } else {
        csr = deviceImp->neoDevice->getDefaultEngine().commandStreamReceiver;
    }

    auto commandQueue = CommandQueue::create(productFamily, device, csr, desc, isCopyOnly);
    if (!commandQueue) {
        return nullptr;
    }

    CommandListAllocatorFn allocator = nullptr;
    if (productFamily < IGFX_MAX_PRODUCT) {
        allocator = commandListFactoryImmediate[productFamily];
    }

    CommandListImp *commandList = nullptr;
    if (allocator) {
        commandList = static_cast<CommandListImp *>((*allocator)(CommandList::commandListimmediateIddsPerBlock));

        commandList->initialize(device, isCopyOnly);
    }

    if (!commandList) {
        commandQueue->destroy();
        return nullptr;
    }

    commandList->cmdQImmediate = commandQueue;
    commandList->cmdListType = CommandListType::TYPE_IMMEDIATE;
    commandList->cmdQImmediateDesc = desc;
    commandList->commandListPreemptionMode = device->getDevicePreemptionMode();

    return commandList;
}

} // namespace L0
