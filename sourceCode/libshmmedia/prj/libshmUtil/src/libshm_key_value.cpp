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
#include "libshm_key_value.h"

namespace tvushm {

    KeyValParam::KeyValParam()
    {
        //BufferCtrlInit(&_buf);
    }

    KeyValParam::~KeyValParam()
    {
        Clear();
    }

    bool KeyValParam::IsEmpty(void) const
    {
        return (_paramMap.size()==0);
    }

    void KeyValParam::Clear()
    {
        //BufferCtrlRelease(&_buf);
        _paramMap.clear();
    }

    Variant&KeyValParam::GetParameter(Key key)
    {
        return _paramMap[key];
    }

    const Variant&KeyValParam::GetParameter(Key key) const
    {
        KeyValMap::const_iterator paramIt=_paramMap.find(key);
        if (paramIt==_paramMap.end())
        {
            return GetDefaultValue();
        }
        return paramIt->second;
    }

    bool KeyValParam::HasParameter(Key key) const
    {
        return _paramMap.count(key)>0;
    }

    void KeyValParam::SetParameter(Key key,const Value&value)
    {
        _paramMap[key].AssignFrom(value);
    }

    void KeyValParam::SetParameter(Key key,Value&value)
    {
        _paramMap[key].AssignFrom(value);
    }

    void KeyValParam::SetParameterByReference(Key key,Value&value)
    {
        _paramMap[key].AssignFromByReference(value);
    }

    int KeyValParam::AppendToBuffer(BufferController_t &buffer,bool noPrepare)
    {
        //fields:
        //number of key-values,
        //key-value: key, value.
        int totalBytesEncoded=0;
        int pos0 = BufferCtrlTellCurPos(&buffer);
        {
            if (!noPrepare)
            {
                int totalBytesNeeded=GetCompactBytesNumNeeded();
                BufferCtrlAllocBuf(&buffer, totalBytesNeeded);
            }

            FAILED_PUSH_DATA(BufferCtrlCompactEncodeValueU32(&buffer, _paramMap.size()));

            for (KeyValMap::iterator paramIt=_paramMap.begin();paramIt!=_paramMap.end();paramIt++)
            {
                Key key=paramIt->first;
                Value&value=paramIt->second;
                FAILED_PUSH_DATA(BufferCtrlCompactEncodeValueU32(&buffer, key));
                FAILED_PUSH_DATA(BufferCtrlCompactEncodeVariant(&buffer, value));
            }
            int pos1 = BufferCtrlTellCurPos(&buffer);
            totalBytesEncoded=pos1-pos0;
            //BufferCtrlSeek(&buffer, pos0, SEEK_SET);
        }
        return totalBytesEncoded;
    }

    int KeyValParam::ExtractFromBuffer(BufferController_t&buffer,bool referenceOnly)
    {
        int totalBytesDecoded=0;
        int pos0 = BufferCtrlTellCurPos(&buffer);
        {
            uint32 extractedParamsNum=0;
            {
                int bytesDecoded=BufferCtrlCompactDecodeValueU32(&buffer, extractedParamsNum);
                if (bytesDecoded<=0)
                {
                    BufferCtrlSeek(&buffer, pos0, SEEK_SET);
                    return bytesDecoded;
                }
            }
            _paramMap.clear();
            for (uint32 i=0;i<extractedParamsNum;i++)
            {
                Key extractedKey=0;
                {
                    int bytesDecoded=BufferCtrlCompactDecodeValueU32(&buffer, extractedKey);
                    if (bytesDecoded<=0)
                    {
                        BufferCtrlSeek(&buffer, pos0, SEEK_SET);
                        return bytesDecoded;
                    }
                }

                Value&value=_paramMap[extractedKey];
                {
                    int bytesDecoded=BufferCtrlCompactDecodeVariant(&buffer, value,referenceOnly);
                    if (bytesDecoded<=0)
                    {
                        BufferCtrlSeek(&buffer, pos0, SEEK_SET);
                        return bytesDecoded;
                    }
                }
            }
            int pos1 = BufferCtrlTellCurPos(&buffer);
            totalBytesDecoded=pos1-pos0;
            //BufferCtrlSeek(&buffer, pos0, SEEK_SET);
        }
        return totalBytesDecoded;
    }

    void KeyValParam::SetParamAsU8(Key k, uint8_t v)
    {
        KeyValMap &map = _paramMap;
        map[k].SetAsUint32(v, Variant::ByteType);
    }

    void KeyValParam::SetParamAsU16(Key k, uint16_t v)
    {
        KeyValMap &map = _paramMap;
        map[k].SetAsUint32(v, Variant::ShortType);
    }

    void KeyValParam::SetParamAsU32(Key k, uint32_t v)
    {
        KeyValMap &map = _paramMap;
        map[k].SetAsUint32(v);
    }

    void KeyValParam::SetParamAsU64(Key k, uint64_t v)
    {
        KeyValMap &map = _paramMap;
        map[k].SetAsUint64(v);
    }

    void KeyValParam::SetParamAsU128(Key k, const uint128_t &v)
    {
        KeyValMap &map = _paramMap;
        map[k].SetAsUint128(v);
    }

    void KeyValParam::SetParamAsString(Key k, const std::string &v)
    {
        KeyValMap &map = _paramMap;
        map[k].SetAsString(v);
    }

    void KeyValParam::SetParamAsBytes(Key k, const Bytes &v)
    {
        KeyValMap &map = _paramMap;
        map[k].SetAsBytes(v);
    }

    void KeyValParam::SetParamAsBytes(Key k, const uint8_t *p, uint32_t n)
    {
        Bytes o;
        o.AttachBuf(p, n);
        SetParamAsBytes(k, o);
        return;
    }

    void KeyValParam::SetParamAsBytesReference(Key k, const uint8_t *p, uint32_t n)
    {
        KeyValMap &map = _paramMap;
        map[k].SetAsBytesByReference((uint8_t *)p, n);
    }

    int KeyValParam::GetCompactBytesNumNeeded(void)
	{
		int totalBytesNeeded=0;
		uint32_t paramsNum=0;
		for (KeyValMap::iterator paramIt=_paramMap.begin();paramIt!=_paramMap.end();paramIt++)
		{
			Key key=paramIt->first;
			Value&value=paramIt->second;
			totalBytesNeeded+=BufferCtrlCompactGetBytesCountU32(key);
			totalBytesNeeded+=BufferCtrlCompactGetBytesCountVariant(value);
			paramsNum++;
		}
		totalBytesNeeded+=BufferCtrlCompactGetBytesCountU32(paramsNum);
		return totalBytesNeeded;
	}

    const Variant&KeyValParam::GetDefaultValue() const
    {
        static Value defaultValue;
		return defaultValue;
    }

    Variant&KeyValParam::GetDefaultValue()
    {
        static Value defaultValue;
		return defaultValue;
    }
}
