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
/******************************************************************
 *  Description:
 *      libshm internal head files
 *
 *  History:
 *      May 14th, 2015, lotus initialized it
 *
*******************************************************************/

#ifndef _LIB_MEDIASHM_VARIABLE_ITEM_INTERNAL_H
#define _LIB_MEDIASHM_VARIABLE_ITEM_INTERNAL_H     

#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include "sharememory.h"
#include "shm_variable_item_ring_buff.h"
#include "libshm_media_struct.h"
#include "libshm_media_protocol.h"
#include "libshm_media_variable_item.h"
#include "libshm_media_protocol_internal.h"

#if _TVU_VIARIABLE_SHM_FEATURE_ENABLE

#define _LIBSHMMEDIA_PROTOCOL_APIS_DONE 1

class CTvuVariableItemRingShmCtx
{
private:
    uint32_t                    m_uVersion;
    CTvuVariableItemBaseShm     *m_pShmObj;
    bool                        m_bGotMediaHead;
    libshm_media_head_param_t   m_oMediaHead;
    void                        *m_pOpaq;
    libshm_media_readcb_t       m_fnReadCb;
    int64_t                     m_i64LastSendSysTime;
public:
    CTvuVariableItemRingShmCtx()
    {
        m_uVersion      = 0;
        m_pShmObj       = NULL;
        m_bGotMediaHead = false;
        memset((void *)&m_oMediaHead, 0, sizeof(libshm_media_head_param_t));
        m_pOpaq         = NULL;
        m_fnReadCb      = NULL;
        m_i64LastSendSysTime = 0;
    }

    ~CTvuVariableItemRingShmCtx()
    {
        m_uVersion    = 0;
        if (m_pShmObj) {
            delete  m_pShmObj;
            m_pShmObj    = NULL;
        }
        memset((void *)this, 0, sizeof(CTvuVariableItemRingShmCtx));
    }

    uint32_t GetShmTotalSize()
    {
        CTvuVariableItemBaseShm    *pshm           = (CTvuVariableItemBaseShm *)m_pShmObj;
        uint64_t ih = 0, is = 0, ic = 0;
        pshm->GetInfo(&ih, &is, &ic);
        return is;
    }

    uint32_t GetItemCounts()
    {
        CTvuVariableItemBaseShm    *pshm           = (CTvuVariableItemBaseShm *)m_pShmObj;
        return pshm->GetItemCounts();
    }

    uint32_t GetHeadLen()
    {
        CTvuVariableItemBaseShm    *pshm = (CTvuVariableItemBaseShm *)m_pShmObj;
        return pshm->GetHeadLen();
    }

    uint64_t GetWIndex()
    {
        CTvuVariableItemBaseShm    *pshm = (CTvuVariableItemBaseShm *)m_pShmObj;
        return pshm->GetWriteIndex();
    }

    uint64_t GetRIndex()
    {
        CTvuVariableItemBaseShm    *pshm = (CTvuVariableItemBaseShm *)m_pShmObj;
        return pshm->GetReadIndex();
    }

    int PollSendable(uint32_t timeout)
    {
        CTvuVariableItemBaseShm    *pshm           = (CTvuVariableItemBaseShm *)m_pShmObj;
        if (CheckCloseFlag()) {
            const char      *pShmName       = pshm->GetName();
            DEBUG_INFO("share memory[%s] server close flag, shoule destroy shm object at once\n", pShmName);
            return -1;
        }
        return pshm->Sendable();
    }

