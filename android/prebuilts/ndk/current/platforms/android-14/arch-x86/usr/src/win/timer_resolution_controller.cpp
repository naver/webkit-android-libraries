/*
 * Copyright (C) 2016 Naver Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "timer_resolution_controller.h"

#include <win/ws2_shims.h>
#include <timeapi.h>

#if defined(WIN32) || defined(_WINDOWS)

#include <chrono>

static const int timerResolution = 1; // To improve timer resolution, we call timeBeginPeriod/timeEndPeriod with this value to increase timer resolution to 1ms.
static const int highResolutionThresholdMsec = 16; // Only activate high-res timer for sub-16ms timers (Windows can fire timers at 16ms intervals without changing the system resolution).
std::atomic<int> TimerResolutionController::highResolutionTimerCount = 0;

TimerResolutionController::TimerResolutionController(timeval* timeout)
    : m_highPrecisionTimerUsed(useHighResolutionTimerIfNeeded(timeout))
{
}

TimerResolutionController::~TimerResolutionController()
{
    if (!m_highPrecisionTimerUsed)
        return;

    if (--highResolutionTimerCount == 0)
        ::timeEndPeriod(timerResolution);
}

bool TimerResolutionController::useHighResolutionTimerIfNeeded(timeval* timeout)
{
    if (!timeout || !timerisset(timeout))
        return false;

    std::chrono::milliseconds delay = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds(timeout->tv_sec) + std::chrono::microseconds(timeout->tv_usec));
    DWORD dueTime;
    if (delay.count() > USER_TIMER_MAXIMUM)
        dueTime = USER_TIMER_MAXIMUM;
    else if (delay.count() < 0)
        dueTime = 0;
    else
        dueTime = static_cast<DWORD>(delay.count());

    if (dueTime < highResolutionThresholdMsec) {
        if (++highResolutionTimerCount == 1)
            ::timeBeginPeriod(timerResolution);
        return true;
    }
    return false;
}

#endif
