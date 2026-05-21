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

    cmake -G "Visual Studio 18 2026" -A %BUILD_ARCH% ^
    -B "build_%BUILD_ARCH%_release" ^
    -DCMAKE_INSTALL_PREFIX="out_%BUILD_ARCH%_release" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" ^
    -DBUILD_TESTING=OFF ^
    -DJPEGLI_ENABLE_DOXYGEN=OFF ^
    -DJPEGLI_ENABLE_MANPAGES=OFF ^
    -DJPEGLI_ENABLE_BENCHMARK=OFF ^
    -DJPEGLI_ENABLE_JNI=OFF ^
    -DJPEGLI_ENABLE_OPENEXR=OFF ^
    -DJPEGLI_STATIC=ON ^
    -DCMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE=ON

    cmake --build "build_%BUILD_ARCH%_release" --config Release --target install

    cd ..\

:copy_files
    xcopy "jpegli\out_%BUILD_ARCH%_release\include\jpegli\*.h" "include\%BUILD_ARCH%\" /y
    xcopy "jpegli\lib\jpegli\common.h" "include\%BUILD_ARCH%\lib\jpegli\" /y
    xcopy "jpegli\lib\jpegli\decode.h" "include\%BUILD_ARCH%\lib\jpegli\" /y
    xcopy "jpegli\lib\jpegli\types.h" "include\%BUILD_ARCH%\lib\jpegli\" /y
    xcopy "jpegli\lib\base\include_jpeglib.h" "include\%BUILD_ARCH%\lib\base\" /y

    xcopy "jpegli\build_%BUILD_ARCH%_release\lib\Release\jpegli-static.lib" "lib\%BUILD_ARCH%\Release\" /y
    xcopy "jpegli\out_%BUILD_ARCH%_release\lib\hwy.lib" "lib\%BUILD_ARCH%\Release\" /y

    exit /b
