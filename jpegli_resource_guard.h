// Copyright (c) 2024 - 2026 sincos2854
// Licensed under the MIT License

#pragma once

#include <windows.h>
#include <lib/jpegli/decode.h>

class JpegliResourceGuard
{
public:
    JpegliResourceGuard(jpeg_decompress_struct* cinfop) : cinfop_(cinfop) {}

    ~JpegliResourceGuard()
    {
        if (cinfop_)
        {
            jpegli_destroy_decompress(cinfop_);
        }
    }

    JpegliResourceGuard(const JpegliResourceGuard&) = delete;
    JpegliResourceGuard& operator=(const JpegliResourceGuard&) = delete;

private:
    jpeg_decompress_struct* cinfop_ = nullptr;
};

struct IccProfileDeleter
{
    void operator()(JOCTET* profile_data)
    {
        free(profile_data);
    }
};

using IccProfile = std::unique_ptr<JOCTET, IccProfileDeleter>;
