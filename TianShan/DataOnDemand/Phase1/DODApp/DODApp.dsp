# Microsoft Developer Studio Project File - Name="DODApp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=DODApp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DODApp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DODApp.mak" CFG="DODApp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DODApp - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "DODApp - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/DataOnDemand/Phase1/DODApp", FXXAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DODApp - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /Od /I "." /I ".." /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/TianShan/common" /I "$(ZQProjsPath)/Common" /I "$(ICE_ROOT)/include/stlport" /I "$(ICE_ROOT)/include" /I "$(ITVSDKPATH)\include" /D "WIN32" /D "NDEBUG" /D "_PHASE1" /D "_CONSOLE" /D "_AFXDLL" /D "UNICODE" /D "_UNICODE" /D "_STLP_NEW_PLATFORM_SDK" /D _WIN32_WINNT=0x0500 /D "XML_STATIC" /FR /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ITVSDKPATH)/include" /i "$(ZQProjsPath)/build" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ice.lib freeze.lib iceutil.lib Ws2_32.lib /nologo /subsystem:console /incremental:yes /debug /machine:I386 /libpath:"$[zqprojs]\TianShan\bin" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ITVSDKPATH)\Lib\Debug" /libpath:"$(ITVSDKPATH)\Lib\Release"

!ELSEIF  "$(CFG)" == "DODApp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I ".." /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/TianShan/common" /I "$(ZQProjsPath)/Common" /I "$(ICE_ROOT)/include/stlport" /I "$(ICE_ROOT)/include" /I "$(ITVSDKPATH)\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_PHASE1" /D "UNICODE" /D "_UNICODE" /D "_DEGUB" /D "_STLP_NEW_PLATFORM_SDK" /D _WIN32_WINNT=0x0500 /D "XML_STATIC" /D "_STLP_DEBUG" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ITVSDKPATH)/include" /i "$(ZQProjsPath)/build" /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 iced.lib freezed.lib iceutild.lib Ws2_32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"$[zqprojs]\TianShan\bin" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ITVSDKPATH)\Lib\Debug"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "DODApp - Win32 Release"
# Name "DODApp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ActiveChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\ActiveChannelMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ActiveFolderChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\ActiveMsgChannel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\BaseSchangeServiceApplication.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\ChannelPublishPointImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\DataPublisherImpl.cpp

!IF  "$(CFG)" == "DODApp - Win32 Release"

!ELSEIF  "$(CFG)" == "DODApp - Win32 Debug"

# ADD CPP /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DataStream.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\DestinationImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\dodapp.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\DODAppEx.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\DODAppImpl.cpp

!IF  "$(CFG)" == "DODApp - Win32 Release"

!ELSEIF  "$(CFG)" == "DODApp - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DODAppMain.cpp
# End Source File
# Begin Source File

SOURCE=.\DODAppSvc.cpp

!IF  "$(CFG)" == "DODApp - Win32 Release"

!ELSEIF  "$(CFG)" == "DODApp - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FolderChannelImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\global.cpp
# End Source File
# Begin Source File

SOURCE=.\MessageChannelImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\MiniDump.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\NotifyMsgChannel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\SchangeServiceAppMain.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\ScReporter.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Util.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ActiveChannel.h
# End Source File
# Begin Source File

SOURCE=.\ActiveChannelMgr.h
# End Source File
# Begin Source File

SOURCE=.\ActiveFolderChannel.h
# End Source File
# Begin Source File

SOURCE=.\ActiveMsgChannel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\BaseSchangeServiceApplication.h
# End Source File
# Begin Source File

SOURCE=.\ChannelPublishPointImpl.h
# End Source File
# Begin Source File

SOURCE=.\DataPublisherImpl.h
# End Source File
# Begin Source File

SOURCE=.\DataStream.h
# End Source File
# Begin Source File

SOURCE=.\definition.h
# End Source File
# Begin Source File

SOURCE=.\DestinationImpl.h
# End Source File
# Begin Source File

SOURCE=.\dodapp.h
# End Source File
# Begin Source File

SOURCE=.\DODAppEx.h
# End Source File
# Begin Source File

SOURCE=.\DODAppImpl.h
# End Source File
# Begin Source File

SOURCE=.\DODAppMain.h
# End Source File
# Begin Source File

SOURCE=.\DODAppSvc.h
# End Source File
# Begin Source File

SOURCE=.\FolderChannelImpl.h
# End Source File
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=.\MessageChannelImpl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\MiniDump.h
# End Source File
# Begin Source File

SOURCE=.\NotifyMsgChannel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\ScReporter.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Util.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\ZQ_common_conf.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\DODApp.rc
# End Source File
# End Group
# Begin Group "Ice Files"

# PROP Default_Filter "ice"
# Begin Source File

SOURCE=..\..\ice\DataStream.ice

!IF  "$(CFG)" == "DODApp - Win32 Release"

# Begin Custom Build
InputPath=..\..\ice\DataStream.ice

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice $(InputPath)

"DataStream.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"DataStream.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "DODApp - Win32 Debug"

# Begin Custom Build
InputPath=..\..\ice\DataStream.ice

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice $(InputPath)

"DataStream.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"DataStream.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\ice\dodapp.ice

!IF  "$(CFG)" == "DODApp - Win32 Release"

# Begin Custom Build
InputPath=..\..\ice\dodapp.ice

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice $(InputPath)

"DODApp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"DODApp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "DODApp - Win32 Debug"

# Begin Custom Build
InputPath=..\..\ice\dodapp.ice

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice $(InputPath)

"DODApp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"DODApp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DODAppEx.ice

!IF  "$(CFG)" == "DODApp - Win32 Release"

# Begin Custom Build
InputPath=.\DODAppEx.ice

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)/DataOnDemand/Ice $(InputPath)

"DODAppEx.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"DODAppEx.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "DODApp - Win32 Debug"

# Begin Custom Build - Compiling ICE ...
InputPath=.\DODAppEx.ice

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)/DataOnDemand/Ice $(InputPath)

"DODAppEx.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"DODAppEx.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
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
