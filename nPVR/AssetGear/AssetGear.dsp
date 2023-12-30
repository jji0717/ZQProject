# Microsoft Developer Studio Project File - Name="AssetGear" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=AssetGear - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AssetGear.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AssetGear.mak" CFG="AssetGear - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AssetGear - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "AssetGear - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/nPVR/AssetGear", TAOAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AssetGear - Win32 Release"

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
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /Od /I "$(ITVSDKPATH)/include" /I "../../Common" /I "../../ISA/ISA_15" /I "../../ISA/common" /I "$(ACE_ROOT)/TAO/include" /I "$(ACE_ROOT)/TAO/include/TAO/ORBSVCS" /I "." /I ".." /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "UNICODE" /D "_UNICODE" /D _WIN32_WINNT=0x0400 /D "_AFXDLL" /D "SOAP_DEBUG" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "$(ZQProjsPath)/build" /i "$(ITVSDKPATH)/" /i "../" /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 common15.lib TAO_Strategies.lib TAO_PortableServer.lib TAO_CosNaming.lib TAO_CosNotification.lib TAO_CosEvent.lib TAO.lib ace.lib reporter.lib manPkgU.lib idsapi.lib cfgpkgU.lib appshell.lib /nologo /subsystem:console /pdb:"../exe/AssetGear.pdb" /debug /machine:I386 /out:"../exe/AssetGear.exe" /pdbtype:sept /libpath:"../../ISA/exe" /libpath:"$(ITVSDKPATH)/lib/Release" /libpath:"$(ACE_ROOT)/TAO/LIB"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "AssetGear - Win32 Debug"

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
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "$(ITVSDKPATH)/include" /I "../../Common" /I "../../ISA/ISA_15" /I "../../ISA/common" /I "$(ACE_ROOT)/TAO/include" /I "$(ACE_ROOT)/TAO/include/TAO/ORBSVCS" /I "." /I ".." /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_AFXDLL" /D "UNICODE" /D "_UNICODE" /D _WIN32_WINNT=0x0400 /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "$(ZQProjsPath)/build" /i "$(ITVSDKPATH)/" /i "../" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 common15_d.lib TAO_Messagingd.lib TAO_CosNotificationd.lib TAO_Strategiesd.lib TAO_PortableServerd.lib TAO_CosNamingd.lib TAO_CosEventd.lib TAOd.lib aced.lib appshell_d.lib DllExport_d.lib cfgpkgU_d.lib CLog_d.lib manPkgU_d.lib reporter_d.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:"../exe/AssetGear_d.pdb" /debug /machine:I386 /nodefaultlib:"pmC.obj" /out:"../exe/AssetGear_d.exe" /libpath:"../../ISA/exe" /libpath:"$(ITVSDKPATH)/lib/Debug" /libpath:"$(ACE_ROOT)/TAO/LIB"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "AssetGear - Win32 Release"
# Name "AssetGear - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\asset_impl.cpp
# End Source File
# Begin Source File

SOURCE=.\assetfactory_impl.cpp
# End Source File
# Begin Source File

SOURCE=.\AssetGear_main.cpp
# End Source File
# Begin Source File

SOURCE=.\AssetGear_soap.cpp
# End Source File
# Begin Source File

SOURCE=.\AssetGear_svr.cpp
# End Source File
# Begin Source File

SOURCE=.\ContentRoutine.cpp
# End Source File
# Begin Source File

SOURCE=.\metadatalist_impl.cpp
# End Source File
# Begin Source File

SOURCE=.\metadatalistfactory_impl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\MiniDump.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\NativeThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\NativeThreadPool.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\Semaphore.cpp
# End Source File
# Begin Source File

SOURCE=.\soapC.cpp
# End Source File
# Begin Source File

SOURCE=.\soapServer.cpp
# End Source File
# Begin Source File

SOURCE=..\stdsoap2.cpp
# End Source File
# Begin Source File

SOURCE=.\SystemMd.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\asset_impl.h
# End Source File
# Begin Source File

SOURCE=.\AssetGear_soap.h
# End Source File
# Begin Source File

SOURCE=..\AssetGearService.h

!IF  "$(CFG)" == "AssetGear - Win32 Release"

# Begin Custom Build
InputPath=..\AssetGearService.h

BuildCmds= \
	$(GSOAPPATH)/soapcpp2.exe -I$(GSOAPPATH) $(InputPath)

"soapStub.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapH.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapC.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapClient.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapServer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapClientLib.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapServerLib.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapAssetGearServiceSoapBindingProxy.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapAssetGearServiceSoapBindingObject.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"AssetGearServiceSoapBinding.createAsset.req.xml" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"AssetGearServiceSoapBinding.createAsset.res.xml" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"AssetGearServiceSoapBinding.nsmap" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "AssetGear - Win32 Debug"

# Begin Custom Build
InputPath=..\AssetGearService.h

BuildCmds= \
	$(GSOAPPATH)/soapcpp2.exe -I$(GSOAPPATH) $(InputPath)

"soapStub.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapH.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapC.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapClient.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapServer.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapClientLib.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapServerLib.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapAssetGearServiceSoapBindingProxy.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"soapAssetGearServiceSoapBindingObject.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"AssetGearServiceSoapBinding.createAsset.req.xml" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"AssetGearServiceSoapBinding.createAsset.res.xml" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"AssetGearServiceSoapBinding.nsmap" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ContentRoutine.h
# End Source File
# Begin Source File

SOURCE=.\metadata_impl.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\MiniDump.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\NativeThread.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\NativeThreadPool.h
# End Source File
# Begin Source File

SOURCE=..\PMClient.h
# End Source File
# Begin Source File

SOURCE=.\soapAssetGearServiceSoapBindingObject.h
# End Source File
# Begin Source File

SOURCE=.\soapH.h
# End Source File
# Begin Source File

SOURCE=..\stdsoap2.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\AssetGear.rc
# End Source File
# End Group
# Begin Group "wsdl"

# PROP Default_Filter "wsdl"
# Begin Source File

SOURCE=..\AssetGearService.wsdl

!IF  "$(CFG)" == "AssetGear - Win32 Release"

# Begin Custom Build
InputPath=..\AssetGearService.wsdl

"../AssetGearService.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%GSOAPPATH%\wsdl2h.exe -t%GSOAPPATH%\typemap.dat -f $(InputPath) -NAssetGear -nAssetGear

# End Custom Build

!ELSEIF  "$(CFG)" == "AssetGear - Win32 Debug"

# Begin Custom Build
InputPath=..\AssetGearService.wsdl

"../AssetGearService.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%GSOAPPATH%\wsdl2h.exe -t%GSOAPPATH%\typemap.dat -f $(InputPath) -NAssetGear -nAssetGear

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\AssetGear.createAsset.req.xml
# End Source File
# End Target
# End Project
