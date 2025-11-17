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

#ifdef Log
#undef Log
#endif

#include <string>

#define TVU_MAX_CACHED_LOG_LINES_NUM                    (100*1000)

namespace tvushm
{
    class Log
    {
    public:
        enum Level
        {
            UnknownLevel,
            SystemLevel,
            ErrorLevel,
            WarnLevel,
            InfoLevel,
            DebugLevel,
            ArchiveLevel,
            RawLevel
        };

        typedef int (*DumpLogCallback)(int level,const char*message, ...);

    public:
        Log(void);
        Log(Level level);
        virtual ~Log(void);
    public:
        void SetLevel(Level level);
        Level GetLevel(void);
        bool IsLevelEnabled(Level level);
    public:
        bool IsSystemEnabled(void);
        bool IsErrorEnabled(void);
        bool IsWarnEnabled(void);
        bool IsInfoEnabled(void);
        bool IsDebugEnabled(void);
        bool IsArchiveEnabled(void);
        bool IsRawEnabled(void);
        static Log&GetDefaultLog();

    public:
        void VerboseLog(Level level,const char*message);
        void VerboseLog(Level level,const std::string& message);

    protected:
        static DumpLogCallback  _dumpLogCallback;
        static std::string      _logTag;
        static int LevelMapInteger(Level val);
    public:
        static void SetLogCallback(DumpLogCallback cb);
        static void SetLogTag(const char *tag);
    protected:
        void LogMessage(Level level,const char*tag,const char*message);
        void LogMessage(Level level,const char*message);
        void VerboseLogMessage(Level level,const char*message);
    private:
        Level _level;
    };
}

#define TVU_DIRECT_LOG(log,level,message) \
    do \
    {\
        tvushm::Formatter __tvu__formatter; \
        __tvu__formatter << "[" << __FUNCTION__ << "," << __LINE__ << "]" << (message) << "\n"; \
        (log).VerboseLog(level,(__tvu__formatter).c_str()); \
    }while(0)

#define TVU_LOG_VERBOSE(log,message) \
    do \
    {\
        if ((log).IsLevelEnabled(tvu::Log::DebugLevel) && (log).IsDetailEnabled()) \
        { \
            TVU_DIRECT_LOG(log,tvu::Log::DebugLevel,message); \
        } \
    }while(0)

#define TVU_LOG(log,level,message) \
    do \
    {\
        if ((log).IsLevelEnabled(level)) \
        { \
            TVU_DIRECT_LOG(log,level,message); \
        } \
    }while(0)

#define TVU_LOG_DEBUG(log,message) TVU_LOG(log,tvushm::Log::DebugLevel,message)
#define TVU_LOG_INFO(log,message) TVU_LOG(log,tvushm::Log::InfoLevel,message)
#define TVU_LOG_WARN(log,message) TVU_LOG(log,tvushm::Log::WarnLevel,message)
#define TVU_LOG_ERROR(log,message) TVU_LOG(log,tvushm::Log::ErrorLevel,message)
#define TVU_LOG_SYSTEM(log,message) TVU_LOG(log,tvushm::Log::SystemLevel,message)
#define TVU_LOG_ARCHIVE(log,message) TVU_LOG(log,tvushm::Log::ArchiveLevel,message)
#define TVU_LOG_RAW(log,message) TVU_LOG(log,tvu::tvushm::RawLevel,message)


#define _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(tagName,seconds,maxIntervalCount,logName,level,message) \
    do \
    {\
        tvushm::uint64 _tvu_now; \
        tvushm::TimeUtils::GetCurrentTimeInUsec(&_tvu_now);\
        bool boutLog = false; \
        static tvushm::uint64 _tvu_lastTime_##__LINE__ = 0;\
        static tvushm::uint64 _tvu_counts_##__LINE__ = 0;\
        if (_tvu_now>=_tvu_lastTime_##__LINE__+TVU_SEC2USEC(seconds)) { \
            boutLog = true; _tvu_lastTime_##__LINE__=_tvu_now; _tvu_counts_##__LINE__ = 0;\
        }  else {\
            if (_tvu_counts_##__LINE__ < maxIntervalCount) {\
                boutLog = true;\
            }\
        } \
        if (boutLog) {\
            TVU_LOG(logName,level,message);\
            _tvu_counts_##__LINE__++;\
        } \
    } while (0)


#define TVU_HEARTBEAT_DEBUG(seconds,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(NULL,seconds,1,logName,tvushm::Log::DebugLevel,message)

#define TVU_HEARTBEAT_INFO(seconds,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(NULL,seconds,1,logName,tvushm::Log::InfoLevel,message)

#define TVU_HEARTBEAT_WARN(seconds,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(NULL,seconds,1,logName,tvushm::Log::WarnLevel,message)

