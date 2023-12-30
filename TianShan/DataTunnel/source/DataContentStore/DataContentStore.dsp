# Microsoft Developer Studio Project File - Name="DODContentStore" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=DODContentStore - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DODContentStore.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DODContentStore.mak" CFG="DODContentStore - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DODContentStore - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "DODContentStore - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DODContentStore - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /Od /I "." /I ".." /I "$(ICE_ROOT)/include/stlport" /I "$(ZQPROJSPATH)\Common" /I "$(ZQProjsPath)/TianShan/common" /I "$(ZQProjsPath)/TianShan/include" /I "$(ICE_ROOT)/include" /I "$(ZQPROJSPATH)\Generic\ContentProcess" /I "$(ZQProjsPath)/TianShan\Ice" /I "itv_parse\Parser\inc" /I "itv_parse\z_lib\inc" /I "$(ZQProjsPath)\TianShan\Shell\ZQSNMPManPkg" /I "$(ZQProjsPath)\TianShan\Shell\ZQCfgPkg" /I "$(ExpatPath)\Include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "_STLP_NEW_PLATFORM_SDK" /D _WIN32_WINNT=0x0500 /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)/build" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Ice.lib IceUtil.lib Freeze.lib IceStorm.lib Ws2_32.lib /nologo /subsystem:console /incremental:yes /debug /machine:I386 /libpath:"$(ICE_ROOT)/lib"

!ELSEIF  "$(CFG)" == "DODContentStore - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I ".." /I "$(ICE_ROOT)/include/stlport" /I "$(ZQPROJSPATH)\Common" /I "$(ZQProjsPath)/TianShan/common" /I "$(ZQProjsPath)/TianShan/include" /I "$(ICE_ROOT)/include" /I "$(ZQPROJSPATH)\Generic\ContentProcess" /I "$(ZQProjsPath)/TianShan\Ice" /I "itv_parse\Parser\inc" /I "itv_parse\z_lib\inc" /I "$(ZQProjsPath)\TianShan\Shell\ZQSNMPManPkg" /I "$(ZQProjsPath)\TianShan\Shell\ZQCfgPkg" /I "$(ExpatPath)\Include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /D _WIN32_WINNT=0x0500 /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)/build" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 User32.lib Iced.lib IceUtild.lib Freezed.lib IceStormd.lib Ws2_32.lib Ws2_32.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /out:"Debug/DODContentStore_d.exe" /pdbtype:sept /libpath:"$(ICE_ROOT)/lib"

!ENDIF 

# Begin Target

# Name "DODContentStore - Win32 Release"
# Name "DODContentStore - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\Common\BaseZQServiceApplication.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Generic\ContentProcess\bufferpool.cpp

!IF  "$(CFG)" == "DODContentStore - Win32 Release"

!ELSEIF  "$(CFG)" == "DODContentStore - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\Common\ConfigLoader.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\DataTsWrapperRender.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\DODContentStore.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\DODContentStore.rc
# End Source File
# Begin Source File

SOURCE=.\DODContentStoreImpl.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\DODContentStoreServ.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\DODDataSource.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\DODGraphFactory.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Generic\ContentProcess\GraphFilter.cpp

!IF  "$(CFG)" == "DODContentStore - Win32 Release"

!ELSEIF  "$(CFG)" == "DODContentStore - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\Generic\ContentProcess\GraphPool.cpp

!IF  "$(CFG)" == "DODContentStore - Win32 Release"

!ELSEIF  "$(CFG)" == "DODContentStore - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\TianShan\common\IceLog.cpp

!IF  "$(CFG)" == "DODContentStore - Win32 Release"

!ELSEIF  "$(CFG)" == "DODContentStore - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Log.cpp

!IF  "$(CFG)" == "DODContentStore - Win32 Release"

!ELSEIF  "$(CFG)" == "DODContentStore - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\Common\MiniDump.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\Generic\ContentProcess\NTFSFileIORender.cpp

!IF  "$(CFG)" == "DODContentStore - Win32 Release"

!ELSEIF  "$(CFG)" == "DODContentStore - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

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

SOURCE=..\..\..\Generic\ContentProcess\bufferpool.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\ConfigLoader.h
# End Source File
# Begin Source File

SOURCE=.\DataTsWrapperRender.h
# End Source File
# Begin Source File

SOURCE=.\DODContentStore.h
# End Source File
# Begin Source File

SOURCE=.\DODContentStoreImpl.h
# End Source File
# Begin Source File

SOURCE=.\DODContentStoreServ.h
# End Source File
# Begin Source File

SOURCE=.\DODDataSource.h
# End Source File
# Begin Source File

SOURCE=.\DODGraphFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Generic\ContentProcess\GraphFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Generic\ContentProcess\GraphPool.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\MiniDump.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Generic\ContentProcess\NTFSFileIORender.h
# End Source File
# Begin Source File

SOURCE=.\PrivateDataDefine.h
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
# Begin Source File

SOURCE=..\..\ice\DODContentStore.ice

!IF  "$(CFG)" == "DODContentStore - Win32 Release"

# Begin Custom Build
InputPath=..\..\ice\DODContentStore.ice

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe                       -I$(ICE_ROOT)/slice                               -I$(ZQPROJSPATH)/TianShan/Ice                       ..\..\ice\DODContentStore.ice

"DODContentStore.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"DODContentStore.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "DODContentStore - Win32 Debug"

# Begin Custom Build
InputPath=..\..\ice\DODContentStore.ice

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe                       -I$(ICE_ROOT)/slice                               -I$(ZQPROJSPATH)/TianShan/Ice                       ..\..\ice\DODContentStore.ice

"DODContentStore.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"DODContentStore.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
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
