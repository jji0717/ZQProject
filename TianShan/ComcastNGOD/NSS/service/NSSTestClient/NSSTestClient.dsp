# Microsoft Developer Studio Project File - Name="NSSTestClient" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=NSSTestClient - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NSSTestClient.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NSSTestClient.mak" CFG="NSSTestClient - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NSSTestClient - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "NSSTestClient - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NSSTestClient - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "NSSTestClient - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../../../Ice" /I "$(ZQProjsPath)/TianShan/Shell/ZQSNMPManPkg" /I "$(STLPORT_ROOT)" /I "$(EXPATPATH)/include" /I "$(ZQPROJSPATH)\tianshan\shell\zqsnmpmanpkg" /I "$(RegExppKit)" /I "$(ZQProjsPath)\TianShan\Weiwoo\service" /I "$(ZQProjsPath)\TianShan\Shell\ZQSNMPManPkg" /I "$(ZQProjsPath)\TianShan\AccreditedPath" /D "_DEBUG" /D _WIN32_WINNT=0x0500 /D "_STLP_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "WITH_ICESTORM" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:console /debug /machine:I386 /out:"../../../../bin/NSSTestClient.exe" /pdbtype:sept /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ITVSDKPATH)/lib/debug"

!ENDIF 

# Begin Target

# Name "NSSTestClient - Win32 Release"
# Name "NSSTestClient - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\..\Common\ConfigLoader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Common\getopt.cpp
# End Source File
# Begin Source File

SOURCE=..\..\NSS.cpp
# End Source File
# Begin Source File

SOURCE=.\NSSTestClient.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\PathCommand.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\PathFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\PathHelperMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\PathManagerImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\PathSvcEnv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\ServiceGroupDict.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\ServiceGroupToStreamLink.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\StorageDict.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\StorageLinkToTicket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\StorageToStorageLink.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\StreamerDict.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\StreamerToStorageLink.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\StreamerToStreamLink.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\StreamLinkToTicket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\TsPathAdmin.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\..\common\ConfigLoader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Common\getopt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\pho\IpEdgePHO.h
# End Source File
# Begin Source File

SOURCE=..\..\NSS.h
# End Source File
# Begin Source File

SOURCE=.\NSSClient.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\PathCommand.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\PathFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\PathHelperMgr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\PathManagerImpl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\PathSvcEnv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\ServiceGroupDict.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\ServiceGroupToStreamLink.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\StorageDict.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\StorageLinkToTicket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\StorageToStorageLink.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\StreamerDict.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\StreamerToStorageLink.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\StreamerToStreamLink.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\StreamLinkToTicket.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\AccreditedPath\TsPathAdmin.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Weiwoo\service\WeiwooConfig.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\NSS.ICE

!IF  "$(CFG)" == "NSSTestClient - Win32 Release"

!ELSEIF  "$(CFG)" == "NSSTestClient - Win32 Debug"

# Begin Custom Build
InputDir=\build\zqprojs\TianShan\ComcastNGOD\NSS
InputPath=..\..\NSS.ICE
InputName=NSS

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/ICE --output-dir .. $(InputDir)\$(InputName).ice

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
