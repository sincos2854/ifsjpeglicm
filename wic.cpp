// Copyright (c) 2024 - 2026 sincos2854
// Licensed under the MIT License

#include <wrl\client.h>
#include <wincodec.h>
#include "spi00in.h"
#include "wic.h"

using namespace Microsoft::WRL;

int SpiWic::Decode(const BYTE* data, size_t size, PictureHandle& h_bitmap_info, PictureHandle& h_bitmap)
{
    ComPtr<IWICImagingFactory> pFactory;

    auto hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IWICImagingFactory,
        reinterpret_cast<void**>(pFactory.GetAddressOf())
    );

    if (hr == CO_E_NOTINITIALIZED)
    {
        auto error = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
        if (SUCCEEDED(error))
        {
            initialized_ = true;
        }

        error = CoCreateInstance(
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IWICImagingFactory,
            reinterpret_cast<void**>(pFactory.GetAddressOf())
        );
        if (FAILED(error)) return SPI_OTHER_ERROR;
    }
    else if (FAILED(hr))
    {
        return SPI_OTHER_ERROR;
    }

    ComPtr<IWICStream> pStream;

    hr = pFactory->CreateStream(pStream.GetAddressOf());
    if (FAILED(hr)) return SPI_OTHER_ERROR;

    hr = pStream->InitializeFromMemory(const_cast<BYTE*>(data), static_cast<DWORD>(size));
    if (FAILED(hr)) return SPI_OTHER_ERROR;

    ComPtr<IWICBitmapDecoder> pDecoder;

    hr = pFactory->CreateDecoderFromStream(
        pStream.Get(),
        NULL,
        WICDecodeMetadataCacheOnLoad,
        pDecoder.GetAddressOf()
    );
    if (FAILED(hr)) return SPI_OTHER_ERROR;

    ComPtr<IWICBitmapFrameDecode> pFrameDecode;

    hr = pDecoder->GetFrame(0, pFrameDecode.GetAddressOf());
    if (FAILED(hr)) return SPI_OTHER_ERROR;

    UINT width = 0, height = 0;

    hr = pFrameDecode->GetSize(&width, &height);
    if (FAILED(hr)) return SPI_OTHER_ERROR;

    DWORD stride = width * 4;
    size_t bitmap_size = static_cast<size_t>(height) * stride;

    double dpi_x = 0., dpi_y = 0.;

    hr = pFrameDecode->GetResolution(&dpi_x, &dpi_y);
    if (FAILED(hr)) return SPI_OTHER_ERROR;

    h_bitmap_info = PictureHandle(LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof(BITMAPINFO)));
    if (!h_bitmap_info) return SPI_NO_MEMORY;

    auto auto_unlock_header = std::make_unique<AutoUnlockBitmapHeader>(h_bitmap_info.get());
    auto bitmap_header = auto_unlock_header->GetBitmapHeader();
    if (!bitmap_header) return SPI_NO_MEMORY;

    bitmap_header->biSize = sizeof(BITMAPINFOHEADER);
    bitmap_header->biWidth = width;
    bitmap_header->biHeight = height;
    bitmap_header->biPlanes = 1;
    bitmap_header->biBitCount = 32;
    bitmap_header->biSizeImage = static_cast<DWORD>(bitmap_size);
    bitmap_header->biXPelsPerMeter = static_cast<LONG>(dpi_x * 39.37);
    bitmap_header->biYPelsPerMeter = static_cast<LONG>(dpi_y * 39.37);

    ComPtr<IWICFormatConverter> pConverter;

    hr = pFactory->CreateFormatConverter(pConverter.GetAddressOf());
    if (FAILED(hr)) return SPI_OTHER_ERROR;

    hr = pConverter->Initialize(
        pFrameDecode.Get(),
        GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone,
        NULL,
        1.,
        WICBitmapPaletteTypeCustom
    );
    if (FAILED(hr)) return SPI_OTHER_ERROR;

    h_bitmap = PictureHandle(LocalAlloc(LMEM_MOVEABLE, bitmap_size));
    if (!h_bitmap) return SPI_NO_MEMORY;

    auto auto_unlock_bitmap = std::make_unique<AutoUnlockBitmap>(h_bitmap.get());
    auto bitmap = auto_unlock_bitmap->GetBitmap();
    if (!bitmap) return SPI_NO_MEMORY;

    hr = pConverter->CopyPixels(NULL, stride, static_cast<UINT>(bitmap_size), bitmap);
    if (FAILED(hr)) return SPI_OTHER_ERROR;

    // Faster than IWICBitmapFlipRotator
    size_t half_height = static_cast<size_t>(height) / 2;
    auto line = std::make_unique_for_overwrite<BYTE[]>(stride);
    for (size_t j = 0; j < half_height; j++)
    {
        std::memcpy(line.get(), bitmap + stride * j, stride);
        std::memcpy(bitmap + stride * j, bitmap + stride * (height - j - 1), stride);
        std::memcpy(bitmap + stride * (height - j - 1), line.get(), stride);
    }

	return SPI_ALL_RIGHT;
}