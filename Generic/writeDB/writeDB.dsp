# Microsoft Developer Studio Project File - Name="writeDB" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=writeDB - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "writeDB.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "writeDB.mak" CFG="writeDB - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "writeDB - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "writeDB - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "writeDB - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(ICE_ROOT)\include" /I "$(STLPORT_ROOT)" /I "$(ZQPROJSPATH)\tianshan\ice" /I "$(ZQPROJSPATH)\tianshan\common" /I "$(ZQPROJSPATH)\common" /I "." /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "_STLP_NEW_PLATFORM_SDK" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 TianShanIce.lib ice.lib iceutil.lib freeze.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"$(ZQPROJSPATH)\tianshan\bin\writeDB.exe" /libpath:"$(ICE_ROOT)\lib" /libpath:"$(ZQPROJSPATH)\tianshan\bin"

!ELSEIF  "$(CFG)" == "writeDB - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(ICE_ROOT)\include" /I "$(STLPORT_ROOT)" /I "$(ZQPROJSPATH)\tianshan\ice" /I "$(ZQPROJSPATH)\tianshan\common" /I "$(ZQPROJSPATH)\common" /I "." /D "_WINSOCK2API_" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "_STLP_DEBUG" /D "_STLP_NEW_PLATFORM_SDK" /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 TianShanIce_d.lib iced.lib iceutild.lib freezed.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"$(ICE_ROOT)\lib" /libpath:"$(ZQPROJSPATH)\tianshan\bin"

!ENDIF 

# Begin Target

# Name "writeDB - Win32 Release"
# Name "writeDB - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ServiceGroupDict.cpp
# End Source File
# Begin Source File

SOURCE=.\ServiceGroupToStreamLink.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StorageDict.cpp
# End Source File
# Begin Source File

SOURCE=.\StorageLinkToTicket.cpp
# End Source File
# Begin Source File

SOURCE=.\StorageToStorageLink.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamerDict.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamerToStorageLink.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamerToStreamLink.cpp
# End Source File
# Begin Source File

SOURCE=.\StreamLinkToTicket.cpp
# End Source File
# Begin Source File

SOURCE=.\TsPathAdmin.cpp
# End Source File
# Begin Source File

SOURCE=.\writeDB.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ServiceGroupDict.h
# End Source File
# Begin Source File

SOURCE=.\ServiceGroupToStreamLink.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\StorageDict.h
# End Source File
# Begin Source File

SOURCE=.\StorageLinkToTicket.h
# End Source File
# Begin Source File

SOURCE=.\StorageToStorageLink.h
# End Source File
# Begin Source File

SOURCE=.\StreamerDict.h
# End Source File
# Begin Source File

SOURCE=.\StreamerToStorageLink.h
# End Source File
# Begin Source File

SOURCE=.\StreamerToStreamLink.h
# End Source File
# Begin Source File

SOURCE=.\StreamLinkToTicket.h
# End Source File
# Begin Source File

SOURCE=.\stroprt.h
# End Source File
# Begin Source File

SOURCE=.\TsPathAdmin.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\TsPathAdmin.ICE

!IF  "$(CFG)" == "writeDB - Win32 Release"

# Begin Custom Build
InputPath=.\TsPathAdmin.ICE
InputName=TsPathAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --dict "TianShanIce::Transport::ServiceGroupDict,long,TianShanIce::Transport::ServiceGroup" --dict-index "TianShanIce::Transport::ServiceGroupDict,id" ServiceGroupDict --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --dict "TianShanIce::Transport::StorageDict,string,TianShanIce::Transport::Storage" --dict-index "TianShanIce::Transport::StorageDict,netId,case-insensitive" StorageDict --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --dict "TianShanIce::Transport::StreamerDict,string,TianShanIce::Transport::Streamer" --dict-index "TianShanIce::Transport::StreamerDict,netId,case-insensitive" StreamerDict --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --index "TianShanIce::Transport::StorageToStorageLink,TianShanIce::Transport::StorageLink,storageId" StorageToStorageLink --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --index "TianShanIce::Transport::StreamerToStorageLink,TianShanIce::Transport::StorageLink,streamerId" StreamerToStorageLink --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --index "TianShanIce::Transport::StreamerToStreamLink,TianShanIce::Transport::StreamLink,streamerId" StreamerToStreamLink --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --index "TianShanIce::Transport::ServiceGroupToStreamLink,TianShanIce::Transport::StreamLink,servicegroupId" ServiceGroupToStreamLink --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --index "TianShanIce::Transport::StorageLinkToTicket,TianShanIce::Transport::PathTicket,storageLinkIden" StorageLinkToTicket --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --index "TianShanIce::Transport::StreamLinkToTicket,TianShanIce::Transport::PathTicket,streamLinkIden" StreamLinkToTicket --output-dir .\ .\$(InputName).ICE \
	

".\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\ServiceGroupDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\ServiceGroupDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\StorageDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\StorageDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\StreamerDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\StreamerDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\StorageLinkIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\StorageLinkIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\StreamLinkIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\StreamLinkIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\ADPAllocIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\ADPAllocIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "writeDB - Win32 Debug"

# Begin Custom Build
InputPath=.\TsPathAdmin.ICE
InputName=TsPathAdmin

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --dict "TianShanIce::Transport::ServiceGroupDict,long,TianShanIce::Transport::ServiceGroup" --dict-index "TianShanIce::Transport::ServiceGroupDict,id" ServiceGroupDict --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --dict "TianShanIce::Transport::StorageDict,string,TianShanIce::Transport::Storage" --dict-index "TianShanIce::Transport::StorageDict,netId,case-insensitive" StorageDict --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --dict "TianShanIce::Transport::StreamerDict,string,TianShanIce::Transport::Streamer" --dict-index "TianShanIce::Transport::StreamerDict,netId,case-insensitive" StreamerDict --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --index "TianShanIce::Transport::StorageToStorageLink,TianShanIce::Transport::StorageLink,storageId" StorageToStorageLink --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --index "TianShanIce::Transport::StreamerToStorageLink,TianShanIce::Transport::StorageLink,streamerId" StreamerToStorageLink --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --index "TianShanIce::Transport::StreamerToStreamLink,TianShanIce::Transport::StreamLink,streamerId" StreamerToStreamLink --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --index "TianShanIce::Transport::ServiceGroupToStreamLink,TianShanIce::Transport::StreamLink,servicegroupId" ServiceGroupToStreamLink --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --index "TianShanIce::Transport::StorageLinkToTicket,TianShanIce::Transport::PathTicket,storageLinkIden" StorageLinkToTicket --output-dir .\ .\$(InputName).ICE \
	$(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice  --index "TianShanIce::Transport::StreamLinkToTicket,TianShanIce::Transport::PathTicket,streamLinkIden" StreamLinkToTicket --output-dir .\ .\$(InputName).ICE \
	

".\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\ServiceGroupDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\ServiceGroupDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\StorageDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\StorageDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\StreamerDict.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\StreamerDict.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\StorageLinkIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\StorageLinkIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\StreamLinkIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\StreamLinkIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\ADPAllocIndex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

".\ADPAllocIndex.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
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
