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
#include "libshm_variant.h"
#include <stdexcept>
#include <stdlib.h>

namespace tvushm
{
	Variant::Variant(void)
	{
		Initialize();
	}

	Variant::~Variant(void)
	{
		Clear();
	}


    Variant::Variant(const char v)
    {
        SetAsUint32(v, CharType);
    }

    Variant::Variant(const uint8_t v)
    {
        SetAsUint32(v, ByteType);
    }

    Variant::Variant(const uint16_t v)
    {
        SetAsUint32(v, ShortType);
    }

    Variant::Variant(const int32_t v)
    {
        SetAsInt32(v);
    }

    Variant::Variant(const uint32_t v)
    {
        SetAsUint32(v);
    }

    Variant::Variant(const uint64_t v)
    {
        SetAsUint64(v);
    }

    Variant::Variant(const uint128_t&v)
    {
        SetAsUint128(v);
    }

    Variant::Variant(const std::string&v)
    {
        SetAsString(v);
    }

    Variant::Variant(const Bytes&v)
    {
        SetAsBytes(v);
    }

    Variant::Variant(const Variant&value)
	{
		AssignFrom(value);
	}

    Variant&Variant::operator=(const char v)
    {
        SetAsUint32(v, CharType);
        return *this;
    }

    Variant&Variant::operator=(const uint8_t v)
    {
        SetAsUint32(v, ByteType);
        return *this;
    }

    Variant&Variant::operator=(const uint16_t v)
    {
        SetAsUint32(v, ShortType);
        return *this;
    }

    Variant&Variant::operator=(const int32_t v)
    {
        SetAsInt32(v);
        return *this;
    }

    Variant&Variant::operator=(const uint32_t v)
    {
        SetAsUint32(v);
        return *this;
    }

    Variant&Variant::operator=(const uint64_t v)
    {
        SetAsUint64(v);
        return *this;
    }

    Variant&Variant::operator=(const uint128_t&v)
    {
        SetAsUint128(v);
        return *this;
    }

    Variant&Variant::operator=(const std::string&v)
    {
        SetAsString(v);
        return *this;
    }

    Variant&Variant::operator=(const Bytes&v)
    {
        SetAsBytes(v);
        return *this;
    }

    Variant&Variant::operator=(const Variant&value)
	{
		return AssignFrom(value);
	}

	void Variant::Initialize(void)
	{
		_valueType=NullType;
		_uint64Value=0;
	}

	void Variant::Clear()
	{
		if (_valueType==NullType)
		{
			return;
		}
		switch(_valueType)
		{
		case Uint128Type:
			if (_uint128ValuePtr!=NULL)
			{
				delete _uint128ValuePtr;
				_uint128ValuePtr=NULL;
			}
			break;
		case StringType:
			if (_stringValuePtr!=NULL)
			{
				delete _stringValuePtr;
				_stringValuePtr=NULL;
			}
			break;
		case BytesType:
			if (_bytesValuePtr!=NULL)
			{
				delete _bytesValuePtr;
				_bytesValuePtr=NULL;
			}
			break;
		default:
			break;
		}
		_valueType=NullType;
		_uint64Value=0;
	}

    Variant::ValueType Variant::GetType() const
	{
		return _valueType;
	}


    int32 Variant::GetAsInt32(bool strict) const
	{
		switch(_valueType)
		{
		case NullType:
			return 0;
		case Variant::CharType:
		case Variant::ByteType:
		case Variant::ShortType:
		case Variant::WordType:
		case Variant::Int32Type:
		case Variant::Uint32Type:
			return _int32Value;
		case Variant::Uint64Type:
		case Variant::Int64Type:
			return (int32)_uint64Value;
		case Variant::FloatType:
			return (int32)_floatValue;
		case Variant::DoubleType:
			return (int32)_doubleValue;
		default:
			{
				return 0;
			}
		}
	}

	void Variant::SetAsInt32(int32 value)
	{
		Clear();
		_valueType=Int32Type;
		_int32Value=value;
	}

	uint32 Variant::GetAsUint32(bool strict) const
	{
		switch(_valueType)
		{
		case NullType:
			return 0;
		case Variant::CharType:
		case Variant::ByteType:
		case Variant::ShortType:
		case Variant::WordType:
		case Variant::Int32Type:
		case Variant::Uint32Type:
			return _uint32Value;
		case Variant::Uint64Type:
		case Variant::Int64Type:
			return (uint32)_uint64Value;
		case Variant::FloatType:
			return (uint32)_floatValue;
		case Variant::DoubleType:
			return (uint32)_doubleValue;
		default:
			{
				return 0;
			}
		}
	}

