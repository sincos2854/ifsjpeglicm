@echo off
cd /d "%~dp0"

if not exist "jpegli" (
    git clone https://github.com/google/jpegli.git --recursive

    cd jpegli
    git checkout --recurse-submodules c7d342c25d3587ac0110c915989ad94f414b6c8d
    cd ..

    git apply -p1 --directory=jpegli/third_party/libjpeg-turbo patch_libjpeg_turbo.patch
)
