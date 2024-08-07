@echo off
set PROJECT=%cd%
set PROJECT_BUILD_DIR=%PROJECT%\build

echo Installing ffmpeg libraries
winget install Gyan.FFmpeg.Shared

echo Copying runtime libraries to PROJECT_BUILD_DIR
pushd %LOCALAPPDATA%\Microsoft\WinGet\Packages

FOR /F "tokens=* USEBACKQ" %%F IN (`dir /s /b Gyan.FFmpeg.Shared* ^| dir /s /b /ad ffmpeg*`) DO (
SET FFMPEG_DIR=%%F
)

mkdir %PROJECT%\bin\windows

xcopy /d %FFMPEG_DIR%\bin\*.dll %PROJECT%\bin\windows

echo Done.

popd
