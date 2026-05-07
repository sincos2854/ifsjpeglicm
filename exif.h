// Copyright (c) 2024 - 2026 sincos2854
// Licensed under the MIT License

#pragma once

#include "spi00in.h"
#include <string>
#include <bit>

constexpr int EXIF_ERROR = -1;
constexpr int EXIF_NO_ORIENTATION = 0;

constexpr char EXIF_SIGN[] = "Exif\0\0";
constexpr auto EXIF_SIGN_BYTES = std::size(EXIF_SIGN) - 1;

constexpr std::string_view BIG_ENDIAN_SIGN = "MM";
constexpr std::string_view LITTLE_ENDIAN_SIGN = "II";

constexpr UINT TIFF_SIGN_OFFSET = 0x02;
constexpr WORD TIFF_SIGN = 0x002A;

constexpr UINT IFD0_POINTER_STORAGE_ADDRESS = 0x04;
constexpr DWORD MIN_IFD0_ADDRESS = 0x08;

constexpr UINT TAG_BYTES = 12;

constexpr WORD ORIENTATION_TAG = 0x0112;
constexpr WORD ORIENTATION_VALUE_TYPE = 3;
constexpr DWORD ORIENTATION_VALUE_COUNT = 1;
constexpr WORD MIN_ORIENTATION = 1;
constexpr WORD MAX_ORIENTATION = 8;


class Exif
{
public:
    Exif() {}
    ~Exif() {}

    static bool CheckExif(LPCBYTE data, UINT data_length)
    {
        return EXIF_SIGN_BYTES <= data_length && std::memcmp(data, EXIF_SIGN, EXIF_SIGN_BYTES) == 0 ? true : false;
    }

    int GetOrientation(LPCBYTE data, UINT data_length);

private:
    bool GetEndian(void);
    bool CheckTiff(void) const;
    DWORD GetIFD0Address(void) const;
    bool MovePointer(UINT length);

    template <typename T>
    bool GetDataAndMovePointer(T& value)
    {
        if (data_length_ < sizeof(value))
        {
            return false;
        }

        std::memcpy(&value, data_, sizeof(value));

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

private:
    LPCBYTE data_ = nullptr;
    UINT data_length_ = 0;
    bool big_endian_ = false;
};
