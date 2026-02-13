
#include "libshm_media_item_info.h"

namespace tvushm {

    int ItemInfo::GetReadingRet()const
    {
        return readRet_;
    }

    bool ItemInfo::IsReadingSuccess(uint32_t index)const
    {
        return ((readRet_ > 0) && (itemIdex_ == index));
    }

    bool ItemInfo::HasGottenTvutimestamp(uint32_t index)const
    {
        if (!IsReadingSuccess(index))
        {
            return false;
        }

        return bGotTvutimestamp_;
    }

    uint64_t ItemInfo::GetTvutimestamp()const
    {
        return tvutimestamp_;
    }

    uint64_t ItemInfo::GetSafeTvutimestamp(uint32_t index)const
    {
        if (!HasGottenTvutimestamp(index))
        {
            return -1;
        }
        return tvutimestamp_;
    }

    bool ItemInfo::HasGottenPts(uint32_t index)const
    {
        return IsReadingSuccess(index);
    }

    uint64_t ItemInfo::GetPts()const
    {
        return pts_;
    }

    void ItemInfo::HookInit(ItemInfo &node){
        return node.Init();
    }

    void ItemInfo::HookRelease(ItemInfo &node){
        return node.Release();
    }

}

