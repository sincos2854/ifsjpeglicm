// Copyright (c) 2024 sincos2854
// Licensed under the MIT License

#pragma once

#include <windows.h>
#include "lib/jpegli/decode.h"

struct FileHandleDeleter
{
    using pointer = HANDLE;
    void operator()(HANDLE handle)
    {
        if (handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(handle);
        }
    }
};

struct PictureHandleDeleter
{
    using pointer = HANDLE;
    void operator()(HANDLE handle)
    {
        LocalUnlock(handle);
        LocalFree(handle);
    }
};

struct IccProfileDeleter
{
    void operator()(JOCTET* profile_data)
    {
        free(profile_data);
    }
};
