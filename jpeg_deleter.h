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
