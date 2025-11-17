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

Purpose: Foundation Library Implementation File
Modifiers: Tony Guo (tonyguo@tvunetworks.com)
History: Tony Guo on Feb 11, 2014: Created
*/

#include "TvuTimeUtils.h"
#include "TvuMemUtils.h"
#include "TvuFoundationErrorCodes.h"
#include "TvuCommonDefines.h"

#if defined(TVU_MAC)
#include <mach/mach_init.h>
#include <mach/thread_act.h>
#include <mach/mach_port.h>
#endif

#if defined(TVU_IOS)
#include <mach/mach.h>
#endif

#if defined (TVU_LINUX)
#include <sys/time.h>
#include <time.h>
#endif

#if defined (TVU_WINDOWS)
#include <Windows.h>
#include <profileapi.h>
#endif

namespace tvushm
{
    class SystemUtils
    {
    public:
        static inline bool IsSingleThreaded(void)
        {
            return false;
        }
    };

    TimeUtils::TimeUtils(void)
    {
    }

    TimeUtils::~TimeUtils(void)
    {
    }

    ErrorCode TimeUtils::GetCurrentTimeInUsec(uint64 *pTime)
    {
        if ( pTime == 0 )
            return TVU_ERROR_NULL_POINTER;

#ifdef TVU_WINDOWS
        LARGE_INTEGER perfFreq={};
        LARGE_INTEGER perfCount={};
        ULONGLONG secs=0;
        ULONGLONG usecs=0;
        if (!QueryPerformanceFrequency(&perfFreq))
        {
            *pTime=0;
            return TVU_ERROR_FAILED;
        }
        if (!QueryPerformanceCounter(&perfCount))
        {
            *pTime=0;
            return TVU_ERROR_FAILED;
        }
        if (perfFreq.QuadPart==0)
        {
            *pTime=0;
            return TVU_ERROR_FAILED;
        }
        secs = (perfCount.QuadPart / perfFreq.QuadPart) * 1000000;
        usecs = (perfCount.QuadPart % perfFreq.QuadPart) * 1000000 / perfFreq.QuadPart;
        *pTime = secs + usecs;
#elif defined(TVU_ANDROID) || defined(TVU_LINUX) || defined(TVU_MINI) || defined(TVU_MAC) || defined(TVU_IOS)
        int retVal;
        struct timeval tv;
        retVal = gettimeofday( &tv, NULL);
        if ( retVal < 0 )
        {
            *pTime=0;
            return TVU_ERROR_FAILED;
        }

        *pTime = tv.tv_sec;
        *pTime *= 1000000;
        *pTime += tv.tv_usec;
#endif
        return 0;
    }

    uint64 TimeUtils::GetUncachedCurrentTimeInUsec(void)
    {
        //TVU_TRACK_CALLSTACK_OCCURRENCE("total");
        uint64 now=0;
        GetCurrentTimeInUsec(&now);
        _cachedNowPreVersion=now;
        _cachedNowInUsec=now;
        _cachedNowPostVersion=now;
        _cachedNowUpdatedTimes++;
        return now;
    }

    uint64 TimeUtils::GetCurrentTimeInUsec(void)
    {
        if (TimeUtils::CachedCurrentTimeController::effectiveFlags==0)
        {
            return GetUncachedCurrentTimeInUsec();
        }
        else
        {
            if (TimeUtils::CachedCurrentTimeController::isCachedTimeEffective)
            {
                _cachedTimeUsedTimes++;
                if (_cachedTimeUsedTimes>=1000)
                {
                    _cachedTimeUsedTimes=0;
                    return GetUncachedCurrentTimeInUsec();
                }
                else
                {
                    return GetCachedCurrentTimeInUsec();
                }
            }
            else
            {
                uint64 now=GetUncachedCurrentTimeInUsec();
                TimeUtils::CachedCurrentTimeController::isCachedTimeEffective=true;
                if (_cachedTimeUsedTimes!=0)
                {
                    _cachedTimeUsedTimes=0;
                }
                return now;
            }
        }
    }

    uint64 TimeUtils::GetStartedTimeInUsec(void)
    {
        return _firstTimeInUsec;
    }

    uint64 TimeUtils::GetSystemTimeInUsec(void)
    {
        return GetElapsedTimeSinceEpochInUsec();
    }

