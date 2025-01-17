/*
 * Copyright (C) 2020-2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include "shared/source/direct_submission/direct_submission_hw.h"
#include "shared/source/os_interface/windows/windows_defs.h"
struct COMMAND_BUFFER_HEADER_REC;
typedef struct COMMAND_BUFFER_HEADER_REC COMMAND_BUFFER_HEADER;

namespace NEO {

class OsContextWin;
class Wddm;

template <typename GfxFamily, typename Dispatcher>
class WddmDirectSubmission : public DirectSubmissionHw<GfxFamily, Dispatcher> {
  public:
    WddmDirectSubmission(const DirectSubmissionInputParams &inputParams);

    ~WddmDirectSubmission() override;

    void flushMonitorFence() override;

  protected:
    bool allocateOsResources() override;
    bool submit(uint64_t gpuAddress, size_t size) override;

    bool handleResidency() override;
    void handleCompletionFence(uint64_t completionValue, MonitoredFence &fence);
    void ensureRingCompletion() override;
    void handleSwitchRingBuffers() override;
    void handleStopRingBuffer() override;
    uint64_t updateTagValue(bool requireMonitorFence) override;
    bool dispatchMonitorFenceRequired(bool requireMonitorFence) override;
    uint64_t updateTagValueImpl();
    void getTagAddressValue(TagData &tagData) override;
    bool isCompleted(uint32_t ringBufferIndex) override;

    OsContextWin *osContextWin;
    Wddm *wddm;
    MonitoredFence ringFence;
    std::unique_ptr<COMMAND_BUFFER_HEADER> commandBufferHeader;
};
} // namespace NEO
