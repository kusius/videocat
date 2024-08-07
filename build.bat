@echo off
set PROJECT_DIR=%cd%
set SRCDIR=%PROJECT_DIR%\src
set PROJECT_BUILD_DIR=%PROJECT_DIR%\build

set NUKLEAR_DIR=%SRCDIR%\thirdparty\nuklear

set PROJECT_LIB_DIRECTORIES=%SRCDIR%\thirdparty\lib

:: MSVC reads these env variables by default to search for objects, 
:: libraries and include directories
set LIB=%LIB%;%PROJECT_LIB_DIRECTORIES%
set INCLUDE=%INCLUDE%;^
%SRCDIR%;^
%SRCDIR%\thirdparty\libmisb\include;^
%SRCDIR%\thirdparty\ffmpeg;


@REM Debug configuration
set DEBUG_COMPILER_FLAGS=/D_CRT_SECURE_NO_DEPRECATE /nologo /Zi /EHsc /wd4005 /DWINDOWS 

set SOURCE_FILES=%SRCDIR%\thirdparty\libmisb\src\*.c^
 %SRCDIR%\main.c

set DEBUG_LINK_LIBRARIES=avformat.lib^
 avutil.lib^
 avcodec.lib^
 avdevice.lib^
 avfilter.lib^
 swscale.lib^
 ole32.lib^
 user32.lib^
 dxguid.lib^
 dxgi.lib^
 d3d12.lib

 
:: We use pushd popd to make sure 
:: all the compilation output goes into build
mkdir %PROJECT_BUILD_DIR%
pushd %PROJECT_BUILD_DIR%

@rem compile shaders for user interface
fxc.exe /nologo /T vs_5_1 /E vs /O3 /Zpc /Ges /Fh %NUKLEAR_DIR%\nuklear_d3d12_vertex_shader.h /Vn nk_d3d12_vertex_shader /Qstrip_reflect /Qstrip_debug /Qstrip_priv  %NUKLEAR_DIR%\nuklear_d3d12.hlsl
fxc.exe /nologo /T ps_5_1 /E ps /O3 /Zpc /Ges /Fh %NUKLEAR_DIR%\nuklear_d3d12_pixel_shader.h /Vn nk_d3d12_pixel_shader /Qstrip_reflect /Qstrip_debug /Qstrip_priv /enable_unbounded_descriptor_tables  %NUKLEAR_DIR%\nuklear_d3d12.hlsl

cl %DEBUG_COMPILER_FLAGS% %SOURCE_FILES% /link %DEBUG_LINK_LIBRARIES% /out:videocat.exe
del *.obj

popd

xcopy /d %PROJECT_DIR%\bin\windows\* %PROJECT_BUILD_DIR%