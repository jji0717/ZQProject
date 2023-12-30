# Microsoft Developer Studio Project File - Name="ModSvc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=ModSvc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ModSvc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ModSvc.mak" CFG="ModSvc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ModSvc - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "ModSvc - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ModSvc - Win32 Release"

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
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GR /GX /ZI /I "." /I ".." /I "$(ICE_ROOT)/include/stlport" /I "$(ZQPROJSPATH)/common" /I "$(ICE_ROOT)/include/" /I "$(ZQPROJSPATH)/tianshan/ice" /I "..\ICE\\" /I "$(ZQPROJSPATH)/tianshan/common" /I "$(ZQPROJSPATH)/tianshan/include" /I "$(ZQPROJSPATH)/tianshan/shell/ZQCfgPkg" /I "$(ZQPROJSPATH)/tianshan/shell/zqsnmpmanpkg" /I "$(STLPORT_ROOT)" /I "$(ExpatPath)/include" /I "$(RegExppKit)" /D "NDEBUG" /D _WIN32_WINNT=0x0500 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_STLP_NEW_PLATFORM_SDK" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 Freeze.lib Ice.lib IceUtil.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"../../bin/MODAppSvc.pdb" /debug /machine:I386 /out:"../../bin/MODAppSvc.exe" /libpath:"$(ICE_ROOT)/lib" /libpath:"$(EXPATPATH)/lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "ModSvc - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /I "." /I ".." /I "$(ICE_ROOT)/include/stlport" /I "$(ZQPROJSPATH)/common" /I "$(ICE_ROOT)/include/" /I "$(ZQPROJSPATH)/tianshan/ice" /I "..\ICE\\" /I "$(ZQPROJSPATH)/tianshan/common" /I "$(ZQPROJSPATH)/tianshan/include" /I "$(ZQPROJSPATH)/tianshan/shell/ZQCfgPkg" /I "$(ZQPROJSPATH)/tianshan/shell/zqsnmpmanpkg" /I "$(ExpatPath)/include" /I "$(RegExppKit)" /I "$(STLPORT_ROOT)" /D "_DEBUG" /D _WIN32_WINNT=0x0500 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /FD /GZ /Zm1000 /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /i "$(ZQProjsPath)/build" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Freezed.lib Iced.lib IceUtild.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /pdb:"../../bin/ModSvc_d.pdb" /debug /machine:I386 /out:"../../bin/ModSvc_d.exe" /pdbtype:sept /libpath:"$(ICE_ROOT)/lib" /libpath:"$(EXPATPATH)/lib"
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "ModSvc - Win32 Release"
# Name "ModSvc - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\common\BaseZQServiceApplication.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\ConfigHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\ICE\LAMFacade.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\MiniDump.cpp
# End Source File
# Begin Source File

SOURCE=.\ModConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\MODHelperMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ModService.cpp
# End Source File
# Begin Source File

SOURCE=..\ICE\ModSvcIce.cpp
# End Source File
# Begin Source File

SOURCE=.\ModSvcIceImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\ICE\ote.cpp
# End Source File
# Begin Source File

SOURCE=..\ICE\Stream2Purchase.cpp
# End Source File
# Begin Source File

SOURCE=..\ICE\Surf_Tianshan.cpp
# End Source File
# Begin Source File

SOURCE=.\WatchDog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\ZQServiceAppMain.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\Common\ConfigHelper.h
# End Source File
# Begin Source File

SOURCE=..\ICE\LAMFacade.h
# End Source File
# Begin Source File

SOURCE=.\ModCfgLoader.h
# End Source File
# Begin Source File

SOURCE=.\ModConfig.h
# End Source File
# Begin Source File

SOURCE=.\MODHelperMgr.h
# End Source File
# Begin Source File

SOURCE=.\ModService.h
# End Source File
# Begin Source File

SOURCE=..\ICE\ModSvcIce.h
# End Source File
# Begin Source File

SOURCE=.\ModSvcIceImpl.h
# End Source File
# Begin Source File

SOURCE=..\ICE\ote.h
# End Source File
# Begin Source File

SOURCE=..\ICE\Stream2Purchase.h
# End Source File
# Begin Source File

SOURCE=..\ICE\Surf_Tianshan.h
# End Source File
# Begin Source File

SOURCE=.\WatchDog.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\ModService.rc
# End Source File
# Begin Source File

SOURCE=..\MODPlugIn\resource.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "ICE"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ICE\LAMFacade.ice

!IF  "$(CFG)" == "ModSvc - Win32 Release"

# Begin Custom Build
InputPath=..\ICE\LAMFacade.ice
InputName=LAMFacade

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputPath)  --output-dir $(ZQProjsPath)/TianShan/application/ICE

"../ice/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../ice/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModSvc - Win32 Debug"

# Begin Custom Build
InputPath=..\ICE\LAMFacade.ice
InputName=LAMFacade

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputPath)  --output-dir $(ZQProjsPath)/TianShan/application/ICE

"../ice/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../ice/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ICE\ModSvcIce.ice

!IF  "$(CFG)" == "ModSvc - Win32 Release"

# Begin Custom Build
InputPath=..\ICE\ModSvcIce.ice
InputName=ModSvcIce

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)/TianShan/application/ICE $(InputPath) --output-dir $(ZQProjsPath)/TianShan/application/ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)/TianShan/application/ICE --index "ZQTianShan::Application::MOD::Stream2Purchase,ZQTianShan::Application::MOD::ModPurchase,streamId" Stream2Purchase $(InputPath) --output-dir $(ZQProjsPath)/TianShan/application/ICE \
	

"../ice/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../ice/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModSvc - Win32 Debug"

# Begin Custom Build
InputPath=..\ICE\ModSvcIce.ice
InputName=ModSvcIce

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)/TianShan/application/ICE $(InputPath) --output-dir $(ZQProjsPath)/TianShan/application/ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)/TianShan/application/ICE --index "ZQTianShan::Application::MOD::Stream2Purchase,ZQTianShan::Application::MOD::ModPurchase,streamId" Stream2Purchase $(InputPath) --output-dir $(ZQProjsPath)/TianShan/application/ICE \
	

"../ice/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../ice/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ICE\ote.ice

!IF  "$(CFG)" == "ModSvc - Win32 Release"

# Begin Custom Build
InputPath=..\ICE\ote.ice
InputName=ote

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputPath)  --output-dir $(ZQProjsPath)/TianShan/application/ICE

"../ice/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../ice/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModSvc - Win32 Debug"

# Begin Custom Build
InputPath=..\ICE\ote.ice
InputName=ote

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputPath)  --output-dir $(ZQProjsPath)/TianShan/application/ICE

"../ice/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../ice/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\ICE\Surf_Tianshan.ice

!IF  "$(CFG)" == "ModSvc - Win32 Release"

# Begin Custom Build
InputPath=..\ICE\Surf_Tianshan.ice
InputName=Surf_Tianshan

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputPath)  --output-dir $(ZQProjsPath)/TianShan/application/ICE

"../ice/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../ice/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "ModSvc - Win32 Debug"

# Begin Custom Build
InputPath=..\ICE\Surf_Tianshan.ice
InputName=Surf_Tianshan

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputPath)  --output-dir $(ZQProjsPath)/TianShan/application/ICE

"../ice/$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"../ice/$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
