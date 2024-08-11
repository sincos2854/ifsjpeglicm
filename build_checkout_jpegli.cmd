@echo off
cd /d "%~dp0"

if not exist "jpegli" (
    git clone https://github.com/google/jpegli.git --recursive
    cd jpegli
    git checkout 8a4c6dce4793e0011ba34e6768feaab43ed46178
    cd third_party\libjpeg-turbo
    git fetch origin main
    git checkout refs/tags/2.1.91
    cd ..\..\..\
)
