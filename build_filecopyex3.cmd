@echo off
@setlocal

if "%FAR_VERSION%" == "" ( set "FAR_VERSION=Far3" )
if "%CMAKE_PATH%" == "" ( set "CMAKE_PATH=C:\cmake-2.8" )
if "%MINGW_PATH%" == "" ( set "MINGW_PATH=C:\mingw" )
if "%PROJECT_CONFIG%" == "" ( set "PROJECT_CONFIG=Debug" )
if "%BUILD_GEN%" == "" ( set "BUILD_GEN=NMake Makefiles" )
if "%BUILD_COMPILER%" == "" ( set "BUILD_COMPILER=MinGW" )
if "%BUILD_PROJECT%" == "" ( set "BUILD_PROJECT=FileCopyEx3" )
if "%BUILD_BUILD%" == "" ( set "BUILD_BUILD=Build" )
if "%PROJECT_CONF%" == "" ( set "PROJECT_CONF=x86" )
if "%PROJECT_VARS%" == "" ( set "PROJECT_VARS=" )
REM if %BUILD_PLATFORM% == "" ( set "BUILD_PLATFORM=Win32" )

REM set PATH=%CMAKE_PATH%\bin
echo %PATH%

set PROJECT_ROOT=%~dp0
REM echo PROJECT_ROOT=%PROJECT_ROOT%
set PROJECT_BUIILDDIR=%PROJECT_ROOT%\..\filecopyex3-build
REM \%FAR_VERSION%_%PROJECT_CONF%\Plugins\FileCopyEx3
echo %PROJECT_BUIILDDIR%
if not exist "%PROJECT_BUIILDDIR%" ( mkdir "%PROJECT_BUIILDDIR%" > NUL )
cd %PROJECT_BUIILDDIR%

if "%BUILD_COMPILER%" == "MinGW" (
    set PATH=%MINGW_PATH%\bin
    %CMAKE_PATH%\bin\cmake.exe -D CMAKE_BUILD_TYPE="%PROJECT_CONFIG%" -D CONF=%PROJECT_CONF% -G "%BUILD_GEN%" %PROJECT_ROOT%
    mingw32-make.exe -f Makefile
) else (
    call "%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat" %PROJECT_VARS%
    %CMAKE_PATH%\bin\cmake.exe -D CMAKE_BUILD_TYPE="%PROJECT_CONFIG%" -D CONF=%PROJECT_CONF% -G "%BUILD_GEN%" %PROJECT_ROOT%
    REM devenv %BUILD_PROJECT%.sln /%BUILD_BUILD% "%PROJECT_CONFIG%|%BUILD_PLATFORM%" /USEENV /Project "%BUILD_PROJECT%"
    nmake
)

@endlocal
