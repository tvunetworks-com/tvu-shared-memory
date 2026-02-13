/******************************************************
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
 *  File:
 *      sharememory.cpp
 *  CopyRight:
 *      tvu
 *  Description:
 *      Share Memory base class.
 *  History:
 *      In 2012, Eros Lin first initialize it.
 *      In April, 2014, Lotus, adding checking flag to
 *  solve dynamic share memory problem, version from 1 upgrade to 2.
******************************************************/

#if defined (TVU_MINGW)
#include <io.h>
#endif
#include <string.h>
#include <stdio.h>
#include "shmhead.h"
#include "sharememory_internal.h"
#include "buildversion.h"

#define WAIT_MS_NUM                 1000

#if !defined (TVU_WINDOWS) || defined(TVU_MINGW)
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#elif defined(TVU_WINDOWS)
#if (_MSC_VER == 1500)
    #define snprintf(p, count, fmt, ...)    _snprintf(p, (count)-1, fmt, ##__VA_ARGS__)
#endif
#endif

#define SHM_ENDIAN_CONVERT  0 /* shm head is only local, not part of RShm. */

#if SHM_ENDIAN_CONVERT
#define LIBSHMMEDIA_READ_SHM_U64(v)         _tvuutil_letoh64(v)
#define LIBSHMMEDIA_READ_SHM_U32(v)         _tvuutil_letoh32(v)
#define LIBSHMMEDIA_READ_SHM_U16(v)         _tvuutil_letoh16(v)

#define LIBSHMMEDIA_WRITE_SHM_U64(v)        _tvuutil_htole64(v)
#define LIBSHMMEDIA_WRITE_SHM_U32(v)        _tvuutil_htole32(v)
#define LIBSHMMEDIA_WRITE_SHM_U16(v)        _tvuutil_htole16(v)

#else
#define LIBSHMMEDIA_READ_SHM_U64(v)         (v)
#define LIBSHMMEDIA_READ_SHM_U32(v)         (v)
#define LIBSHMMEDIA_READ_SHM_U16(v)         (v)

#define LIBSHMMEDIA_WRITE_SHM_U64(v)        (v)
#define LIBSHMMEDIA_WRITE_SHM_U32(v)        (v)
#define LIBSHMMEDIA_WRITE_SHM_U16(v)        (v)
#endif

unsigned int libshmhead_construct_get_version(const shm_construct_t *pshm)
{
    return LIBSHMMEDIA_READ_SHM_U32(pshm->version);
}

static int default_cb(int level, const char *fmt, ...)
{
    va_list ap;
    va_start(ap,fmt);
    char    slog[1024]  = {0};
    size_t     len         = 0;

    switch (level) {
        case 'i':
            {
                snprintf(slog, 1024, "M[Shm]info --");
                len = strlen(slog);
                vsnprintf(slog + len, 1024 - len, fmt, ap);
            }
            break;
        case 'w':
            {
                snprintf(slog, 1024, "M[Shm]warn --");
                len = strlen(slog);
                vsnprintf(slog + len, 1024 - len, fmt, ap);
            }
            break;
        case 'e':
            {
                snprintf(slog, 1024, "M[Shm]error--");
                len = strlen(slog);
                vsnprintf(slog + len, 1024 - len, fmt, ap);
            }
            break;
        default:
            break;
    }

    fprintf(stdout, "%s", slog);
    fflush(stdout);
    va_end(ap);
    return 0;
}

static int (*gfnShmLogCb)(int level, const char *fmt, ...) = NULL;
static int (*gfnShmLogCb2)(int level, const char *fmt, va_list ap) = NULL;

int libsharememory_set_log_print_internal(int level, const char *fmt, ...)
{
    int ret  = 0;

    if (!gfnShmLogCb && !gfnShmLogCb2)
    {
        return 0;
    }

    do {
        if (gfnShmLogCb)
        {
            va_list ap;
            va_start(ap, fmt);
            char slog[1024] = {0};
            vsnprintf(slog, sizeof(slog)-1, fmt, ap);
            ret = gfnShmLogCb(level, "%s", slog);
            va_end(ap);
        }

        if (gfnShmLogCb2)
        {
            va_list ap;
            va_start(ap, fmt);
            gfnShmLogCb2(level, fmt, ap);
            va_end(ap);
        }


    } while (0);

    return ret;
}


void libsharememory_set_log_cabllback_internal(int(*cb)(int , const char *, ...))
{
    gfnShmLogCb     = cb;
    gfnShmLogCb2    = NULL;
    return;
}

void libsharememory_set_log_cabllback_internal_v2(int(*cb)(int , const char *, va_list ap))
{
    gfnShmLogCb2 = cb;
    gfnShmLogCb = NULL;
    return;
}