	void Variant::SetAsUint32(uint32 value)
	{
		Clear();
		_valueType=Uint32Type;
		_uint32Value=value;
	}

	void Variant::SetAsUint32(uint32 value,ValueType hintType)
	{
		Clear();
		switch(hintType)
		{
		case Variant::CharType:
		case Variant::ByteType:
		case Variant::ShortType:
		case Variant::WordType:
		case Variant::Int32Type:
		case Variant::Uint32Type:
		case Variant::FloatType:
			_valueType=hintType;
			_uint32Value=value;
			break;
		default:
			_valueType=Uint32Type;
			_uint32Value=value;
			break;
		}
	}


	int64 Variant::GetAsInt64(bool strict) const
	{
		switch(_valueType)
		{
		case NullType:
			return 0;
		case Variant::CharType:
		case Variant::ByteType:
		case Variant::ShortType:
		case Variant::WordType:
		case Variant::Int32Type:
		case Variant::Uint32Type:
			return _int32Value;
		case Variant::Uint64Type:
		case Variant::Int64Type:
			return (int64)_int64Value;
		case Variant::FloatType:
			return (int64)_floatValue;
		case Variant::DoubleType:
			return (int64)_doubleValue;
		default:
			{
				return 0;
			}
		}
	}

	void Variant::SetAsInt64(int64 value)
	{
		Clear();
		_valueType=Int64Type;
		_int64Value=value;
	}

	uint64 Variant::GetAsUint64(bool strict) const
	{
		switch(_valueType)
		{
		case NullType:
			return 0;
		case Variant::CharType:
		case Variant::ByteType:
		case Variant::ShortType:
		case Variant::WordType:
		case Variant::Int32Type:
		case Variant::Uint32Type:
			return _uint32Value;
		case Variant::Uint64Type:
		case Variant::Int64Type:
			return (uint64)_uint64Value;
		case Variant::FloatType:
			return (uint64)_floatValue;
		case Variant::DoubleType:
			return (uint64)_doubleValue;
		default:
			{
				return 0;
			}
		}
	}

	void Variant::SetAsUint64(uint64 value)
	{
		Clear();
		_valueType=Uint64Type;
		_uint64Value=value;
	}

	void Variant::SetAsUint64(uint64 value,ValueType hintType)
	{
		Clear();
		switch(hintType)
		{
		case Variant::Int64Type:
		case Variant::Uint64Type:
		case Variant::DoubleType:
			_valueType=hintType;
			_uint64Value=value;
			break;
		default:
			_valueType=Uint64Type;
			_uint64Value=value;
			break;
		}
	}

	double Variant::GetAsDouble(bool strict) const
	{
		switch(_valueType)
		{
		case NullType:
			return 0;
		case Variant::CharType:
			return (double)_charValue;
		case Variant::ShortType:
			return (double)_shortValue;
		case Variant::Int32Type:
			return (double)_int32Value;
		case Variant::ByteType:
		case Variant::WordType:
		case Variant::Uint32Type:
			return _uint32Value;
		case Variant::Uint64Type:
			return (double)_uint64Value;
		case Variant::Int64Type:
			return (double)_int64Value;
		case Variant::FloatType:
			return (double)_floatValue;
		case Variant::DoubleType:
			return _doubleValue;
		default:
			{
				return 0;
			}
		}
	}

	void Variant::SetAsDouble(double value)
	{
		Clear();
		_valueType=DoubleType;
		_doubleValue=value;
	}

	const Uint128& Variant::GetAsUint128(bool strict) const
	{
		switch(_valueType)
		{
		case NullType:
			return GetAsDefaultUint128();
		case Uint128Type:
			if (_uint128ValuePtr==NULL)
			{
				return GetAsDefaultUint128();
			}
			else
			{
				return *_uint128ValuePtr;
			}
		default:
			{
				return GetAsDefaultUint128();
			}
		}
	}

	void Variant::SetAsUint128(const uint128_t& value)
	{
		if (_valueType==Uint128Type)
		{
			if (_uint128ValuePtr==NULL)
			{
				_uint128ValuePtr=new Uint128();
                if (!_uint128ValuePtr)
                {
                    return;
                }
			}
			_uint128ValuePtr->SetValue(value);
		}
		else
		{
			Clear();
			_valueType=Uint128Type;
			_uint128ValuePtr=new Uint128();
            if (!_uint128ValuePtr)
            {
                return;
            }
			_uint128ValuePtr->SetValue(value);
		}
	}


