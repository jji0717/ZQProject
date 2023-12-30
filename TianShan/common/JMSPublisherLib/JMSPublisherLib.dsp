# Microsoft Developer Studio Project File - Name="JMSPublisherLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=JMSPublisherLib - Win32 Unicode Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "JMSPublisherLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "JMSPublisherLib.mak" CFG="JMSPublisherLib - Win32 Unicode Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "JMSPublisherLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "JMSPublisherLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "JMSPublisherLib - Win32 Unicode Release" (based on "Win32 (x86) Static Library")
!MESSAGE "JMSPublisherLib - Win32 Unicode Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "JMSPublisherLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /ZI /Od /I "." /I ".." /I "$(ICE_ROOT)/include/stlport" /I "$(ICE_ROOT)/include" /I "$(ZQProjsPath)/Common" /I "$(ZQPROJSPATH)\generic\JMSCppLib" /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_AFXDLL" /D "_STLP_NEW_PLATFORM_SDK" /D _WIN32_WINNT=0x0500 /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "JMSPublisherLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(ZQPROJSPATH)\generic\JMSCppLib" /I "$(ICE_ROOT)/include/stlport" /I "$(ICE_ROOT)/include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_AFXDLL" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /D _WIN32_WINNT=0x0500 /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\JMSPublisherLib_d.lib"

!ELSEIF  "$(CFG)" == "JMSPublisherLib - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "JMSPublisherLib___Win32_Unicode_Release"
# PROP BASE Intermediate_Dir "JMSPublisherLib___Win32_Unicode_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unicode_Release"
# PROP Intermediate_Dir "Unicode_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(ZQPROJSPATH)\generic\JMSCppLib" /I "$(ICE_ROOT)/include/stlport" /I "$(ICE_ROOT)/include" /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "_STLP_NEW_PLATFORM_SDK" /D _WIN32_WINNT=0x0500 /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GR /GX /ZI /Od /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(ZQPROJSPATH)\generic\JMSCppLib" /I "$(ICE_ROOT)/include/stlport" /I "$(ICE_ROOT)/include" /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "_STLP_NEW_PLATFORM_SDK" /D _WIN32_WINNT=0x0500 /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "JMSPublisherLib - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "JMSPublisherLib___Win32_Unicode_Debug"
# PROP BASE Intermediate_Dir "JMSPublisherLib___Win32_Unicode_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Unicode_Debug"
# PROP Intermediate_Dir "Unicode_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(ZQPROJSPATH)\generic\JMSCppLib" /I "$(ICE_ROOT)/include/stlport" /I "$(ICE_ROOT)/include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /D _WIN32_WINNT=0x0500 /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "." /I ".." /I "$(ZQProjsPath)/Common" /I "$(ZQPROJSPATH)\generic\JMSCppLib" /I "$(ICE_ROOT)/include/stlport" /I "$(ICE_ROOT)/include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_UNICODE" /D "UNICODE" /D "_AFXDLL" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /D _WIN32_WINNT=0x0500 /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"Debug\JMSPublisherLib_d.lib"
# ADD LIB32 /nologo /out:"Debug\JMSPublisherLib_d.lib"

!ENDIF 

# Begin Target

# Name "JMSPublisherLib - Win32 Release"
# Name "JMSPublisherLib - Win32 Debug"
# Name "JMSPublisherLib - Win32 Unicode Release"
# Name "JMSPublisherLib - Win32 Unicode Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\JMSPublisher.cpp
# End Source File
# Begin Source File

SOURCE=..\MessageData.cpp
# End Source File
# Begin Source File

SOURCE=..\MsgContent.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\JMSPublisher.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Log.h
# End Source File
# Begin Source File

SOURCE=..\MessageData.h
# End Source File
# Begin Source File

SOURCE=..\MsgContent.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\NativeThread.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MsgContent.ICE

!IF  "$(CFG)" == "JMSPublisherLib - Win32 Release"

# Begin Custom Build
InputPath=.\MsgContent.ICE
InputName=MsgContent

BuildCmds= \
	$(ICE_ROOT)\bin\slice2freeze.exe --output-dir ..\ -I$(ICE_ROOT)/slice --dict Messages,string,::TianShanIce::common::Message MessageData $(InputPath) \
	$(ICE_ROOT)\bin\slice2cpp.exe --output-dir ..\ -I$(ICE_ROOT)/slice $(InputPath) \
	

"MessageData.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"MessageData.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "JMSPublisherLib - Win32 Debug"

# Begin Custom Build
InputPath=.\MsgContent.ICE
InputName=MsgContent

BuildCmds= \
	$(ICE_ROOT)\bin\slice2freeze.exe --output-dir ..\ -I$(ICE_ROOT)/slice --dict Messages,string,::TianShanIce::common::Message MessageData $(InputPath) \
	$(ICE_ROOT)\bin\slice2cpp.exe --output-dir ..\ -I$(ICE_ROOT)/slice $(InputPath) \
	

"MessageData.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"MessageData.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "JMSPublisherLib - Win32 Unicode Release"

# Begin Custom Build
InputPath=.\MsgContent.ICE
InputName=MsgContent

BuildCmds= \
	$(ICE_ROOT)\bin\slice2freeze.exe --output-dir ..\ -I$(ICE_ROOT)/slice --dict Messages,string,::TianShanIce::common::Message MessageData $(InputPath) \
	$(ICE_ROOT)\bin\slice2cpp.exe --output-dir ..\ -I$(ICE_ROOT)/slice $(InputPath) \
	

"MessageData.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"MessageData.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "JMSPublisherLib - Win32 Unicode Debug"

# Begin Custom Build
InputPath=.\MsgContent.ICE
InputName=MsgContent

BuildCmds= \
	$(ICE_ROOT)\bin\slice2freeze.exe --output-dir ..\ -I$(ICE_ROOT)/slice --dict Messages,string,::TianShanIce::common::Message MessageData $(InputPath) \
	$(ICE_ROOT)\bin\slice2cpp.exe --output-dir ..\ -I$(ICE_ROOT)/slice $(InputPath) \
	

"MessageData.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"MessageData.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
