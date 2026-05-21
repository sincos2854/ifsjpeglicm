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

    cmake -G "Visual Studio 18 2026" -A %BUILD_ARCH% ^
    -B "build_%BUILD_ARCH%_release" ^
    -DCMAKE_INSTALL_PREFIX="out_%BUILD_ARCH%_release" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DBUILD_LIST=core ^
    -DBUILD_opencv_imgcodecs=OFF ^
    -DBUILD_opencv_highgui=OFF ^
    -DBUILD_IPP_IW=OFF ^
    -DWITH_ADE=OFF ^
    -DWITH_1394=OFF ^
    -DWITH_AVIF=OFF ^
    -DWITH_VTK=OFF ^
    -DWITH_EIGEN=OFF ^
    -DWITH_FFMPEG=OFF ^
    -DWITH_GSTREAMER=OFF ^
    -DWITH_IPP=OFF ^
    -DWITH_JASPER=OFF ^
    -DWITH_OPENJPEG=OFF ^
    -DWITH_JPEG=OFF ^
    -DWITH_WEBP=OFF ^
    -DWITH_OPENEXR=OFF ^
    -DWITH_PNG=OFF ^
    -DWITH_WIN32UI=OFF ^
    -DWITH_TBB=OFF ^
    -DWITH_TIFF=OFF ^
    -DWITH_DSHOW=OFF ^
    -DWITH_MSMF=OFF ^
    -DWITH_OPENCL=OFF ^
    -DWITH_DIRECTX=OFF ^
    -DWITH_DIRECTML=OFF ^
    -DWITH_LAPACK=OFF ^
    -DWITH_PROTOBUF=OFF ^
    -DWITH_IMGCODEC_GIF=OFF ^
    -DWITH_IMGCODEC_HDR=OFF ^
    -DWITH_IMGCODEC_SUNRASTER=OFF ^
    -DWITH_IMGCODEC_PXM=OFF ^
    -DWITH_IMGCODEC_PFM=OFF ^
    -DWITH_OBSENSOR=OFF ^
    -DBUILD_opencv_apps=OFF ^
    -DBUILD_DOCS=OFF ^
    -DBUILD_EXAMPLES=OFF ^
    -DBUILD_PACKAGE=OFF ^
    -DBUILD_PERF_TESTS=OFF ^
    -DBUILD_TESTS=OFF ^
    -DBUILD_JAVA=OFF ^
    -DOPENCV_PYTHON_SKIP_DETECTION=ON ^
    -DCV_TRACE=OFF ^
    -DBUILD_opencv_world=OFF

    cmake --build "build_%BUILD_ARCH%_release" --config Release --target install
    
    cd ..\

:copy_files
    xcopy "opencv\out_%BUILD_ARCH%_release\include\opencv2\*" "include\%BUILD_ARCH%\opencv2\"  /y /s /q

    for /f "usebackq delims=" %%i in (`where /r "opencv\out_%BUILD_ARCH%_release" opencv*.lib`) do set "LIB_PATH=%%i"
    xcopy "%LIB_PATH%" "lib\%BUILD_ARCH%\Release\" /y

    exit /b
