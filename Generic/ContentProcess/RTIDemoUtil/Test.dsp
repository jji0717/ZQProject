# Microsoft Developer Studio Project File - Name="Test" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Test - Win32 Unicode Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Test.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Test.mak" CFG="Test - Win32 Unicode Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Test - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Test - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "Test - Win32 Unicode Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "Test - Win32 Unicode Release" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Test - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /ZI /Od /I "." /I "../" /I "$(STLPORT_ROOT)/" /I "$(ZQProjsPath)/TianShan/ContentStore/RtfCPNode/RTFLib/" /I "$(ZQProjsPath)/Common" /I "$(WPCAPSDKPATH)/include" /I "$(VSTRMKITPATH)/Sdk/inc" /D _WIN32_WINNT=0x0500 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_CONSOLE" /D "_STLP_NEW_PLATFORM_SDK" /D "WPCAP" /D "HAVE_REMOTE" /FR /FD /I /RtfCPNode/RTFLib/" " /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib advapi32.lib shell32.lib wpcap.lib ws2_32.lib VstrmDLL.lib /nologo /subsystem:console /incremental:yes /debug /machine:I386 /out:"Release/RTIDemoUtil.exe" /libpath:"$(WPCAPSDKPATH)\lib" /libpath:"$(ICE_ROOT)\lib" /libpath:"$(VSTRMKITPATH)/Sdk/lib"

!ELSEIF  "$(CFG)" == "Test - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "../" /I "$(STLPORT_ROOT)/" /I "$(ZQProjsPath)/TianShan/ContentStore/RtfCPNode/RTFLib/" /I "$(ZQProjsPath)/Common" /I "$(WPCAPSDKPATH)/include" /I "$(VSTRMKITPATH)/Sdk/inc" /D _WIN32_WINNT=0x0500 /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_CONSOLE" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /D "WPCAP" /D "HAVE_REMOTE" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib advapi32.lib shell32.lib wpcap.lib ws2_32.lib VstrmDLL.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/RTIDemoUtil_d.exe" /pdbtype:sept /libpath:"$(WPCAPSDKPATH)\lib" /libpath:"$(ICE_ROOT)\lib" /libpath:"$(VSTRMKITPATH)/Sdk/lib"

!ELSEIF  "$(CFG)" == "Test - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Test___Win32_Unicode_Debug"
# PROP BASE Intermediate_Dir "Test___Win32_Unicode_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Unicode_Debug"
# PROP Intermediate_Dir ".\Unicode_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "../" /I "../../" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "UNICODE" /D "_UNICODE" /FD /GZ /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../" /I "../../" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "UNICODE" /D "_UNICODE" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 snmpapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"../../Exe/ZQSNMPTest.exe" /pdbtype:sept /libpath:"../../Lib"
# ADD LINK32 ZQSnmpManPkgU_d.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"../../Exe/ZQSNMPTestU_d.exe" /pdbtype:sept /libpath:"../../lib"

!ELSEIF  "$(CFG)" == "Test - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Test___Win32_Unicode_Release"
# PROP BASE Intermediate_Dir "Test___Win32_Unicode_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Unicode_Release"
# PROP Intermediate_Dir ".\Unicode_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /I "../" /I "../../" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "MBCS" /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /W3 /GX /O2 /I "../" /I "../../" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ZQSNMPManPkg.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"../../Exe/ZQSNMPTest.exe"
# ADD LINK32 ZQSnmpManPkgU.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"../../Exe/ZQSNMPManTestU.exe" /libpath:"../../lib"

!ENDIF 

# Begin Target

# Name "Test - Win32 Release"
# Name "Test - Win32 Debug"
# Name "Test - Win32 Unicode Debug"
# Name "Test - Win32 Unicode Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\bufferpool.cpp
# End Source File
# Begin Source File

SOURCE=.\CommandLine.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\getopt.cpp
# End Source File
# Begin Source File

SOURCE=..\GraphFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\GraphPool.cpp
# End Source File
# Begin Source File

SOURCE=.\MCastGraph.cpp
# End Source File
# Begin Source File

SOURCE=..\MCastIOSource.cpp
# End Source File
# Begin Source File

SOURCE=..\NTFSFileIORender.cpp
# End Source File
# Begin Source File

SOURCE=..\RTFLibFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\Test.cpp
# End Source File
# Begin Source File

SOURCE=..\TrickFileGenFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\VStrmIORender.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\bufferpool.h
# End Source File
# Begin Source File

SOURCE=.\CommandLine.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\getopt.h
# End Source File
# Begin Source File

SOURCE=..\GraphFilter.h
# End Source File
# Begin Source File

SOURCE=..\GraphPool.h
# End Source File
# Begin Source File

SOURCE=.\MCastGraph.h
# End Source File
# Begin Source File

SOURCE=..\MCastIOSource.h
# End Source File
# Begin Source File

SOURCE=..\NTFSFileIORender.h
# End Source File
# Begin Source File

SOURCE=..\RTFLibFilter.h
# End Source File
# Begin Source File

SOURCE=..\TrickFileGenFilter.h
# End Source File
# Begin Source File

SOURCE=..\VStrmIORender.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
