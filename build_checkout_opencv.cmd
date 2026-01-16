@echo off
cd /d "%~dp0"

if not exist "opencv" (
    git clone -b 4.13.0 --depth 1 https://github.com/opencv/opencv.git
)
