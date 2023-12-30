# Microsoft Developer Studio Project File - Name="ManualSyncAddin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ManualSyncAddin - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ManualSyncAddin.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ManualSyncAddin.mak" CFG="ManualSyncAddin - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ManualSyncAddin - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ManualSyncAddin - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/Generic/DBSync", GFIAAAAA"
# PROP Scc_LocalPath "..\..\..\dbsync"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ManualSyncAddin - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ManualSyncAddin_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Od /I "$(ZQPROJSPATH)\generic\JMSCppLib" /I "$(ZQProjsPath)\common" /I "$(ITVSDKPATH)\include" /I "$(ZQPROJSPATH)\generic\DBSync" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ManualSyncAddin_EXPORTS" /D "_AFXDLL" /D "_WINDLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ZQProjsPath)\BUILD" /i "$(ITVSDKPATH)" /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 mfcs42.lib jmsc.lib jmscpp.lib ws2_32.lib cfgpkgU.lib /nologo /dll /machine:I386 /nodefaultlib:"Nafxcw.lib libcmt.lib" /out:"Release/ManualSyncPlugin.dll" /libpath:"$(ZQPROJSPATH)\generic\JMSCppLib\JMSCpp\lib" /libpath:"$(ITVSDKPATH)\Lib\Release" /verbose:lib
# SUBTRACT LINK32 /pdb:none /incremental:yes /debug

!ELSEIF  "$(CFG)" == "ManualSyncAddin - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ManualSyncAddin_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(ZQPROJSPATH)\generic\JMSCppLib" /I "$(ZQProjsPath)\common" /I "$(ITVSDKPATH)\include" /I "$(ZQPROJSPATH)\generic\DBSync" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "_MBCS" /D "ManualSyncAddin_EXPORTS" /D "_AFXDLL" /D "_WINDLL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ZQProjsPath)\BUILD" /i "$(ITVSDKPATH)" /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 mfcs42d.lib jmsc.lib cfgpkgU_d.lib jmscpp_d.lib ws2_32.lib /nologo /dll /debug /machine:I386 /nodefaultlib:"Nafxcwd.lib" /nodefaultlib:"libcmtd.lib" /out:"Debug/ManualSyncPlugin_d.dll" /pdbtype:sept /libpath:"$(ZQPROJSPATH)\generic\JMSCppLib\JMSCpp\lib" /libpath:"$(ITVSDKPATH)\Lib\Debug" /verbose:lib
# SUBTRACT LINK32 /pdb:none /incremental:no
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Debug\*.dll  ..\ManualSyncAddin_Test\Debug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "ManualSyncAddin - Win32 Release"
# Name "ManualSyncAddin - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\Common\Exception.cpp
# End Source File
# Begin Source File

SOURCE=.\JmsProcThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Common\Log.cpp
# End Source File
# Begin Source File

SOURCE=.\ManualSyncAddin.cpp
# End Source File
# Begin Source File

SOURCE=.\ManualSyncAddin.def
# End Source File
# Begin Source File

SOURCE=.\ManualSyncAddin.rc
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Common\NativeThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Common\Semaphore.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Common\XMLPreference.cpp
# End Source File
# Begin Source File

SOURCE=.\XMLProc.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\Common\Exception.h
# End Source File
# Begin Source File

SOURCE=.\JmsProcThread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Common\Locks.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Common\Log.h
# End Source File
# Begin Source File

SOURCE=..\..\..\DBSync\ManualSyncDef.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Common\NativeThread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Common\XMLPreference.h
# End Source File
# Begin Source File

SOURCE=.\XMLProc.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
