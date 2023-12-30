# Microsoft Developer Studio Project File - Name="ServerLoad" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=ServerLoad - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ServerLoad.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ServerLoad.mak" CFG="ServerLoad - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ServerLoad - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ServerLoad - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ServerLoad - Win32 Release"

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
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /ZI /I "." /I "$(ZQPROJSPATH)/tianshan/common" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "$(ZQPROJSPATH)/common" /I "$(EXPATPATH)\include" /I "$(ZQPROJSPATH)/tianshan/include" /I "$(ZQProjsPath)/TianShan/Shell/ZQCfgPkg" /I "$(ZQProjsPath)/TianShan/Shell/ZQSNMPManPkg" /I "$(RegExppKit)" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_STLP_NEW_PLATFORM_SDK" /D "_AFXDLL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 /nologo /subsystem:console /incremental:yes /debug /machine:I386 /libpath:"$(EXPATPATH)/lib" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ZQPROJSPATH)\tianshan\lib" /libpath:"$(ITVSDKPATH)/lib/release" /libpath:"$(RegExppKit)/bin"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "ServerLoad - Win32 Debug"

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
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /I "." /I "$(ZQPROJSPATH)/tianshan/common" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "$(ZQPROJSPATH)/common" /I "$(EXPATPATH)\include" /I "$(ZQPROJSPATH)/tianshan/include" /I "$(ZQProjsPath)/TianShan/Shell/ZQCfgPkg" /I "$(ZQProjsPath)/TianShan/Shell/ZQSNMPManPkg" /I "$(RegExppKit)" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_STLP_DEBUG" /D "_STLP_NEW_PLATFORM_SDK" /D "_AFXDLL" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:console /pdb:"../../tianshan/bin/ServerLoad_d.pdb" /debug /machine:I386 /out:"../../tianshan/bin/ServerLoad_d.exe" /pdbtype:sept /libpath:"$(EXPATPATH)/lib" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ZQPROJSPATH)\tianshan\lib" /libpath:"$(ITVSDKPATH)/lib/release" /libpath:"$(RegExppKit)/bin"
# SUBTRACT LINK32 /pdb:none /incremental:no
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy E:\ZQProjs1.7\Common\dll\DebugStlp\ZQCommonStlp_d.dll ./ZQCommonStlp_d.dll	copy E:\ZQProjs1.7\Common\dll\DebugStlp\ZQCommonStlp_d.pdb ./ZQCommonStlp_d.pdb
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "ServerLoad - Win32 Release"
# Name "ServerLoad - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\Common\BaseZQServiceApplication.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\ConfigHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\db_datas.cpp
# End Source File
# Begin Source File

SOURCE=.\ParseXMLData.cpp
# End Source File
# Begin Source File

SOURCE=.\SrvrLoadCfg.cpp
# End Source File
# Begin Source File

SOURCE=.\SrvrLoadEnv.cpp
# End Source File
# Begin Source File

SOURCE=.\SrvrLoadSvc.cpp
# End Source File
# Begin Source File

SOURCE=.\stroprt.cpp
# End Source File
# Begin Source File

SOURCE=.\WatchDog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\ZQServiceAppMain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Common\ConfigHelper.h
# End Source File
# Begin Source File

SOURCE=.\db_datas.h
# End Source File
# Begin Source File

SOURCE=.\ParseXMLData.h
# End Source File
# Begin Source File

SOURCE=.\SrvrLoadCfg.h
# End Source File
# Begin Source File

SOURCE=.\SrvrLoadEnv.h
# End Source File
# Begin Source File

SOURCE=.\SrvrLoadSvc.h
# End Source File
# Begin Source File

SOURCE=.\stroprt.h
# End Source File
# Begin Source File

SOURCE=.\WatchDog.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\ServerLoadRS.rc
# End Source File
# End Group
# End Target
# End Project