#define TVU_HEARTBEAT_SYSTEM(seconds,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(NULL,seconds,1,logName,tvushm::Log::SystemLevel,message)

#define TVU_HEARTBEAT_ERROR(seconds,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(NULL,seconds,1,logName,tvushm::Log::ErrorLevel,message)

#define TVU_HEARTBEAT_LOG(seconds,logName,level,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(NULL,seconds,1,logName,level,message)

#define TVU_TAGGED_HEARTBEAT_MULTIPLE_DEBUG(tag,seconds,lines,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(tag,seconds,lines,logName,tvushm::Log::DebugLevel,message)

#define TVU_TAGGED_HEARTBEAT_MULTIPLE_INFO(tag,seconds,lines,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(tag,seconds,lines,logName,tvushm::Log::InfoLevel,message)

#define TVU_TAGGED_HEARTBEAT_MULTIPLE_WARN(tag,seconds,lines,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(tag,seconds,lines,logName,tvushm::Log::WarnLevel,message)

#define TVU_TAGGED_HEARTBEAT_MULTIPLE_SYSTEM(tag,seconds,lines,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(tag,seconds,lines,logName,tvushm::Log::SystemLevel,message)

#define TVU_TAGGED_HEARTBEAT_MULTIPLE_ERROR(tag,seconds,lines,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(tag,seconds,lines,logName,tvushm::Log::ErrorLevel,message)

#define TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(tag,seconds,lines,logName,level,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(tag,seconds,lines,logName,level,message)

#define TVU_HEARTBEAT_MULTIPLE_DEBUG(seconds,lines,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(NULL,seconds,lines,logName,tvushm::Log::DebugLevel,message)

#define TVU_HEARTBEAT_MULTIPLE_INFO(seconds,lines,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(NULL,seconds,lines,logName,tvushm::Log::InfoLevel,message)

#define TVU_HEARTBEAT_MULTIPLE_WARN(seconds,lines,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(NULL,seconds,lines,logName,tvushm::Log::WarnLevel,message)

#define TVU_HEARTBEAT_MULTIPLE_SYSTEM(seconds,lines,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(NULL,seconds,lines,logName,tvushm::Log::SystemLevel,message)

#define TVU_HEARTBEAT_MULTIPLE_ERROR(seconds,lines,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(NULL,seconds,lines,logName,tvushm::Log::ErrorLevel,message)

#define TVU_HEARTBEAT_MULTIPLE_LOG(seconds,lines,logName,level,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(NULL,seconds,lines,logName,level,message)

#define TVU_TAGGED_HEARTBEAT_DEBUG(tag,seconds,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(tag,seconds,1,logName,tvushm::Log::DebugLevel,message)

#define TVU_TAGGED_HEARTBEAT_INFO(tag,seconds,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(tag,seconds,1,logName,tvushm::Log::InfoLevel,message)

#define TVU_TAGGED_HEARTBEAT_WARN(tag,seconds,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(tag,seconds,1,logName,tvushm::Log::WarnLevel,message)

#define TVU_TAGGED_HEARTBEAT_SYSTEM(tag,seconds,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(tag,seconds,1,logName,tvushm::Log::SystemLevel,message)

#define TVU_TAGGED_HEARTBEAT_ERROR(tag,seconds,logName,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(tag,seconds,1,logName,tvushm::Log::ErrorLevel,message)

#define TVU_TAGGED_HEARTBEAT_LOG(tag,seconds,logName,level,message) \
    _TVU_TAGGED_HEARTBEAT_MULTIPLE_LOG(tag,seconds,1,logName,level,message)




#define TVU_TAGGED_LOG(tag,logName,level,message) \
    do \
    {\
        if ((logName).IsLevelEnabled(level)) \
        { \
            TVU_DIRECT_LOG(logName,level,tvushm::Formatter()\
                << "--[" << tag << "]--" \
                <<message); \
        } \
    }while(0)

#define TVU_TAGGED_DEBUG(tag,log,message) TVU_TAGGED_LOG(tag,log,tvushm::Log::DebugLevel,message)
#define TVU_TAGGED_INFO(tag,log,message) TVU_TAGGED_LOG(tag,log,tvushm::Log::InfoLevel,message)
#define TVU_TAGGED_WARN(tag,log,message) TVU_TAGGED_LOG(tag,log,tvushm::Log::WarnLevel,message)
#define TVU_TAGGED_ERROR(tag,log,message) TVU_TAGGED_LOG(tag,log,tvushm::Log::ErrorLevel,message)
#define TVU_TAGGED_SYSTEM(tag,log,message) TVU_TAGGED_LOG(tag,log,tvushm::Log::SystemLevel,message)
