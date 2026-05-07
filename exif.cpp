// Copyright (c) 2024 - 2026 sincos2854
// Licensed under the MIT License

#include "exif.h"

int Exif::GetOrientation(LPCBYTE data, UINT data_length)
{
    data_ = data;
    data_length_ = data_length;
    big_endian_ = false;

    if (!CheckExif(data_, data_length_))
    {
        return EXIF_ERROR;
    }

    if (!MovePointer(EXIF_SIGN_BYTES))
    {
        return EXIF_ERROR;
    }

    if (!GetEndian())
    {
        return EXIF_ERROR;
    }

    if (!CheckTiff())
    {
        return EXIF_ERROR;
    }

    auto ifd0_address = GetIFD0Address();

    if (ifd0_address < MIN_IFD0_ADDRESS)
    {
        return EXIF_ERROR;
    }

    if (!MovePointer(ifd0_address))
    {
        return EXIF_ERROR;
    }

    WORD tag_count = 0;

    if (!GetDataAndMovePointer(tag_count))
    {
        return EXIF_ERROR;
    }

    while (0 < tag_count)
    {
        WORD tag = 0;

        if (!GetDataAndMovePointer(tag))
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

        if (!GetDataAndMovePointer(value_type))
        {
            return EXIF_ERROR;
        }

        if (value_type != ORIENTATION_VALUE_TYPE)
        {
            return EXIF_ERROR;
        }

        DWORD value_count = 0;

        if (!GetDataAndMovePointer(value_count))
        {
            return EXIF_ERROR;
        }

        if (value_count != ORIENTATION_VALUE_COUNT)
        {
            return EXIF_ERROR;
        }

        WORD orientation = 0;

        if (!GetDataAndMovePointer(orientation))
        {
            return EXIF_ERROR;
        }

        if (orientation < MIN_ORIENTATION || MAX_ORIENTATION < orientation)
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

bool Exif::CheckTiff(void) const
{
    WORD sign = 0;

    if (data_length_ < TIFF_SIGN_OFFSET + sizeof(sign))
    {
        return false;
    }

    std::memcpy(&sign, data_ + TIFF_SIGN_OFFSET, sizeof(sign));

    if (big_endian_)
    {
        sign = std::byteswap(sign);
    }

    return sign == TIFF_SIGN;
}

DWORD Exif::GetIFD0Address(void) const
{
    DWORD ifd0_address = 0;

    if (data_length_ < IFD0_POINTER_STORAGE_ADDRESS + sizeof(ifd0_address))
    {
        return 0;
    }

    std::memcpy(&ifd0_address, data_ + IFD0_POINTER_STORAGE_ADDRESS, sizeof(ifd0_address));

    if (big_endian_)
    {
        ifd0_address = std::byteswap(ifd0_address);
    }

    return ifd0_address;
}

bool Exif::MovePointer(UINT length)
{
    if (length <= data_length_)
    {
        data_ += length;
        data_length_ -= length;

        return true;
    }

    data_length_ = 0;

    return false;
}
