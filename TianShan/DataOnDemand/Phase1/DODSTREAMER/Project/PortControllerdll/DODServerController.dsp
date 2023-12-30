# Microsoft Developer Studio Project File - Name="DODServerController" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=DODServerController - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DODServerController.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DODServerController.mak" CFG="DODServerController - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DODServerController - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DODServerController - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/DOD Server/Project/PortControllerdll", VEZAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DODServerController - Win32 Debug"

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
# ADD BASE CPP /nologo /MDd /W3 /Gm /ZI /Od /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /D "_AFXEXT" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /D "_AFXEXT" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
# ADD BASE RSC /l 0x804 /i "$(IntDir)" /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(IntDir)" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 Shlwapi.lib vfw32.lib quartz.lib uuid.lib strmiids.lib strmbasd.lib msvcrtd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib winmm.lib msacm32.lib olepro32.lib oleaut32.lib advapi32.lib comsupp.lib /nologo /subsystem:windows /debug /machine:IX86 /out:"debug\DODServerController.dll" /implib:"$(OutDir)/DODServerController.lib" /pdbtype:sept
# ADD LINK32 Shlwapi.lib vfw32.lib quartz.lib uuid.lib strmiids.lib strmbasd.lib msvcrtd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib winmm.lib msacm32.lib olepro32.lib oleaut32.lib advapi32.lib comsupp.lib /nologo /subsystem:windows /debug /machine:IX86 /out:"debug\DODServerController.dll" /implib:"$(OutDir)/DODServerController.lib" /pdbtype:sept

!ELSEIF  "$(CFG)" == "DODServerController - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Zi /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_AFXEXT" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /Zi /D "WIN32" /D "_WINDOWS" /D "NDEBUG" /D "_AFXEXT" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
# ADD BASE RSC /l 0x804 /i "$(IntDir)" /d "NDEBUG"
# ADD RSC /l 0x804 /i "$(IntDir)" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:IX86 /out:"$(OutDir)/DODServerController.dll" /implib:"$(OutDir)/DODServerController.lib" /pdbtype:sept /opt:ref /opt:icf
# ADD LINK32 /nologo /subsystem:windows /debug /machine:IX86 /out:"$(OutDir)/DODServerController.dll" /implib:"$(OutDir)/DODServerController.lib" /pdbtype:sept /opt:ref /opt:icf

!ENDIF 

# Begin Target

# Name "DODServerController - Win32 Debug"
# Name "DODServerController - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;def;odl;idl;hpj;bat;asm;asmx"
# Begin Source File

SOURCE=.\clog.cpp
DEP_CPP_CLOG_=\
	".\clog.h"\
	".\stdafx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\common.cpp
DEP_CPP_COMMO=\
	".\common.h"\
	".\stdafx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DODChannel.cpp
DEP_CPP_DODCH=\
	".\BroadcastGuid.h"\
	".\clog.h"\
	".\common.h"\
	".\DODChannel.h"\
	".\DODPort.h"\
	".\DODPortCtrl.h"\
	".\interfaceDefination.h"\
	".\IObjectControl.h"\
	".\msxmldom.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"msxml2.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DODPort.cpp
DEP_CPP_DODPO=\
	".\DODPort.h"\
	".\stdafx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DODPortCtrl.cpp
DEP_CPP_DODPOR=\
	".\BroadcastGuid.h"\
	".\clog.h"\
	".\common.h"\
	".\DODChannel.h"\
	".\DODPort.h"\
	".\DODPortCtrl.h"\
	".\interfaceDefination.h"\
	".\IObjectControl.h"\
	".\msxmldom.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"msxml2.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DODServerController.cpp
DEP_CPP_DODSE=\
	".\BroadcastGuid.h"\
	".\clog.h"\
	".\common.h"\
	".\DODChannel.h"\
	".\DODPort.h"\
	".\DODPortCtrl.h"\
	".\DODServerController.h"\
	".\DODSubChannel.h"\
	".\interfaceDefination.h"\
	".\IObjectControl.h"\
	".\msxmldom.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"msxml2.h"\
	
# End Source File
# Begin Source File

SOURCE=.\DODServerController.def
# End Source File
# Begin Source File

SOURCE=.\DODSubChannel.cpp
DEP_CPP_DODSU=\
	".\DODSubChannel.h"\
	".\stdafx.h"\
	
# End Source File
# Begin Source File

SOURCE=.\msxmldom.cpp
DEP_CPP_MSXML=\
	".\msxmldom.h"\
	".\stdafx.h"\
	{$(INCLUDE)}"msxml2.h"\
	
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
DEP_CPP_STDAF=\
	".\stdafx.h"\
	

!IF  "$(CFG)" == "DODServerController - Win32 Debug"

# ADD CPP /nologo /Yc"stdafx.h" /GZ

!ELSEIF  "$(CFG)" == "DODServerController - Win32 Release"

# ADD CPP /nologo /Yc"stdafx.h"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc;xsd"
# Begin Source File

SOURCE=.\BroadcastGuid.h
# End Source File
# Begin Source File

SOURCE=.\clog.h
# End Source File
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\DODChannel.h
# End Source File
# Begin Source File

SOURCE=.\DODPort.h
# End Source File
# Begin Source File

SOURCE=.\DODPortCtrl.h
# End Source File
# Begin Source File

SOURCE=.\DODServerController.h
# End Source File
# Begin Source File

SOURCE=.\DODSubChannel.h
# End Source File
# Begin Source File

SOURCE=.\interfaceDefination.h
# End Source File
# Begin Source File

SOURCE=.\IObjectControl.h
# End Source File
# Begin Source File

SOURCE=.\msxmldom.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx"
# Begin Source File

SOURCE=.\DODServerController.rc
# End Source File
# Begin Source File

SOURCE=.\res\DODServerController.rc2
# End Source File
# End Group
# End Target
# End Project
