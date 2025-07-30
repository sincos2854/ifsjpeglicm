@echo off
cd /d "%~dp0"

call build_checkout_jpegli.cmd

cd jpegli

cmake -G "Visual Studio 17 2022" -A Win32 ^
-B "build_Win32_release" ^
-DCMAKE_INSTALL_PREFIX="out_Win32_release" ^
-DCMAKE_BUILD_TYPE=Release ^
-DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" ^
-DBUILD_TESTING=OFF ^
-DJPEGXL_ENABLE_JNI=OFF ^
-DJPEGXL_ENABLE_SJPEG=OFF ^
-DJPEGXL_ENABLE_OPENEXR=OFF ^
-DJPEGXL_ENABLE_BENCHMARK=OFF ^
-DJPEGXL_STATIC=ON

cmake --build "build_Win32_release" --config Release --target install

cmake -G "Visual Studio 17 2022" -A x64 ^
-B "build_x64_release" ^
-DCMAKE_INSTALL_PREFIX="out_x64_release" ^
-DCMAKE_BUILD_TYPE=Release ^
-DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>" ^
-DBUILD_TESTING=OFF ^
-DJPEGXL_ENABLE_JNI=OFF ^
-DJPEGXL_ENABLE_SJPEG=OFF ^
-DJPEGXL_ENABLE_OPENEXR=OFF ^
-DJPEGXL_ENABLE_BENCHMARK=OFF ^
-DJPEGXL_STATIC=ON

cmake --build "build_x64_release" --config Release --target install

cd ..\

xcopy /y jpegli\build_Win32_release\lib\include\jpegli\*.h include\Win32\
xcopy /y jpegli\lib\jpegli\common.h include\Win32\lib\jpegli\
xcopy /y jpegli\lib\jpegli\decode.h include\Win32\lib\jpegli\
xcopy /y jpegli\lib\jpegli\types.h include\Win32\lib\jpegli\
xcopy /y jpegli\lib\base\include_jpeglib.h include\Win32\lib\base\

xcopy /y jpegli\build_Win32_release\third_party\highway\Release\hwy.lib lib\Win32\Release\
xcopy /y jpegli\build_Win32_release\lib\Release\jpegli-static.lib lib\Win32\Release\

xcopy /y jpegli\build_x64_release\lib\include\jpegli\*.h include\x64\
xcopy /y jpegli\lib\jpegli\common.h include\x64\lib\jpegli\
xcopy /y jpegli\lib\jpegli\decode.h include\x64\lib\jpegli\
xcopy /y jpegli\lib\jpegli\types.h include\x64\lib\jpegli\
xcopy /y jpegli\lib\base\include_jpeglib.h include\x64\lib\base\

xcopy /y jpegli\build_x64_release\third_party\highway\Release\hwy.lib lib\x64\Release\
xcopy /y jpegli\build_x64_release\lib\Release\jpegli-static.lib lib\x64\Release\
