$ErrorActionPreference = "Continue"
$vsPath = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
$vcvars = "$vsPath\VC\Auxiliary\Build\vcvarsall.bat"

# Import VC environment
$tempFile = [IO.Path]::GetTempFileName()
cmd /c "`"$vcvars`" x64 && set > `"$tempFile`""
Get-Content $tempFile | ForEach-Object {
    if ($_ -match "^([^=]+)=(.*)$") {
        [Environment]::SetEnvironmentVariable($matches[1], $matches[2], "Process")
    }
}
Remove-Item $tempFile

Write-Host "=== CONFIGURING ==="
& "C:\Program Files\CMake\bin\cmake.exe" -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release .
if ($LASTEXITCODE -ne 0) { Write-Host "CMAKE CONFIGURE FAILED"; exit 1 }

Write-Host "=== BUILDING ==="
Push-Location build
& nmake
$buildResult = $LASTEXITCODE
Pop-Location
if ($buildResult -ne 0) { Write-Host "BUILD FAILED"; exit 1 }
Write-Host "=== BUILD COMPLETE ==="
