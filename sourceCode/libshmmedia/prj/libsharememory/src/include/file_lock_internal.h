
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
 *      it is the declaration of file lock
 *  History:
 *      Dec 22nd, 2015, lotus first initialize it.
*************************************************************/

#ifndef _TVU_INTERNAL_FILE_LOCK_H
#define _TVU_INTERNAL_FILE_LOCK_H

#if defined TVU_LINUX

#   include <stdio.h>
#   include <unistd.h>
#   include <errno.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <fcntl.h>
#   include <string.h>
#   include <sys/file.h>
#   include "file_lock.h"

#define	TVU_FILELOCK_SH	1	/* Shared lock.  */
#define	TVU_FILELOCK_EX	2 	/* Exclusive lock.  */
#define	TVU_FILELOCK_UN	8	/* Unlock.  */
#define	TVU_FILE_LOCK_NB	4	/* Don't block when locking.  */

#define TVU_FILESTATS_OFF  0
#define TVU_FILESTATS_ON   1

#ifdef __cplusplus
extern "C" {
#endif
/**
 *  Description:
 *      open one file-locker
 *  @parameter:
 *      p   : file lock context
 *      fn  : lock file name
 *
 */
int tvuutil_file_lock_open(file_lock_t *p, const char *fn);

/**
 *  Description:
 *      close one file-locker.
 *  @parameter:
 *      p   : file lock context
 *
 */
int tvuutil_file_lock_close(file_lock_t *p);

/**
 *  Description:
 *      lock the file-locker.
 *  @parameter:
 *      p   : file lock context
 *      b_shared    : whether to use shared lock
 *          0       -   use shared lock, two shared lock would not block
 *          1       -   exclusive block, only one process can use it.
 */
int tvuutil_file_lock_lock(file_lock_t *p, int b_shared);

/**
 *  Description:
 *      unlock  the file-locker.
 *  @parameter:
 *      p   : file lock context
 *
 */
int tvuutil_file_lock_unlock(file_lock_t *p);

#ifdef __cplusplus
}
#endif

#endif

#endif // _TVU_INTERNAL_FILE_LOCK_H

