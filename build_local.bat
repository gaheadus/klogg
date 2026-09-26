@echo off
REM ============================================================================
REM  klogg local build script (Windows / MSVC / Qt 6.7.3)
REM
REM  Output: %~dp0build_root\output\bin\klogg.exe
REM ============================================================================

setlocal EnableDelayedExpansion
chcp 65001 > nul

REM ---- 1. Init VS dev env (prefer 2022, fallback 2019) --------------------

if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" -arch=x64 >nul 2>nul
    goto :VS_OK
)

if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 >nul 2>nul
    goto :VS_OK
)

if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 >nul 2>nul
    goto :VS_OK
)

if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 >nul 2>nul
    goto :VS_OK
)

echo [ERROR] VS 2019/2022 not found.
exit /b 1

:VS_OK
where.exe cl >nul 2>&1
if errorlevel 1 (
    echo [ERROR] MSVC cl not in PATH.
    exit /b 1
)
echo [OK] MSVC cl.exe ready

REM ---- 2. Check Qt 6.7.3 ---------------------------------------------------
if exist "C:\Qt\6.7.3\msvc2022_64\lib\cmake\Qt6\Qt6Config.cmake" (
    set "QT_DIR=C:\Qt\6.7.3\msvc2022_64"
) else if exist "C:\Qt\6.7.3\msvc2019_64\lib\cmake\Qt6\Qt6Config.cmake" (
    set "QT_DIR=C:\Qt\6.7.3\msvc2019_64"
) else (
    echo [ERROR] Qt 6.7.3 not found at C:\Qt\6.7.3
    exit /b 1
)
echo [OK] Qt found: %QT_DIR%

REM ---- 3. Check CMake ------------------------------------------------------
where.exe cmake >nul 2>&1
if errorlevel 1 (
    if exist "C:\Qt\Tools\CMake_64\bin\cmake.exe" (
        set "PATH=C:\Qt\Tools\CMake_64\bin;%PATH%"
    ) else if exist "C:\Qt\Tools\CMake\bin\cmake.exe" (
        set "PATH=C:\Qt\Tools\CMake\bin;%PATH%"
    ) else (
        echo [ERROR] cmake not found.
        exit /b 1
    )
)
echo [OK] CMake ready

REM ---- 4. Check Boost ------------------------------------------------------
if not defined BOOST_ROOT (
    set "BOOST_ROOT=C:\Boost\boost_1_86_0"
)
if not exist "%BOOST_ROOT%\boost\version.hpp" (
    echo [WARN] Boost not found at %BOOST_ROOT%, Hyperscan=OFF
    set "HYPERSCAN_OPT=OFF"
) else (
    echo [OK] Boost found: %BOOST_ROOT%
    set "HYPERSCAN_OPT=ON"
)

REM ---- 5. Configure PATH for build ----------------------------------------
set "PATH=%QT_DIR%\bin;%PATH%"
set "CMAKE_PREFIX_PATH=%QT_DIR%"
where.exe perl >nul 2>&1
if errorlevel 1 (
    echo [INFO] perl not detected, Hyperscan may fail
) else (
    echo [OK] perl ready
)

REM ---- 6. Generate + Build -------------------------------------------------
cd /d "%~dp0"
if not exist build_root mkdir build_root
cd build_root

echo.
echo [INFO] Configuring klogg (Hyperscan=%HYPERSCAN_OPT%)...
cmake -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_BUILD_TYPE=RelWithDebInfo ^
      -DBUILD_TESTS:BOOL=OFF ^
      -DKLOGG_USE_HYPERSCAN=%HYPERSCAN_OPT% ^
      -DKLOGG_USE_VECTORSCAN=OFF ^
      "%~dp0"
if errorlevel 1 (
    echo.
    echo [ERROR] CMake configure failed.
    exit /b 1
)

echo.
echo [INFO] Building klogg (first run: 15-30 min)...
cmake --build . --config RelWithDebInfo -- /m
if errorlevel 1 (
    echo.
    echo [ERROR] Build failed.
    exit /b 1
)

echo.
echo ============================================================
echo BUILD OK
echo Binary: %~dp0build_root\output\bin\klogg.exe
echo ============================================================
endlocal
