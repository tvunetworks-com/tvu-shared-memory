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
#ifndef VARIANT_H
#define VARIANT_H
#pragma once
#include "common_define.h"
#include "libshm_uint128.h"
#include "libshm_cache_buffer.h"

#include <stdint.h>
#include <string>

#ifdef Variant
#undef Variant
#endif

namespace tvushm
{
    typedef CacheBuffer Bytes ;
    class KeyValParam;
	class Variant
	{
	public:
		enum ValueType
		{
			NullType = 0,
			CharType = 1,
			ByteType = 2,
			ShortType = 3,
			WordType = 4,
			Int32Type = 5, //int32
			Uint32Type = 6,
			Int64Type = 7,
			Uint64Type = 8,
			FloatType = 9,
			DoubleType = 0x0a,
			Uint128Type = 0x0b,
			StringType = 0x0c, //utf8 string
			BytesType = 0x0d,
            KeyValueType = 0x0e /* Todo.keyvaluetype would be used for keyValueParam embeded keyValueParam */
		};
	public:
		Variant(void);
		~Variant(void);

        Variant(const char);
        Variant(const uint8_t);
        Variant(const uint16_t);
        Variant(const int32_t);
        Variant(const uint32_t);
        Variant(const uint64_t);
        Variant(const uint128_t&);
        Variant(const std::string&);
        Variant(const Bytes&);
        Variant(const Variant&);
    public:
        Variant&operator=(const char);
        Variant&operator=(const uint8_t);
        Variant&operator=(const uint16_t);
        Variant&operator=(const int32_t);
        Variant&operator=(const uint32_t);
        Variant&operator=(const uint64_t);
        Variant&operator=(const uint128_t&);
        Variant&operator=(const std::string&);
        Variant&operator=(const Bytes&);
        Variant&operator=(const Variant&);
//        bool operator==(const Variant&right) const;
//		bool operator<(const Variant&right) const;
	public:
		void Clear();
		ValueType GetType() const;
    public:

        int32 GetAsInt32(bool strict=false) const;
		void SetAsInt32(int32 value);

		int64 GetAsInt64(bool strict=false) const;
		void SetAsInt64(int64 value);

		uint32 GetAsUint32(bool strict=false) const;
		void SetAsUint32(uint32 value);

		void SetAsUint32(uint32 value,ValueType hintType);

		uint64 GetAsUint64(bool strict=false) const;
		void SetAsUint64(uint64 value);
		void SetAsUint64(uint64 value,ValueType hintType);

		double GetAsDouble(bool strict=false) const;
		void SetAsDouble(double value);

		const Uint128& GetAsUint128(bool strict=false) const;
		void SetAsUint128(const uint128_t& value);

        const std::string& GetAsString(bool strict=false) const;
		void SetAsString(const std::string&value);
		void SetAsString(const char*value,uint32_t len);

		const Bytes& GetAsBytes(bool strict=false) const;
		Bytes& GetAsBytes(bool strict=false);

		void SetAsBytes(const Bytes&value);
		void SetAsBytes(Bytes&value);
		void SetAsBytes(const uint8_t*bytes,uint32_t size);
		void SetAsBytesByReference(uint8_t*bytes,uint32_t size);

    public:
        Variant&AssignFrom(Variant&sourceValue);
        Variant&AssignFrom(const Variant&value);
        Variant&AssignFromByReference(Variant&sourceValue);
    private:
        const Uint128& GetAsDefaultUint128() const;
		const std::string& GetAsDefaultString() const;
		const Bytes& GetAsDefaultBytes() const;
		Bytes& GetAsDefaultBytes();

	private:
		void Initialize(void);

	private:
		ValueType _valueType;
		union
		{
			char _charValue;
			unsigned char _byteValue;
			short _shortValue;
			unsigned short _wordValue;
			int32 _int32Value;
			uint32 _uint32Value;
			int64 _int64Value;
			uint64 _uint64Value;
			float _floatValue;
			double _doubleValue;
			Uint128* _uint128ValuePtr;
            std::string* _stringValuePtr;
			Bytes* _bytesValuePtr;
            KeyValParam* _keyValuePtr;
		};
	};
}

#endif // VARIANT_H
