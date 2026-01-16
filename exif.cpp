// Copyright (c) 2024 - 2026 sincos2854
// Licensed under the MIT License

#include <bit>
#include "exif.h"

int Exif::GetOrientation(BYTE* data, UINT data_length)
{
    data_ = data;
    data_length_ = data_length;

    if (!CheckExif(data_, data_length_))
    {
        return EXIF_ERROR;
    }

    if (!MovePointer(EXIF_SIGN_SIZE))
    {
        return EXIF_ERROR;
    }

    if (!GetEndian())
    {
        return EXIF_ERROR;
    }

    auto ifd0_address = GetIFD0Address();

    if (ifd0_address == 0)
    {
        return EXIF_ERROR;
    }

    if (!MovePointer(ifd0_address))
    {
        return EXIF_ERROR;
    }

    WORD tag_count = 0;

    if (!GetSHORTAndMovePointer(tag_count))
    {
        return EXIF_ERROR;
    }

    while (0 < tag_count)
    {
        WORD tag = 0;

        if (!GetSHORTAndMovePointer(tag))
        {
            return EXIF_ERROR;
        }

        if (tag < ORIENTATION_TAG)
        {
            if (!MovePointer(TAG_BYTES - sizeof(tag)))
            {
                return EXIF_ERROR;
            }

            tag_count--;

            continue;
        }
        else if (ORIENTATION_TAG < tag)
        {
            break;
        }

        WORD value_type = 0;

        if (!GetSHORTAndMovePointer(value_type))
        {
            return EXIF_ERROR;
        }

        if (value_type != 3)
        {
            return EXIF_ERROR;
        }

        DWORD value_count = 0;

        if(!GetLONGAndMovePointer(value_count))
        {
            return EXIF_ERROR;
        }

        WORD orientation = 0;

        if(!GetSHORTAndMovePointer(orientation))
        {
            return EXIF_ERROR;
        }

        return static_cast<int>(orientation);
    }

    return EXIF_NO_ORIENTATION;
}

bool Exif::GetEndian(void)
{
    if (data_length_ < BIG_ENDIAN_SIGN.size())
    {
        return false;
    }

    if (std::memcmp(data_, BIG_ENDIAN_SIGN.data(), BIG_ENDIAN_SIGN.size()) == 0)
    {
        big_endian_ = true;
    }
    else if (std::memcmp(data_, LITTLE_ENDIAN_SIGN.data(), LITTLE_ENDIAN_SIGN.size()) == 0)
    {
        big_endian_ = false;
    }
    else
    {
        return false;
    }

    return true;
}

DWORD Exif::GetIFD0Address(void)
{
    DWORD ifd0_address = 0;

    if (data_length_ < IFD0_POINTER_STORAGE_ADDRESS + sizeof(ifd0_address))
    {
        return 0;
    }

    if (memcpy_s(&ifd0_address, sizeof(ifd0_address), data_ + IFD0_POINTER_STORAGE_ADDRESS, sizeof(ifd0_address)))
    {
        return 0;
    }

    if (big_endian_)
    {
        ifd0_address = std::byteswap(ifd0_address);
    }

    return ifd0_address;
}

bool Exif::MovePointer(UINT length)
{
    if(length < data_length_ )
    {
        data_ += length;
        data_length_ -= length;

        return true;
    }

    data_ += data_length_ - 1;
    data_length_ = 0;

    return false;
}

bool Exif::GetSHORTAndMovePointer(WORD& value)
{
    if (data_length_ < sizeof(value))
    {
        return false;
    }

    if(memcpy_s(&value, sizeof(value), data_, sizeof(value)))
    {
        return false;
    }

    if (big_endian_)
    {
        value = std::byteswap(value);
    }

    if (!MovePointer(sizeof(value)))
    {
        return false;
    }

    return true;
}

bool Exif::GetLONGAndMovePointer(DWORD& value)
{
    if (data_length_ < sizeof(value))
    {
        return false;
    }

    if(memcpy_s(&value, sizeof(value), data_, sizeof(value)))
    {
        return false;
    }

    if (big_endian_)
    {
        value = std::byteswap(value);
    }

    if (!MovePointer(sizeof(value)))
    {
        return false;
    }

    return true;
}
