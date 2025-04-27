$exePath = "C:/Users/pauls/OneDrive/Dokumente/Hochschule Fulda/Master/Klinikum_Fulda/C++_Plus/fractals/build/Release/MandelbrotExplorerQt.exe"
$windeployqtPath = "C:/Qt/6.9.0/msvc2022_64/bin/windeployqt.exe"

# Windeploy ausführen
& $windeployqtPath $exePath

# Exe starten
Start-Process $exePath

# run the script in the background
#Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
#.\deploy_and_run.ps1
