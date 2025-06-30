@echo off
cd /d "%~dp0"

if not exist "jpegli" (
    git clone https://github.com/google/jpegli.git --recursive

    cd jpegli
    git checkout bc19ca2393f79bfe0a4a9518f77e4ad33ce1ab7a
    cd ..

    git apply -p1 --directory=jpegli/third_party/libjpeg-turbo patch_libjpeg_turbo.patch
)
