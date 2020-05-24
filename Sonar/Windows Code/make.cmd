@echo off
rem make.cmd
rem Alex Higgins, higginja AT ece DOT pdx DOT edu
rem 02-Mar-2020 13:09:41
rem 
rem An all-in-one CMake script for Win10. Aimed at generating and building 
rem projects for MS Visual Studio 2019 using MSBuild and x64 architecture
rem but can me modified for other scenarios.
rem 
rem e.g. C:\Project1> .\make -help
rem 

setlocal EnableDelayedExpansion

set PROGNAME=RecordData
set BUILDDIR=cmake_build
set INSTALLDIR="F:\sw\RecordData"
set TGT_PLATFORM=Win32
set "WD=%__CD__%"
set /a make_shiftcnt=0

:checkparams
rem Check commandline args
if "x%~1" == "x-clean" shift& set /a make_shiftcnt+=1& set STEPS=-1& goto :Clean
if "x%~1" == "x-all" shift& set /a make_shiftcnt+=1& set STEPS=0& goto :Generate
if "x%~1" == "x-generate" shift& set /a make_shiftcnt+=1& set STEPS=3& goto :Generate
if "x%~1" == "x-build" shift& set /a make_shiftcnt+=1& set STEPS=2& goto :Build
if "x%~1" == "x-install" shift& set /a make_shiftcnt+=1& set STEPS=1& goto :Install
call :printhelp "%~nx0"
exit /b %ERRORLEVEL%

:Clean
if exist "%WD%\%BUILDDIR%" (
  echo Removing build directory "%BUILDDIR%"...
  rmdir /S /Q "%WD%\%BUILDDIR%"
  goto :eof
) else (
  echo No working build directory found.
  goto :eof
)

:Generate
echo Generating build files...
mkdir %BUILDDIR%
cd %BUILDDIR%
cmake -S ../ -G "Visual Studio 16 2019" -A %TGT_PLATFORM% -Wno-dev
if %STEPS% NEQ 3 (
  set /a STEPS-=1
  cd ..
  goto :Build
) else (
  goto :eof
)

:Build
if not exist "%WD%\%BUILDDIR%\CMakeCache.txt" (
  echo Generate step hasn't been run yet.
  echo.
  exit /b %ERRORLEVEL%
) else (
  echo Building project...
  cd %BUILDDIR%
  cmake --build ./ -j 10 -- /p:Configuration=Release /v:n
  if %STEPS% NEQ 2 (
    set /a STEPS-=1
    cd ..
    goto :Install
  ) else (
    goto :eof
  )
)

:Install
if not exist "%WD%\%BUILDDIR%\Release\%PROGNAME%" (
  echo Build step hasn't been run yet.
  echo.
  exit /b %ERRORLEVEL%
) else (
  echo Installing project...
  cd %BUILDDIR%
  cmake --install ./ --prefix %INSTALLDIR%
)
goto :eof

:printhelp
echo Usage:
echo     %~1 [options]
echo.
echo Options:
echo     -all                           Run full cmake toolchain
echo     -clean                         Move remove existing build directory
echo     -generate ^| -build ^| -install  Run specific cmake toolchain step
echo     -help ^| --help ^| /?            Display this help and exit
echo.
echo Any parameter that cannot be treated as a valid option is treated as 
echo the parameter help.
echo.
exit /b 0

:eof
endlocal
echo on
exit /b 0