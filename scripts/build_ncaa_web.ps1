# Build the verified ULUS10440 executable into a local WebAssembly port.
param(
    [Parameter(Mandatory=$true)][string]$Iso,
    [string]$Emsdk = "$env:USERPROFILE/emsdk",
    [string]$Python = "python",
    [string]$CMake = "cmake",
    [int]$Jobs = 4,
    [switch]$Generate,
    [switch]$Serve,
    [int]$Port = 8621
)
$ErrorActionPreference = 'Stop'
$Root = Split-Path $PSScriptRoot -Parent
function Run([string]$Program, [string[]]$Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Program failed ($LASTEXITCODE)" }
}
Push-Location $Root
try {
    $Iso = (Resolve-Path -LiteralPath $Iso).Path
    $GameRoot = Join-Path $Root 'games/ncaa10/root'
    $Disc = Join-Path $GameRoot 'disc'
    if (-not (Test-Path "$Disc/PSP_GAME/PARAM.SFO")) {
        Run $Python @('scripts/extract_iso.py', $Iso, $Disc)
    }
    Run $Python @('scripts/verify_ncaa10.py', $GameRoot)
    if (-not (Test-Path 'PSPRecomp/CMakeLists.txt')) { throw 'PSPRecomp source is missing; run the repository setup first.' }
    $Profile = Join-Path $Root 'profile'
    if (-not (Test-Path 'PSPRecomp/profiles/web')) {
        New-Item -ItemType Junction -Path 'PSPRecomp/profiles/web' -Target $Profile | Out-Null
    }
    if ($Generate -or -not (Test-Path 'profile/generated/ncaa10/generated_registry.cpp')) {
        Run $CMake @('-S','PSPRecomp','-B','build/tools','-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DPSPRECOMP_PROFILE=','-DPSPRECOMP_BUILD_TESTS=OFF')
        Run $CMake @('--build','build/tools','--target','psp_recomp','-j',"$Jobs")
        Run 'build/tools/psp_recomp.exe' @("$GameRoot/EBOOT.BIN",'--auto','profile/generated/ncaa10','0x08804000','0x4000')
        Run $Python @('scripts/generate_ncaa_sound.py',$Disc,'build/tools/psp_recomp.exe','profile/generated/ncaa10_sound')
    }
    if (-not (Test-Path 'profile/generated/ncaa10_sound/ncaa_sound.cpp')) {
        Run $Python @('scripts/generate_ncaa_sound.py',$Disc,'build/tools/psp_recomp.exe','profile/generated/ncaa10_sound')
    }
    $RawDisc = Join-Path $Disc '__umd.iso'
    if (-not (Test-Path $RawDisc)) { Copy-Item -LiteralPath $Iso -Destination $RawDisc }
    if ((Get-Item -LiteralPath $RawDisc).Length -ne (Get-Item -LiteralPath $Iso).Length) { throw 'The raw streaming disc has a different size from the source ISO.' }
    Run $Python @('scripts/manifest.py',$Disc,"$GameRoot/disc.manifest")
    $Preload = Join-Path $Root 'games/ncaa10/preload'
    New-Item -ItemType Directory -Force -Path "$Preload/ms0" | Out-Null
    Copy-Item -LiteralPath "$GameRoot/EBOOT.BIN" -Destination "$Preload/EBOOT.BIN"
    Copy-Item -LiteralPath "$GameRoot/disc.manifest" -Destination "$Preload/disc.manifest"
    $env:PATH = "$Emsdk;$Emsdk/upstream/emscripten;$env:PATH"
    $env:EMSDK_PYTHON = (Get-Command $Python).Source
    # Debug mode also avoids a hanging temporary-file cleanup in this Windows sandbox.
    $env:EMCC_DEBUG = '1'
    Run $CMake @('-S','PSPRecomp','-B','build/web-ncaa10','-G','Ninja',
        "-DCMAKE_TOOLCHAIN_FILE=$Emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake",
        '-DCMAKE_BUILD_TYPE=Release','-DCMAKE_CXX_FLAGS=-fwasm-exceptions -pthread',
        '-DPSPRECOMP_PROFILE=web','-DPSPWEB_GAME=ncaa10','-DPSPWEB_GUEST_PATH=disc0:/PSP_GAME/SYSDIR/EBOOT.BIN',
        "-DPSPWEB_PRELOAD_DIR=$Preload",'-DPSPRECOMP_BUILD_TESTS=OFF','-DPSPRECOMP_GENERATED_OPT_LEVEL=1','-DPSPWEB_LINK_OPT=0')
    Run $CMake @('--build','build/web-ncaa10','--target','pspweb','-j',"$Jobs")
    Write-Host "Built NCAA Football 10: $Root/build/web-ncaa10/profiles/web/index.html"
    if ($Serve) { Run $Python @('scripts/serve.py','build/web-ncaa10/profiles/web',"$Port",'--mount',"/disc=$Disc") }
} finally { Pop-Location }
