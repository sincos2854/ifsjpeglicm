// Copyright (c) 2024 - 2025 sincos2854
// Licensed under the MIT License

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
    if (std::memcmp(data, "\xFF\xD8\xFF", 3) == 0)
    {
        return true;
    }

    return false;
}

static void error_exit(j_common_ptr cinfo)
{
    (*cinfo->err->output_message)(cinfo);
    throw SPI_OUT_OF_ORDER;
}

int GetPictureInfoEx(LPCWSTR file_name, const BYTE* data, size_t size, PictureInfo* lpInfo)
{
    if (!IsSupportedEx(file_name, data))
    {
        return SPI_NOT_SUPPORT;
    }

    jpeg_decompress_struct cinfo{};
    jpeg_error_mgr jerr{};

    cinfo.err = jpegli_std_error(&jerr);
    jerr.error_exit = error_exit;

    try
    {
        jpegli_create_decompress(&cinfo);
        jpegli_mem_src(&cinfo, data, static_cast<unsigned long>(size));

        if (jpegli_read_header(&cinfo, FALSE) == JPEG_SUSPENDED)
        {
            throw SPI_OUT_OF_ORDER;
        }
    }
    catch (int e)
    {
        jpegli_destroy_decompress(&cinfo);
        return e;
    }
    catch (...)
    {
        jpegli_destroy_decompress(&cinfo);
        return SPI_OTHER_ERROR;
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

    jpeg_decompress_struct cinfo{};
    jpeg_error_mgr jerr{};
    bool cmyk = false;

    cinfo.err = jpegli_std_error(&jerr);
    jerr.error_exit = error_exit;

    try
    {
        jpegli_create_decompress(&cinfo);
        jpegli_mem_src(&cinfo, data, static_cast<unsigned long>(size));

        // Required to get the ICC Profile
        jpegli_save_markers(&cinfo, JPEG_APP0 + 2, 0xFFFF);

        if (jpegli_read_header(&cinfo, TRUE) == JPEG_SUSPENDED)
        {
            throw SPI_OUT_OF_ORDER;
        }

        // CMYK/YCCK
        if (cinfo.out_color_space == JCS_CMYK || cinfo.out_color_space == JCS_YCCK)
        {
            cmyk = true;
            jpegli_destroy_decompress(&cinfo);
        }
        // Not CMYK/YCCK
        else
        {
            cinfo.out_color_space = JCS_EXT_BGRA;

            if (!jpegli_start_decompress(&cinfo))
            {
                throw SPI_OUT_OF_ORDER;
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
                    h_bitmap_info = PictureHandle(LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof(BITMAPV5HEADER) + profile_size));

                    if (!h_bitmap_info)
                    {
                        throw SPI_NO_MEMORY;
                    }

                    auto auto_unlock_header = std::make_unique<AutoUnlockBitmapHeader>(h_bitmap_info.get());

                    if (!auto_unlock_header->MakeV5Header())
                    {
                        throw SPI_MEMORY_ERROR;
                    }

                    auto v5 = auto_unlock_header->GetV5Header();

                    v5->bV5Size = sizeof(BITMAPV5HEADER);
                    v5->bV5CSType = PROFILE_EMBEDDED;
                    v5->bV5ProfileData = sizeof(BITMAPV5HEADER);
                    v5->bV5ProfileSize = profile_size;

                    std::memcpy(reinterpret_cast<BYTE*>(v5) + v5->bV5ProfileData, profile_data.get(), v5->bV5ProfileSize);
                }
            }

            if (!h_bitmap_info)
            {
                h_bitmap_info = PictureHandle(LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof(BITMAPINFO)));

                if (!h_bitmap_info)
                {
                    throw SPI_NO_MEMORY;
                }
            }

            auto auto_unlock_header = std::make_unique<AutoUnlockBitmapHeader>(h_bitmap_info.get());
            auto bitmap_header = auto_unlock_header->GetBitmapHeader();

            if (!bitmap_header)
            {
                throw SPI_MEMORY_ERROR;
            }

            if (!auto_unlock_header->GetV5Header())
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
            h_bitmap = PictureHandle(LocalAlloc(LMEM_MOVEABLE, bitmap_size));

            if (!h_bitmap)
            {
                throw SPI_NO_MEMORY;
            }

            auto auto_unlock_bitmap = std::make_unique<AutoUnlockBitmap>(h_bitmap.get());
            auto bitmap = auto_unlock_bitmap->GetBitmap();

            if (!bitmap)
            {
                throw SPI_MEMORY_ERROR;
            }

            auto ptr = std::make_unique_for_overwrite<JSAMPROW[]>(height);

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
    }
    catch (int e)
    {
        jpegli_destroy_decompress(&cinfo);
        return e;
    }
    catch (...)
    {
        jpegli_destroy_decompress(&cinfo);
        return SPI_OTHER_ERROR;
    }

    if (cmyk)
    {
        auto decoder = std::make_unique<SpiWic>();

        if (int e = decoder->Decode(data, size, h_bitmap_info, h_bitmap); e != SPI_ALL_RIGHT)
        {
            return e;
        }
    }

    if (lpPrgressCallback)
    {
        if (lpPrgressCallback(100, 100, lData))
        {
            return SPI_ABORT;
        }
    }

    *pHBInfo = h_bitmap_info.get();
    *pHBm = h_bitmap.get();

    h_bitmap_info.release();
    h_bitmap.release();

    return SPI_ALL_RIGHT;
}
