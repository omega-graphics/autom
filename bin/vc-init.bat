vswhere -nologo -latest -property installationPath > loc.txt 
set /p loc=<loc.txt 
del loc.txt
"%loc%\VC\Auxiliary\Build\vcvarsall.bat" %PROCESSOR_ARCHITECTURE%

