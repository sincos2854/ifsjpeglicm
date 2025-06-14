// Copyright (c) 2024 - 2025 sincos2854
// Licensed under the MIT License

#include "common.h"
#include "wic.h"
#include "ifsjpeglicm.h"
#include "lib/jpegli/decode.h"

bool IsSupportedEx(LPCWSTR filename, const BYTE* data)
{
    if (!data)
    {
        return false;
    }

    // Check the file header
    if (memcmp(data, "\xFF\xD8\xFF", 3) == 0)
    {
        return true;
    }

    return false;
}

struct ifsjpegli_error_mgr {
    jpeg_error_mgr pub {};
    jmp_buf setjmp_buffer;
};

METHODDEF(void) error_exit(j_common_ptr cinfo)
{
    (*cinfo->err->output_message) (cinfo);
    longjmp(reinterpret_cast<ifsjpegli_error_mgr*>(cinfo->err)->setjmp_buffer, SPI_OUT_OF_ORDER);
}

int GetPictureInfoEx(LPCWSTR file_name, const BYTE* data, size_t size, PictureInfo* lpInfo)
{
    if (!IsSupportedEx(file_name, data))
    {
        return SPI_NOT_SUPPORT;
    }

    jpeg_decompress_struct cinfo {};
    ifsjpegli_error_mgr jerr {};

    cinfo.err = jpegli_std_error(&jerr.pub);
    jerr.pub.error_exit = error_exit;

    if (int e = setjmp(jerr.setjmp_buffer) != 0)
    {
        jpegli_destroy_decompress(&cinfo);
        return e;
    }

    jpegli_create_decompress(&cinfo);
    jpegli_mem_src(&cinfo, data, static_cast<unsigned long>(size));

    if (jpegli_read_header(&cinfo, FALSE) == JPEG_SUSPENDED)
    {
        longjmp(jerr.setjmp_buffer, SPI_OUT_OF_ORDER);
    }

    *lpInfo = {};
    lpInfo->width = cinfo.image_width;
    lpInfo->height = cinfo.image_height;
    lpInfo->x_density = cinfo.X_density;
    lpInfo->y_density = cinfo.Y_density;
    lpInfo->colorDepth = 32;

    jpegli_destroy_decompress(&cinfo);

    return SPI_ALL_RIGHT;
}

