// Copyright (c) 2024 - 2025 sincos2854
// Licensed under the MIT License

#pragma once

#define NOMINMAX
#include <windows.h>
#include <memory>
#include "lib/jpegli/decode.h"

struct FileHandleDeleter
{
    void operator()(HANDLE handle)
    {
        if (handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(handle);
        }
    }
};

using FileHandle = std::unique_ptr<std::remove_pointer<HANDLE>::type, FileHandleDeleter>;

struct PictureHandleDeleter
{
    void operator()(HANDLE handle)
    {
        LocalUnlock(handle);
        LocalFree(handle);
    }
};

using PictureHandle = std::unique_ptr<std::remove_pointer<HANDLE>::type, PictureHandleDeleter>;

struct IccProfileDeleter
{
    void operator()(JOCTET* profile_data)
    {
        free(profile_data);
    }
};
