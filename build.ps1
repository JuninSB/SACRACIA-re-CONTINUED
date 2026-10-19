[CmdletBinding()]
param(
    [ValidateSet('Release', 'Debug')]
    [string] $Configuration = 'Release',

    [ValidateSet('x64', 'Win32', 'x86')]
    [string] $Platform = 'x64',

    [ValidateSet('quiet', 'minimal', 'normal', 'detailed', 'diagnostic')]
    [string] $Verbosity = 'minimal',

    [switch] $Rebuild,
    [switch] $Clean,
    [switch] $NoLog
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$solution    = Join-Path $projectRoot 'RepoHax.sln'

if (-not (Test-Path $solution)) {
    Write-Error "Solution not found: $solution"
    exit 1
}

function Find-MSBuild {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path $vswhere) {
        $path = & $vswhere -latest -prerelease -products * `
            -requires Microsoft.Component.MSBuild `
            -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
        if ($path -and (Test-Path $path)) { return $path }
    }
    $fallback = (Get-Command msbuild.exe -ErrorAction SilentlyContinue).Source
    if ($fallback) { return $fallback }
    return $null
}

$msbuild = Find-MSBuild
if (-not $msbuild) {
    Write-Error 'MSBuild not found. Install Visual Studio 2022 (or Build Tools) with the C++ workload.'
    exit 1
}

$target = if ($Rebuild) { 'Rebuild' } elseif ($Clean) { 'Clean' } else { 'Build' }

$logDir = Join-Path $projectRoot 'build\logs'
if (-not $NoLog) {
    New-Item -ItemType Directory -Force -Path $logDir | Out-Null
    $stamp   = Get-Date -Format 'yyyyMMdd-HHmmss'
    $logFile = Join-Path $logDir "$stamp-$Configuration-$Platform.log"
}

$msbuildArgs = @(
    $solution,
    "/t:$target",
    "/p:Configuration=$Configuration",
    "/p:Platform=$Platform",
    "/v:$Verbosity",
    '/nologo',
    '/m'
)
if (-not $NoLog) {
    $msbuildArgs += "/fl"
    $msbuildArgs += "/flp:LogFile=$logFile;Verbosity=detailed;Encoding=UTF-8"
}

Write-Host "MSBuild      : $msbuild"
Write-Host "Solution     : $solution"
Write-Host "Configuration: $Configuration"
Write-Host "Platform     : $Platform"
Write-Host "Target       : $target"
if (-not $NoLog) { Write-Host "Log file     : $logFile" }
Write-Host ('-' * 60)

$sw = [Diagnostics.Stopwatch]::StartNew()
& $msbuild @msbuildArgs
$exit = $LASTEXITCODE
$sw.Stop()

Write-Host ('-' * 60)
Write-Host ("Elapsed      : {0:N2}s" -f $sw.Elapsed.TotalSeconds)

if ($exit -eq 0) {
    $dll = Join-Path $projectRoot ("build\$Configuration\bin\dwmapi.dll")
    if (Test-Path $dll) {
        $size = (Get-Item $dll).Length / 1KB
        Write-Host ("Output       : {0} ({1:N1} KB)" -f $dll, $size) -ForegroundColor Green
    }
    Write-Host 'Build succeeded.' -ForegroundColor Green
} else {
    Write-Host "Build failed (exit $exit)." -ForegroundColor Red
    if (-not $NoLog) { Write-Host "See log: $logFile" -ForegroundColor Yellow }
}

exit $exit
