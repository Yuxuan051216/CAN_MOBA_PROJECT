param(
    [string]$MounRiverExe = "E:\MounRiver_Studio\eclipsec.exe"
)

$ErrorActionPreference = "Stop"

$projectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$sourceVariantHeader = Join-Path $projectDir "App\build_variant.h"
$outputDir = Join-Path $projectDir "obj"
$projectName = "D_CAN_MOBA"
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
$tempRoot = Join-Path ([System.IO.Path]::GetTempPath()) (
    "CAN_MOBA_BUILD_" + [System.Guid]::NewGuid().ToString("N")
)

if(-not (Test-Path -LiteralPath $MounRiverExe))
{
    throw "MounRiver headless builder not found: $MounRiverExe"
}

$mounRiverDir = Split-Path -Parent $MounRiverExe
$gccBin = Join-Path $mounRiverDir "toolchain\RISC-V Embedded GCC\bin"
$buildToolsBin = Join-Path $mounRiverDir "toolchain\Build Tools\bin"
$gccExe = Join-Path $gccBin "riscv-none-embed-gcc.exe"
$makeExe = Join-Path $buildToolsBin "make.exe"

if(-not (Test-Path -LiteralPath $gccExe))
{
    throw "RISC-V compiler not found: $gccExe"
}
if(-not (Test-Path -LiteralPath $makeExe))
{
    throw "MounRiver make not found: $makeExe"
}

# Headless Eclipse does not inherit MounRiver's GUI launcher toolchain PATH.
$env:PATH = "$gccBin;$buildToolsBin;$env:PATH"

$sourceHeaderText = [System.IO.File]::ReadAllText($sourceVariantHeader)
if($sourceHeaderText -notmatch "CAN_MOBA_BUILD_NODE_ID\s+1U")
{
    throw "Source build_variant.h must remain NODE_ID=1 before dual build"
}

function New-VariantProject([string]$name, [int]$nodeId)
{
    $destination = Join-Path $tempRoot $name
    New-Item -ItemType Directory -Path $destination -Force | Out-Null

    Get-ChildItem -LiteralPath $projectDir -Force |
        Where-Object { $_.Name -ne "obj" } |
        ForEach-Object {
            Copy-Item -LiteralPath $_.FullName `
                      -Destination $destination `
                      -Recurse `
                      -Force
        }

    $variantHeader = Join-Path $destination "App\build_variant.h"
    $content = [System.IO.File]::ReadAllText($variantHeader)
    $updated = [System.Text.RegularExpressions.Regex]::Replace(
        $content,
        "#define\s+CAN_MOBA_BUILD_NODE_ID\s+[12]U",
        "#define CAN_MOBA_BUILD_NODE_ID  ${nodeId}U"
    )
    [System.IO.File]::WriteAllText($variantHeader, $updated, $utf8NoBom)
    return $destination
}

function Invoke-VariantBuild(
    [string]$variantName,
    [int]$nodeId,
    [string]$suffix
)
{
    $variantProject = New-VariantProject $variantName $nodeId
    $workspace = Join-Path $tempRoot ("workspace_" + $suffix)

    Write-Host (
        "Building CAN MOBA Board {0} (NODE_ID={1}, CAN1 PB8/PB9)..." -f
        $suffix, $nodeId
    )

    & $MounRiverExe `
        -nosplash `
        -application org.eclipse.cdt.managedbuilder.core.headlessbuild `
        -data $workspace `
        -import $variantProject `
        -cleanBuild "$projectName/obj"

    if($LASTEXITCODE -ne 0)
    {
        throw "MounRiver build failed with exit code $LASTEXITCODE"
    }

    $variantObj = Join-Path $variantProject "obj"
    $hexPath = Join-Path $variantObj "D_CAN_MOBA.hex"
    if(-not (Test-Path -LiteralPath $hexPath))
    {
        throw "Build completed without output: $hexPath"
    }

    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
    foreach($extension in "hex", "elf", "lst", "map", "siz")
    {
        $source = Join-Path $variantObj "D_CAN_MOBA.$extension"
        if(Test-Path -LiteralPath $source)
        {
            $destination = Join-Path $outputDir "D_CAN_MOBA_$suffix.$extension"
            Copy-Item -LiteralPath $source -Destination $destination -Force

            if($suffix -eq "A")
            {
                $generic = Join-Path $outputDir "D_CAN_MOBA.$extension"
                Copy-Item -LiteralPath $source -Destination $generic -Force
            }
        }
    }
}

try
{
    Invoke-VariantBuild "firmware_A" 1 "A"
    Invoke-VariantBuild "firmware_B" 2 "B"

    $aHex = Join-Path $outputDir "D_CAN_MOBA_A.hex"
    $bHex = Join-Path $outputDir "D_CAN_MOBA_B.hex"
    $aHash = (Get-FileHash -LiteralPath $aHex -Algorithm SHA256).Hash
    $bHash = (Get-FileHash -LiteralPath $bHex -Algorithm SHA256).Hash

    if($aHash -eq $bHash)
    {
        throw "Board A and Board B HEX hashes are identical; NODE_ID build failed"
    }

    $sourceHeaderAfterBuild = [System.IO.File]::ReadAllText($sourceVariantHeader)
    if($sourceHeaderAfterBuild -notmatch "CAN_MOBA_BUILD_NODE_ID\s+1U")
    {
        throw "Source build_variant.h was unexpectedly modified"
    }

    Write-Host ""
    Write-Host "Build complete:"
    Write-Host "  A: $aHex"
    Write-Host "     SHA256=$aHash"
    Write-Host "  B: $bHex"
    Write-Host "     SHA256=$bHash"
}
finally
{
    $resolvedTemp = [System.IO.Path]::GetFullPath($tempRoot)
    $systemTemp = [System.IO.Path]::GetFullPath(
        [System.IO.Path]::GetTempPath()
    )
    if((Test-Path -LiteralPath $resolvedTemp) -and
       $resolvedTemp.StartsWith(
           $systemTemp,
           [System.StringComparison]::OrdinalIgnoreCase
       ))
    {
        Remove-Item -LiteralPath $resolvedTemp -Recurse -Force
    }
}
