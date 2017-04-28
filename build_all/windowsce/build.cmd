@REM Copyright (c) Microsoft. All rights reserved.
@REM Licensed under the MIT license. See LICENSE file in the project root for full license information.

@setlocal EnableExtensions EnableDelayedExpansion
echo off

set current-path=%~dp0
rem // remove trailing slash
set current-path=%current-path:~0,-1%

set build-root=%current-path%\..\..
rem // resolve to fully qualified path
for %%i in ("%build-root%") do set build-root=%%~fi

rem -----------------------------------------------------------------------------
rem -- check prerequisites
rem -----------------------------------------------------------------------------

rem // check that SDK is installed

rem -----------------------------------------------------------------------------
rem -- parse script arguments
rem -----------------------------------------------------------------------------

rem // default build options
set build-clean=0
set build-config=Debug
set build-platform=WindowsCE

rem -----------------------------------------------------------------------------
rem -- build solution with CMake
rem -----------------------------------------------------------------------------

rmdir /s/q "%USERPROFILE%\cmake_su_ce8"
rem no error checking

mkdir "%USERPROFILE%\cmake_su_ce8"
rem no error checking

pushd "%USERPROFILE%\cmake_su_ce8"

rem if you plan to use a different SDK you need to change SDKNAME to the name of your SDK. Ensure that this is also changed in the sample solutions iothub_client_sample_http, simplesample_http and remote_monitoring
set SDKNAME=TORADEX_CE800
set PROCESSOR=arm

cmake -DWINCE=TRUE -DCMAKE_SYSTEM_NAME=WindowsCE -DCMAKE_SYSTEM_VERSION=8.0 -DCMAKE_SYSTEM_PROCESSOR=%PROCESSOR% -DCMAKE_GENERATOR_TOOLSET=CE800 -DCMAKE_GENERATOR_PLATFORM=%SDKNAME% %build-root% -Drun_unittests:bool=OFF -Dskip_samples:BOOL=OFF

if not %errorlevel%==0 exit /b %errorlevel%

rem Currently, only building sample is supported
msbuild "%USERPROFILE%\cmake_su_ce8\samples\iot_c_utility\iot_c_utility.vcxproj
if not %errorlevel%==0 exit /b %errorlevel%
