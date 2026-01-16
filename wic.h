// Copyright (c) 2024 - 2026 sincos2854
// Licensed under the MIT License

#pragma once

#include "common.h"

class SpiWic {
public:
    SpiWic() : initialized_(false) {}
    ~SpiWic() { if (initialized_) CoUninitialize(); }

    int Decode(const BYTE* data, size_t size, PictureHandle& h_bitmap_info, PictureHandle& h_bitmap);

private:
    bool initialized_;
};
