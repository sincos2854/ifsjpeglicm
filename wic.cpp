// Copyright (c) 2024 - 2025 sincos2854
// Licensed under the MIT License

#include <wrl\client.h>
#include <wincodec.h>
#include "spi00in.h"
#include "wic.h"

using namespace Microsoft::WRL;

int SpiWic::Decode(
    const BYTE* data,
    size_t size,
    PictureHandle& h_bitmap_info,
    PictureHandle& h_bitmap
)
{
    LPBITMAPINFOHEADER bitmap_header = nullptr;
    BYTE* bitmap = nullptr;
    UINT profile_size = 0;

    ComPtr<IWICImagingFactory> pFactory;

    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IWICImagingFactory,
        reinterpret_cast<void**>(pFactory.GetAddressOf())
    );

    if (hr == CO_E_NOTINITIALIZED)
    {
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);

        if (hr == S_OK)
        {
            initialized_ = true;
        }
        else if (hr == S_FALSE)
        {
            CoUninitialize();
        }
        else
        {
            if (FAILED(hr)) return SPI_OTHER_ERROR;
        }

        hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IWICImagingFactory,
            reinterpret_cast<void**>(pFactory.GetAddressOf())
        );
        if (FAILED(hr)) return SPI_OTHER_ERROR;
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

    double dpi_x = .0, dpi_y = .0;

    hr = pFrameDecode->GetResolution(&dpi_x, &dpi_y);
    if (FAILED(hr)) return SPI_OTHER_ERROR;

    if (profile_size == 0)
    {
        h_bitmap_info = PictureHandle(
            LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof(BITMAPINFO))
        );
        if (!h_bitmap_info) return SPI_NO_MEMORY;
    }

    bitmap_header = reinterpret_cast<LPBITMAPINFOHEADER>(LocalLock(h_bitmap_info.get()));
    if (!bitmap_header) return SPI_NO_MEMORY;

    if (profile_size == 0)
    {
        bitmap_header->biSize = sizeof(BITMAPINFOHEADER);
    }

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

    h_bitmap = PictureHandle(
        LocalAlloc(LMEM_MOVEABLE, bitmap_size)
    );
    if (!h_bitmap) return SPI_NO_MEMORY;

    bitmap = reinterpret_cast<BYTE*>(LocalLock(h_bitmap.get()));
    if (!bitmap) return SPI_NO_MEMORY;

    hr = pConverter->CopyPixels(NULL, stride, static_cast<UINT>(bitmap_size), bitmap);
    if (FAILED(hr)) return SPI_OTHER_ERROR;

    // Faster than IWICBitmapFlipRotator
    size_t half_height = static_cast<size_t>(height) / 2;
    auto line = std::make_unique<BYTE[]>(stride);
    for (size_t j = 0; j < half_height; j++)
    {
        memcpy(line.get(), bitmap + stride * j, stride);
        memcpy(bitmap + stride * j, bitmap + stride * (height - j - 1), stride);
        memcpy(bitmap + stride * (height - j - 1), line.get(), stride);
    }

	return SPI_ALL_RIGHT;
}