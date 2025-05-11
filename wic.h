// Copyright (c) 2024 sincos2854
// Licensed under the MIT License

#pragma once

#include "common.h"

class SpiWic {
public:
    SpiWic() : initialized_(false) {}
    ~SpiWic() { if (initialized_) CoUninitialize(); }

    int Decode(
        const LPBYTE data,
        size_t size,
        PictureHandle& h_bitmap_info,
        PictureHandle& h_bitmap
    );

private:
    bool initialized_;
};
