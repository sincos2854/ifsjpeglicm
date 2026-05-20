// Copyright (c) 2024 - 2026 sincos2854
// Licensed under the MIT License

#include "ifsjpeglicm.h"
#include "bitmap_handle.h"
#include "exif.h"
#include "wic.h"
#include "jpegli_resource_guard.h"
#include <opencv2/opencv.hpp>

bool IsSupportedEx(LPCWSTR file_name, LPCBYTE file_data)
{
    if (!file_data)
    {
        return false;
    }

    // Check the file header
    if (std::memcmp(file_data, "\xFF\xD8\xFF", 3) == 0)
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

int GetPictureInfoEx(LPCWSTR file_name, LPCBYTE file_data, size_t file_size, PictureInfo* lp_info)
{
    if (!IsSupportedEx(file_name, file_data))
    {
        return SPI_NOT_SUPPORT;
    }

    try
    {
        jpeg_decompress_struct cinfo{};
        jpeg_error_mgr jerr{};
        int orientation = 0;

        cinfo.err = jpegli_std_error(&jerr);
        jerr.error_exit = error_exit;

        JpegliResourceGuard jpegli_guard(&cinfo);

        jpegli_create_decompress(&cinfo);
        jpegli_mem_src(&cinfo, file_data, static_cast<unsigned long>(file_size));

        // Required to get the EXIF data
        jpegli_save_markers(&cinfo, JPEG_APP0 + 1, 0xFFFF);

        if (jpegli_read_header(&cinfo, TRUE) != JPEG_HEADER_OK)
        {
            return SPI_OUT_OF_ORDER;
        }

        for (auto marker = cinfo.marker_list; marker; marker = marker->next)
        {
            if (marker->marker == JPEG_APP0 + 1 && Exif::CheckExif(marker->data, marker->data_length))
            {
                Exif exif;

                orientation = exif.GetOrientation(marker->data, marker->data_length);

                break;
            }
        }

        *lp_info = {};
        lp_info->width = cinfo.image_width;
        lp_info->height = cinfo.image_height;
        lp_info->x_density = cinfo.X_density;
        lp_info->y_density = cinfo.Y_density;
        lp_info->colorDepth = 32;

        if (5 <= orientation)
        {
            lp_info->width = cinfo.image_height;
            lp_info->height = cinfo.image_width;
            lp_info->x_density = cinfo.Y_density;
            lp_info->y_density = cinfo.X_density;
        }
    }
    catch (int e)
    {
        return e;
    }

    return SPI_ALL_RIGHT;
}

int GetPictureEx(LPCWSTR file_name, LPCBYTE file_data, size_t file_size, HLOCAL* out_bitmap_info, HLOCAL* out_bitmap, SUSIE_PROGRESS lp_callback, LONG_PTR lp_data)
{
    if (!IsSupportedEx(file_name, file_data))
    {
        return SPI_NOT_SUPPORT;
    }

    if (lp_callback)
    {
        if (lp_callback(0, 100, lp_data))
        {
            return SPI_ABORT;
        }
    }

    LocalMemHandle h_bitmap_info;
    LocalMemHandle h_bitmap;

    bool cmyk = false;

    try
    {
        jpeg_decompress_struct cinfo{};
        jpeg_error_mgr jerr{};

        cinfo.err = jpegli_std_error(&jerr);
        jerr.error_exit = error_exit;

        JpegliResourceGuard jpegli_guard(&cinfo);

        jpegli_create_decompress(&cinfo);
        jpegli_mem_src(&cinfo, file_data, static_cast<unsigned long>(file_size));

        // Required to get the EXIF data
        jpegli_save_markers(&cinfo, JPEG_APP0 + 1, 0xFFFF);

        // Required to get the ICC Profile
        jpegli_save_markers(&cinfo, JPEG_APP0 + 2, 0xFFFF);

        if (jpegli_read_header(&cinfo, TRUE) != JPEG_HEADER_OK)
        {
            return SPI_OUT_OF_ORDER;
        }

        // CMYK/YCCK
        if (cinfo.out_color_space == JCS_CMYK || cinfo.out_color_space == JCS_YCCK)
        {
            cmyk = true;
        }
        // Not CMYK/YCCK
        else
        {
            LONG width = cinfo.image_width;
            LONG height = cinfo.image_height;
            DWORD stride = width * 4;
            size_t bitmap_size = static_cast<size_t>(height) * stride;

            // Get the ICC Profile
            std::unique_ptr<JOCTET, IccProfileDeleter> profile_data;
            UINT profile_size = 0;

            if (jpegli_read_icc_profile(&cinfo, std::out_ptr(profile_data), &profile_size))
            {
                if (0 < profile_size)
                {
                    h_bitmap_info = LocalMemHandle(LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof(BITMAPV5HEADER) + profile_size));

                    if (!h_bitmap_info)
                    {
                        return SPI_NO_MEMORY;
                    }

                    LockedBitmapHeader locked_header(h_bitmap_info.get());

                    if (!locked_header.InitializeAsV5())
                    {
                        return SPI_MEMORY_ERROR;
                    }

                    auto v5 = locked_header.GetV5Header();

                    v5->bV5Size = sizeof(BITMAPV5HEADER);
                    v5->bV5CSType = PROFILE_EMBEDDED;
                    v5->bV5ProfileData = sizeof(BITMAPV5HEADER);
                    v5->bV5ProfileSize = profile_size;

                    std::memcpy(reinterpret_cast<LPBYTE>(v5) + v5->bV5ProfileData, profile_data.get(), v5->bV5ProfileSize);
                }
            }

            if (!h_bitmap_info)
            {
                h_bitmap_info = LocalMemHandle(LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof(BITMAPINFO)));

                if (!h_bitmap_info)
                {
                    return SPI_NO_MEMORY;
                }
            }

            LockedBitmapHeader locked_header(h_bitmap_info.get());
            auto bitmap_header = locked_header.GetBitmapHeader();

            if (!bitmap_header)
            {
                return SPI_MEMORY_ERROR;
            }

            if (!locked_header.GetV5Header())
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

            // Get Orientation form the EXIF
            int orientation = 0;

            for (auto marker = cinfo.marker_list; marker; marker = marker->next)
            {
                if (marker->marker == JPEG_APP0 + 1 && Exif::CheckExif(marker->data, marker->data_length))
                {
                    Exif exif;

                    orientation = exif.GetOrientation(marker->data , marker->data_length);

                    break;
                }
            }

            // Decode the image
            h_bitmap = LocalMemHandle(LocalAlloc(LMEM_MOVEABLE, bitmap_size));

            if (!h_bitmap)
            {
                return SPI_NO_MEMORY;
            }

            LockedBitmap locked_bitmap(h_bitmap.get());
            auto bitmap = locked_bitmap.GetBitmap();

            if (!bitmap)
            {
                return SPI_MEMORY_ERROR;
            }

            std::unique_ptr<BYTE[]> temp_bitmap;
            LPBYTE ptr_to_bitmap = bitmap;

            if (MIN_ORIENTATION < orientation)
            {
                temp_bitmap = std::make_unique_for_overwrite<BYTE[]>(bitmap_size);
                ptr_to_bitmap = temp_bitmap.get();
            }
            
            auto ptr_array = std::make_unique_for_overwrite<JSAMPROW[]>(height);

            for (LONG j = 0; j < height; j++)
            {
                //ptr_array[j] = ptr_to_bitmap + j * stride;
                ptr_array[j] = ptr_to_bitmap + (height - j - 1) * stride;
            }

            cinfo.out_color_space = JCS_EXT_BGRA;

            if (!jpegli_start_decompress(&cinfo))
            {
                return SPI_OUT_OF_ORDER;
            }

            while (cinfo.output_scanline < cinfo.output_height)
            {
                jpegli_read_scanlines(&cinfo, &ptr_array[cinfo.output_scanline], cinfo.output_height - cinfo.output_scanline);
            }

            if (MIN_ORIENTATION < orientation)
            {
                if (!temp_bitmap)
                {
                    return SPI_OTHER_ERROR;
                }

                // 1: Do nothing
                // 2: Flip horizontally
                // 3: Rotate 180 degrees clockwise
                // 4: Flip vertically
                // 5: Flip horizontally + rotate 270 degrees clockwise
                // 6: Rotate 90 degrees clockwise
                // 7: Flip horizontally + rotate 90 degrees clockwise
                // 8: Rotate 270 degrees clockwise

                int flip_code = -1;
                int rotate_code = -1;

                // Set Flip code
                if (orientation == 2 || orientation == 5 || orientation == 7)
                {
                    flip_code = 1;
                }
                else if (orientation == 4)
                {
                    flip_code = 0;
                }

                // Set Rotate code
                if (orientation == 3)
                {
                    rotate_code = cv::RotateFlags::ROTATE_180;
                }
                else if (orientation == 5 || orientation == 8)
                {
                    //rotate_code = cv::RotateFlags::ROTATE_90_COUNTERCLOCKWISE;
                    rotate_code = cv::RotateFlags::ROTATE_90_CLOCKWISE;
                }
                else if (orientation == 6 || orientation == 7)
                {
                    //rotate_code = cv::RotateFlags::ROTATE_90_CLOCKWISE;
                    rotate_code = cv::RotateFlags::ROTATE_90_COUNTERCLOCKWISE;
                }

                // Swap width and height
                if (5 <= orientation)
                {
                    bitmap_header->biWidth = height;
                    bitmap_header->biHeight = width;
                }

                cv::Mat src_mat(height, width, CV_8UC(4), temp_bitmap.get(), stride);
                cv::Mat dst_mat(bitmap_header->biHeight, bitmap_header->biWidth, src_mat.type(), bitmap, bitmap_header->biWidth * 4);

                if (0 <= flip_code && rotate_code < 0)
                {
                    cv::flip(src_mat, dst_mat, flip_code);
                }
                else if (flip_code < 0 && 0 <= rotate_code)
                {
                    cv::rotate(src_mat, dst_mat, rotate_code);
                }
                else
                {
                    cv::flip(src_mat, src_mat, flip_code);
                    cv::rotate(src_mat, dst_mat, rotate_code);
                }
            }

            jpegli_finish_decompress(&cinfo);
        }
    }
    catch (int e)
    {
        return e;
    }

    if (cmyk)
    {
        Wic wic;

        auto e = wic.Decode(file_data, file_size, h_bitmap_info, h_bitmap);

        if (e != SPI_ALL_RIGHT)
        {
            return e;
        }
    }

    if (lp_callback)
    {
        if (lp_callback(100, 100, lp_data))
        {
            return SPI_ABORT;
        }
    }

    *out_bitmap_info = h_bitmap_info.release();
    *out_bitmap = h_bitmap.release();

    return SPI_ALL_RIGHT;
}