int CTvuBaseShareMemory::InitCreateShm(uint32_t header_len, uint32_t item_count, uint32_t item_length)
{
    MEM_HEADER_V1   *p1 = (MEM_HEADER_V1*)m_pHeader;

    memset(m_pHeader, 0, header_len);

    p1->version     = LIBSHMMEDIA_WRITE_SHM_U32(INVALID_SHM_HEAD_VERSION);
    m_uItemCounts   = item_count;
    p1->item_count  = LIBSHMMEDIA_WRITE_SHM_U32(item_count);
    m_uItemLen      = item_length;
    p1->item_length = LIBSHMMEDIA_WRITE_SHM_U32(item_length);
    m_uHeadLen      = header_len;
    p1->item_offset = LIBSHMMEDIA_WRITE_SHM_U32(header_len);
    p1->item_current= LIBSHMMEDIA_WRITE_SHM_U32(0);

#if _SHM_HEAD_FEATURE_EXT_EABLE
    shm_construct_ext_t *pext = (shm_construct_ext_t *)(m_pHeader + SHM_MEDIA_HEAD_INFO_V4_OFFSET);
    pext->ext_ver   = SHM_CONSTRUCT_EXT_VER;
    pext->reserve   = 0;
    m_uExtBufLen    =
    pext->ext_len   = sizeof(shm_construct_ext_t);
    memset(pext->a_reserve_, 0, sizeof(pext->a_reserve_));
    pext->item_current_64 = 0;
    pext->last_read_time_stamp = 0;
#endif

    m_uVersion      = MEMHEADER_CURRENT_VERSION;
    p1->version     = LIBSHMMEDIA_WRITE_SHM_U32(m_uVersion);
    return 0;
}

int CTvuBaseShareMemory::InitOpenShm()
{
    int             ret     = 0;
    shm_construct_t *p1     = (shm_construct_t *)m_pHeader;
    int             nLoop   = 0;

    while (CHECK_INVALID_SHM_HEAD_VERSION(LIBSHMMEDIA_READ_SHM_U32(p1->version)))
    {
        if (nLoop == WAIT_MS_NUM)
        {
            DEBUG_ERROR("can not get valid shm version\n");
            ret     = -1;
            break;
        }
        _libshm_common_msleep(1);
        nLoop++;
    }

    if (ret == 0)
    {
        m_uVersion      = LIBSHMMEDIA_READ_SHM_U32(p1->version);
        m_uItemLen      = LIBSHMMEDIA_READ_SHM_U32(p1->item_length);
        m_uItemCounts   = LIBSHMMEDIA_READ_SHM_U32(p1->item_count);
        m_uReadIndex    = LIBSHMMEDIA_READ_SHM_U32(p1->item_current);
        m_uHeadLen      = LIBSHMMEDIA_READ_SHM_U32(p1->item_offset);

    #if _SHM_HEAD_FEATURE_EXT_EABLE
        shm_construct_ext_t *pext = (shm_construct_ext_t *)(m_pHeader + SHM_MEDIA_HEAD_INFO_V4_OFFSET);
        m_uExtBufLen = 0;
        switch(pext->ext_ver)
        {
            case kShmConstructExtVer1:
            {
                m_uExtBufLen = pext->ext_len;
            }
            break;
            default:
            {

            }
            break;
        }
    #endif
    }

    return ret;
}

#if defined TVU_WINDOWS || defined TVU_MINGW
CTvuBaseShareMemory::CTvuBaseShareMemory(void)
:m_hMapFile(NULL)
,m_pHeader(NULL)
{
    memset(m_memoryName, '\0', MAX_SHARE_MEMROY_NAME);
    DEBUG_INFO("%s this(0X%x)\n", __FUNCTION__, this);
}

CTvuBaseShareMemory::~CTvuBaseShareMemory(void)
{
    DEBUG_INFO("%s %s this(0X%x)\n", __FUNCTION__, m_memoryName, this);
    CloseMapFile();
}

uint8_t * CTvuBaseShareMemory::CreateOrOpen(const char * pMemoryName, uint32_t header_len, uint32_t item_count, uint32_t item_length, void(*clean_data)(uint8_t *, bool))
{
    if (NULL == m_pHeader) {
        strncpy(m_memoryName, pMemoryName, MAX_SHARE_MEMROY_NAME-1);
        DEBUG_INFO("%s %s this(0X%x)\n", __FUNCTION__, m_memoryName, this);
        m_pHeader   = Open(pMemoryName);

        if (m_pHeader)
        {
            DEBUG_INFO("Open shared memory of file[%s] successfully\n", pMemoryName);
            size_t real_item_count = GetItemCounts();
            size_t real_item_length = GetItemLength();
            if (!(item_count>real_item_count || item_length>real_item_length))
            {
                /* LL: open, should check init status */
                goto EXIT;
            }

            {
                DEBUG_WARN("The shared memory need to be re-create for less size."
                    "name:%s"
                    ",req:{c:%u,l:%u}"
                    ",real:{c:%u,l:%u}"
                    "\n"
                    , pMemoryName
                    , item_count, item_length
                    , real_item_count, real_item_length
                );
                if (clean_data)
                {
                    clean_data(m_pHeader, true);
                }
                m_iFlags = SHM_FLAG_WRITE;
                CloseMapFile();
            }
        }

        m_hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            item_count * item_length + header_len,
            pMemoryName);

        if (!m_hMapFile)
            return NULL;

        m_pHeader = (uint8_t *)MapViewOfFile(m_hMapFile,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);

        if (m_pHeader) {//lotus
            if (InitCreateShm(header_len, item_count, item_length) < 0) {
                DEBUG_ERROR("create init failed\n");
                CloseMapFileAndSleep();
            }
        }

        DEBUG_INFO("create [%s], ver %d, head len : %d, counts : %d, item len : %d, build version{%s}, build version number %d\n"
            , m_memoryName, m_uVersion, m_uHeadLen, m_uItemCounts, m_uItemLen, BUILD_VERSION, BUILD_VERSION_NUM);
    }

