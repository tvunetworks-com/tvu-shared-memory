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
History: Tony Guo on Oct 4, 2016: Created
*/


#include "TvuVaItemSharedMemory.h"
#include "TvuTimeUtils.h"
#include "TvuLog.h"
#include "TvuMemUtils.h"
#include "TvuFormatUtils.h"
#include "TvuCommonDefines.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <sstream>

#if defined(TVU_LINUX) || defined(TVU_MINI)
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif
#include <sys/types.h>

namespace tvushm
{
    static std::string FormatErrno(int errorCode)
    {
        char* errorString=strerror(errorCode);
        if (errorString==NULL)
        {
            return std::string();
        }
        return std::string(errorString);
    }

    SharedMemory::SharedMemory(void)
    {
        _bytes=NULL;
        _size=0;
        _isOwner=false;

#if defined(TVU_WINDOWS)
        _mapFileHandle=INVALID_HANDLE_VALUE;
#elif defined(TVU_LINUX) || defined(TVU_MINI)
        _shmId = -1;
#endif
    }

    SharedMemory::~SharedMemory(void)
    {
        if (_isOwner)
        {
            Destroy();
            _isOwner=false;
        }
        else
        {
            Close();
        }
    }

#if defined(TVU_WINDOWS)

    bool SharedMemory::Open(const char*name)
    {
        HANDLE mapFileHandle=OpenFileMappingA(
            FILE_MAP_ALL_ACCESS,
            FALSE,
            name);
        if (mapFileHandle==NULL)
        {
            DWORD errorCode=GetLastError();
            TVU_HEARTBEAT_MULTIPLE_WARN(1,10,Log::GetDefaultLog(),Formatter("OpenFileMappingA failed.")
                <<" name: "<<(name)
                <<", error: "<<FormatErrno(errorCode));
            return false;
        }

        LPVOID bufferPtr= MapViewOfFile(mapFileHandle,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            0);

        if (bufferPtr == NULL)
        {
            DWORD errorCode=GetLastError();
            TVU_HEARTBEAT_MULTIPLE_WARN(1,10,Log::GetDefaultLog(),Formatter("MapViewOfFile failed.")
                <<" name: "<<(name)
                <<", error: "<<FormatErrno(errorCode));
            CloseHandle(mapFileHandle);
            return false;
        }

        TVU_HEARTBEAT_MULTIPLE_DEBUG(1,10,Log::GetDefaultLog(), Formatter("shared memory opened. handle: ")
            << (uintptr)mapFileHandle
            << ", name: "
            << (name)
            << ", address: " << (uintptr)bufferPtr
            );

        _mapFileHandle=mapFileHandle;
        _bytes=(byte*)bufferPtr;
        _size=0;
        _name=(name);
        _isOwner=false;
        return true;
    }

    bool SharedMemory::Create(const char*name,uint64 size,bool&isNew)
    {
        if (_mapFileHandle!=INVALID_HANDLE_VALUE || _bytes!=NULL)
        {
            return false;
        }

        HANDLE mapFileHandle=CreateFileMappingA(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            (DWORD)(size>>32),
            (DWORD)size,
            name);

        if (mapFileHandle==NULL)
        {
            DWORD errorCode=GetLastError();
            TVU_HEARTBEAT_MULTIPLE_WARN(1,10,Log::GetDefaultLog(),Formatter("CreateFileMappingA failed.")
                <<" name: "<<(name)
                <<", size: "<<size
                <<", error: "<<FormatErrno(errorCode));
            return false;
        }

        uint64 newSize=size;
        DWORD lastError=GetLastError();
        if (lastError==ERROR_ALREADY_EXISTS)
        {
            isNew=false;
            newSize=0;
        }
        else
        {
            isNew=true;
        }

        LPVOID bufferPtr= MapViewOfFile(mapFileHandle,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            (size_t)newSize);

        if (bufferPtr == NULL)
        {
            DWORD errorCode=GetLastError();
            TVU_HEARTBEAT_MULTIPLE_WARN(1,10,Log::GetDefaultLog(),Formatter("MapViewOfFile failed.")
                <<" name: "<<(name)
                <<", size: "<<size
                <<", error: "<< FormatErrno(errorCode));
            CloseHandle(mapFileHandle);
            return false;
        }

        TVU_HEARTBEAT_MULTIPLE_DEBUG(1,10,Log::GetDefaultLog(), Formatter("shared memory created. handle: ")
            << (uintptr)mapFileHandle
            << ", name: "
            << (name)
            << ", address: "
            << (uintptr)bufferPtr
            << ", size: "<<size
            <<", is new: "<<((isNew)?"yes":"no")
            );

        _mapFileHandle=mapFileHandle;
        _bytes=(byte*)bufferPtr;
        _size=size;
        _name=(name);
        _isOwner=isNew;

        return true;
    }

