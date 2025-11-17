/*********************************************************
 *  Copyright 2025 TVU Networks
 *  Licensed under the Apache License, Version 2.0 (the “License”);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *    http://www.apache.org/licenses/LICENSE-2.0
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an “AS IS” BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *********************************************************/
/*
Copyright 2005-2014, TVU networks. All rights reserved.
For internal members in TVU networks only.

Purpose: Foundation Library Header File
Modifiers: Tony Guo (tonyguo@tvunetworks.com)
History: Tony Guo on Feb 11, 2014: Created
*/

#pragma once
#include "TvuShmCrossPlatformExtDefines.h"
#include <time.h>

#if defined(TVU_WINDOWS)
#include <sys/timeb.h>
#include <time.h>
#endif

namespace tvushm
{
    class TimeUtils
    {
    public:
        class CachedCurrentTimeController
        {
        public:
            CachedCurrentTimeController(uintptr effectiveFlagMask);
            ~CachedCurrentTimeController();

        public:
            static uintptr AllocateEffectiveMask();
        public:
            uintptr _effectiveFlagMask;
            static uintptr effectiveFlags;
            static uint32 lastEffectiveFlagIndex;
            static bool isCachedTimeEffective;
        };
    public:
        static ErrorCode GetCurrentTimeInUsec(uint64 *timePtr);
        static uint64 GetCurrentTimeInUsec(void);
        static uint64 GetSystemTimeInUsec(void);
        static uint64 GetCachedCurrentTimeInUsec(void);
        static uint64 GetFastCachedCurrentTimeInUsec(void);
        static uint64 GetUncachedCurrentTimeInUsec(void);
        static uint64 GetStartedTimeInUsec(void);
        static uint64 GetCurrentTimeInUsecQuickly(void);
        static uint64 GetCurrentThreadCpuTimeInNanoSec(void); //nano seconds.
        static uint64 GetElapsedTimeSinceEpochInUsec(void); //Time in micro seconds since midnight (00:00:00), January 1, 1970, coordinated universal time (UTC).
        static int64 GetSignedElapsedTimeInUsec(uint64 beginTimeInUsec,uint64 endTimeInUsec);
        static int64 GetSignedElapsedTimeInUsec(uint64 beginTimeInUsec);
        static int64 GetSignedCachedElapsedTimeInUsec(uint64 beginTimeInUsec);
        static bool IsTimeInUsecExpired(uint64 timeInUsec,uint64 expireInUsec);
        static bool IsTimeInUsecExpired(uint64 timeInUsec,uint64 expireInUsec,uint64 nowInUsec);
        static bool IsTimeInUsecExpiredStrictly(uint64 timeInUsec,uint64 expireInUsec);
        static bool IsTimeInUsecExpiredStrictly(uint64 timeInUsec,uint64 expireInUsec,uint64 nowInUsec);
        static bool IsPeriodicRunnable(uint64& lastOccurTimeInUsec,uint64 periodInUsec);
        static bool IsPeriodicRunnable(uint64& lastOccurTimeInUsec,uint64 periodInUsec,uint64 nowInUsec);
        static bool IsPeriodicRunnableLikely(uint64& lastOccurTimeInUsec,uint64 periodInUsec);
        static uint64 GetCachedTimeUpdatedTimes();
        static uint64 GetSimulatedMonoTimeInUsec();
        static void UpdateSimulatedMonoTimeInUsec(uint64 diffTimeInUsec);
    private:
        TimeUtils(void);
        virtual ~TimeUtils(void);
    private:
        static uint64 _firstTimeInUsec;
        static uint64 _cachedNowPreVersion;
        static uint64 _cachedNowInUsec;
        static uint64 _cachedNowPostVersion;
        static uint64 _cachedNowUpdatedTimes;
        static clock_t _cachedClock;
        static uint32 _cachedTimeUsedTimes;
        static uint64 _simulatedMonoTimeInUsec;
//        TVU_DECLARE_STATIC_LOG(_log);
    };

#define TVU_HAS_LOCK_CURRENT_TIME

#ifdef TVU_HAS_LOCK_CURRENT_TIME
#define TVU_AUTO_LOCK_CURRENT_TIME() \
    static uintptr TVU_AUTO_NAME(_tvu_cctcf_)=tvu::TimeUtils::CachedCurrentTimeController::AllocateEffectiveMask(); \
    tvu::TimeUtils::CachedCurrentTimeController TVU_AUTO_NAME(_tvu_cctc_)(TVU_AUTO_NAME(_tvu_cctcf_))
#else
    #define TVU_AUTO_LOCK_CURRENT_TIME() \
        do{}while(0)
#endif
}
