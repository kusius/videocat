@echo off
set PROJECT=%cd%
set PROJECT_BUILD_DIR=%PROJECT%\build

@REM echo Installing ffmpeg libraries
winget install Gyan.FFmpeg.Shared

echo Copying runtime libraries to PROJECT_BUILD_DIR

FOR /F "tokens=* USEBACKQ" %%F IN (`dir /s /b /ad %LOCALAPPDATA%\Microsoft\WinGet\Packages\ffmpeg*shared*`) DO (
SET FFMPEG_DIR=%%F
)

echo "FFMPEG Directory %FFMPEG_DIR%"

@rem copy ffmpeg runtime libraries
mkdir %PROJECT%\bin\windows
xcopy /d %FFMPEG_DIR%\bin\*.dll %PROJECT%\bin\windows

@rem copy ffmpeg header files
mkdir %PROJECT%\src\thirdparty\ffmpeg
xcopy /d /s /e %FFMPEG_DIR%\include\* %PROJECT%\src\thirdparty\ffmpeg

@rem copy lib files (development libraries)
mkdir %PROJECT%\src\thirdparty\lib
xcopy /d %FFMPEG_DIR%\lib\* %PROJECT%\src\thirdparty\lib

echo Done.

