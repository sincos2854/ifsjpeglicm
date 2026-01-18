// Copyright (c) 2024 - 2026 sincos2854
// Licensed under the MIT License

#pragma once

#include <string>
#include "spi00in.h"

constexpr int EXIF_ERROR = -1;
constexpr int EXIF_NO_ORIENTATION = 0;

constexpr LPCSTR EXIF_SIGN = "Exif\0\0";
constexpr UINT EXIF_SIGN_SIZE = 6;

constexpr std::string_view BIG_ENDIAN_SIGN = "MM";
constexpr std::string_view LITTLE_ENDIAN_SIGN = "II";

constexpr UINT IFD0_POINTER_STORAGE_ADDRESS = 0x04;

constexpr WORD ORIENTATION_TAG = 0x0112;

constexpr UINT SHORT_BYTES = 2;
constexpr UINT LONG_BYTES = 4;
constexpr UINT TAG_BYTES = 12;

class Exif
{
public:
    Exif() : data_(nullptr), data_length_(0), big_endian_(false) {}
    ~Exif() {}

    inline static bool CheckExif(BYTE* data, UINT data_length)
    {
        return EXIF_SIGN_SIZE <= data_length && std::memcmp(data, EXIF_SIGN, EXIF_SIGN_SIZE) == 0 ? true : false;
    }

    int GetOrientation(BYTE* data, UINT data_length);

private:
    bool GetEndian(void);
    DWORD GetIFD0Address(void) const;
    bool MovePointer(UINT length);
    bool GetSHORTAndMovePointer(WORD& value);
    bool GetLONGAndMovePointer(DWORD& value);

private:
    BYTE* data_;
    UINT data_length_;
    bool big_endian_;
};