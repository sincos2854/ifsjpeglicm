@echo off
cd /d "%~dp0"

if not exist "jpegli" (
    git clone https://github.com/google/jpegli.git --recursive

    cd jpegli
    git checkout --recurse-submodules 7cdf212790241868c77dca777dbee14e98128cba
    cd ..

    git apply -p1 --directory=jpegli/third_party/libjpeg-turbo patch_libjpeg_turbo.patch
)
