
@setlocal EnableExtensions EnableDelayedExpansion
@echo off

set current-path=%~dp0
rem // remove trailing slash
set current-path=%current-path:~0,-1%

set repo-build-root=%current-path%\..\..
rem // resolve to fully qualified path
for %%i in ("%repo-build-root%") do set repo-build-root=%%~fi

rem -----------------------------------------------------------------------------
rem -- build (clean) compilembed tool
rem -----------------------------------------------------------------------------

call "%repo-build-root%\tools\compilembed\build.cmd" --clean
if not %errorlevel%==0 exit /b %errorlevel%

rem -----------------------------------------------------------------------------
rem -- build iothub client samples
rem -----------------------------------------------------------------------------

call %repo-build-root%\tools\mbed_build_scripts\release_mbed_project.cmd %repo-build-root%\build_all
if not %errorlevel%==0 exit /b %errorlevel%

call :compile azure_c_shared_hello_world %repo-build-root%\samples\azure_c_shared_hello_world\mbed
if not %errorlevel%==0 exit /b %errorlevel%

goto :eof

:compile
setlocal EnableExtensions
set "project_name=%~1"
set "project_path=%~2"
set "download_bin_path=%~3"
set "cmake_project_bin_path=%project_name%_cmake_build"

mkdir %cmake_project_bin_path%
cd %cmake_project_bin_path%
cmake -Dmbed_repo_name:string=%project_name% -Dmbed_output_bin_path:string=%download_bin_path% %project_path%
set CMAKE_ERROR_CODE=%ERRORLEVEL%
cd ..
exit /b %CMAKE_ERROR_CODE%
goto:eof

