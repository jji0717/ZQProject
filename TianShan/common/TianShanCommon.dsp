# Microsoft Developer Studio Project File - Name="TianShanCommon" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=TianShanCommon - Win32 Unicode Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TianShanCommon.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TianShanCommon.mak" CFG="TianShanCommon - Win32 Unicode Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TianShanCommon - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "TianShanCommon - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "TianShanCommon - Win32 Unicode Release" (based on "Win32 (x86) Static Library")
!MESSAGE "TianShanCommon - Win32 Unicode Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/TianShan/common", XHTAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TianShanCommon - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /I "." /I "$(ZQProjsPath)/Common" /I "$(EXPATPATH)/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../Ice" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D _WIN32_WINNT=0x500 /D "XML_STATIC" /D "ZQCOMMON_DLL" /D "_STLP_NEW_PLATFORM_SDK" /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../bin/TianShanCommon.lib"

!ELSEIF  "$(CFG)" == "TianShanCommon - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I "$(ZQProjsPath)/Common" /I "$(EXPATPATH)/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../Ice" /D "XML_STATIC" /D "ZQCOMMON_DLL" /D _WIN32_WINNT=0x500 /D "_STLP_DEBUG" /D "_MBCS" /D "_LIB" /D "_DEBUG" /D "WIN32" /D "_STLP_NEW_PLATFORM_SDK" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../bin/TianShanCommon_d.lib"

!ELSEIF  "$(CFG)" == "TianShanCommon - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TianShanCommon___Win32_Unicode_Release"
# PROP BASE Intermediate_Dir "TianShanCommon___Win32_Unicode_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unicode_Release"
# PROP Intermediate_Dir "Unicode_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "." /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../Ice" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D _WIN32_WINNT=0x500 /D "XML_STATIC" /D "ZQCOMMON_DLL" /D "_STLP_NEW_PLATFORM_SDK" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "." /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../Ice" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D _WIN32_WINNT=0x500 /D "XML_STATIC" /D "ZQCOMMON_DLL" /D "_STLP_NEW_PLATFORM_SDK" /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"../bin/TianShanCommon.lib"
# ADD LIB32 /nologo /out:"../bin/TianShanCommon.lib"

!ELSEIF  "$(CFG)" == "TianShanCommon - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TianShanCommon___Win32_Unicode_Debug"
# PROP BASE Intermediate_Dir "TianShanCommon___Win32_Unicode_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Unicode_Debug"
# PROP Intermediate_Dir "Unicode_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../Ice" /D "XML_STATIC" /D "ZQCOMMON_DLL" /D _WIN32_WINNT=0x500 /D "_STLP_DEBUG" /D "_MBCS" /D "_LIB" /D "_DEBUG" /D "WIN32" /D "_STLP_NEW_PLATFORM_SDK" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../Ice" /D "XML_STATIC" /D "ZQCOMMON_DLL" /D _WIN32_WINNT=0x500 /D "_STLP_DEBUG" /D "_MBCS" /D "_LIB" /D "_DEBUG" /D "WIN32" /D "_STLP_NEW_PLATFORM_SDK" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"../bin/TianShanCommon_d.lib"
# ADD LIB32 /nologo /out:"../bin/TianShanCommon_d.lib"

!ENDIF 

# Begin Target

# Name "TianShanCommon - Win32 Release"
# Name "TianShanCommon - Win32 Debug"
# Name "TianShanCommon - Win32 Unicode Release"
# Name "TianShanCommon - Win32 Unicode Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\EventChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\HtmlTempl.cpp
# End Source File
# Begin Source File

SOURCE=.\IceLog.cpp
# End Source File
# Begin Source File

SOURCE=.\TianShanDefines.cpp
# End Source File
# Begin Source File

SOURCE=.\ZqAdapter.cpp
# End Source File
# Begin Source File

SOURCE=.\ZqSentryIce.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\EventChannel.h
# End Source File
# Begin Source File

SOURCE=.\HtmlTempl.h
# End Source File
# Begin Source File

SOURCE=.\IceLog.h
# End Source File
# Begin Source File

SOURCE=.\IPathHelperObj.h
# End Source File
# Begin Source File

SOURCE=.\TianShanDefines.h
# End Source File
# Begin Source File

SOURCE=.\ZqSentryIce.h
# End Source File
# End Group
# Begin Group "ICE Files"

# PROP Default_Filter "ICE"
# Begin Source File

SOURCE=.\ZqSentryIce.ICE

!IF  "$(CFG)" == "TianShanCommon - Win32 Release"

# Begin Custom Build
InputDir=.
InputPath=.\ZqSentryIce.ICE
InputName=ZqSentryIce

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I../Ice $(InputDir)/$(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanCommon - Win32 Debug"

# Begin Custom Build
InputDir=.
InputPath=.\ZqSentryIce.ICE
InputName=ZqSentryIce

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I../Ice $(InputDir)/$(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanCommon - Win32 Unicode Release"

# Begin Custom Build
InputDir=.
InputPath=.\ZqSentryIce.ICE
InputName=ZqSentryIce

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I../Ice $(InputDir)/$(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanCommon - Win32 Unicode Debug"

# Begin Custom Build
InputDir=.
InputPath=.\ZqSentryIce.ICE
InputName=ZqSentryIce

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I../Ice $(InputDir)/$(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