EXIT:
    m_iFlags    = SHM_FLAG_WRITE;
    return m_pHeader;
}

uint8_t* CTvuBaseShareMemory::Open(const char * pMemoryName)
{
    if (NULL == m_pHeader) {
        strncpy(m_memoryName, pMemoryName, MAX_SHARE_MEMROY_NAME-1);
        DEBUG_INFO("%s %s this(0X%x)\n", __FUNCTION__, m_memoryName, this);
        m_hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, pMemoryName);
        if (m_hMapFile == NULL)
        {
            DEBUG_ERROR("Could not open file mapping object %s (%d).\n", pMemoryName, GetLastError());
            return NULL;
        }

        m_pHeader = (uint8_t *)MapViewOfFile(m_hMapFile,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);

        if (m_pHeader == NULL){
            DEBUG_ERROR("Could not map view of file %s (%d).\n", pMemoryName, GetLastError());
            ::CloseHandle(m_hMapFile); m_hMapFile = NULL;
        }

        /* LL: open, should check init status */
        if (m_pHeader) {
            if (InitOpenShm() < 0) {
                DEBUG_ERROR("open init failed\n");
                CloseMapFileAndSleep();
            }

            DEBUG_INFO("open [%s], ver %d, head len : %d, counts : %d, item len : %d, build version{%s}, build version number %d\n"
                , m_memoryName, m_uVersion, m_uHeadLen, m_uItemCounts, m_uItemLen, BUILD_VERSION, BUILD_VERSION_NUM);
        }
    }

    m_iFlags    = SHM_FLAG_READ;
    return m_pHeader;
}

int CTvuBaseShareMemory::CloseMapFileAndSleep()
{
    DEBUG_INFO("%s %s this(0X%x)\n", __FUNCTION__, m_memoryName, this);
    if(m_hMapFile) {
        int ret = ::CloseHandle(m_hMapFile);
        if(!ret)
        {
            DEBUG_ERROR("Error %ld occurred closing handle object! header %x \n", GetLastError(), m_pHeader);
        }
        m_hMapFile = NULL;
    }
    int retval = 1;
    if (m_pHeader) {
        retval = ::UnmapViewOfFile(m_pHeader);
        if(!retval)
        {
            DEBUG_ERROR("Error %ld occurred closing the mapping object! header %x \n", GetLastError(), m_pHeader);
        }
        m_pHeader = NULL;
        /* 100ms, waiting other process close shm */
        _libshm_common_msleep(100);
    }
    return retval;
}

int CTvuBaseShareMemory::CloseMapFile()
{
    DEBUG_INFO("%s %s this(0X%x)\n", __FUNCTION__, m_memoryName, this);
    if(m_hMapFile) {
        int ret = ::CloseHandle(m_hMapFile);
        if(!ret)
        {
            DEBUG_ERROR("Error %ld occurred closing handle object! header %x \n", GetLastError(), m_pHeader);
        }
        m_hMapFile = NULL;
    }
    int retval = 1;
    if (m_pHeader) {
        retval = ::UnmapViewOfFile(m_pHeader);
        if(!retval)
        {
            DEBUG_ERROR("Error %ld occurred closing the mapping object! header %x \n", GetLastError(), m_pHeader);
        }
        m_pHeader = NULL;
    }
    return retval;
}

#else
//#include "tvu_util.h"

#if defined(USE_POSIX_SHM)

#define PREFIX_KEY_FILE "/dev/shm"

static inline bool  _is_shm_removed(const char *__name)
{
    struct stat os;
    char    skeyfile[1024]  = {0};

    snprintf(skeyfile, sizeof(skeyfile), "%s/%s", PREFIX_KEY_FILE, __name);
    int ret = stat(skeyfile, &os);

    return ( ret == 0 ) ? false : true;
}

CTvuBaseShareMemory::CTvuBaseShareMemory(void)
: m_uVersion(0)
, m_uHeadLen(0)
, m_uItemLen(0)
, m_uItemCounts(0)
, m_iShmSize(0)
{
    m_iShmId    = -1;
    m_pHeader   = NULL;
    memset(m_memoryName, 0, MAX_SHARE_MEMROY_NAME);
    m_uReadIndex    = 0;
    m_iFlags    = 0;
    m_iShmId=-1;
    m_tmRemoveCheck = 0;
}

