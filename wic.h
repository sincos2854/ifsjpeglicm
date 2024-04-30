// Copyright (c) 2024 sincos2854
// Licensed under the MIT License

#pragma once

#include <memory>
#include "common.h"

class SpiWic {
public:
    SpiWic() : initialized_(false) {}
    ~SpiWic() { if(initialized_) CoUninitialize(); }

    int Decode(
        const LPBYTE data,
        size_t size,
        std::unique_ptr<HANDLE, PictureHandleDeleter>& h_bitmap_info,
        std::unique_ptr<HANDLE, PictureHandleDeleter>& h_bitmap
    );

private:
    bool initialized_;
};
