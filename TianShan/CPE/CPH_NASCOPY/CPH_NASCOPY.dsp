# Microsoft Developer Studio Project File - Name="CPH_NasCopy" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=CPH_NasCopy - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CPH_NASCOPY.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CPH_NASCOPY.mak" CFG="CPH_NasCopy - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CPH_NasCopy - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "CPH_NasCopy - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "$/ZQProjs/TianShan/CPE/CPH_NasCopy"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CPH_NasCopy - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CPH_RTI_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /Od /I "." /I ".." /I "../CPH_NasCopy" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "$(ZQPROJSPATH)/common" /I "$(VSTRMKITPATH)/SDK/inc" /I "../." /I "../../Ice" /I "../../common" /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ITVSDKPATH)\include\\" /I "$(ZQProjsPath)\TianShan\Shell\ZQSNMPManPkg" /I "$(ZQProjsPath)\Generic\RTFLib" /I "..\CPH_RDS" /I "$(WPCAPSDKPATH)/Include" /I "$(RegExppKit)" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_STLP_NEW_PLATFORM_SDK" /D "_USRDLL" /D "CPH_RTI_EXPORTS" /D "USER_VERSION" /D "LOGFMTWITHTID" /D _WIN32_WINNT=0x0500 /D "WPCAP" /D "HAVE_REMOTE" /D "CHECK_WITH_GLOG" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ZQProjsPath)/build" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ice.lib IceUtil.lib /nologo /dll /pdb:"../../bin/CPH_NasCopy.pdb" /debug /machine:I386 /out:"../../bin/CPH_NasCopy.dll" /libpath:"$(ITVSDKPATH)/lib/release" /libpath:"$(ICE_ROOT)/lib" /libpath:"../exe" /libpath:"$(VSTRMKITPATH)/MCPSDK" /libpath:"$(VSTRMKITPATH)/SDK/lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "CPH_NasCopy - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CPH_RTI_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "." /I ".." /I "../CPH_NasCopy" /I "$(ZQPROJSPATH)/common" /I "$(VSTRMKITPATH)/SDK/inc" /I "../." /I "../../Ice" /I "../../common" /I "$(ICE_ROOT)/include/stlport" /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ITVSDKPATH)\include\\" /I "$(ZQProjsPath)\TianShan\Shell\ZQSNMPManPkg" /I "$(ZQProjsPath)\Generic\RTFLib" /I "..\CPH_RDS" /I "$(WPCAPSDKPATH)/Include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CPH_RTI_EXPORTS" /D "USER_VERSION" /D "_STLP_NEW_PLATFORM_SDK" /D "LOGFMTWITHTID" /D "_STLP_DEBUG" /D _WIN32_WINNT=0x0500 /D "WPCAP" /D "HAVE_REMOTE" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ZQProjsPath)/build" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wpcap.lib ws2_32.lib /nologo /dll /pdb:"../../bin/CPH_RTI_d.pdb" /debug /machine:I386 /out:"../../bin/CPH_RTI_d.dll" /pdbtype:sept /libpath:"$(ITVSDKPATH)/lib/release" /libpath:"$(ICE_ROOT)/lib" /libpath:"../exe" /libpath:"$(VSTRMKITPATH)/MCPSDK" /libpath:"$(VSTRMKITPATH)/SDK/lib" /libpath:"$(WPCAPSDKPATH)/Lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "CPH_NasCopy - Win32 Release"
# Name "CPH_NasCopy - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\BaseCPH.cpp
# End Source File
# Begin Source File

SOURCE=..\CPH_RDS\BaseFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\CPH_RDS\BaseGraph.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\ConfigHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\CPE.cpp
# End Source File
# Begin Source File

SOURCE=.\CPH_NasCopy.cpp
# End Source File
# Begin Source File

SOURCE=.\CPH_NasCopy.rc
# End Source File
# Begin Source File

SOURCE=.\CPH_NasCopyCfg.cpp
# End Source File
# Begin Source File

SOURCE=..\CPH_RDS\LibBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\CPH_RDS\LibQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\NasCopySource.cpp
# End Source File
# Begin Source File

SOURCE=..\CPH_RDS\NTFSFileSetSource.cpp
# End Source File
# Begin Source File

SOURCE=..\CPH_RDS\NTFSFsTar.cpp
# End Source File
# Begin Source File

SOURCE=..\CPH_RDS\NTFSSource.cpp
# End Source File
# Begin Source File

SOURCE=..\CPH_RDS\NTFSTarget.cpp
# End Source File
# Begin Source File

SOURCE=..\CPH_RDS\PushSource.cpp
# End Source File
# Begin Source File

SOURCE=..\QueueBufMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SourceFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\CPH_RDS\TargetFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\CPH_RDS\Utils.cpp
# End Source File
# Begin Source File

SOURCE=..\CPH_RDS\VstrmFilesetTarget.cpp
# End Source File
# Begin Source File

SOURCE=..\CPH_RDS\VStrmTarget.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CPH_NasCopy.h
# End Source File
# Begin Source File

SOURCE=.\CPH_NasCopyCfg.h
# End Source File
# Begin Source File

SOURCE=".\NasCopySource .h"
# End Source File
# Begin Source File

SOURCE="..\CPH_RDS\NTFSFileSetSource .h"
# End Source File
# Begin Source File

SOURCE=..\CPH_RDS\NTFSSource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\CPH_RDS\TrickImportUser.h
# End Source File
# Begin Source File

SOURCE=..\CPH_RDS\VStrmTarget.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
