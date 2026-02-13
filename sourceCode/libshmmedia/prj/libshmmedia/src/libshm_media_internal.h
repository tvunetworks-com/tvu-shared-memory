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

#ifndef _LIB_MEDIASHM_INTERNAL_H
#define _LIB_MEDIASHM_INTERNAL_H     

#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include "libshm_media_protocol.h"
#include "sharememory_internal.h"
#include "libshm_media.h"
#include "libshm_media_struct.h"
#include "libshm_media_raw_data_opt.h"
#include "libshm_media_audio_track_channel_proto_internal.h"
#include "libshm_media_item_info.h"
#include <malloc.h>
#include <assert.h>
#include <vector>

#define _LIBSHMMEDIA_PROTOCOL_APIS_DONE 1

class CLibShmMediaCtx
{
public:
    CLibShmMediaCtx()
    {
        m_uVersion      = 0;
        m_uItemVer      = 0;
        m_pShmObj       = NULL;
        m_bGotMediaHead = false;
        LibShmMediaHeadParamInit(&m_oMediaHead, sizeof(libshm_media_head_param_t));
        m_pOpaq         = NULL;
        m_fnReadCb      = NULL;
        m_i64LastSendSysTime = 0;
        //_itemIndex = 0;
    }

    ~CLibShmMediaCtx()
    {
        m_uVersion      = 0;
        m_uItemVer      = 0;
        if (m_pShmObj) {
            delete  m_pShmObj;
            m_pShmObj    = NULL;
        }
        memset((void *)this, 0, sizeof(CLibShmMediaCtx));
    }

    uint32_t GetWIndex()
    {
        CTvuBaseShareMemory    *pshm           = (CTvuBaseShareMemory *)m_pShmObj;
        return pshm->GetWriteIndex();
    }

    uint32_t GetRIndex()
    {
        CTvuBaseShareMemory    *pshm           = (CTvuBaseShareMemory *)m_pShmObj;
        return pshm->GetReadIndex();
    }

    void SetRIndex(uint32_t ind)
    {
        CTvuBaseShareMemory    *pshm           = (CTvuBaseShareMemory *)m_pShmObj;
        return pshm->SetReadIndex(ind);
    }

    const uint8_t* GetHeadAddr()
    {
        CTvuBaseShareMemory    *pshm           = (CTvuBaseShareMemory *)m_pShmObj;
        return pshm->GetHeader();
    }

    uint32_t GetHeadLen()
    {
        CTvuBaseShareMemory    *pshm           = (CTvuBaseShareMemory *)m_pShmObj;
        return pshm->GetHeadLen();
    }

    uint32_t GetHeadVer()
    {
        CTvuBaseShareMemory    *pshm           = (CTvuBaseShareMemory *)m_pShmObj;
        return pshm->GetShmVersion();
    }


    uint32_t GetItemLen()
    {
        CTvuBaseShareMemory    *pshm           = (CTvuBaseShareMemory *)m_pShmObj;
        return pshm->GetItemLength();
    }

    uint32_t GetItemCounts()
    {
        CTvuBaseShareMemory    *pshm           = (CTvuBaseShareMemory *)m_pShmObj;
        return pshm->GetItemCounts();
    }

    uint32_t GetItemOffset()
    {
        CTvuBaseShareMemory    *pshm           = (CTvuBaseShareMemory *)m_pShmObj;
        return pshm->GetItemOffset();
    }

    int PollSendable(unsigned int timeout)
    {
        CTvuBaseShareMemory    *pshm           = (CTvuBaseShareMemory *)m_pShmObj;
        if (CheckCloseFlag()) {
            const char      *pShmName       = pshm->GetName();
            DEBUG_INFO("share memory[%s] server close flag, shoule destroy shm object at once\n", pShmName);
            return -1;
        }
        return pshm->Sendable();
    }

