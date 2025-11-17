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
#include "libshm_media_protocol_log_internal.h"
#include <stdio.h>
#include <stdarg.h>

static int (*gfnShmMediaProtoLogCb)(int level, const char *fmt, ...) = NULL;
static int (*gfnShmMediaProtoLogCb2)(int level, const char *fmt, va_list ap) = NULL;

void LibShmMediaProtoLogPrintInternal(int level, const char *fmt, ...)
{
    if (!gfnShmMediaProtoLogCb && !gfnShmMediaProtoLogCb2)
    {
        return;
    }

    if (gfnShmMediaProtoLogCb)
    {
        va_list ap;
        va_start(ap, fmt);
        char slog[1024] = {0};
        vsnprintf(slog, sizeof(slog)-1, fmt, ap);
        gfnShmMediaProtoLogCb(level, "%s", slog);
        va_end(ap);
    }

    if (gfnShmMediaProtoLogCb2)
    {
        va_list ap;
        va_start(ap, fmt);
        gfnShmMediaProtoLogCb2(level, fmt, ap);
        va_end(ap);
    }
    return;
}

void LibShmMediaProtoSetLogCbInternal(int(*cb)(int , const char *, ...))
{
    gfnShmMediaProtoLogCb     = cb;
    gfnShmMediaProtoLogCb2    = NULL;
    return;
}

void LibShmMediaProtoSetLogCbInternalV2(int(*cb)(int , const char *, va_list ap))
{
    gfnShmMediaProtoLogCb2 = cb;
    gfnShmMediaProtoLogCb = NULL;
    return;
}
