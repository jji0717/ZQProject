# Microsoft Developer Studio Project File - Name="SiteAdmin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=SiteAdmin - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SiteAdmin.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SiteAdmin.mak" CFG="SiteAdmin - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SiteAdmin - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "SiteAdmin - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "$/ZQProjs/TianShan/SiteAdminSvc/admin"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SiteAdmin - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "." /I ".." /I "../../Ice" /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "XML_STATIC" /D _WIN32_WINNT=0x500 /D "_STLP_NEW_PLATFORM_SDK" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:console /machine:I386 /out:"../../bin/sac.exe" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ACE_ROOT)/../expat/lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(ZQProjsPath)\Common\dll\ReleaseStlp\*.dll ..\..\bin
# End Special Build Tool

!ELSEIF  "$(CFG)" == "SiteAdmin - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I ".." /I "../../Ice" /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../../SiteAdminSvc/" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D _WIN32_WINNT=0x0500 /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /D "WITH_ICESTORM" /D "ZQCOMMON_DLL" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib /nologo /subsystem:console /debug /machine:I386 /out:"../../bin/sac.exe" /pdbtype:sept /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ACE_ROOT)/../expat/lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(ZQProjsPath)\Common\dll\DebugStlp\*.dll ..\..\bin
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "SiteAdmin - Win32 Release"
# Name "SiteAdmin - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\admin.cpp
# End Source File
# Begin Source File

SOURCE=.\AdminConsole.cpp
# End Source File
# Begin Source File

SOURCE=.\ConsoleGrammar.cpp
# End Source File
# Begin Source File

SOURCE=.\ConsoleScanner.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\getopt.cpp
# End Source File
# Begin Source File

SOURCE=..\SiteAdminSvc.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\admin.h
# End Source File
# Begin Source File

SOURCE=.\AdminConsole.h
# End Source File
# Begin Source File

SOURCE=.\ConsoleGrammar.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\ConsoleGrammar.y

!IF  "$(CFG)" == "SiteAdmin - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "SiteAdmin - Win32 Debug"

# Begin Custom Build
InputPath=.\ConsoleGrammar.y
InputName=ConsoleGrammar

BuildCmds= \
	set PATH=D:\usr\cygwin\bin;%PATH% \
	del /f $(InputName).h \
	bison -d -l $(InputName).y -o $(InputName).cpp \
	ren $(InputName).hpp $(InputName).h \
	

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ConsoleScanner.l

!IF  "$(CFG)" == "SiteAdmin - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "SiteAdmin - Win32 Debug"

# Begin Custom Build
InputPath=.\ConsoleScanner.l
InputName=ConsoleScanner

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=%PATH%;D:\usr\cygwin\bin 
	flex -t $(InputName).l  >$(InputName).cpp 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SiteAdmin2.rc

!IF  "$(CFG)" == "SiteAdmin - Win32 Release"

# ADD BASE RSC /l 0x804
# ADD RSC /l 0x804 /i "." /i "../../../build"

!ELSEIF  "$(CFG)" == "SiteAdmin - Win32 Debug"

# ADD BASE RSC /l 0x804
# ADD RSC /l 0x409 /i "." /i "../../../build"

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