CTvuBaseShareMemory::~CTvuBaseShareMemory(void)
{
    CloseMapFile();
}

uint8_t *CTvuBaseShareMemory::CreateOrOpen(const char *pMemoryName, uint32_t header_len, uint32_t item_count, uint32_t item_length, void (*clean_data)(uint8_t *, bool))
{
    return CreateOrOpen(pMemoryName, header_len, item_count, item_length, S_IRUSR | S_IWUSR, clean_data);
}

uint8_t *CTvuBaseShareMemory::CreateOrOpen(const char *pMemoryName, uint32_t header_len, uint32_t item_count, uint32_t item_length, mode_t mode, void (*clean_data)(uint8_t *, bool))
{
    if (!m_pHeader)
    {
        size_t isize = 1LL * item_count * item_length + header_len;
        snprintf(m_memoryName, MAX_SHARE_MEMROY_NAME-1, "%s", pMemoryName);
        m_pHeader   = _open(pMemoryName, true);

        if (m_pHeader)
        {
            DEBUG_INFO("Open shared memory of file[%s] successfully\n", pMemoryName);
            size_t real_item_count = GetItemCounts();
            size_t real_item_length = GetItemLength();
            if (!(item_count>real_item_count || item_length>real_item_length))
            {
                /* LL: open, should check init status */
                goto EXIT;
            }

            {
                DEBUG_WARN("The shared memory need to be re-create for less size."
                           "name:%s"
                           ",req:{c:%u,l:%u}"
                           ",real:{c:%u,l:%u}"
                           "\n"
                           , pMemoryName
                           , item_count, item_length
                           , real_item_count, real_item_length
                           );
                if (clean_data)
                {
                    clean_data(m_pHeader, true);
                }
                m_iFlags = SHM_FLAG_WRITE;
                CloseMapFile();
            }
        }

        int shm_id=shm_open(pMemoryName, O_CREAT | O_RDWR, mode);

        if (shm_id == -1)
        {
            DEBUG_ERROR("SHM Create %s failed %d.\n",pMemoryName,errno);
            return NULL;
        }

        if (ftruncate(shm_id, (size_t)isize) == -1)
        {
            int errorCode=errno;
            DEBUG_ERROR("failed to ftruncate share memeory size. expected size: %llu, error: %d"
                , isize
                , errorCode
            );
            shm_unlink(pMemoryName);
            return NULL;
        }


        m_iFlags = SHM_FLAG_WRITE; /* if shmat failed, make sure closing action go through removing share memory action */
        m_iShmId    = shm_id;
        m_iShmSize  = isize;

        m_pHeader = (uint8_t *)mmap(NULL, (size_t)isize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);
        if (m_pHeader == MAP_FAILED || m_pHeader == NULL)
        {
            int errorCode=errno;
            DEBUG_ERROR("failed to mmap share memeory size. expected size: %llu, error: %d"
                , isize
                , errorCode
            );
            shm_unlink(pMemoryName);
            return NULL;
        }

        {
            if (InitCreateShm(header_len, item_count, item_length) < 0)
            {
                DEBUG_ERROR("create init %s failed\n",pMemoryName);
                CloseMapFileAndSleep();
            }

            DEBUG_INFO("create const shm success."\
                "nm:%s, id:%d, ver %d, head len : %d, counts : %d, item len : %d, mem address %p, build version{%s}, build version number %d\n"
                , m_memoryName, m_iShmId, \
                m_uVersion, m_uHeadLen, m_uItemCounts, \
                m_uItemLen, m_pHeader, BUILD_VERSION, BUILD_VERSION_NUM
            );
        }
    }

EXIT:
    m_iFlags    = SHM_FLAG_WRITE;
    return m_pHeader;
}

bool CTvuBaseShareMemory::_isShmRemovedFromKernal()
{
    bool  bret = false;
    int64_t now = _libshm_get_sys_ms64();

#define kShmRemovedSatusCheckTimeDuration   (1000LL)

    if (now - m_tmRemoveCheck > kShmRemovedSatusCheckTimeDuration)
    {
        bret = _is_shm_removed(m_memoryName);
        m_tmRemoveCheck = now;
    }

    return bret;
}

