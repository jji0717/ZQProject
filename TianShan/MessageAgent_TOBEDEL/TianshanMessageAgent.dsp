# Microsoft Developer Studio Project File - Name="TianshanMessageAgent" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=TianshanMessageAgent - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TianshanMessageAgent.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TianshanMessageAgent.mak" CFG="TianshanMessageAgent - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TianshanMessageAgent - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "TianshanMessageAgent - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TianshanMessageAgent - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQPROJSPATH)\generic\JMSCppLib" /I "$(ICE_ROOT)/include/stlport" /I "$(ICE_ROOT)/include" /I "$(ZQProjsPath)/TianShan/common" /I "$(ITVSDKPATH)\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "_STLP_NEW_PLATFORM_SDK" /D _WIN32_WINNT=0x0500 /D "WITH_ICESTORM" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)/build" /i "$(ITVSDKPATH)" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 Ice.lib IceUtil.lib Freeze.lib IceStorm.lib /nologo /subsystem:console /machine:I386 /out:"Release/MessageAgent.exe" /libpath:"$(ZQProjsPath)/lib" /libpath:"$(ZQProjsPath)/TianShan/bin" /libpath:"$(ITVSDKPATH)/lib/release" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ZQPROJSPATH)\generic\JMSCppLib\JMSCpp\lib\\"
# SUBTRACT LINK32 /incremental:yes /debug

!ELSEIF  "$(CFG)" == "TianshanMessageAgent - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQPROJSPATH)\generic\JMSCppLib" /I "$(ICE_ROOT)/include/stlport" /I "$(ICE_ROOT)/include" /I "$(ZQProjsPath)/TianShan/common" /I "$(ITVSDKPATH)\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /D _WIN32_WINNT=0x0500 /D "WITH_ICESTORM" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)/build" /i "$(ITVSDKPATH)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Iced.lib IceUtild.lib Freezed.lib IceStormd.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/MessageAgent.exe" /pdbtype:sept /libpath:"$(ZQProjsPath)/lib" /libpath:"$(ZQProjsPath)/TianShan/bin" /libpath:"$(ITVSDKPATH)/lib/debug" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ZQPROJSPATH)\generic\JMSCppLib\JMSCpp\lib\\"

!ENDIF 

# Begin Target

# Name "TianshanMessageAgent - Win32 Release"
# Name "TianshanMessageAgent - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\Common\BaseSchangeServiceApplication.cpp
# End Source File
# Begin Source File

SOURCE=.\CSProvisionEventHelper.cpp

!IF  "$(CFG)" == "TianshanMessageAgent - Win32 Release"

!ELSEIF  "$(CFG)" == "TianshanMessageAgent - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\JMSPublisher.cpp

!IF  "$(CFG)" == "TianshanMessageAgent - Win32 Release"

!ELSEIF  "$(CFG)" == "TianshanMessageAgent - Win32 Debug"

# ADD CPP /D _WIN32_WINNT=0x0400
# SUBTRACT CPP /D _WIN32_WINNT=0x0500 /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Common\Log.cpp
# End Source File
# Begin Source File

SOURCE=.\MessageAgent.cpp
# End Source File
# Begin Source File

SOURCE=.\MessageAgentServ.cpp
# End Source File
# Begin Source File

SOURCE=.\MessageData.cpp
# End Source File
# Begin Source File

SOURCE=.\ProvisionEventI.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\SchangeServiceAppMain.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\ScReporter.cpp
# End Source File
# Begin Source File

SOURCE=.\TianshanMessageAgent.rc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Common\BaseSchangeServiceApplication.h
# End Source File
# Begin Source File

SOURCE=.\CSProvisionEventHelper.h
# End Source File
# Begin Source File

SOURCE=.\JMSPublisher.h
# End Source File
# Begin Source File

SOURCE=.\MessageAgent.h
# End Source File
# Begin Source File

SOURCE=.\MessageAgentServ.h
# End Source File
# Begin Source File

SOURCE=.\MessageData.h
# End Source File
# Begin Source File

SOURCE=.\ProvisionEventI.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\ScReporter.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\MessageAgent.ICE

!IF  "$(CFG)" == "TianshanMessageAgent - Win32 Release"

# Begin Custom Build
InputPath=.\MessageAgent.ICE
InputName=MessageAgent

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/TianShan/Ice $(InputPath) \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/TianShan/Ice --dict Messages,string,::TianShanIce::MessageAgent::Message MessageData $(InputPath) \
	

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"MessageData.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"MessageData.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianshanMessageAgent - Win32 Debug"

# Begin Custom Build
InputPath=.\MessageAgent.ICE
InputName=MessageAgent

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/TianShan/Ice $(InputPath) \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQPROJSPATH)/TianShan/Ice --dict Messages,string,::TianShanIce::MessageAgent::Message MessageData $(InputPath) \
	

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"MessageData.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"MessageData.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
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
