# Microsoft Developer Studio Project File - Name="Weiwoo2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Weiwoo2 - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Weiwoo2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Weiwoo2.mak" CFG="Weiwoo2 - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Weiwoo2 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Weiwoo2 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/TianShan/Weiwoo", GXSAAAAA"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Weiwoo2 - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../../Ice" /I "$(ITVSDKPATH)\include\\" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "XML_STATIC" /D _WIN32_WINNT=0x500 /D "_STLP_NEW_PLATFORM_SDK" /D "WITH_ICESTORM" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:console /machine:I386 /out:"../bin/Weiwoo.exe" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ITVSDKPATH)\lib\release"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(ZQProjsPath)\Common\dll\ReleaseStlp\*.dll ..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Weiwoo2 - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../../Ice" /I "$(ITVSDKPATH)\include\\" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D _WIN32_WINNT=0x0500 /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /D "WITH_ICESTORM" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:console /profile /map /debug /machine:I386 /out:"../bin/Weiwoo_d.exe" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ITVSDKPATH)\lib\debug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(ZQProjsPath)\Common\dll\DebugStlp\*.dll ..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Weiwoo2 - Win32 Release"
# Name "Weiwoo2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\AppDict.cpp
# End Source File
# Begin Source File

SOURCE=..\AppToMount.cpp
# End Source File
# Begin Source File

SOURCE=..\BusinessRouterImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\getopt.cpp
# End Source File
# Begin Source File

SOURCE=..\IdToSess.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\ScLog.cpp
# End Source File
# Begin Source File

SOURCE=..\SessionCommand.cpp
# End Source File
# Begin Source File

SOURCE=..\SessionImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\SessionState.cpp
# End Source File
# Begin Source File

SOURCE=..\SessionWatchDog.cpp
# End Source File
# Begin Source File

SOURCE=..\SiteDict.cpp
# End Source File
# Begin Source File

SOURCE=..\SiteToMount.cpp
# End Source File
# Begin Source File

SOURCE=..\WeiwooAdmin.cpp
# End Source File
# Begin Source File

SOURCE=.\WeiwooCmd.cpp
# End Source File
# Begin Source File

SOURCE=..\WeiwooFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\WeiwooSvcEnv.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\AppDict.h
# End Source File
# Begin Source File

SOURCE=..\AppToMount.h
# End Source File
# Begin Source File

SOURCE=..\BusinessRouterImpl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\getopt.h
# End Source File
# Begin Source File

SOURCE=..\IdToSess.h
# End Source File
# Begin Source File

SOURCE=..\..\..\COMMON\ScLog.h
# End Source File
# Begin Source File

SOURCE=..\SessionCommand.h
# End Source File
# Begin Source File

SOURCE=..\SessionImpl.h
# End Source File
# Begin Source File

SOURCE=..\SessionState.h
# End Source File
# Begin Source File

SOURCE=..\SessionWatchDog.h
# End Source File
# Begin Source File

SOURCE=..\SiteDict.h
# End Source File
# Begin Source File

SOURCE=..\SiteToMount.h
# End Source File
# Begin Source File

SOURCE=..\..\Ice\TianShanDefines.h
# End Source File
# Begin Source File

SOURCE=..\WeiwooAdmin.h
# End Source File
# Begin Source File

SOURCE=..\WeiwooFactory.h
# End Source File
# Begin Source File

SOURCE=..\WeiwooSvcEnv.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icon_weiwoo.ico
# End Source File
# Begin Source File

SOURCE=.\Weiwoo2.rc

!IF  "$(CFG)" == "Weiwoo2 - Win32 Release"

# ADD BASE RSC /l 0x804
# ADD RSC /l 0x804 /i "." /i "$(ZQProjsPath)/build"

!ELSEIF  "$(CFG)" == "Weiwoo2 - Win32 Debug"

# ADD BASE RSC /l 0x804
# ADD RSC /l 0x804 /i "." /i "$(ZQProjsPath)/build"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WeiwooAdmin.ICE

!IF  "$(CFG)" == "Weiwoo2 - Win32 Release"

!ELSEIF  "$(CFG)" == "Weiwoo2 - Win32 Debug"

# Begin Custom Build
InputPath=.\WeiwooAdmin.ICE
InputName=WeiwooAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --dict "TianShanIce::Weiwoo::SiteDict,string,TianShanIce::Weiwoo::Site" --dict-index "TianShanIce::Weiwoo::SiteDict,name" SiteDict --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --dict "TianShanIce::Weiwoo::AppDict,string,TianShanIce::Weiwoo::AppInfo" --dict-index "TianShanIce::Weiwoo::AppDict,name" AppDict --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::Weiwoo::SiteToMount,TianShanIce::Weiwoo::AppMount,siteName,case-insensitive" SiteToMount --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::Weiwoo::AppToMount,TianShanIce::Weiwoo::AppMount,appName,case-insensitive" AppToMount --output-dir .. ../$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice --index "TianShanIce::Weiwoo::IdToSess,TianShanIce::Weiwoo::SessionEx,sessId,case-sensitive" IdToSess --output-dir .. ../$(InputName).ice \
	

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../SiteDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../SiteDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../IdToSess.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../IdToSess.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../AppDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../AppDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../SiteToMount.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../SiteToMount.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../AppToMount.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../AppToMount.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