    const std::string& Variant::GetAsString(bool strict) const
	{
		switch(_valueType)
		{
		case NullType:
			return GetAsDefaultString();
		case StringType:
			if (_stringValuePtr==NULL)
			{
				return GetAsDefaultString();
			}
			else
			{
				return *_stringValuePtr;
			}
		default:
			{
				return GetAsDefaultString();
			}
		}
	}

	void Variant::SetAsString(const std::string&value)
	{
		if (_valueType==StringType)
		{
			if (_stringValuePtr==NULL)
			{
				_stringValuePtr=new std::string(value);
			}
			else
			{
				_stringValuePtr->assign(value);
			}
		}
		else
		{
			Clear();
			_valueType=StringType;
			_stringValuePtr=new std::string(value);
		}
	}

	void Variant::SetAsString(const char*value,uint32_t len)
	{
		if (_valueType==StringType)
		{
			if (_stringValuePtr==NULL)
			{
				_stringValuePtr=new std::string(value,len);
			}
			else
			{
				_stringValuePtr->assign(value,len);
			}
		}
		else
		{
			Clear();
			_valueType=StringType;
			_stringValuePtr=new std::string(value,len);
		}
	}

	const Bytes& Variant::GetAsBytes(bool strict) const
	{
		switch(_valueType)
		{
		case NullType:
			return GetAsDefaultBytes();
		case BytesType:
			if (_bytesValuePtr==NULL)
			{
				return GetAsDefaultBytes();
			}
			else
			{
				return *_bytesValuePtr;
			}
		default:
			{
				return GetAsDefaultBytes();
			}
		}
	}

	Bytes& Variant::GetAsBytes(bool strict)
	{
		switch(_valueType)
		{
		case NullType:
			return GetAsDefaultBytes();
		case BytesType:
			if (_bytesValuePtr==NULL)
			{
				return GetAsDefaultBytes();
			}
			else
			{
				return *_bytesValuePtr;
			}
		default:
			{
				return GetAsDefaultBytes();
			}
		}
	}

	void Variant::SetAsBytes(const Bytes&value)
	{
		if (_valueType==BytesType)
		{
			if (_bytesValuePtr==NULL)
			{
				_bytesValuePtr=new Bytes();
                if (!_bytesValuePtr)
                {
                    return;
                }
			}
			_bytesValuePtr->Copy(value.GetBufAddr(), value.GetBufLen());
		}
		else
		{
			Clear();
			_valueType=BytesType;
            _bytesValuePtr=new Bytes();
            if (!_bytesValuePtr)
            {
                return;
            }
			_bytesValuePtr->Copy(value.GetBufAddr(), value.GetBufLen());
		}
	}

	void Variant::SetAsBytes(Bytes&value)
	{
		if (_valueType==BytesType)
		{
            if (_bytesValuePtr==NULL)
			{
				_bytesValuePtr=new Bytes();
                if (!_bytesValuePtr)
                {
                    return;
                }
			}
			_bytesValuePtr->Copy(value.GetBufAddr(), value.GetBufLen());
		}
		else
		{
            Clear();
			_valueType=BytesType;
            _bytesValuePtr=new Bytes();
            if (!_bytesValuePtr)
            {
                return;
            }
			_bytesValuePtr->Copy(value.GetBufAddr(), value.GetBufLen());
		}
	}

	void Variant::SetAsBytes(const uint8_t*bytes,uint32_t size)
	{
		if (_valueType==BytesType)
		{
            if (_bytesValuePtr==NULL)
			{
				_bytesValuePtr=new Bytes();
                if (!_bytesValuePtr)
                {
                    return;
                }
			}
			_bytesValuePtr->Copy(bytes, size);
		}
		else
		{
            Clear();
			_valueType=BytesType;
            _bytesValuePtr=new Bytes();
            if (!_bytesValuePtr)
            {
                return;
            }
			_bytesValuePtr->Copy(bytes, size);
		}
	}

	void Variant::SetAsBytesByReference(uint8_t*bytes,uint32_t size)
	{
		if (_valueType==BytesType)
		{
            if (_bytesValuePtr==NULL)
			{
				_bytesValuePtr=new Bytes();
                if (!_bytesValuePtr)
                {
                    return;
                }
			}
			_bytesValuePtr->AttachBuf(bytes, size);
		}
		else
		{
            Clear();
			_valueType=BytesType;
            _bytesValuePtr=new Bytes();
            if (!_bytesValuePtr)
            {
                return;
            }
			_bytesValuePtr->AttachBuf(bytes, size);
		}
	}

