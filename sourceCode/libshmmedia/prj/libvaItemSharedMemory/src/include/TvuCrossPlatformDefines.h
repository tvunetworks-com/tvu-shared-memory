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

//only defines. no including other headers.

#if defined(TVU_WINDOWS)
#elif defined(TVU_LINUX)
#elif defined(TVU_MINI)
#elif defined(TVU_ANDROID)
#elif defined(TVU_MAC)
#elif defined(TVU_IOS)
#elif defined(_WIN32)
#define TVU_WINDOWS
#elif defined(__MACH__)
#define TVU_MAC
#elif defined(__linux__)
#define TVU_LINUX
#else
#error Can not determine platform type
#endif

#if defined(TVU_WINDOWS)

#define tvu_memcpy_s(dst,dst_size,src,src_size) memcpy_s(dst,dst_size,src,src_size)
#define tvu_memmove_s(dst,dst_size,src,src_size) memmove_s(dst,dst_size,src,src_size)

#define TVU_SIZE_MAX SIZE_MAX
#define TVU_INFINITE INFINITE

#ifndef TVU_SOCKET_HANDLE
#define TVU_SOCKET_HANDLE SOCKET
#endif

#ifndef TVU_INVALID_SOCKET
#define TVU_INVALID_SOCKET INVALID_SOCKET
#endif

#ifndef TVU_SOCKET_ERROR
#define TVU_SOCKET_ERROR SOCKET_ERROR
#endif

#ifndef TVU_ERROR_WOULDBLOCK
#define TVU_ERROR_WOULDBLOCK WSAEWOULDBLOCK
#endif

#define tvu_strcasecmp _stricmp
#define tvu_closesocket closesocket
#define tvu_socklen_t int

#define TVU_TLS_DECL_SPEC __declspec(thread)

#define TVU_SHUTDOWN_READ_FLAG			SD_RECEIVE
#define TVU_SHUTDOWN_WRITE_FLAG			SD_SEND
#define TVU_SHUTDOWN_READWRITE_FLAG		SD_BOTH

#ifndef FD_SETSIZE
#define FD_SETSIZE		1024
#endif /* FD_SETSIZE */

#define TVU_FD_SETSIZE	FD_SETSIZE

#define TVU_FUNCTION_NAME __FUNCTION__

#define TVU_PRETTY_FUNCTION_NAME __FUNCSIG__

#define TVU_TAGGED_FUNCTION_NAME __FUNCTION__

#ifndef TVU_THREAD_ID
#define TVU_THREAD_ID unsigned int
#endif

#define TVU_INVALID_THREAD_ID 0

#define TVU_MAX_HOSTNAME_LENGTH MAX_COMPUTERNAME_LENGTH

#pragma warning(disable: 4127)

#ifdef _DEBUG
#define TVU_DEBUG_BREAK() ::_CrtDbgBreak()
#else
#define TVU_DEBUG_BREAK()
#endif

#define TVU_UNREFERENCED(v) (v)

#ifndef TVU_CURRENT_THREAD_ID_T
#define TVU_CURRENT_THREAD_ID_T unsigned int
#endif

#ifndef TVU_PID_T
#define TVU_PID_T uint32
#endif

#ifndef TVU_STDCALL
#define TVU_STDCALL __stdcall
#endif

#define TVU_IFF_TUN			0
#define TVU_IFF_TAP			0

#define tvu_snprintf_s _snprintf_s

#define TVU_FD_SET FD_SET

#define tvu_nfds_t ULONG
#define tvu_poll WSAPoll

#define TVU_PATH_SEPARATOR	'\\'

#define TVU_IOVEC WSABUF

#define TVU_IOVEC_SET(iov,bytes,bytesNum) \
    do \
    { \
        (iov).len=(bytesNum); \
        (iov).buf=(CHAR*)(bytes); \
    }while(0)

#elif defined(TVU_LINUX)  || defined(TVU_MINI) || defined(TVU_ANDROID) || defined(TVU_MAC) || defined(TVU_IOS)

#ifndef TVU_DERIVED_LINUX
#define TVU_DERIVED_LINUX
#endif

#define tvu_memcpy_s(dst,dst_size,src,src_size) memcpy(dst,src,std::min((dst_size),(src_size)))

#define tvu_memmove_s(dst,dst_size,src,src_size) memmove(dst,src,std::min((dst_size),(src_size)))

#define TVU_SIZE_MAX ((size32)-1)
#define TVU_INFINITE 0xFFFFFFFF

#ifndef TVU_SOCKET_HANDLE
#define TVU_SOCKET_HANDLE int
#endif

#ifndef TVU_INVALID_SOCKET
#define TVU_INVALID_SOCKET (TVU_SOCKET_HANDLE)(~0)
#endif

#ifndef TVU_SOCKET_ERROR
#define TVU_SOCKET_ERROR (-1)
#endif

#ifndef TVU_ERROR_WOULDBLOCK
#define TVU_ERROR_WOULDBLOCK EAGAIN
#endif

#define tvu_strcasecmp strcasecmp
#define tvu_closesocket close
#define tvu_socklen_t socklen_t

#define TVU_SHUTDOWN_READ_FLAG			SHUT_RD
#define TVU_SHUTDOWN_WRITE_FLAG		SHUT_WR
#define TVU_SHUTDOWN_READWRITE_FLAG	SHUT_RDWR

#define TVU_TLS_DECL_SPEC __thread

#ifdef __FD_SETSIZE
#define TVU_FD_SETSIZE	__FD_SETSIZE
#else
#define TVU_FD_SETSIZE	FD_SETSIZE
#endif

#define TVU_FUNCTION_NAME __FUNCTION__

#if defined(__GNUC__)
#define TVU_PRETTY_FUNCTION_NAME __PRETTY_FUNCTION__
#else
#define TVU_PRETTY_FUNCTION_NAME __FUNCTION__
#endif

#define TVU_TAGGED_FUNCTION_NAME TVU_PRETTY_FUNCTION_NAME


#ifndef TVU_THREAD_ID
#define TVU_THREAD_ID pthread_t
#endif

#define TVU_INVALID_THREAD_ID (pthread_t)0

#ifndef HOST_NAME_MAX
#define TVU_MAX_HOSTNAME_LENGTH 32
#else
#define TVU_MAX_HOSTNAME_LENGTH HOST_NAME_MAX
#endif

#define TVU_DEBUG_BREAK()

#define TVU_UNREFERENCED(v) (void)(v)

#ifndef TVU_CURRENT_THREAD_ID_T
#define TVU_CURRENT_THREAD_ID_T pid_t
#endif

#ifndef TVU_PID_T
#define TVU_PID_T pid_t
#endif

#ifndef TVU_STDCALL
#define TVU_STDCALL
#endif

#define TVU_IFF_TUN			IFF_TUN
#define TVU_IFF_TAP			IFF_TAP

#define tvu_snprintf_s(buffer,sizeofBuffer,count,format,...) \
    snprintf(buffer,sizeofBuffer,format,##__VA_ARGS__)

#define TVU_FD_SET(fd, set) do \
    { \
        if ((uint32)fd<FD_SETSIZE) \
        { \
            FD_SET(fd,set); \
        } \
    } while (0)

#define tvu_nfds_t nfds_t
#define tvu_poll poll

#define TVU_PATH_SEPARATOR	'/'

#define TVU_IOVEC iovec

#define TVU_IOVEC_SET(iov,bytes,bytesNum) \
    do \
    { \
        (iov).iov_base=(bytes); \
        (iov).iov_len=(bytesNum); \
    }while(0)

#else
#error Platform not supported
#endif
