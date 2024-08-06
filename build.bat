@echo off
set PROJECT_DIR=C:\Projects\videocat
set SRCDIR=%PROJECT_DIR%\src
set PROJECT_BUILD_DIR=%PROJECT_DIR%\build

set PROJECT_LIB_DIRECTORIES=%SRCDIR%\thirdparty\libs;%LOCALAPPDATA%\Microsoft\WinGet\Packages\Gyan.FFmpeg.Shared_Microsoft.Winget.Source_8wekyb3d8bbwe\ffmpeg-7.0.2-full_build-shared\lib

:: MSVC reads these env variables by default to search for objects, 
:: libraries and include directories
set LIB=%LIB%;%PROJECT_LIB_DIRECTORIES%
set INCLUDE=%INCLUDE%;%SRCDIR%;%SRCDIR%\thirdparty\libmisb\include;%LOCALAPPDATA%\Microsoft\WinGet\Packages\Gyan.FFmpeg.Shared_Microsoft.Winget.Source_8wekyb3d8bbwe\ffmpeg-7.0.2-full_build-shared\include


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
@echo on
cl %DEBUG_COMPILER_FLAGS% %SOURCE_FILES% %DEBUG_LINK_LIBRARIES% /link /OUT:videocat.exe
del *.obj

popd
