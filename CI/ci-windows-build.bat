@echo off

:: clone libraries git (set env variable to GIT_SSH_COMMAND maybe use setx once as this key won't change) 
set GIT_SSH_COMMAND=ssh -i E:\\\gitlab\\\id_rsa 

:: use 4 threads for parallel compilation of the project
set CL=/MP4

:: determine VS version and set variables
if "%COMPILER%" == "VS2012" (
set QT_COMPILERPREFIX=msvc2012
set VS_COMPILERVERSION_LONG=11.0
set VS_COMPILERVERSION_SHORT=11
set VS_EDITION_YEAR=2012
set VS_EDITION_PATH= 11.0
)
if "%COMPILER%" == "VS2013" (
set QT_COMPILERPREFIX=msvc2013
set VS_COMPILERVERSION_LONG=12.0
set VS_COMPILERVERSION_SHORT=12
set VS_EDITION_YEAR=2013
set VS_EDITION_PATH= 12.0
)
if "%COMPILER%" == "VS2015" (
set QT_COMPILERPREFIX=msvc2015
set VS_COMPILERVERSION_LONG=14.0
set VS_COMPILERVERSION_SHORT=14
set VS_EDITION_YEAR=2015
set VS_EDITION_PATH= 14.0
)
if "%COMPILER%" == "VS2017" (
set QT_COMPILERPREFIX=msvc2017
set VS_COMPILERVERSION_LONG=15.0
set VS_COMPILERVERSION_SHORT=15
set VS_EDITION_YEAR=2017
::VS2017 default install path is different from other versions
set VS_EDITION_PATH=\2017\Professional
)

set BUILD_PLATFORM=%COMPILER%

:: determine architecture and set variables
if "%ARCHITECTURE%" == "x64" (
set ARCHBITS=_64
set ARCH_VS= Win64
set STRING_ARCH=64-Bit
) else (
set ARCHBITS=
set ARCH_VS=
set STRING_ARCH=32-Bit
)

set GENERATOR=Visual Studio %VS_COMPILERVERSION_SHORT% %VS_EDITION_YEAR%%ARCH_VS%
set VS_PATH="C:\Program Files (x86)\Microsoft Visual Studio%VS_EDITION_PATH%\Common7\IDE\devenv.com"




::setlocal enabledelayedexpansion
for /D %%s in (.\assignment*) do (
    @echo "Processing " %%s
    cd %%s
    :: "continue" is not cool to simulate in batch scripts (extra- call out of loop etc.)
    if exist CMakeLists.txt (
        mkdir "%%s-build"
        cd "%%s-build"
        del *.exe
        IF %errorlevel% NEQ 0 exit /b %errorlevel%
        "C:\Program Files\CMake\bin\cmake.exe" -G "%GENERATOR%" -DCMAKE_BUILD_TYPE=Release ..
        IF %errorlevel% NEQ 0 exit /b %errorlevel%
        %VS_PATH% /Build "Release" %%s.sln /Project "ALL_BUILD"
        IF %errorlevel% NEQ 0 exit /b %errorlevel%
        cd ..
    ) else (
        echo "CMakeLists.txt not found. Skipping."
    )
    cd ..
)
:: endlocal



:: Assignment01
:: mkdir a01-build
:: cd a01-build
:: del *.exe
:: "C:\Program Files\CMake\bin\cmake.exe" -G "%GENERATOR%" -DCMAKE_BUILD_TYPE=Release ../assignment01
:: IF %errorlevel% NEQ 0 exit /b %errorlevel%
:: %VS_PATH% /Build "Release" Assignment01.sln /Project "ALL_BUILD"
:: cd ..
:: Assignment02 was no programming task

:: Assignment03
:: mkdir a03-build
:: cd a03-build
:: del *.exe
:: "C:\Program Files\CMake\bin\cmake.exe" -G "%GENERATOR%" -DCMAKE_BUILD_TYPE=Release ../assignment03
:: IF %errorlevel% NEQ 0 exit /b %errorlevel%
:: %VS_PATH% /Build "Release" Assignment03.sln /Project "ALL_BUILD"
:: cd ..



IF %errorlevel% NEQ 0 exit /b %errorlevel%