int GetPictureEx(LPCWSTR file_name, const BYTE* data, size_t size, HANDLE* pHBInfo, HANDLE* pHBm, ProgressCallback lpPrgressCallback, LONG_PTR lData)
{
    if (!IsSupportedEx(file_name, data))
    {
        return SPI_NOT_SUPPORT;
    }

    if (lpPrgressCallback)
    {
        if (lpPrgressCallback(0, 100, lData))
        {
            return SPI_ABORT;
        }
    }

    PictureHandle h_bitmap_info;
    PictureHandle h_bitmap;
    LPBITMAPINFOHEADER bitmap_header = nullptr;
    BYTE* bitmap = nullptr;

    jpeg_decompress_struct cinfo{};
    ifsjpegli_error_mgr jerr{};

    cinfo.err = jpegli_std_error(&jerr.pub);
    jerr.pub.error_exit = error_exit;

    if (int e = setjmp(jerr.setjmp_buffer) != 0)
    {
        jpegli_destroy_decompress(&cinfo);
        return e;
    }

    jpegli_create_decompress(&cinfo);
    jpegli_mem_src(&cinfo, data, static_cast<unsigned long>(size));

    // Required to get the ICC Profile
    jpegli_save_markers(&cinfo, JPEG_APP0 + 2, 0xFFFF);

    if (jpegli_read_header(&cinfo, TRUE) == JPEG_SUSPENDED)
    {
        longjmp(jerr.setjmp_buffer, SPI_OUT_OF_ORDER);
    }

    // CMYK/YCCK
    if (cinfo.out_color_space == JCS_CMYK)
    {
        jpegli_destroy_decompress(&cinfo);

        auto decoder = std::make_unique<SpiWic>();

        if (int e = decoder->Decode(data, size, h_bitmap_info, h_bitmap) != SPI_ALL_RIGHT)
        {
            return e;
        }

        if (!h_bitmap_info || !h_bitmap)
        {
            return SPI_OTHER_ERROR;
        }
    }
    // Not CMYK/YCCK
    else
    {
        cinfo.out_color_space = JCS_EXT_BGRA;

        if (!jpegli_start_decompress(&cinfo))
        {
            longjmp(jerr.setjmp_buffer, SPI_OUT_OF_ORDER);
        }

        LONG width = cinfo.output_width;
        LONG height = cinfo.output_height;
        DWORD stride = width * 4;
        size_t bitmap_size = static_cast<size_t>(height) * stride;

        // Get the ICC Profile
        std::unique_ptr<JOCTET, IccProfileDeleter> profile_data;
        UINT profile_size = 0;

        if (jpegli_read_icc_profile(&cinfo, std::out_ptr(profile_data), &profile_size))
        {
            if (0 < profile_size)
            {
                h_bitmap_info = PictureHandle(
                    LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof(BITMAPV5HEADER) + profile_size)
                );
                if (!h_bitmap_info)
                {
                    longjmp(jerr.setjmp_buffer, SPI_NO_MEMORY);
                }

                LPBITMAPV5HEADER v5 = reinterpret_cast<LPBITMAPV5HEADER>(LocalLock(h_bitmap_info.get()));
                if (!v5)
                {
                    longjmp(jerr.setjmp_buffer, SPI_MEMORY_ERROR);
                }

                v5->bV5Size = sizeof(BITMAPV5HEADER);
                v5->bV5CSType = PROFILE_EMBEDDED;
                v5->bV5ProfileData = sizeof(BITMAPV5HEADER);
                v5->bV5ProfileSize = profile_size;
                memcpy(reinterpret_cast<BYTE*>(v5) + v5->bV5ProfileData, profile_data.get(), v5->bV5ProfileSize);

                LocalUnlock(h_bitmap_info.get());
            }
        }

        if (profile_size == 0)
        {
            h_bitmap_info = PictureHandle(
                LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof(BITMAPINFO))
            );
            if (!h_bitmap_info)
            {
                longjmp(jerr.setjmp_buffer, SPI_NO_MEMORY);
            }
        }

        bitmap_header = reinterpret_cast<LPBITMAPINFOHEADER>(LocalLock(h_bitmap_info.get()));
        if (!bitmap_header)
        {
            longjmp(jerr.setjmp_buffer, SPI_MEMORY_ERROR);
        }

        if (profile_size == 0)
        {
            bitmap_header->biSize = sizeof(BITMAPINFOHEADER);
        }
        bitmap_header->biWidth = width;
        bitmap_header->biHeight = height;
        bitmap_header->biPlanes = 1;
        bitmap_header->biBitCount = 32;
        bitmap_header->biSizeImage = static_cast<DWORD>(bitmap_size);
        bitmap_header->biXPelsPerMeter = static_cast<LONG>(cinfo.X_density * 39.37);
        bitmap_header->biYPelsPerMeter = static_cast<LONG>(cinfo.Y_density * 39.37);

        // Decode the image
        h_bitmap = PictureHandle(
            LocalAlloc(LMEM_MOVEABLE, bitmap_size)
        );
        if (!h_bitmap)
        {
            longjmp(jerr.setjmp_buffer, SPI_NO_MEMORY);
        }
        bitmap = reinterpret_cast<BYTE*>(LocalLock(h_bitmap.get()));
        if (!bitmap)
        {
            longjmp(jerr.setjmp_buffer, SPI_MEMORY_ERROR);
        }

        auto ptr = std::make_unique<JSAMPROW[]>(height);

        for (LONG j = 0; j < height; j++)
        {
            ptr[j] = bitmap + (height - j - 1) * stride;
        }

        while (cinfo.output_scanline < cinfo.output_height)
        {
            jpegli_read_scanlines(&cinfo, &ptr[cinfo.output_scanline], cinfo.output_height - cinfo.output_scanline);
        }

        jpegli_finish_decompress(&cinfo);
        jpegli_destroy_decompress(&cinfo);
    }

    if (lpPrgressCallback)
    {
        if (lpPrgressCallback(100, 100, lData))
        {
            return SPI_ABORT;
        }
    }

    LocalUnlock(h_bitmap.get());
    LocalUnlock(h_bitmap_info.get());

    *pHBInfo = h_bitmap_info.get();
    *pHBm = h_bitmap.get();

    h_bitmap_info.release();
    h_bitmap.release();

    return SPI_ALL_RIGHT;
}