uint8_t *CTvuBaseShareMemory::_open(const char *pMemoryName, bool bForWriting)
{
    if (NULL == m_pHeader)
    {
        snprintf(m_memoryName, MAX_SHARE_MEMROY_NAME-1, "%s", pMemoryName);
        int shm_id = -1;

        shm_id  = shm_open(pMemoryName, O_RDWR, S_IRUSR | S_IWUSR);

        if (shm_id == -1)
        {
            DEBUG_ERROR("SHM Open[%s], failed %d.\n",pMemoryName, errno);
            return NULL;
        }

        m_iShmId    = shm_id;

        if (bForWriting)
        {
            m_iFlags    = SHM_FLAG_WRITE;
        }
        else
        {
            m_iFlags    = SHM_FLAG_READ;
        }


        struct stat shmStat;
        memset(&shmStat, 0, sizeof(shmStat));

        if (fstat(shm_id,&shmStat)==-1)
        {
            int errorCode=errno;
            DEBUG_ERROR("failed to get shared memory state, error %d\n"
                , errorCode
            );
            return NULL;
        }

        size_t existingSize= shmStat.st_size;

        m_pHeader = (uint8_t *)mmap(NULL, existingSize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);

        if (m_pHeader == MAP_FAILED || m_pHeader == NULL)
        {
            DEBUG_ERROR("SHM Open[%s] failed, shm attached failed, errno %d.\n",pMemoryName, errno);
            m_pHeader   = NULL;
            CloseMapFileAndSleep();
            return m_pHeader;
        }

        m_iShmSize = existingSize;

        if (InitOpenShm() < 0) {
            DEBUG_ERROR("open init  %s failed\n",pMemoryName);
            CloseMapFileAndSleep();
        }

        DEBUG_INFO("open const shm success."
                   "nm:%s, id:%d, ver %d, head len : %d, "
                   "counts : %d, item len : %d, mem address %p, build version{%s}, build version number %d\n"
                   , m_memoryName, m_iShmId, m_uVersion, m_uHeadLen, m_uItemCounts, m_uItemLen, m_pHeader, BUILD_VERSION, BUILD_VERSION_NUM);
    }

    return m_pHeader;
}

uint8_t *CTvuBaseShareMemory::Open(const char *pMemoryName)
{
    return _open(pMemoryName, false);
}

static int _remove_shm_from_kernal(const char *__name)
{
    int retval = 0;
    retval = shm_unlink(__name);
    return retval;
}

int CTvuBaseShareMemory::CloseMapFile()
{
    int retval  = 0;
    int shm_id  = -1;

    shm_id  = m_iShmId;

    if (m_pHeader)
    {
        DEBUG_INFO(
                    "deattached memory from process."
                    "name:%s\n"
                    , m_memoryName
                    );
        munmap((void *)m_pHeader, m_iShmSize);
        m_pHeader   = NULL;
    }

    if (shm_id != -1)
    {
        close(shm_id);
    }

    if (shm_id != -1 && m_iFlags & SHM_FLAG_WRITE)
    {
        retval = _remove_shm_from_kernal(m_memoryName);
        if (retval != 0)
        {
            DEBUG_ERROR("@[%s, %d]remove shm [%s]failed, ret %d\n"
                , __FUNCTION__, __LINE__, m_memoryName, retval);
        }
    }

    m_iShmId    = -1;
    return retval;
}

int CTvuBaseShareMemory::CloseMapFileAndSleep()
{
    int retval = CloseMapFile();
    /* 100ms, waiting other process close shm */
    _libshm_common_msleep(100);
    return retval;
}

int CTvuBaseShareMemory::RemoveShmFromKernal(const char *shmname)
{    
    if (!shmname)
    {
        return 0;
    }

    int ret = _remove_shm_from_kernal(shmname);

    if (ret != 0)
    {
        DEBUG_ERROR("@[%s, %d]remove shm [%s]failed, ret %d\n"
            , __FUNCTION__, __LINE__, shmname, ret);
    }
    return ret;
}

#else


static inline bool  _is_shm_removed(int shmid)
{
    struct shmid_ds obuf = {0};
    int ret = shmctl(shmid, IPC_STAT, &obuf);
    if (ret != 0)
    {
        return true;
    }

    if (obuf.shm_perm.__key == 0x00 || (obuf.shm_perm.mode & SHM_DEST))
    {
        return true;
    }

    return false;
}

CTvuBaseShareMemory::CTvuBaseShareMemory(void)
: m_uVersion(0)
, m_uHeadLen(0)
, m_uItemLen(0)
, m_uItemCounts(0)
{
    m_iKey      = -1;
    m_iShmId    = -1;

    m_pHeader   = NULL;
    memset(m_memoryName, 0, MAX_SHARE_MEMROY_NAME);
    m_uReadIndex    = 0;
    m_iFlags    = 0;
    m_iKey = -1;
    m_iShmId=-1;
}

CTvuBaseShareMemory::~CTvuBaseShareMemory(void)
{
    CloseMapFile();
}

#define PREFIX_KEY_FILE "/dev/shm"

static key_t create_key_file(const char *file, int bcreateFile)
{
    key_t   key = -1;
    char    skeyfile[256]  = {0};

    snprintf(skeyfile, sizeof(skeyfile), "%s/%s", PREFIX_KEY_FILE, file);

    if (bcreateFile)
    {
        if (access(skeyfile, F_OK) != 0)
        {
            FILE *fp    = fopen(skeyfile, "wb");
            if (fp)
                fclose(fp);
            else
            {
                DEBUG_ERROR("can not create key file[%s]\n", skeyfile);
                return key;
            }
        }
    }

    key = ftok(skeyfile, 0x11);

    return key;
}

