@echo off
cd /d "%~dp0"

if not exist "jpegli" (
    git clone https://github.com/google/jpegli.git --recursive

    cd jpegli
    git checkout --recurse-submodules 12ac27c46927f94eab7e8eaf1e14e6419ab55d33
    cd ..

    git apply -p1 --directory=jpegli/third_party/libjpeg-turbo patch_libjpeg_turbo.patch
)