    int PollReadable(uint32_t timeout)
    {
        CTvuVariableItemBaseShm    *pshm           = (CTvuVariableItemBaseShm *)m_pShmObj;
        int64_t         t1              = _libshm_get_sys_ms64();
        int64_t         t2              = 0;
        int             ret             = -1;

        while (1)
        {
            ret     = pshm->Readable();

            if (ret > 0 || ret < 0)
                break;

            if (CheckCloseFlag()) {
                const char      *pShmName       = pshm->GetName();
                DEBUG_INFO("share memory[%s] client got close flag, shoule destroy shm object at once\n", pShmName);
                ret     = -1;
                break;
            }

            if (timeout <= 0)
                break;

            t2  = _libshm_get_sys_ms64();

            if (t2 >= t1 + timeout) {
                //DEBUG_INFO("readable timeout %d\n", timeout);
                break;
            }

            _libshm_common_msleep(1);
        }

        return ret;
    }

    uint32_t    GetVersion()
    {
        return m_uVersion;
    }

    const char *GetName()
    {
        CTvuVariableItemBaseShm    *pshm           = (CTvuVariableItemBaseShm *)m_pShmObj;
        return pshm->GetName();
    }

    bool IsCreator()
    {
        CTvuVariableItemBaseShm    *pshm           = (CTvuVariableItemBaseShm *)m_pShmObj;
        return pshm->IsCreator();
    }

    bool bHasReaders(unsigned int timeout = 100)
    {
        return m_pShmObj->HasReaders(timeout);
    }

    int CreateShmEntry(const char * pMemoryName, const uint32_t header_len, const uint32_t item_count, const uint64_t item_length);
    int CreateShmEntry(const char * pMemoryName, const uint32_t header_len, const uint32_t item_count, const uint64_t item_length, mode_t mode);
    int OpenShmEntry(const char * pMemoryName, libshm_media_readcb_t cb, void *opaq);
    void SetCloseFlag(bool bclose);
    bool CheckCloseFlag();
    uint8_t *GetItemDataAddr(uint32_t index);
    int SendHead(const libshm_media_head_param_t *pmh);
    int SendData(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi);
    int SendDataWithFrequency1000(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi);
    int PollReadHead(libshm_media_head_param_t *pmh, uint32_t timeout);
    int PollReadData(libshm_media_head_param_t *pmh, libshm_media_item_param_t   *pmi, uint32_t timeout);
    int PollReadDataWithoutIndexStep(libshm_media_head_param_t *pmh, libshm_media_item_param_t   *pmi, uint32_t timeout);
    int ReadIndexStep();
    void SeekReadIndexToWriteIndex();
    void SeekReadIndexToZero();
    void SeekReadIndex(uint64_t rindex);
    uint8_t *applyBuffer(unsigned int size);
    bool commitBuffer(uint8_t *pItemAddr, unsigned int size);

public: /* raw data apis */
    int PollReadRawData(libshmmedia_raw_head_param_t   *pmh, libshmmedia_raw_data_param_t   *pmi, unsigned int timeout);
    uint8_t *applyRawData(size_t len);
    int     commitRawData(const uint8_t *buf, size_t size);

public:
    bool SearchItems(void *, libshmmedia_item_checking_fn_t fn);
    /**
     *  <0 : invalid data
     *  0  : not the searched item
     *  >0 : the searched item.
    **/
    int  IsTheSearchingItem(const void *buf, size_t nbuf, uint64_t pos, void *user, libshmmedia_item_checking_fn_t fn);
public:
    static int RemoveShm(const char *pshmname);
private:
        int _sendV3Data(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi);
        int _sendV4Data(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi);
        uint8_t *_applyV4Buffer(unsigned int size);
        bool _commitV4Buffer(uint8_t *pItemAddr, unsigned int size);
        int _writeV4Buffer(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi
                           , const libshm_media_item_param_internal_t &rii, uint8_t *pItemAddr);
#ifndef _LIBSHMMEDIA_PROTOCOL_APIS_DONE
        int _readV3Data(libshm_media_item_param_t   *pmi, const uint8_t *pItemAddr);
        int _readV4Data(libshm_media_item_param_t   *pmi, const uint8_t *pItemAddr);
#endif
};
#endif // _TVU_VIARIABLE_SHM_FEATURE_ENABLE

#endif
