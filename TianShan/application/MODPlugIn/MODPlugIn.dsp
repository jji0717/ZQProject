# Microsoft Developer Studio Project File - Name="MODPlugIn" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=MODPlugIn - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MODPlugIn.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MODPlugIn.mak" CFG="MODPlugIn - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MODPlugIn - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MODPlugIn - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MODPlugIn - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MODPLUGIN_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "." /I ".." /I "$(ICE_ROOT)/include" /I "$(STLPORT_ROOT)" /I "$(ZQPROJSPATH)/Common" /I "$(ZQPROJSPATH)/TianShan/Common" /I "$(ZQPROJSPATH)/TianShan/Ice" /D _WIN32_WINNT=0x0500 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MODPLUGIN_EXPORTS" /D "_STLP_NEW_PLATFORM_SDK" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ITVSDKPATH)/include" /i "$(ZQProjsPath)/build" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"../../bin/MHO_MODPlugIn.pdb" /debug /machine:I386 /out:"../../bin/MHO_MODPlugIn.dll" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ITVSDKPATH)\lib\release"

!ELSEIF  "$(CFG)" == "MODPlugIn - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MODPLUGIN_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I ".." /I "$(ICE_ROOT)/include" /I "$(STLPORT_ROOT)" /I "$(ZQPROJSPATH)/Common" /I "$(ZQPROJSPATH)/TianShan/Common" /I "$(ZQPROJSPATH)/TianShan/Ice" /D _WIN32_WINNT=0x0500 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MODPLUGIN_EXPORTS" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ITVSDKPATH)/include" /i "$(ZQProjsPath)/build" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/MHO_MODPlugIn_d.dll" /pdbtype:sept /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ITVSDKPATH)\lib\Release"

!ENDIF 

# Begin Target

# Name "MODPlugIn - Win32 Release"
# Name "MODPlugIn - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\LAMFacade.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\LAMPlayListQuery.cpp
# End Source File
# Begin Source File

SOURCE=.\MODPlugIn.cpp
# End Source File
# Begin Source File

SOURCE=.\MODplugin.def
# End Source File
# Begin Source File

SOURCE=.\ote.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\OTEAuthorization.cpp
# End Source File
# Begin Source File

SOURCE=.\OTEForTeardownCB.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Surf_Tianshan.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\LAMFacade.h
# End Source File
# Begin Source File

SOURCE=.\LAMPlayListQuery.h
# End Source File
# Begin Source File

SOURCE=.\ote.h
# End Source File
# Begin Source File

SOURCE=.\OTEAuthorization.h
# End Source File
# Begin Source File

SOURCE=.\OTEForTeardownCB.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Surf_Tianshan.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\MODPlugIn.rc
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "ICE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ICE\LAMFacade.ice

!IF  "$(CFG)" == "MODPlugIn - Win32 Release"

# Begin Custom Build
InputPath=..\ICE\LAMFacade.ice
InputName=LAMFacade

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputPath)

"./$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"./$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "MODPlugIn - Win32 Debug"

# Begin Custom Build
InputPath=..\ICE\LAMFacade.ice
InputName=LAMFacade

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputPath)

"./$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"./$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ICE\ote.ice

!IF  "$(CFG)" == "MODPlugIn - Win32 Release"

# Begin Custom Build
InputPath=..\ICE\ote.ice
InputName=ote

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputPath)

"./$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"./$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "MODPlugIn - Win32 Debug"

# Begin Custom Build
InputPath=..\ICE\ote.ice
InputName=ote

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputPath)

"./$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"./$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ICE\Surf_Tianshan.ice

!IF  "$(CFG)" == "MODPlugIn - Win32 Release"

# Begin Custom Build
InputPath=..\ICE\Surf_Tianshan.ice
InputName=Surf_Tianshan

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputPath)

"./$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"./$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "MODPlugIn - Win32 Debug"

# Begin Custom Build
InputPath=..\ICE\Surf_Tianshan.ice
InputName=Surf_Tianshan

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputPath)

"./$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"./$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
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
