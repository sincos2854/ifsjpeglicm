@echo off
cd /d "%~dp0"

set ARCH=%~1

call build_checkout_opencv.cmd

if "%ARCH%" equ "Win32" (
    call :build %ARCH%
) else (
    if "%ARCH%" equ "x64" (
        call :build %ARCH%
    ) else (
        call :build Win32
        call :build x64
    )
)

exit /b

:build
    set BUILD_ARCH=%~1

    if exist "opencv\out_%BUILD_ARCH%_release" (
        goto :copy_files
    )

    cd opencv

    rd /s /q build_%BUILD_ARCH%_release >nul 2>&1

    cmake -G "Visual Studio 17 2022" -A %BUILD_ARCH% ^
    -B "build_%BUILD_ARCH%_release" ^
    -DCMAKE_INSTALL_PREFIX="out_%BUILD_ARCH%_release" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DBUILD_TESTS=OFF ^
    -DBUILD_PERF_TESTS=OFF ^
    -DBUILD_EXAMPLES=OFF ^
    -DBUILD_opencv_apps=OFF ^
    -DWITH_OPENCL=OFF ^
    -DWITH_FFMPEG=OFF ^
    -DWITH_GSTREAMER=OFF ^
    -DWITH_MSMF=OFF ^
    -DWITH_DSHOW=OFF ^
    -DWITH_1394=OFF ^
    -DWITH_PROTOBUF=OFF ^
    -DVIDEOIO_ENABLE_PLUGINS=OFF ^
    -DOPENCV_DNN_OPENCL=OFF ^
    -DBUILD_opencv_dnn=OFF ^
    -DBUILD_opencv_features2d=OFF ^
    -DBUILD_opencv_flann=OFF ^
    -DBUILD_opencv_gapi=OFF ^
    -DBUILD_opencv_highgui=OFF ^
    -DBUILD_opencv_imgcodecs=OFF ^
    -DBUILD_opencv_imgproc=OFF ^
    -DBUILD_opencv_ml=OFF ^
    -DBUILD_opencv_photo=OFF ^
    -DBUILD_opencv_video=OFF ^
    -DBUILD_opencv_videoio=OFF ^
    -DWITH_TBB=OFF ^
    -DBUILD_TBB=OFF ^
    -DWITH_IPP=OFF ^
    -DBUILD_IPP_IW=OFF ^
    -DBUILD_ITT=OFF ^
    -DWITH_WIN32UI=OFF ^
    -DWITH_DIRECTX=OFF ^
    -DWITH_IMGCODEC_HDR=OFF ^
    -DWITH_IMGCODEC_SUNRASTER=OFF ^
    -DWITH_IMGCODEC_PXM=OFF ^
    -DWITH_IMGCODEC_PFM=OFF ^
    -DBUILD_ZLIB=OFF ^
    -DWITH_PNG=OFF ^
    -DWITH_JPEG=OFF ^
    -DWITH_TIFF=OFF ^
    -DWITH_WEBP=OFF ^
    -DWITH_OPENJPEG=OFF ^
    -DWITH_JASPER=OFF ^
    -DWITH_OPENEXR=OFF

    cmake --build "build_%BUILD_ARCH%_release" --config Release --target install
    
    cd ..\

:copy_files
    xcopy /s /y "opencv\out_%BUILD_ARCH%_release\include\opencv2\*" "include\%BUILD_ARCH%\opencv2\"

    for /f "usebackq delims=" %%a in (`where /r "opencv\out_%BUILD_ARCH%_release" opencv*.lib`) do set "LIB_PATH=%%a"
    xcopy /y "%LIB_PATH%" "lib\%BUILD_ARCH%\Release\"

    exit /b
