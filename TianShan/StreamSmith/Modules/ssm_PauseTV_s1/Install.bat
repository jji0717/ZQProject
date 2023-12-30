setlocal
set TARGETDIR=C:\TianShan
set EXEDIR=%TARGETDIR%\bin
set CONFIGDIR=%TARGETDIR%\etc

if not exist %EXEDIR% md %EXEDIR%
if not exist %CONFIGDIR% md %CONFIGDIR%

xcopy /fys ssm_PauseTV_s1.dll %EXEDIR%
xcopy /fys ssm_PauseTV_s1.xml %CONFIGDIR%

regsetup "SOFTWARE\Seachange\TianShan\CurrentVersion\Services\RtspProxy" "pluginConfigFilePath" S "%CONFIGDIR%\ssm_PauseTV_s1.xml"

endlocal