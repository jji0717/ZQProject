rem set ZQPROJSPATH= D:\work\project\ZQProjs
set PATH=%PATH%;%ANT_HOME%\bin
rem "C:\Program Files\Microsoft Visual Studio\Common\MSDev98\Bin\MSDEV.EXE" %ZQPROJSPATH%/tianshan/ice/TianShanIce.dsw /MAKE "TianShanIce - Win32 Java" /REBUILD
rem antbuild %ZQPROJSPATH%/tianshan/ice %ZQPROJSPATH%/tianshan/ice/ant.xml
%ICE_ROOT%\bin\slice2java.exe -I%ICE_ROOT%\slice -I%ZQPROJSPATH%\tianshan\ice ..\Service\StreamSmithAdmin.ice --output-dir .\java
cd %ZQPROJSPATH%\tianshan\streamsmith\streamsmithsdk
xcopy /S /Y  %ZQPROJSPATH%\tianshan\ice\java .\java\
antbuild %ZQPROJSPATH%\tianshan\streamsmith\streamsmithsdk %ZQPROJSPATH%\tianshan\streamsmith\streamsmithsdk\ant.xml
copy %ICE_ROOT%\java2\ice.jar .\bin\
