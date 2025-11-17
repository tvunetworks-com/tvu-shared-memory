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

#include "libshm_uint128.h"

#ifdef TVU_WINDOWS
#include <rpc.h>
#elif defined(TVU_ANDROID)
#elif defined(TVU_MINI)
#elif defined(TVU_LINUX)
//#include <uuid/uuid.h>
#endif

namespace tvushm
{
	Uint128::Uint128(void)
	{
		namedQwords.lowQword=0;
		namedQwords.highQword=0;
	}

	Uint128::~Uint128(void)
	{
	}

	bool Uint128::operator<(const uint128_t&value) const
	{
		if (namedQwords.highQword<value.namedQwords.highQword)
		{
			return true;
		}
		if (namedQwords.highQword>value.namedQwords.highQword)
		{
			return false;
		}
		return namedQwords.lowQword<value.namedQwords.lowQword;
	}

	bool Uint128::operator>(const uint128_t&value) const
	{
		if (namedQwords.highQword>value.namedQwords.highQword)
		{
			return true;
		}
		if (namedQwords.highQword<value.namedQwords.highQword)
		{
			return false;
		}
		return namedQwords.lowQword>value.namedQwords.lowQword;
	}
	bool Uint128::operator<=(const uint128_t&value) const
	{
		if (namedQwords.highQword<value.namedQwords.highQword)
		{
			return true;
		}
		if (namedQwords.highQword>value.namedQwords.highQword)
		{
			return false;
		}
		return namedQwords.lowQword<=value.namedQwords.lowQword;
	}

	bool Uint128::operator>=(const uint128_t&value) const
	{
		if (namedQwords.highQword>value.namedQwords.highQword)
		{
			return true;
		}
		if (namedQwords.highQword<value.namedQwords.highQword)
		{
			return false;
		}
		return namedQwords.lowQword>=value.namedQwords.lowQword;
	}

	bool Uint128::operator==(const uint128_t&value) const
	{
		return namedQwords.lowQword==value.namedQwords.lowQword &&
			namedQwords.highQword==value.namedQwords.highQword;
	}

	bool Uint128::operator!=(const uint128_t&value) const
	{
		return namedQwords.lowQword!=value.namedQwords.lowQword ||
			namedQwords.highQword!=value.namedQwords.highQword;
	}

	bool Uint128::IsZero(void) const
	{
		return namedQwords.lowQword==0 &&
			namedQwords.highQword==0;
	}

	Uint128&Uint128::SetBytes(const byte*inBytes)
	{
		for (int i=0;i<16;i++)
		{
			bytes[i]=inBytes[i];
		}
		return *this;
	}
	Uint128&Uint128::SetBytesRev(const byte*inBytes)
	{
		for (int i=0;i<16;i++)
		{
			bytes[i]=inBytes[15-i];
		}
		return *this;
	}

//#ifdef TVU_WINDOWS
//	Uint128&Uint128::SetAsUuid(void)
//	{
//		UUID uuid={0};
//		UuidCreate(&uuid);
//		if (ByteUtils::IsLittleEndian())
//		{
//			SetBytes((unsigned char*)&uuid);
//		}
//		else
//		{
//			SetBytesRev((unsigned char*)&uuid);
//		}
//		return *this;
//	}
//#elif defined(TVU_ANDROID) || defined(TVU_MINI) || defined(TVU_LINUX) || defined(TVU_MAC) || defined(TVU_IOS)
//	Uint128&Uint128::SetAsUuid(void)
//	{
//		unsigned char uuid[16];
//		MemUtils::RandomFill(uuid,sizeof(uuid));
//		if (ByteUtils::IsLittleEndian())
//		{
//			SetBytes(uuid);
//		}
//		else
//		{
//			SetBytesRev(uuid);
//		}
//		return *this;
//	}
//#endif
	void Uint128::Reset(void)
	{
		namedQwords.lowQword=0;
		namedQwords.highQword=0;
	}

	void Uint128::SetValue(const uint128_t&value)
	{
		namedQwords.lowQword=value.namedQwords.lowQword;
		namedQwords.highQword=value.namedQwords.highQword;
	}
}
