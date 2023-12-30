@echo off
if "%SymbolServer%"=="" (set SymbolServer=hawkeye:10080)
if "%SymbolDir%"=="" (set SymbolDir=C:\sym)
if not exist "%SymbolDir%" (mkdir %SymbolDir%)
set tsmod=%1
set tsver=%2
set x64=%3
if "%tsmod%"=="" (goto usage)
if "%tsver%"=="" (goto usage)
curl "http://%SymbolServer%/symbols?f=%tsmod%&v=%tsver%&%x64%" -s --compressed -o %SymbolDir%\%tsmod%.pdb  -w %%{http_code}
goto end

:usage
echo getpdb NAME VERSION [x64]
:end