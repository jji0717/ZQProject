# Microsoft Developer Studio Project File - Name="NSSSvc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=NSSSvc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NSSSvc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NSSSvc.mak" CFG="NSSSvc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NSSSvc - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "NSSSvc - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NSSSvc - Win32 Release"

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

!ELSEIF  "$(CFG)" == "NSSSvc - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "../../../Ice" /I "$(ZQProjsPath)/TianShan/Shell/ZQSNMPManPkg" /I "..\NSSRtspSession" /I "$(STLPORT_ROOT)" /I "$(EXPATPATH)/include" /I "$(ZQPROJSPATH)\tianshan\shell\zqsnmpmanpkg" /I "$(RegExppKit)" /I "../NSSRtspSession/" /I "../service" /I "$(ZQProjsPath)/TianShan/Shell/ZQAppShell" /I "$(ZQProjsPath)/TianShan/Shell/ZQCfgPkg" /D "_DEBUG" /D _WIN32_WINNT=0x0500 /D "_STLP_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /D "WITH_ICESTORM" /D "LOGFMTWITHTID" /FD /Zm500 /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib libexpatMT.lib iced.lib iceutild.lib freezed.lib WS2_32.lib $(ZQProjsPath)\TianShan\ComcastNGOD\NSS\NSSRtspSession\Debug\NSSRtspSession.lib /nologo /subsystem:console /debug /machine:I386 /out:"../../../bin/NSSSvc.exe" /pdbtype:sept /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ITVSDKPATH)\lib\debug"

!ENDIF 

# Begin Target

# Name "NSSSvc - Win32 Release"
# Name "NSSSvc - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\Common\BaseZQServiceApplication.cpp
# End Source File
# Begin Source File

SOURCE=.\CECommon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Common\ConfigHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Common\ConfigLoader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Common\MiniDump.cpp
# End Source File
# Begin Source File

SOURCE=..\NSS.cpp
# End Source File
# Begin Source File

SOURCE=..\service\NSSConfig.cpp
# End Source File
# Begin Source File

SOURCE=..\service\NSSEnv.cpp
# End Source File
# Begin Source File

SOURCE=..\NSSEx.cpp
# End Source File
# Begin Source File

SOURCE=..\service\NSSFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\service\NSSImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\NSSSvc.cpp
# End Source File
# Begin Source File

SOURCE=..\SessionIdx.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Common\ZQServiceAppMain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\Common\BaseZQServiceApplication.h
# End Source File
# Begin Source File

SOURCE=.\CECommon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Common\ConfigHelper.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\Common\MiniDump.h
# End Source File
# Begin Source File

SOURCE=..\service\NSSConfig.h
# End Source File
# Begin Source File

SOURCE=..\service\NSSEnv.h
# End Source File
# Begin Source File

SOURCE=..\service\NSSFactory.h
# End Source File
# Begin Source File

SOURCE=..\service\NSSImpl.h
# End Source File
# Begin Source File

SOURCE=.\NSSSvc.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\NSS.ICE

!IF  "$(CFG)" == "NSSSvc - Win32 Release"

!ELSEIF  "$(CFG)" == "NSSSvc - Win32 Debug"

# Begin Custom Build
InputDir=\build\zqprojs\TianShan\ComcastNGOD\NSS
InputPath=..\NSS.ICE
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
# Begin Source File

SOURCE=..\NSSEx.ICE

!IF  "$(CFG)" == "NSSSvc - Win32 Release"

!ELSEIF  "$(CFG)" == "NSSSvc - Win32 Debug"

# Begin Custom Build
InputDir=\build\zqprojs\TianShan\ComcastNGOD\NSS
InputPath=..\NSSEx.ICE
InputName=NSSEx

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/ICE --output-dir .. $(InputDir)\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/ICE --index "TianShanIce::Streamer::NGODStreamServer::SessionIdx,TianShanIce::Streamer::NGODStreamServer::NGODStreamEx,sessKey,case-sensitive" SessionIdx --output-dir .. $(InputDir)\$(InputName).ice \
	

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../SessionIdx.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../SessionIdx.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
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
