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
#ifndef UINT128_H
#define UINT128_H
#include "common_define.h"
#include "uint128_t.h"

#ifdef Uint128
#undef Uint128
#endif

namespace tvushm
{
	class Uint128
		:public uint128_t
	{
	public:
		Uint128(void);
		~Uint128(void);
	public:
		bool operator<(const uint128_t&value) const;
		bool operator>(const uint128_t&value) const;
		bool operator<=(const uint128_t&value) const;
		bool operator>=(const uint128_t&value) const;
		bool operator==(const uint128_t&value) const;
		bool operator!=(const uint128_t&value) const;
		bool IsZero(void) const;
		Uint128&SetBytes(const byte*inBytes);
		Uint128&SetBytesRev(const byte*inBytes);
		//Uint128&SetAsUuid(void);
		void SetValue(const uint128_t&value);
		void Reset(void);
	};
}

#endif // UINT128_H
