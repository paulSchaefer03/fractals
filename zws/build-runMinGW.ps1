# build‑run.ps1
$ErrorActionPreference = "Stop"        # Script bricht bei Fehler ab

cmake --build build --config Release
cmake --install build --config Release

Move-Item -Force install\MandelbrotExplorerQt.exe install\bin\

Write-Host "Starte MandelbrotExplorerQt ..."
& install\bin\MandelbrotExplorerQt.exe
