/*************************************************************
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
 *********************************************************
 *  Description:
 *      it is the accomplish of file lock
 *  History:
 *      Dec 22nd, 2015, lotus first initialize it.
*************************************************************/
#include "file_lock_internal.h"

#if defined TVU_LINUX
int tvuutil_file_lock_open(file_lock_t *p, const char *fn)
{
    int     ret = -1;
    int     flag=O_RDWR|O_CREAT;
    if (!p)
        return ret;

    memset((void *)p, 0, sizeof(file_lock_t));

    //p->fn   = strdup(fn);
    p->fd   = open(fn, flag, 0666);

    if (p->fd  < 0)
    {
        p->stats    = TVU_FILESTATS_OFF;
        ret         = -1;
    }
    else
    {
        p->stats    = TVU_FILESTATS_ON;
        ret         = 0;
    }

    return ret;
}

int tvuutil_file_lock_close(file_lock_t *p)
{
    int     ret = 0;
    if (!p)
        return ret;

    if (p->stats == TVU_FILESTATS_ON) {
        close(p->fd);
        p->fd   = 0;
        p->stats = TVU_FILESTATS_OFF;
    }

    return ret;
}

int tvuutil_file_lock_lock(file_lock_t *p, int b_shared)
{
    return flock(p->fd, b_shared?LOCK_SH:LOCK_EX);
}

int tvuutil_file_lock_unlock(file_lock_t *p)
{
    return flock(p->fd, LOCK_UN);
}

#endif
