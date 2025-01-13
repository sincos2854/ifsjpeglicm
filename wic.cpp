// Copyright (c) 2024 sincos2854
// Licensed under the MIT License

#include <wrl\client.h>
#include <wincodec.h>
#include "spi00in.h"
#include "wic.h"

using namespace Microsoft::WRL;

int SpiWic::Decode(
    const LPBYTE data,
    size_t size,
    std::unique_ptr<HANDLE, PictureHandleDeleter>& h_bitmap_info,
    std::unique_ptr<HANDLE, PictureHandleDeleter>& h_bitmap
)
{
    LPBITMAPINFOHEADER bitmap_header = nullptr;
    LPBYTE bitmap = nullptr;
    UINT profile_size = 0;

    ComPtr<IWICImagingFactory> pFactory;

    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IWICImagingFactory,
        reinterpret_cast<LPVOID*>(pFactory.GetAddressOf())
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
            reinterpret_cast<LPVOID*>(pFactory.GetAddressOf())
        );
        if (FAILED(hr)) return SPI_OTHER_ERROR;
    }

    ComPtr<IWICStream> pStream;

    hr = pFactory->CreateStream(pStream.GetAddressOf());
    if (FAILED(hr)) return SPI_OTHER_ERROR;

    hr = pStream->InitializeFromMemory(data, static_cast<DWORD>(size));
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

    UINT count = 0;

    hr = pFrameDecode->GetColorContexts(0, NULL, &count);
    if (FAILED(hr)) return SPI_OTHER_ERROR;

    if (count)
    {
        ComPtr<IWICColorContext> pColorContext;

        hr = pFactory->CreateColorContext(pColorContext.GetAddressOf());
        if (FAILED(hr)) return SPI_OTHER_ERROR;

        hr = pFrameDecode->GetColorContexts(1, pColorContext.GetAddressOf(), &count);
        if (FAILED(hr)) return SPI_OTHER_ERROR;

        WICColorContextType type = WICColorContextUninitialized;

        hr = pColorContext->GetType(&type);
        if (FAILED(hr)) return SPI_OTHER_ERROR;

        if (type == WICColorContextProfile) {

            hr = pColorContext->GetProfileBytes(0, NULL, &profile_size);
            if (FAILED(hr)) return SPI_OTHER_ERROR;

            if (0 < profile_size)
            {
                h_bitmap_info = std::unique_ptr<HANDLE, PictureHandleDeleter>(
                    LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof(BITMAPV5HEADER) + profile_size), PictureHandleDeleter()
                );
                if (!h_bitmap_info) return SPI_NO_MEMORY;

                LPBITMAPV5HEADER v5 = reinterpret_cast<LPBITMAPV5HEADER>(LocalLock(h_bitmap_info.get()));
                if (!v5) return SPI_NO_MEMORY;

                v5->bV5Size = sizeof(BITMAPV5HEADER);
                v5->bV5CSType = PROFILE_EMBEDDED;
                v5->bV5ProfileData = sizeof(BITMAPV5HEADER);
                v5->bV5ProfileSize = profile_size;

                UINT r = 0;

                hr = pColorContext->GetProfileBytes(v5->bV5ProfileSize, reinterpret_cast<LPBYTE>(v5) + v5->bV5ProfileData, &r);
                if (FAILED(hr) || v5->bV5ProfileSize != r) return SPI_OTHER_ERROR;

                LocalUnlock(h_bitmap_info.get());
            }
        }
    }

    if (profile_size == 0)
    {
        h_bitmap_info = std::unique_ptr<HANDLE, PictureHandleDeleter>(
            LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, sizeof(BITMAPINFO)), PictureHandleDeleter()
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

    h_bitmap = std::unique_ptr<HANDLE, PictureHandleDeleter>(
        LocalAlloc(LMEM_MOVEABLE, bitmap_size), PictureHandleDeleter()
    );
    if (!h_bitmap) return SPI_NO_MEMORY;

    bitmap = reinterpret_cast<LPBYTE>(LocalLock(h_bitmap.get()));
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