// Copyright (c) 2024 - 2026 sincos2854
// Licensed under the MIT License

#pragma once

#define NOMINMAX
#include <windows.h>
#include <lib/jpegli/decode.h>

struct IccProfileDeleter
{
    void operator()(JOCTET* profile_data)
    {
        free(profile_data);
    }
};

class JepegResourceGuard
{
public:
    JepegResourceGuard(jpeg_decompress_struct* cinfop) : cinfop_(cinfop) {}

    ~JepegResourceGuard()
    {
        if (cinfop_)
        {
            jpegli_destroy_decompress(cinfop_);
        }
    }

    JepegResourceGuard(const JepegResourceGuard&) = delete;
    JepegResourceGuard& operator=(const JepegResourceGuard&) = delete;

private:
    jpeg_decompress_struct* cinfop_ = nullptr;
};
