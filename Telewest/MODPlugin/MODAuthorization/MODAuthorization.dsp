# Microsoft Developer Studio Project File - Name="MODAuthorization" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=MODAuthorization - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MODAuthorization.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MODAuthorization.mak" CFG="MODAuthorization - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MODAuthorization - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MODAuthorization - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/Telewest/MODPlugin/MODAuthorization", BOFAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MODAuthorization - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /Od /I "$(ITVSDKPATH)\include" /I "..\..\..\common" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "MACAPIDLL_DEF" /D "_WINDLL" /D "_AFXDLL" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /fo"MODAuthorization.res" /i "$(ZQPROJSPATH)/build" /i "$(ITVSDKPATH)" /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 cfgpkgU.lib Reporter.lib ManPkgU.lib wsock32.lib /nologo /subsystem:windows /dll /pdb:"..\..\..\..\..\Exe\MODAuthorization.pdb" /debug /debugtype:both /machine:I386 /out:"Release/MultiverseAuthorization.dll" /libpath:"$(ITVSDKPATH)\lib\release"
# SUBTRACT LINK32 /pdb:none /incremental:yes /map

!ELSEIF  "$(CFG)" == "MODAuthorization - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "$(ITVSDKPATH)\include" /I "..\..\..\common" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "MACAPIDLL_DEF" /D "_WINDLL" /D "_AFXDLL" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "$(ZQPROJSPATH)/build" /i "$(ITVSDKPATH)" /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 cfgpkgU_d.lib Reporter_d.lib ManPkgU_d.lib wsock32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"..\..\..\..\..\Exe\MODAuthorization_d.pdb" /debug /machine:I386 /out:"Debug\MultiverseAuthorization_d.dll" /libpath:"$(ITVSDKPATH)\lib\debug"
# SUBTRACT LINK32 /pdb:none /map

!ENDIF 

# Begin Target

# Name "MODAuthorization - Win32 Release"
# Name "MODAuthorization - Win32 Debug"
# Begin Group "Resource"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MoDSoapInterface.wsdl

!IF  "$(CFG)" == "MODAuthorization - Win32 Release"

# Begin Custom Build
InputPath=.\MoDSoapInterface.wsdl

"MoDSoapInterface.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	wsdl2h.exe -oMoDSoapInterface.h .\MoDSoapInterface.wsdl

# End Custom Build

!ELSEIF  "$(CFG)" == "MODAuthorization - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\AppSite.cpp
# End Source File
# Begin Source File

SOURCE=.\AppSite.h
# End Source File
# Begin Source File

SOURCE=.\config.cpp
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=.\entry.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Exception.cpp

!IF  "$(CFG)" == "MODAuthorization - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MODAuthorization - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mactst.cpp
# End Source File
# Begin Source File

SOURCE=.\mactst.h
# End Source File
# Begin Source File

SOURCE=.\mod.h
# End Source File
# Begin Source File

SOURCE=.\MODAuthorization.rc

!IF  "$(CFG)" == "MODAuthorization - Win32 Release"

!ELSEIF  "$(CFG)" == "MODAuthorization - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ModAuthReporter.cpp
# End Source File
# Begin Source File

SOURCE=.\ModAuthReporter.h
# End Source File
# Begin Source File

SOURCE=.\MoDSoapInterface.h

!IF  "$(CFG)" == "MODAuthorization - Win32 Release"

# Begin Custom Build
InputPath=.\MoDSoapInterface.h

"MoDSoapInterfaceMoDSoapInterfaceSoapBindingProxy.h MoDSoapInterfaceStub.h MoDSoapInterfaceH.h MoDSoapInterfaceC.cpp MoDSoapInterfaceClient.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	.\soapcpp2.exe -pMoDSoapInterface "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "MODAuthorization - Win32 Debug"

# Begin Custom Build
InputPath=.\MoDSoapInterface.h

"MoDSoapInterfaceStub.h MoDSoapInterfaceH.h MoDSoapInterfaceC.cpp MoDSoapInterfaceClient.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	.\soapcpp2.exe -pMoDSoapInterface "$(InputPath)"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MoDSoapInterfaceC.cpp
# End Source File
# Begin Source File

SOURCE=.\MoDSoapInterfaceClient.cpp
# End Source File
# Begin Source File

SOURCE=.\MoDSoapInterfaceH.h
# End Source File
# Begin Source File

SOURCE=.\MoDSoapInterfaceMoDSoapInterfaceSoapBindingProxy.h
# End Source File
# Begin Source File

SOURCE=.\MoDSoapInterfaceStub.h
# End Source File
# Begin Source File

SOURCE=.\ModSoapWrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\ModSoapWrapper.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\stdsoap2.cpp
# End Source File
# Begin Source File

SOURCE=.\stdsoap2.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\XMLPreference.cpp

!IF  "$(CFG)" == "MODAuthorization - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "MODAuthorization - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Target
# End Project
