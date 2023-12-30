md ..\RTFLib_SDK
md ..\RTFLib_SDK\exe
md ..\RTFLib_SDK\exe\Win32
md ..\RTFLib_SDK\exe\x64
md ..\RTFLib_SDK\Include
md ..\RTFLib_SDK\lib
md ..\RTFLib_SDK\lib\Win32
md ..\RTFLib_SDK\lib\x64
xcopy /fy Release\RTFLib.dll ..\RTFLib_SDK\exe\x64
xcopy /fy Release\RTFLib.pdb ..\RTFLib_SDK\exe\x64
xcopy /fy Release\RTFLib.lib ..\RTFLib_SDK\lib\x64
xcopy /fy RTFLib.h ..\RTFLib_SDK\include

