@echo off
set PROJECT=%cd%
set PROJECT_BUILD_DIR=%PROJECT%\build

echo Installing ffmpeg libraries
winget install Gyan.FFmpeg.Shared

echo Copying runtime libraries to PROJECT_BUILD_DIR

FOR /F "tokens=* USEBACKQ" %%F IN (`dir /s /b /ad %LOCALAPPDATA%\Microsoft\WinGet\Packages\ffmpeg*shared*`) DO (
SET FFMPEG_DIR=%%F
)

mkdir %PROJECT%\bin\windows

xcopy /d %FFMPEG_DIR%\bin\*.dll %PROJECT%\bin\windows

echo Done.