static int remove_key_file(const char *file)
{
    int ret = 0;
#if 0 /* not remove key file, make key file always in system. */
    char    skeyfile[256]  = {0};

    if (!file || !file[0])
    {
        return ret;
    }

    snprintf(skeyfile, sizeof(skeyfile), "%s/%s", PREFIX_KEY_FILE, file);

    if (access(skeyfile, F_OK) == 0)
    {
       ret = unlink(skeyfile);
       DEBUG_INFO("remove key file[%s] from os, ret %d\n", skeyfile, ret);
    }
#endif
    return ret;
}

uint8_t *CTvuBaseShareMemory::CreateOrOpen(const char *pMemoryName, uint32_t header_len, uint32_t item_count, uint32_t item_length)
{
    if (!m_pHeader)
    {
        size_t isize = item_count * item_length + header_len;
        strncpy(m_memoryName, pMemoryName, MAX_SHARE_MEMROY_NAME-1);
        m_pHeader   = _open(pMemoryName, true);

        if (m_pHeader)
        {
            DEBUG_ERROR("Open shared memory of file[%s] successfully\n", pMemoryName);
            /* LL: open, should check init status */
            goto EXIT;
        }

        key_t key = -1;
        key = create_key_file(pMemoryName, 1);

        m_iKey  = key;

        if (-1 == key)
        {
            DEBUG_ERROR("Can not get key from file[%s]\n", pMemoryName);
            return NULL;
        }

        int shm_id = shmget(key, isize, IPC_CREAT | IPC_EXCL | 0666);
        if (shm_id == -1)
        {
            DEBUG_ERROR("SHM Create %s failed %d.\n",pMemoryName,errno);
            return NULL;
        }

        m_iFlags = SHM_FLAG_WRITE; /* if shmat failed, make sure closing action go through removing share memory action */
        m_iShmId    = shm_id;

        m_pHeader = (uint8_t *)shmat(shm_id, NULL, 0);
        if (m_pHeader != (void *) -1)
        {
            if (InitCreateShm(header_len, item_count, item_length) < 0)
            {
                DEBUG_ERROR("create init %s failed\n",pMemoryName);
                CloseMapFileAndSleep();
            }

            DEBUG_INFO("create [name=>%s, key=>0x%x, id=>%d], "\
                "ver %d, head len : %d, counts : %d, item len : %d, mem address %p, build version{%s}, build version number %d\n"
                , m_memoryName, m_iKey, m_iShmId, \
                m_uVersion, m_uHeadLen, m_uItemCounts, \
                m_uItemLen, m_pHeader, BUILD_VERSION, BUILD_VERSION_NUM
            );
        }
        else
        {
            DEBUG_ERROR("Create SHM %s failed.\n",pMemoryName);
            m_pHeader   = NULL;
            CloseMapFileAndSleep();
        }
    }

EXIT:
    m_iFlags    = SHM_FLAG_WRITE;
    return m_pHeader;
}

bool CTvuBaseShareMemory::_isShmRemovedFromKernal()
{
    return _is_shm_removed(m_iShmId);
}

uint8_t *CTvuBaseShareMemory::_open(const char *pMemoryName, bool bForWriting)
{
    if (NULL == m_pHeader)
    {
        strncpy(m_memoryName, pMemoryName, MAX_SHARE_MEMROY_NAME-1);
        int shm_id = -1;
        key_t key = -1;

        key = create_key_file(pMemoryName, 0);
        m_iKey  = key;

        if (-1 == key)
        {
            DEBUG_ERROR("Can not get key from file[%s]\n", pMemoryName);
            return NULL;
        }

        shm_id  = shmget(key, 0, 0);

        if (shm_id == -1)
        {
            DEBUG_ERROR("SHM Open[%s], key[0x%08x] did not have shmid, failed %d.\n",pMemoryName, key, errno);
            return NULL;
        }

        m_iShmId    = shm_id;

        if (bForWriting)
        {
            m_iFlags    = SHM_FLAG_WRITE;
        }
        else
        {
            m_iFlags    = SHM_FLAG_READ;
        }

        m_pHeader = (uint8_t *)shmat(shm_id, NULL, 0);

        if (m_pHeader == (void *) -1)
        {
            DEBUG_ERROR("SHM Open[%s] failed, key[0x%08x], shm attached failed, errno %d.\n",pMemoryName, key, errno);
            m_pHeader   = NULL;
            CloseMapFileAndSleep();
            return m_pHeader;
        }

        DEBUG_INFO("Open SHM Success %s %p\n", pMemoryName, m_pHeader);
        if (InitOpenShm() < 0) {
            DEBUG_ERROR("open init  %s failed\n",pMemoryName);
            CloseMapFileAndSleep();
        }

        DEBUG_INFO("open [name=>%s, key=>0x%x, id=>%d], ver %d, head len : %d, "\
            "counts : %d, item len : %d, mem address %p, build version{%s}, build version number %d\n"
            , m_memoryName, m_iKey, m_iShmId, m_uVersion, m_uHeadLen, m_uItemCounts, m_uItemLen, m_pHeader, BUILD_VERSION, BUILD_VERSION_NUM);
    }

    
    return m_pHeader;
}