    void SharedMemory::Close(void)
    {
        if (_bytes!=NULL)
        {
            UnmapViewOfFile(_bytes);
            _bytes=NULL;
            _size=0;
        }

        if (_mapFileHandle!=INVALID_HANDLE_VALUE)
        {
            CloseHandle(_mapFileHandle);
            _mapFileHandle=INVALID_HANDLE_VALUE;
        }
    }

    void SharedMemory::Destroy(void)
    {
        Close();
    }

    bool SharedMemory::IsValid() const
    {
        return _bytes!=NULL && _mapFileHandle!=INVALID_HANDLE_VALUE;
    }

#elif defined(TVU_LINUX) || defined(TVU_MINI)

    bool SharedMemory::Open(const char*name)
    {
        if (_shmId!=-1 || _bytes!=NULL)
        {
            return false;
        }

        //try open;
        int shmId=shm_open(    name, O_RDWR,
            S_IRUSR | S_IWUSR);

        if (shmId == -1)
        {
            int errorCode=errno;
            TVU_HEARTBEAT_MULTIPLE_WARN(10,10,Log::GetDefaultLog(), Formatter("shm_open failed. name:(")
                << (name)
                << "), error: "
                << FormatErrno(errorCode)
                );
            return false;
        }

        //find the existing size;
        struct stat shmStat;
        MemUtils::Initialize(shmStat);

        if (fstat(shmId,&shmStat)==-1)
        {
            int errorCode=errno;
            TVU_HEARTBEAT_MULTIPLE_WARN(1,10,Log::GetDefaultLog(), Formatter("failed to get shared memory state: ")
                << ", error: "
                << FormatErrno(errorCode)
                );
            return false;
        }

        size_t existingSize= shmStat.st_size;

        byte*bytes= static_cast<byte*>(mmap(NULL, existingSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmId, 0));
        if (bytes==MAP_FAILED || bytes==NULL)
        {
            int errorCode=errno;
            TVU_HEARTBEAT_MULTIPLE_WARN(1,10,Log::GetDefaultLog(), Formatter("failed to map memory. error: ")
                << FormatErrno(errorCode)
                );
            return false;
        }

        TVU_HEARTBEAT_MULTIPLE_DEBUG(1,10,Log::GetDefaultLog(), Formatter("shared memory opened. id: ")
            << shmId
            << ", name: "
            << (name)
            << ", address: "<< (uintptr)bytes
            << ", existing size: "<< existingSize
            );

        _shmId=shmId;
        _bytes=bytes;
        _size=existingSize;
        _name=name;
        _isOwner=false;
        return true;
    }