    Variant&Variant::AssignFrom(Variant&value)
	{
		Clear();
		switch(value._valueType)
		{
		case NullType:
			break;
		case Variant::CharType:
		case Variant::ByteType:
		case Variant::ShortType:
		case Variant::WordType:
		case Variant::Int32Type:
		case Variant::Uint32Type:
		case Variant::Uint64Type:
		case Variant::Int64Type:
		case Variant::FloatType:
		case Variant::DoubleType:
			_valueType=value._valueType;
			_uint64Value=value._uint64Value;
			break;
		case Variant::Uint128Type:
			_valueType=value._valueType;
			if (value._uint128ValuePtr!=NULL)
			{
				_uint128ValuePtr=new Uint128(*value._uint128ValuePtr);
			}
			break;
		case Variant::StringType:
			_valueType=value._valueType;
			if (value._stringValuePtr!=NULL)
			{
				_stringValuePtr=new std::string(*value._stringValuePtr);
			}
			break;
		case Variant::BytesType:
			_valueType=value._valueType;
			if (value._bytesValuePtr!=NULL)
			{
                Bytes &s = *value._bytesValuePtr;
				_bytesValuePtr=new Bytes();
                if (!_bytesValuePtr)
                {
                    break;
                }
                _bytesValuePtr->Copy(s.GetBufAddr(), s.GetBufLen());
			}
			break;
		default:
            break;
		}
		return *this;
	}

    Variant&Variant::AssignFrom(const Variant&value)
	{
        Clear();
		switch(value._valueType)
		{
		case NullType:
			break;
		case Variant::CharType:
		case Variant::ByteType:
		case Variant::ShortType:
		case Variant::WordType:
		case Variant::Int32Type:
		case Variant::Uint32Type:
		case Variant::Uint64Type:
		case Variant::Int64Type:
		case Variant::FloatType:
		case Variant::DoubleType:
			_valueType=value._valueType;
			_uint64Value=value._uint64Value;
			break;
		case Variant::Uint128Type:
			_valueType=value._valueType;
			if (value._uint128ValuePtr!=NULL)
			{
				_uint128ValuePtr=new Uint128(*value._uint128ValuePtr);
			}
			break;
		case Variant::StringType:
			_valueType=value._valueType;
			if (value._stringValuePtr!=NULL)
			{
				_stringValuePtr=new std::string(*value._stringValuePtr);
			}
			break;
		case Variant::BytesType:
			_valueType=value._valueType;
			if (value._bytesValuePtr!=NULL)
			{
                Bytes &s = *value._bytesValuePtr;
				_bytesValuePtr=new Bytes();
                if (!_bytesValuePtr)
                {
                    break;
                }
                _bytesValuePtr->Copy(s.GetBufAddr(), s.GetBufLen());
			}
			break;
		default:
            break;
		}
		return *this;
	}

    Variant&Variant::AssignFromByReference(Variant&value)
	{
		Clear();
		switch(value._valueType)
		{
		case NullType:
			break;
		case Variant::CharType:
		case Variant::ByteType:
		case Variant::ShortType:
		case Variant::WordType:
		case Variant::Int32Type:
		case Variant::Uint32Type:
		case Variant::Uint64Type:
		case Variant::Int64Type:
		case Variant::FloatType:
		case Variant::DoubleType:
			_valueType=value._valueType;
			_uint64Value=value._uint64Value;
			break;
		case Variant::Uint128Type:
			_valueType=value._valueType;
			if (value._uint128ValuePtr!=NULL)
			{
				_uint128ValuePtr=new Uint128(*value._uint128ValuePtr);
			}
			break;
		case Variant::StringType:
			_valueType=value._valueType;
			if (value._stringValuePtr!=NULL)
			{
				_stringValuePtr=new std::string(*value._stringValuePtr);
			}
			break;
		case Variant::BytesType:
			_valueType=value._valueType;
			if (value._bytesValuePtr!=NULL)
			{
                _valueType=BytesType;

                Bytes &s = *value._bytesValuePtr;
				_bytesValuePtr=new Bytes();
                if (!_bytesValuePtr)
                {
                    break;
                }
                _bytesValuePtr->AttachBuf(s.GetBufAddr(), s.GetBufLen());
			}
			break;
		default:
			break;
		}
		return *this;
	}

    const Uint128& Variant::GetAsDefaultUint128() const
	{
		static Uint128 defaultValue;
		return defaultValue;
	}

    const std::string& Variant::GetAsDefaultString() const
	{
		static std::string defaultValue;
		return defaultValue;
	}

	const Bytes& Variant::GetAsDefaultBytes() const
	{
		static Bytes defaultValue;
		return defaultValue;
	}

	Bytes& Variant::GetAsDefaultBytes()
	{
		static Bytes defaultValue;
		return defaultValue;
	}
}
