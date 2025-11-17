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
#ifndef LIBSHM_KEY_VALUE_H
#define LIBSHM_KEY_VALUE_H

#include "libshm_variant.h"
#include "buffer_controller.h"
#include <stdint.h>
#include <map>

namespace tvushm
{
	class KeyValParam
	{
	public:
		typedef uint32_t Key;
		typedef Variant Value;
		typedef std::map<Key,Value> KeyValMap;
	public:
		KeyValParam(void);
		virtual ~KeyValParam(void);
    public:
        bool IsEmpty(void) const;
		void Clear();
        Variant&GetParameter(Key key);
		const Variant&GetParameter(Key key) const;
        bool HasParameter(Key key) const;
		void SetParameter(Key key,const Value&value);
		void SetParameter(Key key,Value&value);
		void SetParameterByReference(Key key,Value&value);

    public:
        int AppendToBuffer(BufferController_t &buffer,bool noPrepare=false);
		int ExtractFromBuffer(BufferController_t&buffer,bool referenceOnly=false);

    public:
        void SetParamAsU8(Key k, uint8_t v);
        void SetParamAsU16(Key k, uint16_t v);
        void SetParamAsU32(Key k, uint32_t v);
        void SetParamAsU64(Key k, uint64_t v);
        void SetParamAsU128(Key k, const uint128_t &v);
        void SetParamAsString(Key k, const std::string &v);
        void SetParamAsBytes(Key k, const Bytes &v);
        void SetParamAsBytes(Key k, const uint8_t *p, uint32_t n);
        void SetParamAsBytesReference(Key k, const uint8_t *p, uint32_t n);
    public:
        int GetCompactBytesNumNeeded(void);
    private:
		const Variant&GetDefaultValue() const;
		Variant&GetDefaultValue();
    private:
		KeyValMap _paramMap;/* store key-value */
        //BufferController_t _buf;/* store binary */
	};
}

#endif // LIBSHM_KEY_VALUE_H
