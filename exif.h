// Copyright (c) 2024 - 2026 sincos2854
// Licensed under the MIT License

#pragma once

#include "common.h"
#include <string_view>
#include <bit>

inline constexpr int EXIF_ERROR = -1;
inline constexpr int EXIF_NO_ORIENTATION = 0;

inline constexpr char EXIF_SIGN_BYTES[] = { 'E', 'x', 'i', 'f', '\0', '\0' };
inline constexpr auto EXIF_SIGN_SIZE = std::size(EXIF_SIGN_BYTES);

inline constexpr std::string_view LE_SIGN_BYTES = "II";
inline constexpr std::string_view BE_SIGN_BYTES = "MM";
static_assert(LE_SIGN_BYTES.size() == BE_SIGN_BYTES.size());

inline constexpr UINT TIFF_SIGN_OFFSET = static_cast<UINT>(LE_SIGN_BYTES.size());
inline constexpr WORD TIFF_SIGN = 0x002A;

inline constexpr UINT IFD0_POINTER_OFFSET = TIFF_SIGN_OFFSET + static_cast<UINT>(sizeof(TIFF_SIGN));
inline constexpr DWORD MIN_IFD0_OFFSET = IFD0_POINTER_OFFSET + static_cast<DWORD>(sizeof(DWORD));

inline constexpr UINT TAG_SIZE = 12;

inline constexpr WORD ORIENTATION_TAG = 0x0112;
inline constexpr WORD ORIENTATION_VALUE_TYPE = 3;
inline constexpr DWORD ORIENTATION_VALUE_COUNT = 1;
inline constexpr WORD MIN_ORIENTATION = 1;
inline constexpr WORD MAX_ORIENTATION = 8;


class Exif
{
public:
    Exif() {}
    ~Exif() {}

    static bool CheckExif(LPCBYTE data, UINT data_length)
    {
        return EXIF_SIGN_SIZE <= data_length && std::memcmp(data, EXIF_SIGN_BYTES, EXIF_SIGN_SIZE) == 0;
    }

    int GetOrientation(LPCBYTE data, UINT data_length);

private:
    bool GetEndian(void);
    bool CheckTiff(void) const;
    DWORD GetIFD0Offset(void) const;
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
