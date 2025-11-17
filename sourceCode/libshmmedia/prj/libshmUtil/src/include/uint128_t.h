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
#ifndef UINT128_T_H
#define UINT128_T_H

#include "common_define.h"

#ifdef uint128_t
#undef uint128_t
#endif

namespace tvushm
{
	struct uint128_t
	{
		struct NamedBytes
		{
			byte byte0;
			byte byte1;
			byte byte2;
			byte byte3;
			byte byte4;
			byte byte5;
			byte byte6;
			byte byte7;
			byte byte8;
			byte byte9;
			byte byte10;
			byte byte11;
			byte byte12;
			byte byte13;
			byte byte14;
			byte byte15;
		};

		struct NamedDwords
		{
			uint32 dword0;
			uint32 dword1;
			uint32 dword2;
			uint32 dword3;
		};

		struct NamedWords
		{
			uint16 word0;
			uint16 word1;
			uint16 word2;
			uint16 word3;
		};

		struct NamedQwords
		{
			uint64 lowQword;
			uint64 highQword;
		};

		union
		{
			byte bytes[16];
			NamedBytes namedBytes;

			uint16 words[8];
			NamedWords namedWords;

			uint32 dwords[4];
			NamedDwords namedDwords;

			uint64 qwords[2];
			NamedQwords namedQwords;
		};
	};
}

#endif // UINT128_T_H
