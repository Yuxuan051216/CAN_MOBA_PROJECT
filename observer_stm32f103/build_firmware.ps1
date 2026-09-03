$ErrorActionPreference = "Stop"

$make = "E:\MounRiver_Studio\toolchain\Build Tools\bin\make.exe"
$toolchain = "E:/MounRiver_Studio/toolchain/arm-none-eabi-gcc/bin"

if (-not (Test-Path -LiteralPath $make)) {
    throw "GNU Make not found: $make"
}
if (-not (Test-Path -LiteralPath "$toolchain/arm-none-eabi-gcc.exe")) {
    throw "GNU Arm compiler not found: $toolchain"
}

$projectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Push-Location $projectDir
try {
    & $make -j4 "TOOLCHAIN=$toolchain"
    if ($LASTEXITCODE -ne 0) {
        throw "Firmware build failed"
    }
}
finally {
    Pop-Location
}

Write-Host "ELF: $projectDir\build\observer_stm32f103.elf"
Write-Host "HEX: $projectDir\build\observer_stm32f103.hex"
