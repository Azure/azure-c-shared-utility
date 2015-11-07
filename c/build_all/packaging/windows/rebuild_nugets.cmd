@REM Copyright (c) Microsoft. All rights reserved.
@REM Licensed under the MIT license. See LICENSE file in the project root for full license information.

@setlocal EnableExtensions EnableDelayedExpansion
@echo off

rem - Specify the Azure SDK client build root
set current-path=%~dp0
set current-path=%current-path:~0,-1%
set client-root=%current-path%\..\..\..\..
for %%i in ("%client-root%") do set client-root=%%~fi
echo Client root is %client-root%

pushd %client-root%\c\build_all\packaging\windows\

where /q nuget.exe
if not !errorlevel! == 0 (
@Echo Azure Shared Utility needs to download nuget.exe from https://www.nuget.org/nuget.exe 
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

del *.nupkg

rem -- Copy all Win32 files from cmake build directory to the repo directory
xcopy /q /y /R %USERPROFILE%\shared-util\Debug\*.* %client-root%\shared-util\c\win32\debug\*.*
if %errorlevel% neq 0 exit /b %errorlevel%

echo copy 64

rem -- Copy all x64 files from cmake build directory to the repo directory
xcopy /q /y /R %USERPROFILE%\shared-util_x64\Debug\*.* %client-root%\shared-util\c\x64\debug\*.*
if %errorlevel% neq 0 exit /b %errorlevel%

echo Package Nuget

rem -- Package Nuget
nuget pack Microsoft.Azure.C.SharedUtility.nuspec

rmdir %client-root%\shared-util /S /Q

popd
