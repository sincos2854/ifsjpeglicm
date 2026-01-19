// Copyright (c) 2024 - 2026 sincos2854
// Licensed under the MIT License

#pragma once

#include "spi00in.h"
#include <string>
#include <bit>

constexpr int EXIF_ERROR = -1;
constexpr int EXIF_NO_ORIENTATION = 0;

constexpr LPCSTR EXIF_SIGN = "Exif\0\0";
constexpr UINT EXIF_SIGN_SIZE = 6;

constexpr std::string_view BIG_ENDIAN_SIGN = "MM";
constexpr std::string_view LITTLE_ENDIAN_SIGN = "II";

constexpr UINT IFD0_POINTER_STORAGE_ADDRESS = 0x04;

constexpr WORD ORIENTATION_TAG = 0x0112;

constexpr UINT TAG_BYTES = 12;

class Exif
{
public:
    Exif() : data_(nullptr), data_length_(0), big_endian_(false) {}
    ~Exif() {}

    static bool CheckExif(const BYTE* data, UINT data_length)
    {
        return EXIF_SIGN_SIZE <= data_length && std::memcmp(data, EXIF_SIGN, EXIF_SIGN_SIZE) == 0 ? true : false;
    }

    int GetOrientation(const BYTE* data, UINT data_length);

private:
    bool GetEndian(void);
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
    const BYTE* data_;
    UINT data_length_;
    bool big_endian_;
};