@echo off
set PROJECT_DIR=%cd%
set SRCDIR=%PROJECT_DIR%\src
set PROJECT_BUILD_DIR=%PROJECT_DIR%\build

FOR /F "tokens=* USEBACKQ" %%F IN (`dir /s /b Gyan.FFmpeg.Shared* ^| dir /s /b /ad ffmpeg*`) DO (
SET FFMPEG_DIR=%%F
)

set PROJECT_LIB_DIRECTORIES=%SRCDIR%\thirdparty\libs;^
%FFMPEG_DIR%\lib

:: MSVC reads these env variables by default to search for objects, 
:: libraries and include directories
set LIB=%LIB%;%PROJECT_LIB_DIRECTORIES%
set INCLUDE=%INCLUDE%;^
%SRCDIR%;^
%SRCDIR%\thirdparty\libmisb\include;^
%FFMPEG_DIR%\include


@REM Debug configuration
set DEBUG_COMPILER_FLAGS=/nologo /Zi /EHsc /wd4005 /DWINDOWS

set SOURCE_FILES=^
 %SRCDIR%\main.c^
 %SRCDIR%\thirdparty\libmisb\src\*.c

set DEBUG_LINK_LIBRARIES=avformat.lib^
 avutil.lib^
 avcodec.lib^
 avdevice.lib^
 avfilter.lib^
 swscale.lib^
 ole32.lib

 
:: We use pushd popd to make sure 
:: all the compilation output goes into build
mkdir %PROJECT_BUILD_DIR%
pushd %PROJECT_BUILD_DIR%

cl %DEBUG_COMPILER_FLAGS% %SOURCE_FILES% /link %DEBUG_LINK_LIBRARIES% /OUT:videocat.exe
del *.obj

popd

xcopy /d %PROJECT_DIR%\bin\windows\* %PROJECT_BUILD_DIR%