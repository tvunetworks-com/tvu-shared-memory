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
History: Tony Guo on Oct 4, 2016: Created
*/

#pragma once
#include <string>
#include <stdint.h>
#include <algorithm>

#if defined(TVU_WINDOWS)
#include <Windows.h>
//#include <winbase.h>
#endif

//#ifdef _WIN32
//#include <winsock2.h>
//#elif defined(__linux__)
//
//#ifndef __USE_UNIX98
//#define __USE_UNIX98
//#endif
//#include <stdint.h>
//#else
//#error Platform not supported
//#endif
//
//#include <string>

namespace tvushm
{
//#ifdef _WIN32
//#ifndef _TVU_UINT64_DEFINED
//#define _TVU_UINT64_DEFINED
//	typedef unsigned __int64 uint64;
//#endif
//#elif defined(__linux__)
//#ifndef _TVU_UINT64_DEFINED
//#define _TVU_UINT64_DEFINED
//	typedef uint64_t uint64;
//#endif
//#else
//#error Platform not supported
//#endif

	class SharedMemory
	{
	public:
		SharedMemory(void);
		~SharedMemory(void);

	public:
		bool Open(const char*name);
        bool Create(const char*name,uint64_t size,bool&isNew);
		void Close();
		void Destroy();
        unsigned char* GetBytes(void)const;
        size_t GetSize(void) const;
		bool IsValid() const;
	private:
#if defined(TVU_WINDOWS)
		HANDLE _mapFileHandle;
#elif defined(TVU_LINUX) || defined(TVU_MINI)
		int _shmId;
#endif
		unsigned char* _bytes;
        uint64_t _size;
		std::string _name;
		bool _isOwner;
	};
}
