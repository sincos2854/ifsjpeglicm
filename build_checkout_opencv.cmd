@echo off
cd /d "%~dp0"

if not exist "opencv" (
    git clone -b 5.1.0 --depth 1 https://github.com/opencv/opencv.git
)
