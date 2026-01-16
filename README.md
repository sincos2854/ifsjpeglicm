# ifsjpeglicm

[Jpegli](https://github.com/google/jpegli) Susie Plug-in for JPEG(*.jpeg, *jpg) file.  
Google's announcement : [Introducing Jpegli: A New JPEG Coding Library | Google Open Source Blog](https://opensource.googleblog.com/2024/04/introducing-jpegli-new-jpeg-coding-library.html)  
(Use WIC(Windows Imaging Component) for CMYK/YCCK JPEG file.)

## Download

From Releases page

## Features

- 32bit(.spi) and 64bit(.sph)
- ANSI and Unicode (e.g. GetPicture and GetPictureW)
- Always returns a 32bit BGRA bitmap
- ICC Profile (Need a viewer that supports color management like [susico](http://www.vector.co.jp/soft/dl/winnt/art/se515212.html))

## Libraries Used

- [Jpegli](https://github.com/google/jpegli) (Dependency: [Highway](https://github.com/google/highway))
- [OpenCV](https://github.com/opencv/opencv)

## Inspired by

- [TORO's Software library(Win32/Win64 Plugin)](http://toro.d.dooo.jp/slplugin.html)
- [uyjulian/ifjxl: JPEG XL plugin for Susie Image Viewer](https://github.com/uyjulian/ifjxl)
- [Mr-Ojii/ifheif: HEIF/AVIF Susie plugin](https://github.com/Mr-Ojii/ifheif)
- [BPGファイル用Susieプラグインを埋め込みプロファイル対応にする : やんま まのblog（仮）](http://blog.livedoor.jp/yamma_ma/archives/44473876.html)
