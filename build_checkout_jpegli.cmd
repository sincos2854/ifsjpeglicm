@echo off
cd /d "%~dp0"

if not exist "jpegli" (
    git clone https://github.com/google/jpegli.git --recursive
    cd jpegli
    git checkout 5126d62d24d368f0ceadd53454653edeb9086386
    cd third_party\libjpeg-turbo
    git fetch origin main
    git checkout refs/tags/2.1.91
    cd ..\..\..\
)