uint8_t *CTvuBaseShareMemory::Open(const char *pMemoryName)
{
    return _open(pMemoryName, false);
}

static int _remove_shm_from_kernal(int shmid)
{
    int retval = 0;
    //OPT:: firstly not remove it, later optimize it
    retval = shmctl(shmid, IPC_RMID, NULL);
    if (retval < 0)
    {
        DEBUG_ERROR("remove shared memory failed\n");
    }
    else
    {
        DEBUG_INFO("remove shared memory from os, ret %d\n", retval);
    }
    return retval;
}

int CTvuBaseShareMemory::CloseMapFile()
{
    int retval  = 0;
    int shm_id  = -1;

    m_iKey  = -1;
    shm_id  = m_iShmId;

    if (m_pHeader)
    {
        DEBUG_INFO("deattached memory from process\n");
        shmdt((void *)m_pHeader);
        m_pHeader   = NULL;
    }

    if (shm_id != -1 && m_iFlags & SHM_FLAG_WRITE)
    {
        retval = _remove_shm_from_kernal(shm_id);
        remove_key_file(m_memoryName);
    }

    m_iShmId    = -1;
    return retval;
}

int CTvuBaseShareMemory::CloseMapFileAndSleep()
{
    int retval = CloseMapFile();

    /* 100ms, waiting other process close shm */
    _libshm_common_msleep(100);
    return retval;
}

int CTvuBaseShareMemory::RemoveShmFromKernal(const char *shmname)
{
    key_t key = -1;
    int shmid = -1;
    
    if (!shmname)
    {
        return 0;
    }

    key = create_key_file(shmname, 0);

    if (-1 == key)
    {
        DEBUG_INFO("could not get the key fronm shm[%s]\n", shmname);
        return 0;
    }

    shmid = shmget(key, 0, 0);

    if (shmid == -1)
    {
        DEBUG_INFO("could not get the shmid from shm[%s]\n", shmname);
        return 0;
    }

    return _remove_shm_from_kernal(shmid);
}

#endif

#endif


uint32_t    CTvuBaseShareMemory::GetItemLength()
{
    return m_uItemLen;
}

uint32_t    CTvuBaseShareMemory::GetItemCounts()
{
    return m_uItemCounts;
}

uint32_t    CTvuBaseShareMemory::GetItemOffset()
{
    return m_uHeadLen;
}

uint32_t    CTvuBaseShareMemory::GetShmVersion()
{
    return m_uVersion;
}

int     CTvuBaseShareMemory::GetShmFlag()
{
    return  m_iFlags;
}

uint32_t  CTvuBaseShareMemory::GetReadIndex()
{
    return m_uReadIndex;
}

void CTvuBaseShareMemory::SetReadIndex(uint32_t index)
{
    m_uReadIndex = index;
}

uint32_t    CTvuBaseShareMemory::GetShmVerBeforeCreate()
{
    return MEMHEADER_CURRENT_VERSION;
}

uint32_t    CTvuBaseShareMemory::GetExtBuffLen() { return m_uExtBufLen; }
uint32_t    CTvuBaseShareMemory::GetPreExtBuffSize() { return sizeof(shm_construct_ext_t); }

uint32_t    CTvuBaseShareMemory::GetWriteIndex()
{
    uint32_t        index           = 0;
    shm_construct_t   *p1           = (shm_construct_t*)m_pHeader;
    index    = LIBSHMMEDIA_READ_SHM_U32(p1->item_current);
    return  index;
}

uint8_t *CTvuBaseShareMemory::GetItemAddrByIndex(uint32_t index)
{
    shm_construct_t *p1         = (shm_construct_t*)m_pHeader;
    uint8_t         *pItem      = m_pHeader + LIBSHMMEDIA_READ_SHM_U32(p1->item_offset) \
                                  + 1LL * (index % LIBSHMMEDIA_READ_SHM_U32(p1->item_count))*LIBSHMMEDIA_READ_SHM_U32(p1->item_length);
    return pItem;
}

uint8_t *CTvuBaseShareMemory::GetWriteItemAddr()
{
    shm_construct_t *p1         = (shm_construct_t*)m_pHeader;
    uint8_t         *pItem      = m_pHeader + LIBSHMMEDIA_READ_SHM_U32(p1->item_offset) \
                                  + 1LL * (LIBSHMMEDIA_READ_SHM_U32(p1->item_current) % LIBSHMMEDIA_READ_SHM_U32(p1->item_count))*LIBSHMMEDIA_READ_SHM_U32(p1->item_length);
    return pItem;
}

void CTvuBaseShareMemory::SetFlag(uint32_t flag)
{
    if (flag)
        m_iFlags = flag;
    return;
}

void CTvuBaseShareMemory::FinishWrite()
{
    shm_construct_t *p1         = (shm_construct_t*)m_pHeader;

    if (m_iFlags & SHM_FLAG_WRITE) {
#if SHM_ENDIAN_CONVERT
        uint32_t windx = LIBSHMMEDIA_READ_SHM_U32(p1->item_current);
        p1->item_current = LIBSHMMEDIA_WRITE_SHM_U32(windx+1);
#else
        p1->item_current++;
#endif
    }
    return;
}

