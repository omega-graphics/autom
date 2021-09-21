set LOC="C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe" -latest -property installationPath

for /f "usebackq tokens=*" %%a in (`%LOC%`) do %%a/VC/Auxiliary/Build/vcvarsall.bat %PROCESSOR_ARCHITECTURE%
