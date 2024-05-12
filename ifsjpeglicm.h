// Copyright (c) 2024 sincos2854
// Licensed under the MIT License

#pragma once

#include <string>
#include "spi00in.h"
#include "version.h"

#define COPYRIGHT L"Jpegli Susie Plug-in Ver." PROJECT_VERSION L" (c) 2024 sincos2854"

#define EXTENSION1 L".jpeg"
#define EXTENSION2 L".jpg"

#define PLUGIN_INFO3 L"*" EXTENSION1 L";*" EXTENSION2
#define PLOGIN_INFO4 L"JPEG file(*" EXTENSION1 L";*" EXTENSION2 L")"

static const wchar_t* plugin_info[]{
    L"00IN",
    COPYRIGHT,
    PLUGIN_INFO3,
    PLOGIN_INFO4,
};

#define HEADER_MIN_SIZE 3

constexpr static std::wstring_view extensions[]{
    EXTENSION1,
    EXTENSION2
};

bool IsSupportedEx(LPCWSTR filename, const LPBYTE data);
int GetPictureInfoEx(LPCWSTR file_name, const LPBYTE data, size_t size, PictureInfo* lpInfo);
int GetPictureEx(LPCWSTR file_name, const LPBYTE data, size_t size, HANDLE* pHBInfo, HANDLE* pHBm, ProgressCallback lpPrgressCallback, LONG_PTR lData);