    uint64 TimeUtils::GetCachedCurrentTimeInUsec(void)
    {
        if (SystemUtils::IsSingleThreaded())
        {
            return _cachedNowInUsec;
        }
        uint64 now=_cachedNowInUsec;
        if (now==_cachedNowPreVersion && now==_cachedNowPostVersion)
        {
            //untouched;
            return now;
        }
        else
        {
            return GetUncachedCurrentTimeInUsec();
        }
    }

    uint64 TimeUtils::GetFastCachedCurrentTimeInUsec(void)
    {
        return _cachedNowInUsec;
    }

    uint64 TimeUtils::GetCurrentTimeInUsecQuickly(void)
    {
#if defined(TVU_WINDOWS)
        clock_t currentClock=clock();
        if (currentClock==_cachedClock)
        {
            return _cachedNowInUsec;
        }
        _cachedClock=currentClock;
#endif
        return GetCurrentTimeInUsec();
    }

    uint64 TimeUtils::GetCurrentThreadCpuTimeInNanoSec(void)
    {
#ifdef TVU_WINDOWS
        uint64 creationTime=0;
        uint64 exitTime=0;
        uint64 kernelTime=0;
        uint64 userTime=0;
        ::GetThreadTimes(::GetCurrentThread(),
            (LPFILETIME)&creationTime,(LPFILETIME)&exitTime,(LPFILETIME)&kernelTime,(LPFILETIME)&userTime);
        return (kernelTime+userTime)*100;

#elif defined(TVU_MAC) || defined(TVU_IOS)
        mach_port_t thread;
        kern_return_t kr;
        mach_msg_type_number_t count;
        thread_basic_info_data_t info;
        uint64 total_ns = 0;

        thread = mach_thread_self();
        count = THREAD_BASIC_INFO_COUNT;
        kr = thread_info(thread, THREAD_BASIC_INFO, (thread_info_t) &info, &count);

        if (kr == KERN_SUCCESS && (info.flags & TH_FLAGS_IDLE) == 0) {
            total_ns += info.user_time.seconds*TVU_1KMLL;
            total_ns += info.user_time.microseconds*TVU_1KLL;
            total_ns += info.system_time.seconds*TVU_1KMLL;
            total_ns += info.system_time.microseconds*TVU_1KLL;
        }else {
            // should not happen
            //printf("Could not retreive thread info.");
        }

        mach_port_deallocate(mach_task_self(), thread);
        return total_ns;

#elif defined(TVU_ANDROID) || defined(TVU_LINUX) || defined(TVU_MINI)
        struct timespec ts;
        MemUtils::Initialize(ts);
        clock_gettime(CLOCK_THREAD_CPUTIME_ID,&ts);
        return (ts.tv_sec*TVU_1KMLL)+(ts.tv_nsec);
#endif
    }

    bool TimeUtils::IsTimeInUsecExpired(uint64 timeInUsec,uint64 expireInUsec)
    {
        uint64 now=GetCurrentTimeInUsec();
        return IsTimeInUsecExpired(timeInUsec,expireInUsec,now);
    }

    bool TimeUtils::IsTimeInUsecExpired(uint64 timeInUsec,uint64 expireInUsec,uint64 nowInUsec)
    {
        return nowInUsec>=timeInUsec+expireInUsec;
    }

    bool TimeUtils::IsTimeInUsecExpiredStrictly(uint64 timeInUsec,uint64 expireInUsec)
    {
        uint64 now=GetCurrentTimeInUsec();
        return IsTimeInUsecExpiredStrictly(timeInUsec,expireInUsec,now);
    }

    bool TimeUtils::IsTimeInUsecExpiredStrictly(uint64 timeInUsec,uint64 expireInUsec,uint64 nowInUsec)
    {
        return nowInUsec>=timeInUsec+expireInUsec || timeInUsec>nowInUsec+TVU_MSEC2USEC(3000);
    }

