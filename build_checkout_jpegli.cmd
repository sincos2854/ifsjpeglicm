@echo off
cd /d "%~dp0"

if not exist "jpegli" (
    git clone https://github.com/google/jpegli.git --recursive
    git apply -p1 --directory=jpegli/third_party/libjpeg-turbo patch_libjpeg_turbo.patch
)
