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
#include <string>
#include <sstream>

namespace tvushm
{
    class FormatUtils;

    class Formatter
    {
    public:
        Formatter();
        ~Formatter();
        Formatter(const std::string&initValue);
        Formatter(const char* initValue);

        Formatter& operator<<(bool value);
        Formatter& operator<<(char value);
        Formatter& operator<<(const char* value);
        Formatter& operator<<(const std::string& value);
        Formatter& operator<<(unsigned char value);
        Formatter& operator<<(unsigned short value);
        Formatter& operator<<(int TVU_W64 value);
        Formatter& operator<<(unsigned int TVU_W64 value);
        Formatter& operator<<(long value);
        Formatter& operator<<(unsigned long TVU_W64 value);
        Formatter& operator<<(longlong value);
        Formatter& operator<<(ulonglong value);
        Formatter& operator<<(float value);
        Formatter& operator<<(double value);
        Formatter& operator<<(long double value);
        Formatter& operator<<(const Formatter&formatter);
        Formatter& operator<<(byte* value);

        const char*c_str();
        bool IsEmpty() const;

        Formatter&MakeWritable() const;
        Formatter& AppendString(const char*buffer,size32 length);

    private:
        char    _formatStr[1024];
    };
}
