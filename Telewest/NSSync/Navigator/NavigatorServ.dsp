# Microsoft Developer Studio Project File - Name="NavigatorServ" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=NavigatorServ - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NavigatorServ.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NavigatorServ.mak" CFG="NavigatorServ - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NavigatorServ - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "NavigatorServ - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NavigatorServ - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "_AFXDLL" /D "_MBCS" /Gm PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD CPP /nologo /MDd /ZI /W3 /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "_AFXDLL" /D "_MBCS" /Gm PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 
# ADD RSC /l 1033 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  odbc32.lib /nologo /out:"Debug\Navigator.exe" /incremental:yes /debug /pdb:"Debug\Navigator.pdb" /pdbtype:sept /subsystem:console /machine:ix86 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  odbc32.lib /nologo /out:"Debug\Navigator.exe" /incremental:yes /debug /pdb:"Debug\Navigator.pdb" /pdbtype:sept /subsystem:console /machine:ix86 

!ELSEIF  "$(CFG)" == "NavigatorServ - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseServ"
# PROP BASE Intermediate_Dir "ReleaseServ"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseServ"
# PROP Intermediate_Dir "ReleaseServ"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /Zi /W3 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_AFXDLL" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /c /GX 
# ADD CPP /nologo /MD /Zi /W3 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_AFXDLL" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1033 
# ADD RSC /l 1033 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  odbc32.lib /nologo /out:"ReleaseServ\NSSync.exe" /incremental:no /debug /pdbtype:sept /subsystem:console /opt:ref /opt:icf /machine:ix86 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  odbc32.lib /nologo /out:"ReleaseServ\NSSync.exe" /incremental:no /debug /pdbtype:sept /subsystem:console /opt:ref /opt:icf /machine:ix86 

!ENDIF

# Begin Target

# Name "NavigatorServ - Win32 Debug"
# Name "NavigatorServ - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;def;odl;idl;hpj;bat;asm;asmx"
# Begin Source File

SOURCE=$(ZQProjsPath)\Common\BaseSchangeServiceApplication.cpp
# End Source File
# Begin Source File

SOURCE=.\NavigationService.cpp
# End Source File
# Begin Source File

SOURCE=.\nsBuilder.cpp
# End Source File
# Begin Source File

SOURCE=.\nsOptimizer.cpp
# End Source File
# Begin Source File

SOURCE=.\nsTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\nsTimerChecker.cpp
# End Source File
# Begin Source File

SOURCE=$(ZQProjsPath)\Common\SchangeServiceAppMain.cpp
# End Source File
# Begin Source File

SOURCE=$(ZQProjsPath)\Common\ScReporter.cpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc;xsd"
# Begin Source File

SOURCE=$(ZQProjsPath)\Common\BaseSchangeServiceApplication.h
# End Source File
# Begin Source File

SOURCE=.\NavigationService.h
# End Source File
# Begin Source File

SOURCE=.\ns_def.h
# End Source File
# Begin Source File

SOURCE=.\nsBuilder.h
# End Source File
# Begin Source File

SOURCE=.\nsOptimizer.h
# End Source File
# Begin Source File

SOURCE=.\nsTimer.h
# End Source File
# Begin Source File

SOURCE=.\nsTimerChecker.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=$(ZQProjsPath)\Common\ScReporter.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx"
# Begin Source File

SOURCE=.\Navigator.rc
# End Source File
# End Group
# Begin Group "NAV"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\navRebuilder.cpp
# End Source File
# Begin Source File

SOURCE=.\navRebuilder.h
# End Source File
# Begin Source File

SOURCE=.\navRemover.cpp
# End Source File
# Begin Source File

SOURCE=.\navRemover.h
# End Source File
# Begin Source File

SOURCE=.\navRenamer.cpp
# End Source File
# Begin Source File

SOURCE=.\navRenamer.h
# End Source File
# Begin Source File

SOURCE=.\navSplUpdater.cpp
# End Source File
# Begin Source File

SOURCE=.\navSplUpdater.h
# End Source File
# Begin Source File

SOURCE=.\navWorker.cpp
# End Source File
# Begin Source File

SOURCE=.\navWorker.h
# End Source File
# Begin Source File

SOURCE=.\navWorkerProvider.cpp
# End Source File
# Begin Source File

SOURCE=.\navWorkerProvider.h
# End Source File
# End Group
# Begin Group "DB"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CNAVTable.cpp
# End Source File
# Begin Source File

SOURCE=.\CNAVTable.h
# End Source File
# Begin Source File

SOURCE=.\WQ_INFO.cpp
# End Source File
# Begin Source File

SOURCE=.\WQ_INFO.h
# End Source File
# End Group
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=$(ZQProjsPath)\Common\Exception.cpp
# End Source File
# Begin Source File

SOURCE=$(ZQProjsPath)\Common\Exception.h
# End Source File
# Begin Source File

SOURCE=$(ZQProjsPath)\Common\Locks.h
# End Source File
# Begin Source File

SOURCE=$(ZQProjsPath)\Common\Log.cpp
# End Source File
# Begin Source File

SOURCE=$(ZQProjsPath)\Common\Log.h
# End Source File
# Begin Source File

SOURCE=$(ZQProjsPath)\Common\ScLog.cpp
# End Source File
# Begin Source File

SOURCE=$(ZQProjsPath)\Common\ScLog.h
# End Source File
# Begin Source File

SOURCE=$(ZQProjsPath)\Common\Thread.cpp
# End Source File
# Begin Source File

SOURCE=$(ZQProjsPath)\Common\Thread.h
# End Source File
# Begin Source File

SOURCE=$(ZQProjsPath)\Common\ZQ_common_conf.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project

