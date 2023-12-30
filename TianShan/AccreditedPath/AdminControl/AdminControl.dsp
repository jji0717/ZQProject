# Microsoft Developer Studio Project File - Name="AdminControl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=AdminControl - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AdminControl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AdminControl.mak" CFG="AdminControl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AdminControl - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "AdminControl - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/TianShan/AccreditedPath/AdminControl", QVXAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AdminControl - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(ZQProjsPath)\Generic\ColorBarContainer\Source\Include" /I "." /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/TianShan/common" /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "$(ZQProjsPath)/TianShan/Weiwoo" /I "$(ZQProjsPath)/TianShan/AccreditedPath" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_CONSOLE" /D _WIN32_WINNT=0x0500 /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /D "ZQCOMMON_DLL" /D "WITH_ICESTORM" /YX"StdAfx.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)/build" /i "$(ITVSDKPATH)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 TianShanIce_d.lib TianShanCommon_d.lib ZQCommonStlp_d.lib Freezed.lib Iced.lib IceUtild.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../bin/AdminControl_d.dll" /pdbtype:sept /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ZQPROJSPATH)/TianShan/bin" /libpath:"$(ZQPROJSPATH)/TianShan/lib" /libpath:"./Include"
# Begin Custom Build - Performing registration
OutDir=.\Debug
TargetPath=\work\project\ZQProjs\TianShan\bin\AdminControl_d.dll
InputPath=\work\project\ZQProjs\TianShan\bin\AdminControl_d.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "AdminControl - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "AdminControl___Win32_Release"
# PROP BASE Intermediate_Dir "AdminControl___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /O1 /I "$(ZQProjsPath)\Generic\ColorBarContainer\Source\Include" /I "." /I ".." /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/TianShan/common" /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "_ATL_STATIC_REGISTRY" /D "_ATL_MIN_CRT" /D _WIN32_WINNT=0x500 /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "ZQCOMMON_DLL" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MD /W3 /GR /GX /ZI /Od /I "$(ZQProjsPath)\Generic\ColorBarContainer\Source\Include" /I "." /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/TianShan/common" /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "$(ZQProjsPath)/TianShan/Weiwoo" /I "$(ZQProjsPath)/TianShan/AccreditedPath" /I "$(ZQProjsPath)/TianShan/SiteAdminSvc" /I "$(ZQProjsPath)/TianShan/Weiwoo/service" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D _WIN32_WINNT=0x500 /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "ZQCOMMON_DLL" /D "_ATL_STATIC_REGISTRY" /D "WITH_ICESTORM" /YX"StdAfx.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)/build" /i "$(ITVSDKPATH)" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 TianShanIce.lib TianShanCommon.lib ZQCommonStlp.lib Freeze.lib Iced.lib IceUtild.lib IceStorm.lib IceGridd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../../bin/AdminControl.dll" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ZQPROJSPATH)/TianShan/bin" /libpath:"$(ZQPROJSPATH)/TianShan/lib" /libpath:"./Include"
# ADD LINK32 Freeze.lib Ice.lib IceUtil.lib IceStorm.lib IceGrid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:windows /dll /pdb:"../../bin/AdminControl.pdb" /debug /machine:I386 /out:"../../bin/AdminControl.dll" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ZQPROJSPATH)/TianShan/bin" /libpath:"$(ZQPROJSPATH)/TianShan/lib" /libpath:"./Include"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build - Performing registration
OutDir=.\Release
TargetPath=\work\project\ZQProjs\TianShan\bin\AdminControl.dll
InputPath=\work\project\ZQProjs\TianShan\bin\AdminControl.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "AdminControl - Win32 Debug"
# Name "AdminControl - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AdminControl.cpp
# End Source File
# Begin Source File

SOURCE=.\AdminControl.def
# End Source File
# Begin Source File

SOURCE=.\AdminControl.idl
# ADD MTL /tlb ".\AdminControl.tlb" /h "AdminControl.h" /iid "AdminControl_i.c" /Oicf
# End Source File
# Begin Source File

SOURCE=.\AdminControl.rc
# End Source File
# Begin Source File

SOURCE=.\AdminCtrl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Generic\ColorBarContainer\Source\Include\Draw.cpp
# End Source File
# Begin Source File

SOURCE=.\MyDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\SiteAdminSvc\SiteAdminSvc.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\TsPathAdmin.cpp
# ADD CPP /I ".."
# End Source File
# Begin Source File

SOURCE=..\..\Ice\TsSite.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Ice\TsSRM.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Ice\TsStorage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Ice\TsStreamer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Ice\TsTransport.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Weiwoo\WeiwooAdmin.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AdminCtrl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Generic\ColorBarContainer\Source\Include\Draw.h
# End Source File
# Begin Source File

SOURCE=.\MyDialog.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\adminctr.bmp
# End Source File
# Begin Source File

SOURCE=.\AdminCtrl.rgs
# End Source File
# Begin Source File

SOURCE=.\App.bmp
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

SOURCE=.\Drives.bmp
# End Source File
# Begin Source File

SOURCE=.\Sergroup.bmp
# End Source File
# Begin Source File

SOURCE=.\shoot.ico
# End Source File
# Begin Source File

SOURCE=.\Site.bmp
# End Source File
# Begin Source File

SOURCE=..\..\SiteAdminSvc\SiteAdminSvc.ICE

!IF  "$(CFG)" == "AdminControl - Win32 Debug"

!ELSEIF  "$(CFG)" == "AdminControl - Win32 Release"

# Begin Custom Build
InputPath=..\..\SiteAdminSvc\SiteAdminSvc.ICE
InputName=SiteAdminSvc

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I../../Ice --output-dir ../../SiteAdminSvc/  ../../SiteAdminSvc\SiteAdminSvc.ICE

"../../SiteAdminSvc/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../SiteAdminSvc/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Storage.bmp
# End Source File
# Begin Source File

SOURCE=.\Storage1.bmp
# End Source File
# Begin Source File

SOURCE=.\StorageL.bmp
# End Source File
# Begin Source File

SOURCE=.\StorageLink.bmp
# End Source File
# Begin Source File

SOURCE=.\Streamer.bmp
# End Source File
# Begin Source File

SOURCE=.\StreamL.bmp
# End Source File
# Begin Source File

SOURCE=.\Streamlink.bmp
# End Source File
# Begin Source File

SOURCE=..\TsPathAdmin.ICE

!IF  "$(CFG)" == "AdminControl - Win32 Debug"

# Begin Custom Build
InputPath=..\TsPathAdmin.ICE
InputName=TsPathAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice --output-dir .. ../$(InputName).ice

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "AdminControl - Win32 Release"

# Begin Custom Build
InputPath=..\TsPathAdmin.ICE
InputName=TsPathAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I../../ice --output-dir .. ../$(InputName).ice

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Weiwoo\WeiwooAdmin.ICE

!IF  "$(CFG)" == "AdminControl - Win32 Debug"

# Begin Custom Build
InputPath=..\..\Weiwoo\WeiwooAdmin.ICE
InputName=WeiwooAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice --output-dir ../../Weiwoo/ ../../Weiwoo/$(InputName).ice

"../../Weiwoo/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../Weiwoo/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "AdminControl - Win32 Release"

# Begin Custom Build
InputPath=..\..\Weiwoo\WeiwooAdmin.ICE
InputName=WeiwooAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice --output-dir ../../Weiwoo/ ../../Weiwoo/$(InputName).ice

"../../Weiwoo/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../../Weiwoo/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
