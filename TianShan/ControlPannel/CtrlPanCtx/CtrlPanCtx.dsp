# Microsoft Developer Studio Project File - Name="CtrlPanCtx" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=CtrlPanCtx - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CtrlPanCtx.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CtrlPanCtx.mak" CFG="CtrlPanCtx - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CtrlPanCtx - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "CtrlPanCtx - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CtrlPanCtx - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(ZQProjsPath)\Generic\ColorBarContainer\Source\Include" /I "." /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/TianShan/common" /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "$(ZQProjsPath)/TianShan/ControlPannel/ZQSNMPOperPkg" /I "$(ZQProjsPath)/TianShan/ControlPannel/TsCtrlClient" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D _WIN32_WINNT=0x500 /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "ZQCOMMON_DLL" /D "WITH_ICESTORM" /D "_STLP_DEBUG" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)/build" /i "$(ITVSDKPATH)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Freeze.lib Ice.lib IceUtil.lib IceStorm.lib IceGrid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../bin/CtrlPanCtx_d.dll" /pdbtype:sept /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ZQPROJSPATH)/TianShan/bin" /libpath:"$(ZQPROJSPATH)/TianShan/lib"
# Begin Custom Build - Performing registration
OutDir=.\Debug
TargetPath=\ZQProjs\TianShan\bin\CtrlPanCtx_d.dll
InputPath=\ZQProjs\TianShan\bin\CtrlPanCtx_d.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "CtrlPanCtx - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "CtrlPanCtx_Release"
# PROP BASE Intermediate_Dir "CtrlPanCtx_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /D "_ATL_MIN_CRT" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /ZI /Od /I "$(ZQProjsPath)\Generic\ColorBarContainer\Source\Include" /I "." /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/TianShan/common" /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "$(ZQProjsPath)/TianShan/ControlPannel/ZQSNMPOperPkg" /I "$(ZQProjsPath)/TianShan/ControlPannel/TsCtrlClient" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D _WIN32_WINNT=0x500 /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "ZQCOMMON_DLL" /D "_ATL_STATIC_REGISTRY" /D "WITH_ICESTORM" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)/build" /i "$(ITVSDKPATH)" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 Freeze.lib Ice.lib IceUtil.lib IceStorm.lib IceGrid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../bin/CtrlPanCtx.dll" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ZQPROJSPATH)/TianShan/bin" /libpath:"$(ZQPROJSPATH)/TianShan/lib"
# SUBTRACT LINK32 /pdb:none /nodefaultlib
# Begin Custom Build - Performing registration
OutDir=.\Release
TargetPath=\ZQProjs\TianShan\bin\CtrlPanCtx.dll
InputPath=\ZQProjs\TianShan\bin\CtrlPanCtx.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "CtrlPanCtx - Win32 Debug"
# Name "CtrlPanCtx - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AttribeDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\DataDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgTabViewCtrl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Generic\ColorBarContainer\Source\Include\Draw.cpp
# End Source File
# Begin Source File

SOURCE=.\EventAddDataThread.cpp
# End Source File
# Begin Source File

SOURCE=.\EventLog.cpp
# End Source File
# Begin Source File

SOURCE=.\EventsDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Generic\ColorBarContainer\Source\Include\IBMSBaseWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\ItemDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\MyTabViewCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\SNMPVarDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\SortClass.cpp
# End Source File
# Begin Source File

SOURCE=.\SortHeadCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\SortListViewCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\WTLTabViewCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\ZQInfoClientControl.cpp
# End Source File
# Begin Source File

SOURCE=.\ZQInfoClientControl.def
# End Source File
# Begin Source File

SOURCE=.\ZQInfoClientControl.idl
# ADD MTL /tlb ".\CtrlPanCtx.tlb" /h "CtrlPanCtx.h" /iid "CtrlPanCtx_i.c" /Oicf
# End Source File
# Begin Source File

SOURCE=.\ZQInfoClientControl.rc
# End Source File
# Begin Source File

SOURCE=.\ZQInfoClientControlps.def
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\ZQInfoClientCtrl.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AttribeDialog.h
# End Source File
# Begin Source File

SOURCE=.\ButtonXP.h
# End Source File
# Begin Source File

SOURCE=.\DataDialog.h
# End Source File
# Begin Source File

SOURCE=.\DlgTabViewCtrl.h
# End Source File
# Begin Source File

SOURCE=.\EventAddDataThread.h
# End Source File
# Begin Source File

SOURCE=.\EventLog.h
# End Source File
# Begin Source File

SOURCE=.\EventsDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Generic\ColorBarContainer\Source\Include\IBMSBaseWnd.h
# End Source File
# Begin Source File

SOURCE=.\ItemDialog.h
# End Source File
# Begin Source File

SOURCE=.\MyTabViewCtrl.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SNMPVarDialog.h
# End Source File
# Begin Source File

SOURCE=.\SortClass.h
# End Source File
# Begin Source File

SOURCE=.\SortHeadCtrl.h
# End Source File
# Begin Source File

SOURCE=.\SortListViewCtrl.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\WTLTabViewCtrl.h
# End Source File
# Begin Source File

SOURCE=.\ZQInfoClientCtrl.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\4.bmp
# End Source File
# Begin Source File

SOURCE=.\ArrowDown.bmp
# End Source File
# Begin Source File

SOURCE=.\arrowup.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmap2.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmap3.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmap4.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\Errors.bmp
# End Source File
# Begin Source File

SOURCE=.\Info.bmp
# End Source File
# Begin Source File

SOURCE=.\Information.bmp
# End Source File
# Begin Source File

SOURCE=.\level.bmp
# End Source File
# Begin Source File

SOURCE=.\StatusToolBar.bmp
# End Source File
# Begin Source File

SOURCE=.\tabb.bmp
# End Source File
# Begin Source File

SOURCE=.\Warnings.bmp
# End Source File
# Begin Source File

SOURCE=.\zqinfocl.bmp
# End Source File
# Begin Source File

SOURCE=.\ZQInfoClientCtrl.rgs
# End Source File
# End Group
# End Target
# End Project
