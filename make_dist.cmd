REM @echo off
REM @setlocal

set PLUGINNAME=FileCopyEx3

set FARVER=%1
if "%FARVER%" equ "" set FARVER=Far3
set PLUGINARCH=%2
if "%PLUGINARCH%" equ "" set PLUGINARCH=x86

:: Get plugin version from resource
for /F "tokens=2,3 skip=2" %%i in (src\version.hpp) do set %%i=%%~j
if "%PLUGIN_VERSION_TXT%" equ "" echo Undefined version & exit 1
set PLUGINVER=%PLUGIN_VERSION_TXT%

:: Package name
set PKGNAME=%PLUGINNAME%-%PLUGINVER%_%FARVER%_%PLUGINARCH%.7z

:: Create temp directory
set PKGDIR=build\%PLUGINNAME%\%FARVER%
set PKGDIRARCH=%PKGDIR%\%PLUGINARCH%
if exist %PKGDIRARCH% rmdir /S /Q %PKGDIRARCH%

:: Copy files
if not exist %PKGDIR% ( mkdir %PKGDIR% > NUL )
mkdir %PKGDIRARCH% > NUL

REM set PKGDIRARCH=%FARVER%_%PLUGINARCH%\Plugins\%PLUGINNAME%

if exist *.lng copy *.lng %PKGDIRARCH% > NUL
if exist *.hlf copy *.hlf %PKGDIRARCH% > NUL
if exist *.md copy *.md %PKGDIRARCH% > NUL
if exist LICENSE.txt copy LICENSE.txt %PKGDIRARCH% > NUL
if exist changelog.txt copy changelog.txt %PKGDIRARCH% > NUL

REM if not exist "%PKGDIRARCH%\doc" ( mkdir "%PKGDIRARCH%\doc" )
if not exist "%PKGDIRARCH%\resource" ( mkdir "%PKGDIRARCH%\resource" )
copy /Y data\*.hlf %PKGDIRARCH%
copy /Y data\*.lng %PKGDIRARCH%
copy /Y data\*.diz %PKGDIRARCH%
copy /Y data\resource\*.* %PKGDIRARCH%\resource
copy /Y ..\%PLUGINNAME%-build\%PLUGINNAME%.dll %PKGDIRARCH%

:: Make archive
set ARCHIVE_NAME=..\..\..\%PKGNAME%
if exist %PKGDIRARCH%\%PLUGINNAME%.dll (
  if exist "C:\Program Files\7-Zip\7z.exe" (
    pushd %PKGDIRARCH%
    if exist %ARCHIVE_NAME% del %ARCHIVE_NAME%
    call "C:\Program Files\7-Zip\7z.exe" a -mx9 -t7z -r %ARCHIVE_NAME% *
    popd
    if errorlevel 1 echo Error creating archive & exit 1 /b
    echo Package %PKGNAME% created
  )
)
exit 0 /b
