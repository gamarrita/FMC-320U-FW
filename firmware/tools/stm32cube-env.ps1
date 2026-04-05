Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-UserProfilePath {
    if ($env:USERPROFILE) {
        return $env:USERPROFILE
    }

    return [Environment]::GetFolderPath("UserProfile")
}

function Get-BestExtensionDirectory {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Prefix
    )

    $extensionsRoot = Join-Path (Get-UserProfilePath) ".vscode\extensions"
    $escapedPrefix = [Regex]::Escape($Prefix)
    $matches = @(
        Get-ChildItem -Path $extensionsRoot -Directory -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -like "$Prefix*" } |
            ForEach-Object {
            $version = [Version]"0.0.0"
            if ($_.Name -match "^${escapedPrefix}(?<version>\d+(?:\.\d+)+)") {
                try {
                    $version = [Version]$matches["version"]
                }
                catch {
                    $version = [Version]"0.0.0"
                }
            }

            [PSCustomObject]@{
                Directory = $_
                ParsedVersion = $version
            }
            } |
            Sort-Object ParsedVersion, @{ Expression = { $_.Directory.LastWriteTimeUtc }; Descending = $true } -Descending
    )

    if (-not $matches) {
        throw "No se encontro una extension que coincida con '$Prefix*' en '$extensionsRoot'."
    }

    if ($matches.Count -gt 1) {
        Write-Host "Seleccionando extension '$($matches[0].Directory.Name)' entre $($matches.Count) candidatos para '$Prefix*'."
    }

    return $matches[0].Directory.FullName
}

function Get-RequiredFilePath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$BaseDirectory,
        [Parameter(Mandatory = $true)]
        [string]$RelativePath
    )

    $path = Join-Path $BaseDirectory $RelativePath
    if (-not (Test-Path -LiteralPath $path)) {
        throw "No se encontro '$RelativePath' dentro de '$BaseDirectory'."
    }

    return $path
}

function Invoke-CubeText {
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Arguments
    )

    $output = & $script:CubeExe @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Fallo al ejecutar cube $($Arguments -join ' ')."
    }

    return ($output | ForEach-Object { "$_" }) -join "`n"
}

function Get-CubeSettingValue {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Key,
        [Parameter(Mandatory = $true)]
        [string]$EnvironmentVariable
    )

    $line = Invoke-CubeText -Arguments @("--get-current-value", $Key)
    $parts = $line -split "=", 2
    if ($parts.Count -ne 2 -or [string]::IsNullOrWhiteSpace($parts[1])) {
        throw "No se pudo obtener '$Key' desde cube."
    }

    $value = $parts[1].Trim()
    [Environment]::SetEnvironmentVariable($EnvironmentVariable, $value, "Process")
    return $value
}

function Set-EnvironmentFromResolveOutput {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Text
    )

    $resolvedPath = $null

    foreach ($line in ($Text -split "`r?`n")) {
        if ($line -match "^\s*([A-Z0-9_]+)='(.*)'$") {
            $name = $matches[1]
            $value = $matches[2]

            if ($name -eq "PATH") {
                $resolvedPath = $value
                continue
            }

            [Environment]::SetEnvironmentVariable($name, $value, "Process")
        }
    }

    return $resolvedPath
}

function Get-ResolvedCommandDirectory {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Text
    )

    foreach ($line in ($Text -split "`r?`n")) {
        if ($line -match "^Command:\s*'(.*)'$") {
            return Split-Path -Parent $matches[1]
        }
    }

    return $null
}

function Prepend-UniquePathEntries {
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Entries
    )

    $items = New-Object System.Collections.Generic.List[string]
    $seen = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)

    foreach ($entry in $Entries) {
        if ([string]::IsNullOrWhiteSpace($entry)) {
            continue
        }

        foreach ($candidate in ($entry -split ";")) {
            if ([string]::IsNullOrWhiteSpace($candidate)) {
                continue
            }

            $trimmed = $candidate.Trim()
            if ($seen.Add($trimmed)) {
                $items.Add($trimmed)
            }
        }
    }

    [Environment]::SetEnvironmentVariable("PATH", ($items -join ";"), "Process")
}

$coreExtension = Get-BestExtensionDirectory -Prefix "stmicroelectronics.stm32cube-ide-core-"
$buildExtension = Get-BestExtensionDirectory -Prefix "stmicroelectronics.stm32cube-ide-build-cmake-"

$script:CubeExe = Get-RequiredFilePath -BaseDirectory $coreExtension -RelativePath "resources\binaries\win32\x86_64\cube.exe"
$cubeCmakeExe = Get-RequiredFilePath -BaseDirectory $buildExtension -RelativePath "resources\cube-cmake\win32\x86_64\cube-cmake.exe"

$cubeDirectory = Split-Path -Parent $script:CubeExe
$cubeCmakeDirectory = Split-Path -Parent $cubeCmakeExe

[Environment]::SetEnvironmentVariable("CUBE_EXE", $script:CubeExe, "Process")
[Environment]::SetEnvironmentVariable("CUBE_CMAKE_EXE", $cubeCmakeExe, "Process")

$cubeBundlePath = Get-CubeSettingValue -Key "cube_bundle_path" -EnvironmentVariable "CUBE_BUNDLE_PATH"
$cmsisPackRoot = Get-CubeSettingValue -Key "cmsis_pack_root" -EnvironmentVariable "CMSIS_PACK_ROOT"
$cmakeResolveText = Invoke-CubeText -Arguments @("--resolve", "cmake")
$clangdResolveText = Invoke-CubeText -Arguments @("--resolve", "starm-clangd")
$cmakeResolvedPath = Set-EnvironmentFromResolveOutput -Text $cmakeResolveText
$clangdResolvedPath = Set-EnvironmentFromResolveOutput -Text $clangdResolveText
$clangdCommandDirectory = Get-ResolvedCommandDirectory -Text $clangdResolveText

Prepend-UniquePathEntries -Entries @(
    $cubeDirectory,
    $cubeCmakeDirectory,
    $cmakeResolvedPath,
    $clangdResolvedPath,
    $clangdCommandDirectory,
    $env:PATH
)

Write-Host "STM32Cube environment loaded."
Write-Host "  CUBE_EXE=$env:CUBE_EXE"
Write-Host "  CUBE_CMAKE_EXE=$env:CUBE_CMAKE_EXE"
Write-Host "  CUBE_BUNDLE_PATH=$cubeBundlePath"
Write-Host "  CMSIS_PACK_ROOT=$cmsisPackRoot"
if ($env:GCC_TOOLCHAIN_ROOT) {
    Write-Host "  GCC_TOOLCHAIN_ROOT=$env:GCC_TOOLCHAIN_ROOT"
}
