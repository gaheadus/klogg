# Download all klogg dependencies from GitHub/GitLab

$depsRoot = "D:\ziliao\code\opensource\klogg\build_root\_deps"

# GitHub packages (gh:owner/repo@tag -> source dir)
$githubPackages = @(
    @{name="type_safe"; repo="foonathan/type_safe"; tag="0.2.4"},
    @{name="CRoaring"; repo="RoaringBitmap/CRoaring"; tag="4.2.1"},
    @{name="streamvbyte"; repo="lemire/streamvbyte"; tag="1.0.0"},
    @{name="maddy"; repo="variar/maddy"; tag="602e26613e624535e2de883b3f2c98a16729d1d4"},
    @{name="KF5Archive"; repo="variar/klogg_karchive"; tag="f546bf6ae66a8d34b43da5a41afcfbf4e1a47906"},
    @{name="robin_hood"; repo="martinus/robin-hood-hashing"; tag="3.11.2"},
    @{name="KDSingleApplication"; repo="variar/KDSingleApplication"; tag="5b30db30266f92bc01f1439777803ce8dbf16c79"},
    @{name="xxHash"; repo="Cyan4973/xxHash"; tag="v0.8.1"},
    @{name="whereami"; repo="gpakosz/whereami"; tag="dcb52a058dc14530ba9ae05e4339bd3ddfae0e0e"},
    @{name="exprtk"; repo="variar/klogg_exprtk"; tag="1f9f4cd7d2620b7b24232de9ea22908d63913459"},
    @{name="KDToolBox"; repo="KDAB/KDToolBox"; tag="6468867d1a46eabe1bcb2cd342f338fe66f06675"},
    @{name="efsw"; repo="SpartanJ/efsw"; tag="1.4.1"},
    @{name="tbb"; repo="variar/oneTBB"; tag="c9be1ac2930f02dea523003ed801b4489f3e6b6e"},
    @{name="mimalloc"; repo="microsoft/mimalloc"; tag="v2.1.7"}
)

Write-Host "Downloading klogg dependencies..." -ForegroundColor Cyan

foreach ($pkg in $githubPackages) {
    $srcDir = "$depsRoot\$($pkg.name)-src"
    if (Test-Path "$srcDir\CMakeLists.txt" -PathType Leaf) {
        Write-Host "  [SKIP] $($pkg.name) already exists" -ForegroundColor Yellow
        continue
    }
    
    Write-Host "  Cloning $($pkg.name)..." -ForegroundColor Green
    $url = "https://github.com/$($pkg.repo).git"
    git clone --depth 1 --branch $pkg.tag $url $srcDir 2>&1 | Out-Null
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  [OK] $($pkg.name)" -ForegroundColor Green
    } else {
        Write-Host "  [FAIL] $($pkg.name)" -ForegroundColor Red
    }
}

# GitLab package (Uchardet)
$srcDir = "$depsRoot\Uchardet-src"
if (-not (Test-Path "$srcDir\CMakeLists.txt" -PathType Leaf)) {
    Write-Host "  Cloning Uchardet from GitLab..." -ForegroundColor Green
    git clone --depth 1 --branch 0.0.8 "https://gitlab.freedesktop.org/uchardet/uchardet.git" $srcDir 2>&1 | Out-Null
    if ($LASTEXITCODE -eq 0) {
        Write-Host "  [OK] Uchardet" -ForegroundColor Green
    } else {
        Write-Host "  [FAIL] Uchardet" -ForegroundColor Red
    }
} else {
    Write-Host "  [SKIP] Uchardet already exists" -ForegroundColor Yellow
}

Write-Host "`nDone! All dependencies downloaded." -ForegroundColor Cyan
