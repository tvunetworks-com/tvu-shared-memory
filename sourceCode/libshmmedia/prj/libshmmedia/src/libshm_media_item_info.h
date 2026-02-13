/*********************************************************
 * File:
 *  libshm_media_item_info.h
 * Description:
 *  used to manage the item information nodes for the all items of shm.
 *  For the reader could need to read out all item nodes at first for searching read.
 * -------------------------------------------------------
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

#ifndef LIBSHM_MEDIA_ITEM_INFO_H
#define LIBSHM_MEDIA_ITEM_INFO_H

#include "libshm_media_protocol.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <memory>

namespace tvushm {

    class ItemInfo
    {
    public:
        int     readRet_; // > 0 means data ready.
        uint32_t itemIdex_;
        libshm_media_head_param_t curHead_;
        libshm_media_item_param_t curItem_;
        libshmmedia_extend_data_info_t curExt_;
        bool        bGotTvutimestamp_;
        uint64_t    tvutimestamp_;
        uint64_t    pts_;
        ItemInfo()
        {
            Reset();
        }

        virtual ~ItemInfo()
        {
            Reset();
        }

        void Reset()
        {
            readRet_ = 0;
            itemIdex_ = 0;
            LibShmMediaHeadParamInit(&curHead_, sizeof(curHead_));
            LibShmMediaItemParamInit(&curItem_, sizeof(curItem_));
            memset(&curExt_, 0, sizeof(curExt_));
            bGotTvutimestamp_ = false;
            tvutimestamp_ = 0;
            pts_ = 0;
        }

        void Init()
        {
            Reset();
        }

        void Release()
        {
            Reset();
        }

        int GetReadingRet()const;
        bool IsReadingSuccess(uint32_t index)const;
        bool HasGottenTvutimestamp(uint32_t index)const;
        uint64_t GetTvutimestamp()const;
        uint64_t GetSafeTvutimestamp(uint32_t index)const;
        bool HasGottenPts(uint32_t index)const;
        uint64_t GetPts()const;

        static void HookInit(ItemInfo &);
        static void HookRelease(ItemInfo &);
    };

    typedef std::shared_ptr<ItemInfo> ItemInfoSharePtr;
    typedef std::function<void(ItemInfo&)> ItemInfoIntializedFun;
    typedef std::function<void(ItemInfo&)> ItemInfoDeintializedFun;
}

#endif // LIBSHM_MEDIA_ITEM_INFO_H
