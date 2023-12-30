# Microsoft Developer Studio Project File - Name="JMSDispatch" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=JMSDispatch - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "JMSDispatch.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "JMSDispatch.mak" CFG="JMSDispatch - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "JMSDispatch - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "JMSDispatch - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/DataOnDemand/Phase1/DODApp/JMSDispatch", IXXAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "JMSDispatch - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GR /GX /ZI /Od /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "MYJMSDISPATCH_EXPORTS" /D "_STLP_DEBUG" /D "_STLP_NEW_PLATFORM_SDK" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /i "$(ITVSDKPATH)/include" /i "$(ZQProjsPath)/build" /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Freezed.lib Iced.lib IceUtild.lib libexpatMT.lib jmsc.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"MSVCRT.lib" /pdbtype:sept /libpath:"$(ICE_ROOT)/lib" /libpath:"$(EXPATPATH)/lib" /libpath:"$(ITVSDKPATH)\Lib\Debug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy                       debug\*.dll                       ..\                   	copy                       debug\*.lib                       ..\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "JMSDispatch - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "JMSDispatch___Win32_Release"
# PROP BASE Intermediate_Dir "JMSDispatch___Win32_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "MYJMSDISPATCH_EXPORTS" /D "_STLP_DEBUG" /D "_STLP_NEW_PLATFORM_SDK" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MD /W3 /GR /GX /Od /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "MYJMSDISPATCH_EXPORTS" /D "_STLP_NEW_PLATFORM_SDK" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /i "$(ITVSDKPATH)/include" /i "$(ZQProjsPath)/build" /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 Freezed.lib Iced.lib IceUtild.lib libexpatMT.lib jmscpp_d.lib jmsc.lib /nologo /subsystem:windows /dll /debug /machine:I386 /nodefaultlib:"MSVCRT.lib" /pdbtype:sept /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ACE_ROOT)/../expat/lib" /libpath:"$(ITVSDKPATH)\Lib\Debug"
# ADD LINK32 Freeze.lib Ice.lib IceUtil.lib libexpatMT.lib jmsc.lib /nologo /subsystem:windows /dll /incremental:no /machine:I386 /pdbtype:sept /libpath:"$(ICE_ROOT)/lib" /libpath:"$(EXPATPATH)/lib" /libpath:"$(ITVSDKPATH)\Lib\Release"
# SUBTRACT LINK32 /debug
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy                        release\*.dll                       ..                    	copy                        release\*.lib                       ..\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "JMSDispatch - Win32 Debug"
# Name "JMSDispatch - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\dispatch.cpp
# End Source File
# Begin Source File

SOURCE=.\DisPatchMain.cpp
# End Source File
# Begin Source File

SOURCE=.\DODApp.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\JMS.cpp
# End Source File
# Begin Source File

SOURCE=.\JMSDispatch.cpp
# End Source File
# Begin Source File

SOURCE=.\JMSDispatch.def
# End Source File
# Begin Source File

SOURCE=.\JmsDispatchdll.cpp
# End Source File
# Begin Source File

SOURCE=.\Markup.cpp
# End Source File
# Begin Source File

SOURCE=.\QueueManageMent.cpp
# End Source File
# Begin Source File

SOURCE=.\ReceiveJmsMsg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\dispatch.h
# End Source File
# Begin Source File

SOURCE=.\DisPatchMain.h
# End Source File
# Begin Source File

SOURCE=.\DODApp.h
# End Source File
# Begin Source File

SOURCE=.\JMS.h
# End Source File
# Begin Source File

SOURCE=.\JMSDispatch.h
# End Source File
# Begin Source File

SOURCE=.\JmsDispatchdll.h
# End Source File
# Begin Source File

SOURCE=.\Markup.h
# End Source File
# Begin Source File

SOURCE=.\MessageMacro.h
# End Source File
# Begin Source File

SOURCE=.\QueueManageMent.h
# End Source File
# Begin Source File

SOURCE=.\ReceiveJmsMsg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\JMSDispatch.rc
# End Source File
# Begin Source File

SOURCE=.\res\JMSDispatch.rc2
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\ice\DODApp.ice

!IF  "$(CFG)" == "JMSDispatch - Win32 Debug"

# Begin Custom Build
InputPath=..\..\..\ice\DODApp.ice

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice $(InputPath)

"DODApp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"DODApp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "JMSDispatch - Win32 Release"

# Begin Custom Build
InputPath=..\..\..\ice\DODApp.ice

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)/DataOnDemand/Ice $(ZQProjsPath)/DataOnDemand/Ice/DODApp.ice

"DODApp.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"DODApp.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
