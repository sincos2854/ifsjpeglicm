// Copyright (c) 2024 - 2026 sincos2854
// Licensed under the MIT License

#pragma once

#include "bitmap_handle.h"

class Wic {
public:
    Wic() {}
    ~Wic() { if (initialized_) CoUninitialize(); }

    int Decode(LPCBYTE file_data, size_t file_size, LocalMemHandle& h_bitmap_info, LocalMemHandle& h_bitmap);

private:
    bool initialized_ = false;
};