void CTvuBaseShareMemory::FinishRead()
{
    if (m_iFlags & SHM_FLAG_READ) {
        m_uReadIndex++;
    }
}

bool CTvuBaseShareMemory::HasReaders(unsigned int timeout)
{
    bool bret = true;
#if _SHM_HEAD_FEATURE_EXT_EABLE
    shm_construct_ext_t *pext = (shm_construct_ext_t *)(m_pHeader + SHM_MEDIA_HEAD_INFO_V4_OFFSET);
    switch(pext->ext_ver)
    {
        case kShmConstructExtVer1:
        {
            uint64_t now = _libshm_get_sys_ms64();
            uint64_t time_stamp = pext->last_read_time_stamp;

            if ((int64_t)(now - time_stamp) > timeout || (time_stamp == 0) )
            {
                bret = false;
            }
        }
        break;
        default:
        {

        }
        break;
    }
#endif

    return bret;
}

int CTvuBaseShareMemory::Sendable()
{
    if (m_iFlags & SHM_FLAG_WRITE) {
#ifdef TVU_LINUX
        if (_isShmRemovedFromKernal())
        {
            return -1;
        }
#endif
        return  1;  // model regards reading faster then writing
    } else {
        return -1;
    }
}

int CTvuBaseShareMemory::_readable(bool bClosed)
{
    if (m_iFlags & SHM_FLAG_READ)
    {
//        if (bClosed)
//        {
//            /**
//             * Aug 19th 2025.
//             * LLL, fixed looping read the old shm when it was closed & rindex much less than windex.
//            **/
//            uint32_t count = GetItemCounts();
//            shm_construct_t *p1 = (shm_construct_t *)m_pHeader;
//            uint32_t    r   = m_uReadIndex % count;
//            uint32_t    w   = p1->item_current % count;

//            if (r==w)
//            {
//                return 0;
//            }
//            else
//            {
//                return 1;
//            }
//        }
//        else
        {
            shm_construct_t *p1 = (shm_construct_t *)m_pHeader;
            uint32_t    r   = m_uReadIndex;
            uint32_t    w   = LIBSHMMEDIA_READ_SHM_U32(p1->item_current);
            int gap = w - r;
            uint32_t count = GetItemCounts();
            uint32_t maxgap = count;

            if (maxgap < 3)
            {
                maxgap = 3;
            }

            if (gap > 0 && (uint32_t)gap >= maxgap)
            {
                DEBUG_ERROR_CR("reading index step slowly."
                            "gap:%d,cnt:%u,r:%u,w:%u"
                            , gap, count,r,w
                            );
                r += count * (gap/count);
                m_uReadIndex = r;
                gap = w - r;
            }

            return gap;
        }
    }
    else
    {
        return -1;
    }
}

void CTvuBaseShareMemory::_setReadTime()
{
#if _SHM_HEAD_FEATURE_EXT_EABLE
    shm_construct_ext_t *pext = (shm_construct_ext_t *)(m_pHeader + SHM_MEDIA_HEAD_INFO_V4_OFFSET);
    switch(pext->ext_ver)
    {
        case kShmConstructExtVer1:
        {
            pext->last_read_time_stamp = _libshm_get_sys_ms64();
        }
        break;
        default:
        {

        }
        break;
    }
#endif
    return;
}

int CTvuBaseShareMemory::Readable(bool bClosed)
{
    int ret = _readable(bClosed);

#ifdef TVU_LINUX
    if (ret == 0)
    {
        if (_isShmRemovedFromKernal())
        {
            ret = -1;
        }
    }
#endif
    _setReadTime();
    return ret;
}

//static void printf_header(MEM_ITEM_HEADER_V1 * pItemHeader)
//{
//    LIBPLAYER_LOG_PRINT_OUT(AV_LOG_INFO, "%d %d %d %d %d %d %d %lld %lld %lld %d %d %d %d %d %lld %lld %lld %d %d %d"
//        , pItemHeader->version
//        , pItemHeader->length
//        , pItemHeader->videolen
//        , pItemHeader->audiolen
//        , pItemHeader->extralength
//        , pItemHeader->videofourcc
//        , pItemHeader->frametype
//        , pItemHeader->videocreatetime
//        , pItemHeader->videopts
//        , pItemHeader->videodts
//        , pItemHeader->width
//        , pItemHeader->height
//        , pItemHeader->duration
//        , pItemHeader->scale
//        , pItemHeader->audiofourcc
//        , pItemHeader->audiocreatetime
//        , pItemHeader->audiopts
//        , pItemHeader->audiodts
//        , pItemHeader->channels
//        , pItemHeader->depth
//        , pItemHeader->samplerate
//    );
//    return;
//}

bool CTvuBaseShareMemory::IsCreator()
{
    return (m_iFlags & SHM_FLAG_WRITE) ? true : false;
}

