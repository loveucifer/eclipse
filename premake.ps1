$ErrorActionPreference = "Stop"

$premake = if ($env:PREMAKE) { $env:PREMAKE } else { "premake5" }
$arguments = @("--file=$PSScriptRoot/premake5.lua") + $args

if ($arguments[-1] -eq "gen") {
    $arguments[-1] = "vs2022"
}

& $premake @arguments
exit $LASTEXITCODE