    bool SharedMemory::Create(const char*name,uint64_t size,bool&isNew)
    {
        if (_shmId!=-1 || _bytes!=NULL)
        {
            return false;
        }

        //try open;
        int shmId=shm_open(    name, O_RDWR,
            S_IRUSR | S_IWUSR);

        if (shmId == -1)
        {
            shmId=shm_open(name, O_CREAT | O_RDWR,
                S_IRUSR | S_IWUSR);

            if (shmId == -1)
            {
                int errorCode=errno;
                TVU_HEARTBEAT_MULTIPLE_WARN(1,10,Log::GetDefaultLog(), Formatter("shm_open failed. name: ")
                    << (name)
                    << "), error: "
                    << FormatErrno(errorCode)
                    );
                return false;
            }

            if (ftruncate(shmId, (size_t)size) == -1)
            {
                int errorCode=errno;
                TVU_HEARTBEAT_MULTIPLE_WARN(1,10,Log::GetDefaultLog(), Formatter("failed to adjust share memeory size. expected size: ")
                    << size
                    << ", error: "
                    << FormatErrno(errorCode)
                    );
                return false;
            }

            byte*bytes= static_cast<byte*>(mmap(NULL, (size_t)size, PROT_READ | PROT_WRITE, MAP_SHARED, shmId, 0));
            if (bytes==MAP_FAILED || bytes==NULL)
            {
                int errorCode=errno;
                TVU_HEARTBEAT_MULTIPLE_WARN(1,10,Log::GetDefaultLog(), Formatter("failed to map memory. error: ")
                    << FormatErrno(errorCode)
                    );
                return false;
            }

            isNew=true;

            TVU_HEARTBEAT_MULTIPLE_DEBUG(1,10, Log::GetDefaultLog(), Formatter("shared memory created. id: ")
                << shmId
                << ", name: "
                << (name)
                << ", address: "<< (uintptr)bytes
                << ", size: "    << size
                <<", is new: "<< isNew
                );
            _shmId=shmId;
            _bytes=bytes;
            _size=size;
            _name=name;
            _isOwner=isNew;
            return true;
        }
        else
        {
            //find the existing size;
            struct stat shmStat;
            MemUtils::Initialize(shmStat);

            if (fstat(shmId,&shmStat)==-1)
            {
                int errorCode=errno;
                TVU_HEARTBEAT_MULTIPLE_WARN(1,10,Log::GetDefaultLog(), Formatter("failed to get shared memory state: ")
                    << ", error: "
                    << FormatErrno(errorCode)
                    );
                return false;
            }

            size_t existingSize= shmStat.st_size;

            byte*bytes= static_cast<byte*>(mmap(NULL, existingSize, PROT_READ | PROT_WRITE, MAP_SHARED, shmId, 0));
            if (bytes==MAP_FAILED || bytes==NULL)
            {
                int errorCode=errno;
                TVU_HEARTBEAT_MULTIPLE_WARN(1,10,Log::GetDefaultLog(), Formatter("failed to map memory. error: ")
                    << FormatErrno(errorCode)
                    );
                return false;
            }

            isNew=false;

            TVU_HEARTBEAT_MULTIPLE_DEBUG(1,10,Log::GetDefaultLog(), Formatter("shared memory opened. id: ")
                << shmId
                << ", name: "
                <<(name)
                << ", address: "<< (uintptr)bytes
                << ", existing size: "    << existingSize
                << ", expected size: "    << size
                <<", is new: "<<isNew
                );

            _shmId=shmId;
            _bytes=bytes;
            _size=existingSize;
            _name=name;
            _isOwner=isNew;
            return true;
        }
    }

    void SharedMemory::Close(void)
    {
        if (_bytes!=NULL)
        {
            munmap(_bytes,_size);
            _bytes=NULL;
            _size=0;
        }
        if (_shmId!=-1)
        {
            close(_shmId);
            _shmId=-1;
        }
    }

    void SharedMemory::Destroy(void)
    {
        if (_bytes!=NULL)
        {
            munmap(_bytes,_size);
            _bytes=NULL;
            _size=0;
        }
        if (_shmId!=-1)
        {
            shm_unlink(_name.c_str());
            close(_shmId);
            _shmId=-1;
            _isOwner=false;
        }
    }

    bool SharedMemory::IsValid() const
    {
        return _bytes!=NULL && _shmId!=-1;
    }
#else
    bool SharedMemory::Open(const char*name)
    {
        TVU_UNREFERENCED(name);
        return false;
    }

    bool SharedMemory::Create(const char*name,uint64 size,bool&isNew)
    {
        TVU_UNREFERENCED(name);
        TVU_UNREFERENCED(size);
        return false;
    }

    void SharedMemory::Close(void)
    {
    }

    void SharedMemory::Destroy(void)
    {
    }

    bool SharedMemory::IsValid() const
    {
        return false;
    }
#endif

    unsigned char* SharedMemory::GetBytes(void) const
    {
        return _bytes;
    }

    size_t SharedMemory::GetSize(void) const
    {
        return (size_t)_size;
    }


}
