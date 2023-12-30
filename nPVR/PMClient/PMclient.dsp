# Microsoft Developer Studio Project File - Name="PMclient" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PMclient - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PMclient.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PMclient.mak" CFG="PMclient - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PMclient - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PMclient - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PMclient - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PMCLIENT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /Od /I "." /I ".." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PMCLIENT_EXPORTS" /D "SOAP_DEBUG" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "$(ZQProjsPath)/build" /i "$(ITVSDKPATH)/" /i "../" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /dll /pdb:"../exe/PMclient.pdb" /debug /machine:I386 /out:"../exe/PMclient.dll"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "PMclient - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PMCLIENT_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /I ".." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "PMCLIENT_EXPORTS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "$(ZQProjsPath)/build" /i "$(ITVSDKPATH)/" /i "../" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /dll /pdb:"../exe/PMclient_d.pdb" /debug /machine:I386 /out:"../exe/PMclient_d.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "PMclient - Win32 Release"
# Name "PMclient - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\PMClient.cpp
# End Source File
# Begin Source File

SOURCE=.\soapC.cpp
# End Source File
# Begin Source File

SOURCE=.\soapClient.cpp
# End Source File
# Begin Source File

SOURCE=..\stdsoap2.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\PMClient.h
# End Source File
# Begin Source File

SOURCE=..\PMService.h

!IF  "$(CFG)" == "PMclient - Win32 Release"

# Begin Custom Build
InputPath=..\PMService.h

BuildCmds= \
	$(GSOAPPATH)/soapcpp2.exe -I$(GSOAPPATH) $(InputPath)

"ProgramManagementServiceSoapBinding.nsmap" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ProgramManagementServiceSoapBinding.queryMetaData.req.xml" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ProgramManagementServiceSoapBinding.queryMetaData.res.xml" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapC.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapClient.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapClientLib.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapH.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapProgramManagementServiceSoapBindingObject.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapProgramManagementServiceSoapBindingProxy.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapServer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapServerLib.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapStub.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "PMclient - Win32 Debug"

# Begin Custom Build
InputPath=..\PMService.h

BuildCmds= \
	$(GSOAPPATH)/soapcpp2.exe -I$(GSOAPPATH) $(InputPath)

"ProgramManagementServiceSoapBinding.nsmap" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ProgramManagementServiceSoapBinding.queryMetaData.req.xml" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ProgramManagementServiceSoapBinding.queryMetaData.res.xml" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapC.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapClient.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapClientLib.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapH.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapProgramManagementServiceSoapBindingObject.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapProgramManagementServiceSoapBindingProxy.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapServer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapServerLib.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapStub.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\soapProgramManagementServiceSoapBindingProxy.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\PMclient.rc
# End Source File
# End Group
# Begin Group "wsdl"

# PROP Default_Filter "wsdl"
# Begin Source File

SOURCE=..\PMService.wsdl

!IF  "$(CFG)" == "PMclient - Win32 Release"

# Begin Custom Build
InputPath=..\PMService.wsdl

"../PMService.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%GSOAPPATH%\wsdl2h.exe -t%GSOAPPATH%\typemap.dat -f $(InputPath) -Npm -npm

# End Custom Build

!ELSEIF  "$(CFG)" == "PMclient - Win32 Debug"

# Begin Custom Build
InputPath=..\PMService.wsdl

"../PMService.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%GSOAPPATH%\wsdl2h.exe -t%GSOAPPATH%\typemap.dat -f $(InputPath) -Npm -npm

# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
