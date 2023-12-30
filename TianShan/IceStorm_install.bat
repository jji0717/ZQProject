setlocal
set InstallPath=C:\Ice-3.2.1-VC80
set TargetPath=%InstallPath%\bin
copy config.icebox %TargetPath% /Y
copy config.service %TargetPath% /Y
if not exist %TargetPath%\data md %TargetPath%\data
%ICEROOT%\bin\icebox.exe --install TianShanIceStorm --Ice.Config=%TargetPath%\config.icebox
endlocal