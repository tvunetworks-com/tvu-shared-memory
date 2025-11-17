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

#include "TvuLog.h"
#include <stdio.h>

namespace tvushm
{
    Log::Log(void)
    {
        _level = InfoLevel;
    }

    Log::Log(Level level)
    {
        _level=level;
    }

    Log::~Log(void)
    {
    }

    void Log::SetLevel(Level level)
    {
        _level=level;
    }

    Log::Level Log::GetLevel(void)
    {
        return _level;
    }

    bool Log::IsLevelEnabled(Level level)
    {
        if (level>GetLevel())
        {
            return false;
        }
        return true;
    }

    bool Log::IsSystemEnabled(void)
    {
        return IsLevelEnabled(SystemLevel);
    }

    bool Log::IsErrorEnabled(void)
    {
        return IsLevelEnabled(ErrorLevel);
    }

    bool Log::IsWarnEnabled(void)
    {
        return IsLevelEnabled(WarnLevel);
    }

    bool Log::IsInfoEnabled(void)
    {
        return IsLevelEnabled(InfoLevel);
    }

    bool Log::IsDebugEnabled(void)
    {
        return IsLevelEnabled(DebugLevel);
    }

    bool Log::IsArchiveEnabled(void)
    {
        return IsLevelEnabled(ArchiveLevel);
    }

    bool Log::IsRawEnabled(void)
    {
        return IsLevelEnabled(RawLevel);
    }

    void Log::LogMessage(Level level,const char*message)
    {
        LogMessage(level, Log::_logTag.empty()?NULL:Log::_logTag.c_str(),message);
    }

    void Log::LogMessage(Level level,const char*tag,const char*message)
    {
        if (!IsLevelEnabled(level))
        {
            return;
        }

        if (_dumpLogCallback)
        {
            int iv = LevelMapInteger(level);
            _dumpLogCallback(iv, "%s%s%s%s"
                , tag?"T[":""
                , tag?tag:""
                , tag?"]-":""
                , message);
        }
    }

    void Log::VerboseLog(Level level,const char*message)
    {
        VerboseLogMessage(level,message);
    }

    void Log::VerboseLog(Level level,const std::string& message)
    {
        VerboseLogMessage(level,message.c_str());
    }

    void Log::VerboseLogMessage(Level level,const char*message)
    {
        if (!IsLevelEnabled(level))
        {
            return;
        }

        LogMessage(level,message);
    }

    Log&Log::GetDefaultLog()
    {
        static Log defaultLog;
        return defaultLog;
    }

    int Log::LevelMapInteger(Log::Level val)
    {
        int i = 'd';
        switch (val) {
        case InfoLevel:
        {
            i = 'i';
            break;
        }
        case SystemLevel:
        {
            i = 's';
            break;
        }
        case WarnLevel:
        {
            i = 'w';
            break;
        }
        case ErrorLevel:
        {
            i = 'e';
            break;
        }
        case ArchiveLevel:
        {
            i = 'a';
            break;
        }
        case RawLevel:
        {
            i = 'r';
            break;
        }
        default:
        {
            i = 'D';
            break;
        }
        }

        return i;
    }

    void Log::SetLogCallback(Log::DumpLogCallback cb)
    {
        Log::_dumpLogCallback = cb;
    }

    void Log::SetLogTag(const char *tag)
    {
        if (tag)
        {
            Log::_logTag = tag;
        }
    }

    Log::DumpLogCallback Log::_dumpLogCallback;
    std::string Log::_logTag;
}

