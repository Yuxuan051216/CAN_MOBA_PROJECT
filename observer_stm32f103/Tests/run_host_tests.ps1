$ErrorActionPreference = "Stop"

$gcc = "E:\Dev-Cpp\MinGW64\bin\gcc.exe"
if (-not (Test-Path -LiteralPath $gcc)) {
    throw "Native GCC not found: $gcc"
}

$testDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectDir = Split-Path -Parent $testDir
$buildDir = Join-Path $testDir "build"
New-Item -ItemType Directory -Path $buildDir -Force | Out-Null
$env:PATH = "$(Split-Path -Parent $gcc);$env:PATH"
$env:TEMP = $buildDir
$env:TMP = $buildDir

$exe = Join-Path $buildDir "observer_host_test.exe"
& $gcc `
    -std=c11 -Wall -Wextra -Werror `
    "-I$(Join-Path $projectDir 'Core\Inc')" `
    (Join-Path $testDir "observer_host_test.c") `
    (Join-Path $testDir "observer_test_mocks.c") `
    (Join-Path $projectDir "Core\Src\observer_controller.c") `
    (Join-Path $projectDir "Core\Src\can_moba_protocol.c") `
    -o $exe

if ($LASTEXITCODE -ne 0) {
    throw "Host test build failed"
}

& $exe
if ($LASTEXITCODE -ne 0) {
    throw "Host tests failed"
}
