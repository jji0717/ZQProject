# Microsoft Developer Studio Project File - Name="ChODSvc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=ChODSvc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ChODSvc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ChODSvc.mak" CFG="ChODSvc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ChODSvc - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ChODSvc - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "$/ZQProjs/TianShan/ChannelOnDemand/ChODSvc"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ChODSvc - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /Od /I "." /I "../" /I "$(ZQProjsPath)/Common" /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/TianShan/common" /I "$(ICE_ROOT)/include" /I "$(ITVSDKPATH)\include" /I "$(ICE_ROOT)/include/stlport" /I "$(ZQProjsPath)/Generic/JMSCppLib" /I "$(ZQProjsPath)/Tianshan/common" /I "$(ZQProjsPath)/Tianshan/Include" /I "$(ZQProjsPath)/TianShan/Shell/ZQCfgPkg" /I "$(ExpatPath)/include" /I "$(ZQProjsPath)/TianShan/Shell/ZQSNMPManPkg" /I "$(RegExppKit)" /D "_TEST_" /D "WIN32" /D "NDEBUG" /D "WITH_ICESTORM" /D "_CONSOLE" /D "XML_STATIC" /D _WIN32_WINNT=0x500 /D "_STLP_NEW_PLATFORM_SDK" /D "_MBCS" /D "_NO_SYSMON" /D "_AFXDLL" /D "NEWLOGFMT" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\..\..\build" /i "$(ITVSDKPATH)" /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 libexpatMT.lib /nologo /subsystem:console /pdb:"../../bin/ChODSvc.pdb" /debug /machine:I386 /nodefaultlib:"LIBC" /out:"../../bin/ChODSvc.exe" /libpath:"$(EXPATPATH)/lib" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ITVSDKPATH)/lib/release" /libpath:"../../../Generic/JMSCppLib/JMSCpp/lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "ChODSvc - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I "../" /I "$(ZQProjsPath)/Common" /I "$(ZQProjsPath)/TianShan/Ice" /I "$(ZQProjsPath)/TianShan/common" /I "$(ICE_ROOT)/include" /I "$(ITVSDKPATH)\include" /I "$(ICE_ROOT)/include/stlport" /I "$(ZQProjsPath)/Generic/JMSCppLib" /I "$(ZQProjsPath)/Tianshan/common" /I "$(ZQProjsPath)/Tianshan/Include" /I "$(ZQProjsPath)/TianShan/Shell/ZQCfgPkg" /I "$(ExpatPath)/include" /I "$(ZQProjsPath)/TianShan/Shell/ZQSNMPManPkg" /D "WIN32" /D "_DEBUG" /D "WITH_ICESTORM" /D "_CONSOLE" /D "XML_STATIC" /D _WIN32_WINNT=0x500 /D "_STLP_DEBUG" /D "_STLP_NEW_PLATFORM_SDK" /D "_MBCS" /D "_NO_SYSMON" /D "_AFXDLL" /D "NEWLOGFMT" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\..\build" /i "$(ITVSDKPATH)" /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 jmsc.lib /nologo /subsystem:console /pdb:"../../bin/ChODSvc.pdb" /debug /machine:I386 /force /out:"../../bin/ChODSvc.exe" /pdbtype:sept /libpath:"$(EXPATPATH)/lib" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(ITVSDKPATH)/lib/debug" /libpath:"$(ITVSDKPATH)/lib/release" /libpath:"../../Generic/JMSCppLib/JMSCpp/lib" /libpath:"$(3RDPARTYKITS_PATH)\JMSCpp\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "ChODSvc - Win32 Release"
# Name "ChODSvc - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\Common\BaseZQServiceApplication.cpp
# End Source File
# Begin Source File

SOURCE=..\Channel2Purchase.cpp
# End Source File
# Begin Source File

SOURCE=..\ChannelItemDict.cpp
# End Source File
# Begin Source File

SOURCE=..\ChannelNameIndex.cpp
# End Source File
# Begin Source File

SOURCE=..\ChannelOnDemand.cpp
# End Source File
# Begin Source File

SOURCE=..\ChannelOnDemandAppImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\ChannelOnDemandEx.cpp
# End Source File
# Begin Source File

SOURCE=..\ChannelPublisherImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\ChannelPublishPointImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\ChODDefines.cpp
# End Source File
# Begin Source File