    int PollReadable(unsigned int timeout)
    {
        CTvuBaseShareMemory    *pshm           = (CTvuBaseShareMemory *)m_pShmObj;
        int64_t         t1              = _libshm_get_sys_ms64();
        int64_t         t2              = 0;
        int             ret             = -1;

        while (1)
        {
            bool bClosed = CheckCloseFlag();
            ret = pshm->Readable(bClosed);

            if (ret > 0 || ret < 0)
                break;

            if (bClosed) {
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

    uint32_t    GetItemVersion()
    {
        return m_uItemVer;
    }

    const char *GetName()
    {
        CTvuBaseShareMemory    *pshm           = (CTvuBaseShareMemory *)m_pShmObj;
        return pshm->GetName();
    }

    bool IsCreator()
    {
        CTvuBaseShareMemory    *pshm           = (CTvuBaseShareMemory *)m_pShmObj;
        return pshm->IsCreator();
    }

    bool bHasReaders(unsigned int timeout = 100)
    {
        return m_pShmObj->HasReaders(timeout);
    }

    int CreateShmEntry(const char * pMemoryName, uint32_t header_len, uint32_t item_count, uint32_t item_length );
    int CreateShmEntry(const char * pMemoryName, uint32_t header_len, uint32_t item_count, uint32_t item_length, mode_t mode);
    int OpenShmEntry(const char * pMemoryName, libshm_media_readcb_t cb, void *opaq);
    void SetCloseFlag(bool bclose);
    bool CheckCloseFlag();
    uint8_t *GetItemDataAddr(uint32_t index);
    inline int FinishWrite();

#ifdef _LIBSHMMEDIA_PROTOCOL_APIS_DONE
    int SendHead(const libshm_media_head_param_t *pmh);
    int SendData(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi);
    int SendDataWithFrequency1000(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi);
    int PollReadHead(libshm_media_head_param_t *pmh, unsigned int timeout);
    int PollReadData(libshm_media_head_param_t *pmh, libshm_media_item_param_t   *pmi
                     , libshmmedia_extend_data_info_t *pext
                     , unsigned int timeout);
    int PollReadDataWithoutIndexStep(libshm_media_head_param_t *pmh
                                     , libshm_media_item_param_t   *pmi
                                     , libshmmedia_extend_data_info_t *pext
                                     , unsigned int timeout);
    int PollReadDateWithTvutimestamp(libshm_media_head_param_t *pmh, libshm_media_item_param_t   *pmi
                                     , libshmmedia_extend_data_info_t *pext
                                     , bool *bFoundTvutimestamp
                                     , bool *bFoundPts
                                     , uint64_t tvutimestamp
                                     , char type, uint64_t pts
                                     );

    int ReadItemData(libshm_media_head_param_t *pmh, libshm_media_item_param_t   *pmi
                     , libshmmedia_extend_data_info_t *pext
                     , unsigned int rindex);
     /*just need parse data out, not need feedback for this API.*/
    int ReadItemData2(unsigned int rindex, libshm_media_head_param_t &mh, libshm_media_item_param_t &mi, libshmmedia_extend_data_info_t &ext);
#else
    int SendHead(const libshm_media_head_param_t *pmh);
    int SendData(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi);
    int PollReadHead(libshm_media_head_param_t *pmh, bool bCheckReadable, unsigned int timeout);
    int _readV4Data(libshm_media_item_param_t   *pmi, const uint32_t rindex, const uint8_t *pItemAddr);
    int PollReadDataWithoutIndexStep(libshm_media_head_param_t *pmh, libshm_media_item_param_t   *pmi, unsigned int timeout);
    int PollReadData(libshm_media_head_param_t *pmh, libshm_media_item_param_t   *pmi, unsigned int timeout);
#endif
    int FinishRead();
    uint32_t SetReadIndex(char type, int64_t pts);
private:
    class ResultRecorder
    {
    public:
        int  cmpRet;
        uint32_t itemIdx;
        const tvushm::ItemInfo *pItem;
        ResultRecorder()
        {
            cmpRet = 0;
            itemIdx = 0;
            pItem = NULL;
        }
        virtual ~ResultRecorder()
        {

        }
        const tvushm::ItemInfo &GetItem()const
        {
            assert(pItem);
            return *pItem;
        }
    };

    int _readOutItemInfor(
        uint32_t rindex
        ,libshm_media_head_param_t &oh
        ,libshm_media_item_param_t &op
        ,libshmmedia_extend_data_info_t &ext
        ,bool &gotTvutimestamp
        ,uint64_t &tvutimestamp
    );

    /**
     * it must return the valid item point.
     **/
    const tvushm::ItemInfo&
    _readOutItemInfor(uint32_t rindex);

    int _readOutItemInfor(uint32_t rindex, ResultRecorder &rec);
    int _readOutFirstItemInfor(ResultRecorder &rec);

    /**
    * return the reading status.
    **/
    int _cmpMatchingTvutimestamp(
        uint32_t rindex
        , const uint64_t &tvutimestamp
        , ResultRecorder &rec
        );

    /**
    * return the reading status.
    **/
    int _cmpMatchingPts(
        uint32_t rindex
        , const uint64_t &pts
        , ResultRecorder &rec
        );
public:
    bool SearchItemWithTvutimestamp(
        const uint64_t &tvutimestamp
        , libshm_media_head_param_t *pmh
        , libshm_media_item_param_t *pmi
        , libshmmedia_extend_data_info_t *pext
        );

    typedef std::function<bool(const uint64_t &)> FnTimestampValid_t;

    typedef std::function<int (
        uint32_t , const uint64_t &
        , ResultRecorder &
        )> FnCmpFetchingItem_t;

    typedef std::function<uint64_t (const tvushm::ItemInfo *)> FnGetItemTimeVal_t;
    typedef std::function<bool (const tvushm::ItemInfo *,uint32_t)> FnHasGottenTimeVal_t;
    typedef std::function<int64_t (uint64_t , uint64_t )> FnTimestampValMinus_t;

    bool SearchFirstItemMatching(const uint64_t &tvutimestamp
                                 , ResultRecorder &matchingItem, FnTimestampValid_t fnTimeValid, FnCmpFetchingItem_t fnCmp
                                 , FnGetItemTimeVal_t fnGetTime
                                 , FnHasGottenTimeVal_t fnHasGotTime
                                 , FnTimestampValMinus_t fnMinus
                                 , const char *module
                                 );

    bool SearchTheFirstMatchingItemWithTvutimestamp(
        const uint64_t &tvutimestamp
        , ResultRecorder &matchingItem
        );
    bool SearchTheFirstMatchingItemWithPts(
        const uint64_t &pts
        , ResultRecorder &matchingItem
        );
#if defined(TVU_LINUX)
    static int RemoveShm(const char *pshmname);
#endif

    int PollReadRawData(libshmmedia_raw_head_param_t   *pmh, libshmmedia_raw_data_param_t   *pmi, unsigned int timeout);
    int ApplyRawData(libshmmedia_raw_data_param_t   *pmi);
    uint8_t *ApplyRawData(size_t len);
    int CommitRawData(size_t commit_len);

    int ApplyItemBuffer(const libshm_media_item_param_t *pmi, libshm_media_item_addr_layout_t *pLayout);
    int CommitItemBuffer(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi);
    private:
        int _sendV4Data(const libshm_media_head_param_t *pmh, const libshm_media_item_param_t *pmi, uint8_t *pItemAddr, uint32_t item_size);
    private:
        uint32_t                    m_uVersion;
        uint32_t                    m_uItemVer;
        CTvuBaseShareMemory         *m_pShmObj;
        bool                        m_bGotMediaHead;
        libshm_media_head_param_t   m_oMediaHead;
        void                        *m_pOpaq;
        libshm_media_readcb_t       m_fnReadCb;
        int64_t                     m_i64LastSendSysTime;
        std::vector<tvushm::ItemInfo>_itemNodes; // this is thread safe for it would be read at one APIs.
};

#endif
