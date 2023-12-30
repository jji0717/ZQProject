# Microsoft Developer Studio Project File - Name="McastRcv" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=McastRcv - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "McastRcv.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "McastRcv.mak" CFG="McastRcv - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "McastRcv - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "McastRcv - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/MulticastForwarding", YBBAAAAA"
# PROP Scc_LocalPath ".."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "McastRcv - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /tlb".\Release\McastRcv.tlb" /win32
# ADD MTL /nologo /tlb".\Release\McastRcv.tlb" /win32
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "." /I "../../Common" /I "$(ITVSDKPATH)/include" /I "C:\IPv6Kit\inc" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "IPV6_ENABLED" /D "_MBCS" /YX /GF /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "C:\IPv6Kit\inc" /I "." /I "../../Common" /I "$(ITVSDKPATH)/include" /I "../inc" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "IPV6_ENABLED" /D "_MBCS" /YX /GF /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:IX86 /out:"c:\mcastfwd\bin\McastRcv.exe" /pdbtype:sept /libpath:"$(ITVSDKPATH)/lib/Release"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:IX86 /out:"c:\mcastfwd\bin\McastRcv.exe" /pdbtype:sept /libpath:"$(ITVSDKPATH)/lib/Release"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "McastRcv - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE MTL /nologo /tlb".\Debug\McastRcv.tlb" /win32
# ADD MTL /nologo /tlb".\Debug\McastRcv.tlb" /win32
# ADD BASE CPP /nologo /MDd /W3 /GX /ZI /Od /I "." /I "../../Common" /I "$(ITVSDKPATH)/include" /I "c:\IPv6Kit\inc" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "IPV6_ENABLED" /D "_MBCS" /GZ
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I "." /I "../../Common" /I "$(ITVSDKPATH)/include" /I "../inc" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "IPV6_ENABLED" /D "UNICODE" /D "_UNICODE" /D "_AFXDLL" /D "__FUNCTION__" /GZ
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "E:\Project\IPv6Kit\inc" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /debug /machine:IX86 /pdbtype:sept /libpath:"$(ITVSDKPATH)/lib/Debug"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /debug /machine:IX86 /pdbtype:sept /libpath:"$(ITVSDKPATH)/lib/Debug"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "McastRcv - Win32 Release"
# Name "McastRcv - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\Common\Exception.cpp

!IF  "$(CFG)" == "McastRcv - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "McastRcv - Win32 Debug"

# ADD CPP /nologo /GX /Od /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Common\getopt.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\InetAddr.cpp

!IF  "$(CFG)" == "McastRcv - Win32 Release"

# ADD CPP /nologo /GX /O2

!ELSEIF  "$(CFG)" == "McastRcv - Win32 Debug"

# ADD CPP /nologo /GX /Od /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Common\Log.cpp
# End Source File
# Begin Source File

SOURCE=..\McastListener.cpp
# End Source File
# Begin Source File

SOURCE=.\McastRcv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\Socket.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\Thread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\UDPSocket.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Common\Exception.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\getopt.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\InetAddr.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Locks.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Log.h
# End Source File
# Begin Source File

SOURCE=..\McastListener.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Socket.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\Thread.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\UDPSocket.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project