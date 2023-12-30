setlocal

set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe 

cd sdk

set cwd=%CD%



rd /s/q lib
rd /s/q doc
rd /s/q include

cd ..\doc

call dox.bat SDK
cd %cwd%

mkdir include
mkdir lib
mkdir doc

for %%f in (..\doc ..\ChannelOnDemand\doc) do copy %%f\*.chm doc\ && copy %%f\*.pdf doc\ && copy %%f\*readme*.* doc\ 

rem copy /y ..\Ice\*.ICE include\
for %%f in (TianShanDefines.h TianShanDefines.cpp TianShanIce.dsp) do copy /y ..\common\%%f include\
copy /y %ZQPROJSPATH%\common\Exception.h include\
copy /y %ZQPROJSPATH%\common\Guid.h include\
copy /y %ZQPROJSPATH%\common\Log.h include\
copy /y %ZQPROJSPATH%\common\Variant.h include\
copy /y %ZQPROJSPATH%\common\ZQ_common_conf.h include\
copy /y %ZQPROJSPATH%\TianShan\streamsmith\StreamSmithModule.h include\

rem for %%f in (TianShanIce_d.lib TianShanIce.lib TianShanIce_d.dll TianShanIce.dll TianShanIceSrc.jar TianShanIce.jar) do copy /y ..\bin\%%f lib\
for %%f in (TianShanIce_d.lib TianShanIce.lib) do copy /y ..\bin\%%f lib\
copy /y ..\bin\*.jar lib\
copy /y %ICE_ROOT%\lib\java2\*.jar lib\
copy /y %ZQProjsPath%\Common\dll\DebugStlp\*.lib lib\
copy /y %ZQProjsPath%\Common\dll\DebugStlp\*.dll lib\
copy /y %ZQProjsPath%\Common\dll\ReleaseStlp\*.lib lib\
copy /y %ZQProjsPath%\Common\dll\ReleaseStlp\*.dll lib\
copy /y %ZQProjsPath%\Common\dll\ReleaseStlp\*.dll lib\
for %%f in (*.dll *.lib) do copy /y %ICE_ROOT%\bin\%%f lib\
for %%f in (*.dll *.lib) do copy /y %ICE_ROOT%\lib\%%f lib\


%PACKCMD% -r TianShanSDK.zip include lib doc


rem pause
rd /s/q lib
rd /s/q doc
rd /s/q include

cd ..

endlocal
