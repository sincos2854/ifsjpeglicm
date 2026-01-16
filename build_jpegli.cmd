@echo off
cd /d "%~dp0"

set ARCH=%~1

call build_checkout_jpegli.cmd

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

    if exist "jpegli\out_%BUILD_ARCH%_release" (
        goto :copy_files
    )

    cd jpegli

    rd /s /q build_%BUILD_ARCH%_release >nul 2>&1

    cmake -G "Visual Studio 17 2022" -A %BUILD_ARCH% ^
    -B "build_%BUILD_ARCH%_release" ^
    -DCMAKE_INSTALL_PREFIX="out_%BUILD_ARCH%_release" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" ^
    -DBUILD_TESTING=OFF ^
    -DJPEGXL_ENABLE_JNI=OFF ^
    -DJPEGXL_ENABLE_SJPEG=OFF ^
    -DJPEGXL_ENABLE_OPENEXR=OFF ^
    -DJPEGXL_ENABLE_BENCHMARK=OFF ^
    -DJPEGXL_STATIC=ON

    cmake --build "build_%BUILD_ARCH%_release" --config Release --target install

    cd ..\

:copy_files
    xcopy /y "jpegli\build_%BUILD_ARCH%_release\lib\include\jpegli\*.h" "include\%BUILD_ARCH%\"
    xcopy /y "jpegli\lib\jpegli\common.h" "include\%BUILD_ARCH%\lib\jpegli\"
    xcopy /y "jpegli\lib\jpegli\decode.h" "include\%BUILD_ARCH%\lib\jpegli\"
    xcopy /y "jpegli\lib\jpegli\types.h" "include\%BUILD_ARCH%\lib\jpegli\"
    xcopy /y "jpegli\lib\base\include_jpeglib.h" "include\%BUILD_ARCH%\lib\base\"

    xcopy /y "jpegli\build_%BUILD_ARCH%_release\lib\Release\jpegli-static.lib" "lib\%BUILD_ARCH%\Release\"
    xcopy /y "jpegli\out_%BUILD_ARCH%_release\lib\hwy.lib" "lib\%BUILD_ARCH%\Release\"

    exit /b
