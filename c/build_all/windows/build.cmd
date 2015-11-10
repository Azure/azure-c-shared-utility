@REM Copyright (c) Microsoft. All rights reserved.
@REM Licensed under the MIT license. See LICENSE file in the project root for full license information.

@setlocal EnableExtensions EnableDelayedExpansion
@echo off

set CMAKE_DIR=shared-util_Win32

set current-path=%~dp0
rem // remove trailing slash
set current-path=%current-path:~0,-1%

set build-root=%current-path%\..\..
rem // resolve to fully qualified path
for %%i in ("%build-root%") do set build-root=%%~fi

rem ensure nuget.exe exists
where /q nuget.exe
if not !errorlevel! == 0 (
@Echo Azure IoT SDK needs to download nuget.exe from https://www.nuget.org/nuget.exe 
@Echo https://www.nuget.org 
choice /C yn /M "Do you want to download and run nuget.exe?" 
if not !errorlevel!==1 goto :eof
rem if nuget.exe is not found, then ask user
Powershell.exe wget -outf nuget.exe https://nuget.org/nuget.exe
	if not exist .\nuget.exe (
		echo nuget does not exist
		exit /b 1
	)
)
rem -----------------------------------------------------------------------------
rem -- parse script arguments
rem -----------------------------------------------------------------------------

rem // default build options
set build-clean=0
set build-config=
set build-platform=Win32

:args-loop
if "%1" equ "" goto args-done
if "%1" equ "-c" goto arg-build-clean
if "%1" equ "--clean" goto arg-build-clean
if "%1" equ "--config" goto arg-build-config
if "%1" equ "--platform" goto arg-build-platform
call :usage && exit /b 1

:arg-build-clean
set build-clean=1
goto args-continue

:arg-build-config
shift
if "%1" equ "" call :usage && exit /b 1
set build-config=%1
goto args-continue

:arg-build-platform
shift
if "%1" equ "" call :usage && exit /b 1
set build-platform=%1
if %build-platform% == x64 (
	set CMAKE_DIR=shared-util_x64
)
goto args-continue

:args-continue
shift
goto args-loop

:args-done

rem -----------------------------------------------------------------------------
rem -- build with CMAKE
rem -----------------------------------------------------------------------------

echo CMAKE Output Path: %USERPROFILE%\%CMAKE_DIR%

if NOT EXIST %USERPROFILE%\%CMAKE_DIR% GOTO NO_CMAKE_DIR
rmdir /s/q %USERPROFILE%\%CMAKE_DIR%
rem no error checking
:NO_CMAKE_DIR

mkdir %USERPROFILE%\%CMAKE_DIR%
rem no error checking

pushd %USERPROFILE%\%CMAKE_DIR%

if %build-platform% == Win32 (
	echo ***Running CMAKE for Win32***
	cmake %build-root%
	if not %errorlevel%==0 exit /b %errorlevel%
) else (
	echo ***Running CMAKE for Win64***
	cmake %build-root% -G "Visual Studio 14 Win64"
	if not %errorlevel%==0 exit /b %errorlevel%	
)

if not defined build-config (
	echo ***Building both configurations***
	msbuild /m azure_c_shared_utility.sln /p:Configuration=Release
	msbuild /m azure_c_shared_utility.sln /p:Configuration=Debug
	if not %errorlevel%==0 exit /b %errorlevel%
) else (
	echo ***Building %build-config% only***
	msbuild /m azure_c_shared_utility.sln /p:Configuration=%build-config%
	if not %errorlevel%==0 exit /b %errorlevel%
)

ctest -C "debug" -V
if not %errorlevel%==0 exit /b %errorlevel%

popd
goto :eof

:usage
echo build.cmd [options]
echo options:
echo  --config ^<value^>      [Debug] build configuration (e.g. Debug, Release)
echo  --platform ^<value^>    [Win32] build platform (e.g. Win32, x64, ...)
goto :eof
