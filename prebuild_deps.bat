@echo off
REM ============================================================================
REM  prebuild_deps.bat - Git-clone all klogg 3rdparty deps so CPM skips network.
REM  Source goes to build_root/_deps/<name>-src/  (matches 3rdparty/CMakeLists.txt)
REM ============================================================================
setlocal EnableDelayedExpansion

set "DEPS_ROOT=%~dp0build_root\_deps"
if not exist "%DEPS_ROOT%" mkdir "%DEPS_ROOT%"
cd /d "%DEPS_ROOT%"

REM ---- Define dependencies ----
REM    Format: <dst_dir>|<git_url>|<tag_or_branch>

set "DEPS=^
simdutf-src|https://github.com/simdutf/simdutf.git|v5.6.2^
;;type_safe-src|https://github.com/foonathan/type_safe.git|v0.2.4^
;;CRoaring-src|https://github.com/RoaringBitmap/CRoaring.git|v4.2.1^
;;streamvbyte-src|https://github.com/lemire/streamvbyte.git|v1.0.0^
;;maddy-src|https://github.com/variar/maddy.git|602e26613e624535e2de883b3f2c98a16729d1d4^
;;hyperscan-src|https://github.com/variar/hyperscan.git|0931a40e0cf1d7f92189bc546c3491ed5c113f8b^
;;Uchardet-src|https://gitlab.freedesktop.org/uchardet/uchardet.git|v0.0.8^
;;KF5Archive-src|https://github.com/variar/klogg_karchive.git|f546bf6ae66a8d34b43da5a41afcfbf4e1a47906^
;;robin_hood-src|https://github.com/martinus/robin-hood-hashing.git|3.11.2^
;;KDSingleApplication-src|https://github.com/variar/KDSingleApplication.git|5b30db30266f92bc01f1439777803ce8dbf16c79^
;;xxHash-src|https://github.com/Cyan4973/xxHash.git|v0.8.1^
;;whereami-src|https://github.com/gpakosz/whereami.git|dcb52a058dc14530ba9ae05e4339bd3ddfae0e0e^
;;exprtk-src|https://github.com/variar/klogg_exprtk.git|1f9f4cd7d2620b7b24232de9ea22908d63913459^
;;KDToolBox-src|https://github.com/KDAB/KDToolBox.git|6468867d1a46eabe1bcb2cd342f338fe66f06675^
;;efsw-src|https://github.com/SpartanJ/efsw.git|1.4.1^
;;tbb-src|https://github.com/variar/oneTBB.git|c9be1ac2930f02dea523003ed801b4489f3e6b6e^
;;mimalloc-src|https://github.com/microsoft/mimalloc.git|v2.1.7"

echo [INFO] Cloning dependencies to %DEPS_ROOT%
echo.

set "total=0"
set "done=0"
set "skip=0"
set "fail=0"

for %%L in ("%DEPS:;;=:%") do (
    set /a total+=1
    set "entry=%%L"
    set "entry=!entry:"=!"
    for /f "tokens=1,2,3 delims=|" %%a in ("!entry!") do (
        set "name=%%a"
        set "url=%%b"
        set "tag=%%c"

        echo.
        echo [DEPS !total!] !name!  !url!  @!tag!

        if exist "!name!\.git" (
            echo   [SKIP] already cloned
            set /a skip+=1
        ) else (
            if exist "!name!" rd /s /q "!name!"
            git clone --depth 1 --branch "!tag!" "!url!" "!name!" >nul 2>&1
            if errorlevel 1 (
                echo   [WARN] --branch !tag! failed, trying full clone
                git clone --depth 1 "!url!" "!name!" >nul 2>&1
                if errorlevel 1 (
                    echo   [FAIL] !url!
                    set /a fail+=1
                ) else (
                    pushd "!name!" >nul
                    git fetch --depth 1 origin "!tag!" >nul 2>&1
                    git checkout FETCH_HEAD >nul 2>&1
                    popd >nul
                    echo   [OK]   cloned + checked out !tag!
                    set /a done+=1
                )
            ) else (
                echo   [OK]   cloned at !tag!
                set /a done+=1
            )
        )
    )
)

echo.
echo ============================================================
echo Summary: !done! cloned, !skip! skipped, !fail! failed, total !total!
echo ============================================================
endlocal
