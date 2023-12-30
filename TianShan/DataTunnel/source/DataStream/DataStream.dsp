# Microsoft Developer Studio Project File - Name="DataStream" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=DataStream - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DataStream.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DataStream.mak" CFG="DataStream - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DataStream - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "DataStream - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "$/ZQProjs/DataOnDemand/Phase2/DataStream"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DataStream - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /I "." /I "$(ExpatPath)/include" /I "$(STLPORT_ROOT)" /I "$(ZQPROJSPATH)/common" /I "$(ICE_ROOT)\include" /I "../common" /I "$(ZQPROJSPATH)/TianShan/ice" /I "$(ZQPROJSPATH)/TianShan/include" /I "$(ZQPROJSPATH)/TianShan/Shell/zqcfgpkg" /I "$(ZQPROJSPATH)/TianShan/Shell/zqsnmppkg" /I "$(ZQPROJSPATH)/TianShan/Shell/ZQSNMPManPkg" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D _WIN32_WINNT=0x0500 /D "_STLP_NEW_PLATFORM_SDK" /Yu"stdafx.h" /FD /c
# SUBTRACT CPP /O<none>
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ice.lib iceutil.lib /nologo /subsystem:console /profile /debug /machine:I386 /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ZQPROJSPATH)/TianShan/bin" /HEAP:0xC800000,0xC80000

!ELSEIF  "$(CFG)" == "DataStream - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I "$(ExpatPath)/include" /I "$(STLPORT_ROOT)" /I "$(ZQPROJSPATH)/common" /I "$(ICE_ROOT)\include" /I "../common" /I "$(ZQPROJSPATH)/TianShan/ice" /I "$(ZQPROJSPATH)/TianShan/include" /I "$(ZQPROJSPATH)/TianShan/Shell/zqcfgpkg" /I "$(ZQPROJSPATH)/TianShan/Shell/zqsnmppkg" /I "$(ZQPROJSPATH)/TianShan/Shell/ZQSNMPManPkg" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D _WIN32_WINNT=0x0500 /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib iced.lib iceutild.lib /nologo /subsystem:console /profile /debug /machine:I386 /out:"Debug/DataStream_d.exe" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ZQPROJSPATH)/TianShan/bin" /HEAP:0xC800000,0xC80000

!ENDIF 

# Begin Target

# Name "DataStream - Win32 Release"
# Name "DataStream - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\Common\BaseZQServiceApplication.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\BufferManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\ConfigLoader.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\DataDebug.cpp
# End Source File
# Begin Source File

SOURCE=.\DataDef.cpp
# End Source File
# Begin Source File

SOURCE=.\DataDistributer.cpp
# End Source File
# Begin Source File

SOURCE=.\DataProfile.cpp
# End Source File
# Begin Source File

SOURCE=.\DataPusher.cpp
# End Source File
# Begin Source File

SOURCE=.\DataQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\DataReader.cpp
# End Source File
# Begin Source File

SOURCE=.\DataSender.cpp
# End Source File
# Begin Source File

SOURCE=.\DataSource.cpp
# End Source File
# Begin Source File

SOURCE=.\DataStream.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\DataStream.rc
# ADD BASE RSC /l 0x804
# ADD RSC /l 0x804 /i "$(ZQProjsPath)/build"
# End Source File
# Begin Source File

SOURCE=.\DataStreamImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\DataStreamMain.cpp
# End Source File
# Begin Source File

SOURCE=.\DataStreamServiceImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\IceService.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\MiniDump.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\MuxItemImpl.cpp
# End Source File
# Begin Source File

SOURCE=.\NullPacketSource.cpp
# End Source File
# Begin Source File

SOURCE=.\PsiPusher.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=..\Common\TsEncoder.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\ZQServiceAppMain.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\Common\BaseZQServiceApplication.h
# End Source File
# Begin Source File

SOURCE=.\BufferManager.h
# End Source File
# Begin Source File

SOURCE=.\DataDebug.h
# End Source File
# Begin Source File

SOURCE=.\DataDef.h
# End Source File
# Begin Source File

SOURCE=.\DataDistributer.h
# End Source File
# Begin Source File

SOURCE=.\DataProfile.h
# End Source File
# Begin Source File

SOURCE=.\DataPusher.h
# End Source File
# Begin Source File

SOURCE=.\DataQueue.h
# End Source File
# Begin Source File

SOURCE=.\DataReader.h
# End Source File
# Begin Source File

SOURCE=.\DataSender.h
# End Source File
# Begin Source File

SOURCE=.\DataSource.h
# End Source File
# Begin Source File

SOURCE=.\DataStream.h
# End Source File
# Begin Source File

SOURCE=.\DataStreamImpl.h
# End Source File
# Begin Source File

SOURCE=.\DataStreamServiceImpl.h
# End Source File
# Begin Source File

SOURCE=.\IceService.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\MiniDump.h
# End Source File
# Begin Source File

SOURCE=.\MuxItemImpl.h
# End Source File
# Begin Source File

SOURCE=.\NullPacketSource.h
# End Source File
# Begin Source File

SOURCE=.\PsiPusher.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\Common\TsEncoder.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Ice"

# PROP Default_Filter "ice"
# Begin Source File

SOURCE=..\..\ice\DataStream.ice

!IF  "$(CFG)" == "DataStream - Win32 Release"

# Begin Custom Build
InputPath=..\..\ice\DataStream.ice

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice $(InputPath)

"DataStream.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"DataStream.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "DataStream - Win32 Debug"

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
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
