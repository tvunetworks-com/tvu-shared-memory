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
#include "libshm_media_struct.h"
#include "libshm_media_protocol.h"

namespace libshmmediapro {
    static inline int get_length_from_buffer(const uint8_t *pData, int &len)
    {
        uint8_t temp = 0;
        int offset = 0;
        len = 0;
        do
        {
            temp = pData[offset];
            len += temp;
            offset++;
        }while (temp == 0xFF);

        return offset;
    }

    static inline int write_length_to_buffer(uint8_t *pData, int len)
    {
        uint8_t temp = 0;
        int offset = 0;
        do
        {
            if (len >= 0xFF)
            {
                temp = 0xFF;
            }
            else
            {
                temp = len;
            }
            pData[offset] = temp;
            len -= temp;
            offset++;
        }while (temp == 0xFF);

        return offset;
    }
}
