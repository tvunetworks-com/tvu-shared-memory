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

#include "TvuFormatUtils.h"
#include "TvuMemUtils.h"
#include <stdio.h>
#include <string.h>

namespace tvushm
{
    Formatter::Formatter()
    {
        memset(_formatStr, 0, sizeof(_formatStr));
    }

    Formatter::Formatter(const std::string&initValue)
    {
        memset(_formatStr, 0, sizeof(_formatStr));
        (*this)<<initValue;
    }

    Formatter::Formatter(const char* initValue)
    {
        memset(_formatStr, 0, sizeof(_formatStr));
        (*this)<<initValue;
    }


    Formatter::~Formatter()
    {
    }

    Formatter& Formatter::operator<<(bool value)
    {
        if (value)
        {
            {
            size_t n = strlen(_formatStr);
            snprintf(_formatStr+n, sizeof(_formatStr)-1-n, "true");
            n = strlen(_formatStr);
            _formatStr[n] = 0;
            }
        }
        else
        {
            {
            size_t n = strlen(_formatStr);
            snprintf(_formatStr+n, sizeof(_formatStr)-1-n, "false");
            n = strlen(_formatStr);
            _formatStr[n] = 0;
            }
        }
        return *this;
    }

    Formatter& Formatter::operator<<(char value)
    {
        {
        size_t n = strlen(_formatStr);
        snprintf(_formatStr+n, sizeof(_formatStr)-1-n, "%c", value);
        n = strlen(_formatStr);
        _formatStr[n] = 0;
        }
        return *this;
    }

    Formatter& Formatter::operator<<(const char* value)
    {
        const char *p = value;

        if (!p)
        {
            p = "";
        }

        {
        size_t n = strlen(_formatStr);
        snprintf(_formatStr+n, sizeof(_formatStr)-1-n, "%s", p);
        n = strlen(_formatStr);
        _formatStr[n] = 0;
        }
        return *this;
    }

    Formatter& Formatter::operator<<(const std::string& value)
    {
        {
        size_t n = strlen(_formatStr);
        snprintf(_formatStr+n, sizeof(_formatStr)-1-n, "%s", value.c_str());
        n = strlen(_formatStr);
        _formatStr[n] = 0;
        }
        return *this;
    }

    Formatter& Formatter::operator<<(unsigned char value)
    {
        {
        size_t n = strlen(_formatStr);
        snprintf(_formatStr+n, sizeof(_formatStr)-1-n, "%hhu", value);
        n = strlen(_formatStr);
        _formatStr[n] = 0;
        }
        return *this;
    }

    Formatter& Formatter::operator<<(unsigned short value)
    {
        {
        size_t n = strlen(_formatStr);
        snprintf(_formatStr+n, sizeof(_formatStr)-1-n, "%hu", value);
        n = strlen(_formatStr);
        _formatStr[n] = 0;
        }
        return *this;
    }

    Formatter& Formatter::operator<<(int TVU_W64 value)
    {
        {
        size_t n = strlen(_formatStr);
        snprintf(_formatStr+n, sizeof(_formatStr)-1-n, "%d", value);
        n = strlen(_formatStr);
        _formatStr[n] = 0;
        }
        return *this;
    }

    Formatter& Formatter::operator<<(unsigned int TVU_W64 value)
    {
        {
        size_t n = strlen(_formatStr);
        snprintf(_formatStr+n, sizeof(_formatStr)-1-n, "%u", value);
        n = strlen(_formatStr);
        _formatStr[n] = 0;
        }
        return *this;
    }

    Formatter& Formatter::operator<<(long value)
    {
        {
        size_t n = strlen(_formatStr);
        snprintf(_formatStr+n, sizeof(_formatStr)-1-n, "%ld", value);
        n = strlen(_formatStr);
        _formatStr[n] = 0;
        }
        return *this;
    }

    Formatter& Formatter::operator<<(unsigned long TVU_W64 value)
    {
        {
        size_t n = strlen(_formatStr);
        snprintf(_formatStr+n, sizeof(_formatStr)-1-n, "%lu", value);
        n = strlen(_formatStr);
        _formatStr[n] = 0;
        }
        return *this;
    }

    Formatter& Formatter::operator<<(longlong value)
    {
        {
        size_t n = strlen(_formatStr);
        snprintf(_formatStr+n, sizeof(_formatStr)-1-n, "%lld", value);
        n = strlen(_formatStr);
        _formatStr[n] = 0;
        }
        return *this;
    }

    Formatter& Formatter::operator<<(ulonglong value)
    {
        {
        size_t n = strlen(_formatStr);
        snprintf(_formatStr+n, sizeof(_formatStr)-1-n, "%llu", value);
        n = strlen(_formatStr);
        _formatStr[n] = 0;
        }
        return *this;
    }

    Formatter& Formatter::operator<<(float value)
    {
        {
        size_t n = strlen(_formatStr);
        snprintf(_formatStr+n, sizeof(_formatStr)-1-n, "%6.1f", value);
        n = strlen(_formatStr);
        _formatStr[n] = 0;
        }
        return *this;
    }

    Formatter& Formatter::operator<<(double value)
    {
        {
        size_t n = strlen(_formatStr);
        snprintf(_formatStr+n, sizeof(_formatStr)-1-n, "%6.3lf", value);
        n = strlen(_formatStr);
        _formatStr[n] = 0;
        }
        return *this;
    }

    Formatter& Formatter::operator<<(long double value)
    {
        {
        size_t n = strlen(_formatStr);
        snprintf(_formatStr+n, sizeof(_formatStr)-1-n, "%6.3Lf", value);
        n = strlen(_formatStr);
        _formatStr[n] = 0;
        }
        return *this;
    }

    Formatter& Formatter::operator<<(byte* value)
    {
        {
        size_t n = strlen(_formatStr);
        snprintf(_formatStr+n, sizeof(_formatStr)-1-n, "%p", (void *)value);
        n = strlen(_formatStr);
        _formatStr[n] = 0;
        }
        return *this;
    }

    Formatter& Formatter::AppendString(const char*buffer,size32 length)
    {
        for (size32 i=0;i<length;i++)
        {
            (*this)<<buffer[i];
        }
        return *this;
    }

    Formatter& Formatter::operator<<(const Formatter&formatter)
    {
        if (&formatter!=this)
        {
            Formatter&writableFormatter=const_cast<Formatter&>(formatter);
            (*this)<<writableFormatter.c_str();
        }
        return *this;
    }

    const char*Formatter::c_str()
    {
        return _formatStr;
    }

    Formatter&Formatter::MakeWritable() const
    {
        return const_cast<Formatter&>(*this);
    }

    bool Formatter::IsEmpty() const
    {
        return !_formatStr[0];
    }
}
