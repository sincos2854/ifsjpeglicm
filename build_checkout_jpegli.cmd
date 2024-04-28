@echo off
cd /d "%~dp0"

if not exist "libjxl" (
    git clone https://github.com/libjxl/libjxl.git --recursive
    cd libjxl
    git checkout f602da19370166f75f1e7e175db890e8cd480c19
    cd third_party\libjpeg-turbo
    git fetch origin main
    git checkout refs/tags/2.1.91
    cd ..\..\..\
)
