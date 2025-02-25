@echo off
cd /d "%~dp0"

if not exist "jpegli" (
    git clone https://github.com/google/jpegli.git --recursive
    cd jpegli
    git checkout bc19ca2393f79bfe0a4a9518f77e4ad33ce1ab7a
    cd third_party\libjpeg-turbo
    git fetch origin main
    git checkout refs/tags/2.1.91
    cd ..\..\..\
)