SOURCE=..\ChODFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\ChODServ.cpp
# End Source File
# Begin Source File

SOURCE=.\ChODSvc.cpp
# End Source File
# Begin Source File

SOURCE=..\ChODSvcEnv.cpp
# End Source File
# Begin Source File

SOURCE=.\CODConfig.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\ConfigHelper.cpp

!IF  "$(CFG)" == "ChODSvc - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "ChODSvc - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\CtrlNum2ItemAssoc.cpp
# End Source File
# Begin Source File

SOURCE=..\IceAsyncSub.cpp
# End Source File
# Begin Source File

SOURCE=..\JmsMsgSender.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\MiniDump.cpp
# End Source File
# Begin Source File

SOURCE=..\PlaylistEventSinkImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\PlaylistId2Purchase.cpp
# End Source File
# Begin Source File

SOURCE=..\Purchase2ItemAssoc.cpp
# End Source File
# Begin Source File

SOURCE=..\PurchaseImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\PurchaseItemImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\PurchaseRequest.cpp
# End Source File
# Begin Source File

SOURCE=..\stroprt.cpp
# End Source File
# Begin Source File

SOURCE=..\todas.cpp
# End Source File
# Begin Source File

SOURCE=..\WatchDog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\ZQServiceAppMain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Channel2Purchase.h
# End Source File
# Begin Source File

SOURCE=..\ChannelItemDict.h
# End Source File
# Begin Source File

SOURCE=..\ChannelNameIndex.h
# End Source File
# Begin Source File

SOURCE=..\ChannelOnDemand.h
# End Source File
# Begin Source File

SOURCE=..\ChODFactory.h
# End Source File
# Begin Source File

SOURCE=.\ChODServ.h
# End Source File
# Begin Source File

SOURCE=..\ChODSvcEnv.h
# End Source File
# Begin Source File

SOURCE=.\CODConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\ConfigHelper.h
# End Source File
# Begin Source File

SOURCE=..\CtrlNum2ItemAssoc.h
# End Source File
# Begin Source File

SOURCE=..\JmsMsgSender.h
# End Source File
# Begin Source File

SOURCE=..\PlaylistEventSinkImpl.h
# End Source File
# Begin Source File

SOURCE=..\PlaylistId2Purchase.h
# End Source File
# Begin Source File

SOURCE=..\Purchase2ItemAssoc.h
# End Source File
# Begin Source File

SOURCE=..\PurchaseImpl.h
# End Source File
# Begin Source File

SOURCE=..\PurchaseItemImpl.h
# End Source File
# Begin Source File

SOURCE=..\PurchaseRequest.h
# End Source File
# Begin Source File

SOURCE=..\stroprt.h
# End Source File
# Begin Source File

SOURCE=..\todas.h
# End Source File
# Begin Source File

SOURCE=..\WatchDog.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\ChannelOnDemand.ICE

!IF  "$(CFG)" == "ChODSvc - Win32 Release"

# Begin Custom Build
InputPath=..\ChannelOnDemand.ICE
InputName=ChannelOnDemand

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --output-dir .. ..\$(InputName).ice

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ChODSvc - Win32 Debug"

# Begin Custom Build
InputPath=..\ChannelOnDemand.ICE
InputName=ChannelOnDemand

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --output-dir .. ..\$(InputName).ice

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ChannelOnDemand.rc

!IF  "$(CFG)" == "ChODSvc - Win32 Release"

# ADD BASE RSC /l 0x804 /i "\ZQProjs\tianshan\ChannelOnDemand" /i "\ZQProjs1.7.5\TianShan\ChannelOnDemand" /i "\ZQProjs1.7\TianShan\ChannelOnDemand" /i "\projects\ZQProjs\TianShan\ChannelOnDemand"
# SUBTRACT BASE RSC /i "..\..\..\build" /i "$(ITVSDKPATH)"
# ADD RSC /l 0x804 /i "\ZQProjs\tianshan\ChannelOnDemand" /i "\ZQProjs1.7.5\TianShan\ChannelOnDemand" /i "\ZQProjs1.7\TianShan\ChannelOnDemand" /i "." /i "$(ZQProjsPath)/build" /i "\projects\ZQProjs\TianShan\ChannelOnDemand"
# SUBTRACT RSC /i "..\..\..\build" /i "$(ITVSDKPATH)"

