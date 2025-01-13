@echo off
cd /d "%~dp0"

if not exist "jpegli" (
    git clone https://github.com/google/jpegli.git --recursive
    cd jpegli
    git checkout 8d333ebb0d715b0544a59d29724af9f8155aeb52
    cd third_party\libjpeg-turbo
    git fetch origin main
    git checkout refs/tags/2.1.91
    cd ..\..\..\
)
