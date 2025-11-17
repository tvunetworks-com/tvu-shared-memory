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
#ifndef LIBSHM_MEDIA_PROTO_COMMON_H
#define LIBSHM_MEDIA_PROTO_COMMON_H

#if defined(TVU_WINDOWS)

#if defined (LIBSHMMEDIA_DLL_EXPORT)
#define _LIBSHMMEDIA_PROTO_DLL_ __declspec(dllexport)
#elif defined(LIBSHMMEDIA_DLL_IMPORT)
#define _LIBSHMMEDIA_PROTO_DLL_ __declspec(dllimport)
#else
#define _LIBSHMMEDIA_PROTO_DLL_
#endif

#else
#define _LIBSHMMEDIA_PROTO_DLL_ __attribute__((visibility("default")))
#endif

#endif // LIBSHM_MEDIA_PROTO_COMMON_H
