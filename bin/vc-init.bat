"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -nologo -latest -property installationPath > loc.txt
set /p loc=<loc.txt 
del loc.txt
"%loc%\VC\Auxiliary\Build\vcvarsall.bat" %PROCESSOR_ARCHITECTURE%