    bool TimeUtils::IsPeriodicRunnable(uint64& lastOccurTimeInUsec,uint64 periodInUsec)
    {
        uint64 now;
        if (periodInUsec>=TVU_MSEC2USEC(1))
        {
            now=TimeUtils::GetCurrentTimeInUsecQuickly();
        }
        else
        {
            now=TimeUtils::GetCurrentTimeInUsec();
        }
        if (IsTimeInUsecExpiredStrictly(lastOccurTimeInUsec,periodInUsec,now))
        {
            lastOccurTimeInUsec=now;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool TimeUtils::IsPeriodicRunnableLikely(uint64& lastOccurTimeInUsec,uint64 periodInUsec)
    {
        if (!IsTimeInUsecExpiredStrictly(lastOccurTimeInUsec,periodInUsec,_cachedNowInUsec))
        {
            return false;
        }
        return IsPeriodicRunnable(lastOccurTimeInUsec,periodInUsec,GetCachedCurrentTimeInUsec());
    }

    bool TimeUtils::IsPeriodicRunnable(uint64& lastOccurTimeInUsec,uint64 periodInUsec,uint64 nowInUsec)
    {
        if (IsTimeInUsecExpiredStrictly(lastOccurTimeInUsec,periodInUsec,nowInUsec))
        {
            lastOccurTimeInUsec=nowInUsec;
            return true;
        }
        else
        {
            return false;
        }
    }

#ifdef TVU_WINDOWS
    uint64 TimeUtils::GetElapsedTimeSinceEpochInUsec(void)
    {
        struct __timeb64 timeb;
        MemUtils::Initialize(timeb);
        _ftime64_s(&timeb);
        return timeb.time*TVU_1MLL+timeb.millitm*TVU_1KLL;
    }
#elif defined(TVU_ANDROID) || defined(TVU_LINUX) || defined(TVU_MINI) || defined(TVU_MAC) || defined(TVU_IOS)

    uint64 TimeUtils::GetElapsedTimeSinceEpochInUsec(void)
    {
        int retVal;
        struct timeval tv;
        retVal = gettimeofday( &tv, NULL );
        if ( retVal < 0 )
        {
            return 0;
        }
        return tv.tv_sec*TVU_1MLL+tv.tv_usec;
    }
#endif

    int64 TimeUtils::GetSignedElapsedTimeInUsec(uint64 beginTimeInUsec,uint64 endTimeInUsec)
    {
        return (int64)(endTimeInUsec-beginTimeInUsec);
    }

    int64 TimeUtils::GetSignedElapsedTimeInUsec(uint64 beginTimeInUsec)
    {
        return (int64)(TimeUtils::GetCurrentTimeInUsec()-beginTimeInUsec);
    }

    int64 TimeUtils::GetSignedCachedElapsedTimeInUsec(uint64 beginTimeInUsec)
    {
        return (int64)(TimeUtils::GetCachedCurrentTimeInUsec()-beginTimeInUsec);
    }


    TimeUtils::CachedCurrentTimeController::CachedCurrentTimeController(uintptr effectiveFlagMask)
    {
        _effectiveFlagMask=effectiveFlagMask;
        effectiveFlags|=_effectiveFlagMask;
        isCachedTimeEffective=false;
    }

    TimeUtils::CachedCurrentTimeController::~CachedCurrentTimeController()
    {
        effectiveFlags&=~_effectiveFlagMask;
    }

    uintptr TimeUtils::CachedCurrentTimeController::AllocateEffectiveMask()
    {
        uintptr mask=(uintptr)1<<(lastEffectiveFlagIndex%TVU_WORDSIZE);
        lastEffectiveFlagIndex++;
        return mask;
    }

    uint64 TimeUtils::GetCachedTimeUpdatedTimes()
    {
        return _cachedNowUpdatedTimes;
    }

    uint64 TimeUtils::GetSimulatedMonoTimeInUsec()
    {
        return _simulatedMonoTimeInUsec;
    }

    void TimeUtils::UpdateSimulatedMonoTimeInUsec(uint64 diffTimeInUsec)
    {
        _simulatedMonoTimeInUsec+=diffTimeInUsec;
    }

    //TVU_IMPLEMENT_STATIC_LOG(TimeUtils,_log);

    uint64 TimeUtils::_firstTimeInUsec=TimeUtils::GetUncachedCurrentTimeInUsec();
    uint64 TimeUtils::_cachedNowPreVersion=1;
    uint64 TimeUtils::_cachedNowInUsec=0;
    uint64 TimeUtils::_cachedNowPostVersion=2; //make it different on purpose
    uint64 TimeUtils::_cachedNowUpdatedTimes=0;
    uint64 TimeUtils::_simulatedMonoTimeInUsec=0;


    clock_t TimeUtils::_cachedClock=0;

    uint32 TimeUtils::_cachedTimeUsedTimes=0;

    uint32 TimeUtils::CachedCurrentTimeController::lastEffectiveFlagIndex=0;
    uintptr TimeUtils::CachedCurrentTimeController::effectiveFlags=0;
    bool TimeUtils::CachedCurrentTimeController::isCachedTimeEffective=false;
}