!ELSEIF  "$(CFG)" == "ChODSvc - Win32 Debug"

# ADD BASE RSC /l 0x804 /i "\ZQProjs\tianshan\ChannelOnDemand" /i "\ZQProjs1.7.5\TianShan\ChannelOnDemand" /i "\ZQProjs1.7\TianShan\ChannelOnDemand" /i "\projects\ZQProjs\TianShan\ChannelOnDemand"
# SUBTRACT BASE RSC /i "..\..\build" /i "$(ITVSDKPATH)"
# ADD RSC /l 0x804 /i "\ZQProjs\tianshan\ChannelOnDemand" /i "\ZQProjs1.7.5\TianShan\ChannelOnDemand" /i "\ZQProjs1.7\TianShan\ChannelOnDemand" /i "." /i "$(ZQProjsPath)/build" /i "\projects\ZQProjs\TianShan\ChannelOnDemand"
# SUBTRACT RSC /i "..\..\build" /i "$(ITVSDKPATH)"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ChannelOnDemandEx.ICE

!IF  "$(CFG)" == "ChODSvc - Win32 Release"

# Begin Custom Build
InputPath=..\ChannelOnDemandEx.ICE
InputName=ChannelOnDemandEx

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. ..\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --dict "ChannelOnDemand::ChannelItemDict,string,ChannelOnDemand::ChannelItemEx" ChannelItemDict ..\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index "ChannelOnDemand::ChannelNameIndex,ChannelOnDemand::ChannelPublishPoint,onDemandName,case-insensitive" ChannelNameIndex ..\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index "ChannelOnDemand::PlaylistId2Purchase,ChannelOnDemand::ChannelPurchase,playlistId,case-insensitive" PlaylistId2Purchase ..\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index "ChannelOnDemand::Channel2Purchase,ChannelOnDemand::ChannelPurchase,chlPubName,case-insensitive" Channel2Purchase ..\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index "ChannelOnDemand::Purchase2ItemAssoc,ChannelOnDemand::PurchaseItemAssoc,purchaseIdent" Purchase2ItemAssoc ..\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index "ChannelOnDemand::CtrlNum2ItemAssoc,ChannelOnDemand::PurchaseItemAssoc,playlistCtrlNum" CtrlNum2ItemAssoc ..\$(InputName).ice \
	

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ChODSvc - Win32 Debug"

# Begin Custom Build
InputPath=..\ChannelOnDemandEx.ICE
InputName=ChannelOnDemandEx

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. ..\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --dict "ChannelOnDemand::ChannelItemDict,string,ChannelOnDemand::ChannelItemEx" ChannelItemDict ..\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index "ChannelOnDemand::ChannelNameIndex,ChannelOnDemand::ChannelPublishPoint,onDemandName,case-insensitive" ChannelNameIndex ..\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index "ChannelOnDemand::PlaylistId2Purchase,ChannelOnDemand::ChannelPurchase,playlistId,case-insensitive" PlaylistId2Purchase ..\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index "ChannelOnDemand::Channel2Purchase,ChannelOnDemand::ChannelPurchase,chlPubName,case-insensitive" Channel2Purchase ..\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index "ChannelOnDemand::ChItem2ItemAssoc,ChannelOnDemand::PurchaseItemAssoc,channelItemKey,case-insensitive" ChItem2ItemAssoc ..\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index "ChannelOnDemand::Purchase2ItemAssoc,ChannelOnDemand::PurchaseItemAssoc,purchaseIdent" Purchase2ItemAssoc ..\$(InputName).ice \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I.. --output-dir .. --index "ChannelOnDemand::CtrlNum2ItemAssoc,ChannelOnDemand::PurchaseItemAssoc,playlistCtrlNum" CtrlNum2ItemAssoc ..\$(InputName).ice \
	

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\todas.ice

!IF  "$(CFG)" == "ChODSvc - Win32 Release"

# Begin Custom Build
InputPath=..\todas.ice
InputName=todas

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --output-dir .. ..\$(InputName).ice

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ChODSvc - Win32 Debug"

# Begin Custom Build
InputPath=..\todas.ice
InputName=todas

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice --output-dir .. ..\$(InputName).ice

"../$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
