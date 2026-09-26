# prebuild_deps.ps1 - PowerShell 版, 不受 cmd 中文化影响
$ErrorActionPreference = 'Continue'

$depsRoot = "D:\ziliao\code\opensource\klogg\build_root\_deps"
if (-not (Test-Path $depsRoot)) {
    New-Item -ItemType Directory -Force -Path $depsRoot | Out-Null
}

# <name>-src, <url>, <tag>
$deps = @(
    @("simdutf-src",        "https://github.com/simdutf/simdutf.git",                       "v5.6.2"),
    @("type_safe-src",      "https://github.com/foonathan/type_safe.git",                    "v0.2.4"),
    @("CRoaring-src",       "https://github.com/RoaringBitmap/CRoaring.git",                 "v4.2.1"),
    @("streamvbyte-src",    "https://github.com/lemire/streamvbyte.git",                     "v1.0.0"),
    @("maddy-src",          "https://github.com/variar/maddy.git",                           "602e26613e624535e2de883b3f2c98a16729d1d4"),
    @("hyperscan-src",      "https://github.com/variar/hyperscan.git",                       "0931a40e0cf1d7f92189bc546c3491ed5c113f8b"),
    @("Uchardet-src",       "https://gitlab.freedesktop.org/uchardet/uchardet.git",         "v0.0.8"),
    @("KF5Archive-src",     "https://github.com/variar/klogg_karchive.git",                 "f546bf6ae66a8d34b43da5a41afcfbf4e1a47906"),
    @("robin_hood-src",     "https://github.com/martinus/robin-hood-hashing.git",            "3.11.2"),
    @("KDSingleApplication-src", "https://github.com/variar/KDSingleApplication.git",        "5b30db30266f92bc01f1439777803ce8dbf16c79"),
    @("xxHash-src",         "https://github.com/Cyan4973/xxHash.git",                        "v0.8.1"),
    @("whereami-src",       "https://github.com/gpakosz/whereami.git",                       "dcb52a058dc14530ba9ae05e4339bd3ddfae0e0e"),
    @("exprtk-src",         "https://github.com/variar/klogg_exprtk.git",                   "1f9f4cd7d2620b7b24232de9ea22908d63913459"),
    @("KDToolBox-src",      "https://github.com/KDAB/KDToolBox.git",                         "6468867d1a46eabe1bcb2cd342f338fe66f06675"),
    @("efsw-src",           "https://github.com/SpartanJ/efsw.git",                          "1.4.1"),
    @("tbb-src",            "https://github.com/variar/oneTBB.git",                          "c9be1ac2930f02dea523003ed801b4489f3e6b6e"),
    @("mimalloc-src",       "https://github.com/microsoft/mimalloc.git",                     "v2.1.7")
)

$done = 0; $skip = 0; $fail = 0
$i = 0

foreach ($d in $deps) {
    $i++
    $name = $d[0]
    $url  = $d[1]
    $tag  = $d[2]
    $full = Join-Path $depsRoot $name

    Write-Host ""
    Write-Host "[$i/$($deps.Count)] $name <- $url @ $tag"

    if (Test-Path (Join-Path $full '.git')) {
        Write-Host "  [SKIP] already cloned" -ForegroundColor Yellow
        $skip++
        continue
    }

    if (Test-Path $full) {
        Remove-Item -Recurse -Force $full
    }

    # 优先尝试 --branch <tag>
    & git clone --depth 1 --branch $tag $url $full 2>&1 | Out-Null
    if ($LASTEXITCODE -ne 0) {
        # 失败则全量克隆 + 切 tag
        Write-Host "  [WARN] --branch $tag failed, trying full clone..." -ForegroundColor Yellow
        & git clone --depth 1 $url $full 2>&1 | Out-Null
        if ($LASTEXITCODE -ne 0) {
            Write-Host "  [FAIL] cannot clone $url" -ForegroundColor Red
            $fail++
            continue
        }
        Push-Location $full
        & git fetch --depth 1 origin $tag 2>&1 | Out-Null
        & git checkout FETCH_HEAD 2>&1 | Out-Null
        Pop-Location
    }

    if (Test-Path (Join-Path $full '.git')) {
        Write-Host "  [OK]   cloned" -ForegroundColor Green
        $done++
    } else {
        Write-Host "  [FAIL] no .git after clone" -ForegroundColor Red
        $fail++
    }
}

Write-Host ""
Write-Host "============================================================"
Write-Host "Summary: $done cloned, $skip skipped, $fail failed, total $($deps.Count)"
Write-Host "============================================================"
