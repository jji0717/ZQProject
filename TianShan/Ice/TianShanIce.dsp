# Microsoft Developer Studio Project File - Name="TianShanIce" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=TianShanIce - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TianShanIce.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TianShanIce.mak" CFG="TianShanIce - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TianShanIce - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "TianShanIce - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "TianShanIce - Win32 Java" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZQProjs/TianShan/Ice", YRSAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TianShanIce - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /I "$(ZQProjsPath)/Common" /I "$(EXPATPATH)/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "." /D "_MBCS" /D "_LIB" /D _WIN32_WINNT=0x500 /D "XML_STATIC" /D "NDEBUG" /D "WIN32" /D "_STLP_NEW_PLATFORM_SDK" /D "_DLL" /D "_MT" /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../bin/TianShanIce.lib"

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(ZQProjsPath)/Common" /I "$(EXPATPATH)/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "." /D "XML_STATIC" /D _WIN32_WINNT=0x500 /D "_LIB" /D "_MBCS" /D "_DEBUG" /D "WIN32" /D "_STLP_NEW_PLATFORM_SDK" /D "_STLP_DEBUG" /D "_MT" /D "_DLL" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../bin/TianShanIce_d.lib"

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Java"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "TianShanIce___Win32_Java"
# PROP BASE Intermediate_Dir "TianShanIce___Win32_Java"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "TianShanIce___Win32_Java"
# PROP Intermediate_Dir "TianShanIce___Win32_Java"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D _WIN32_WINNT=0x500 /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(ZQProjsPath)/Common" /I "$(ACE_ROOT)/../expat/include" /I "$(ICE_ROOT)/include" /I "$(ICE_ROOT)/include/stlport" /I "." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D _WIN32_WINNT=0x500 /D "XML_STATIC" /D "_STLP_NEW_PLATFORM_SDK" /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"../bin/TianShanIce.lib"
# ADD LIB32 /nologo /out:"../bin/TianShanIce.lib"

!ENDIF 

# Begin Target

# Name "TianShanIce - Win32 Release"
# Name "TianShanIce - Win32 Debug"
# Name "TianShanIce - Win32 Java"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ant.xml

!IF  "$(CFG)" == "TianShanIce - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Java"

USERDEP__ANT_X="java\TianShanIce\BaseService.java"	
# Begin Custom Build
InputPath=.\ant.xml
InputName=ant

"aaa" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	set PATH=$(PATH);$(ANT_HOME)\bin 
	ant -f $(InputName).xml 
	rd /s java 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TianShanIce.cpp

!IF  "$(CFG)" == "TianShanIce - Win32 Release"

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Debug"

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Java"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TsApplication.cpp

!IF  "$(CFG)" == "TianShanIce - Win32 Release"

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Debug"

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Java"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TsContentProv.cpp
# End Source File
# Begin Source File

SOURCE=.\TsEvents.cpp

!IF  "$(CFG)" == "TianShanIce - Win32 Release"

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Debug"

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Java"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TsSite.cpp
# End Source File
# Begin Source File

SOURCE=.\TsSRM.cpp
# End Source File
# Begin Source File

SOURCE=.\TsStorage.cpp
# End Source File
# Begin Source File

SOURCE=.\TsStreamer.cpp

!IF  "$(CFG)" == "TianShanIce - Win32 Release"

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Debug"

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Java"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TsTransport.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\TianShanIce.h
# End Source File
# Begin Source File

SOURCE=.\TsApplication.h
# End Source File
# Begin Source File

SOURCE=.\TsContentProv.h
# End Source File
# Begin Source File

SOURCE=.\TsEvents.h
# End Source File
# Begin Source File

SOURCE=.\TsSite.h
# End Source File
# Begin Source File

SOURCE=.\TsSRM.h
# End Source File
# Begin Source File

SOURCE=.\TsStorage.h
# End Source File
# Begin Source File

SOURCE=.\TsStreamer.h
# End Source File
# Begin Source File

SOURCE=.\TsTransport.h
# End Source File
# End Group
# Begin Group "ICE Files"

# PROP Default_Filter "ICE"
# Begin Source File

SOURCE=.\TianShanIce.ice

!IF  "$(CFG)" == "TianShanIce - Win32 Release"

# Begin Custom Build
InputPath=.\TianShanIce.ice
InputName=TianShanIce

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Debug"

# Begin Custom Build
InputPath=.\TianShanIce.ice
InputName=TianShanIce

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Java"

# Begin Custom Build
InputPath=.\TianShanIce.ice
InputName=TianShanIce

"java/$(InputName).java" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(ICE_ROOT)\bin\slice2java.exe -I$(ICE_ROOT)/slice $(InputName).ice --output-dir java

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TsApplication.ICE

!IF  "$(CFG)" == "TianShanIce - Win32 Release"

# Begin Custom Build
InputPath=.\TsApplication.ICE
InputName=TsApplication

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Debug"

# Begin Custom Build
InputPath=.\TsApplication.ICE
InputName=TsApplication

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Java"

# Begin Custom Build
InputPath=.\TsApplication.ICE
InputName=TsApplication

"java/$(InputName).java" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(ICE_ROOT)\bin\slice2java.exe -I$(ICE_ROOT)/slice $(InputName).ice --output-dir java

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TsContentProv.ICE

!IF  "$(CFG)" == "TianShanIce - Win32 Release"

# Begin Custom Build
InputPath=.\TsContentProv.ICE
InputName=TsContentProv

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Debug"

# Begin Custom Build
InputPath=.\TsContentProv.ICE
InputName=TsContentProv

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Java"

# Begin Custom Build
InputPath=.\TsContentProv.ICE
InputName=TsContentProv

"java/$(InputName).java" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(ICE_ROOT)\bin\slice2java.exe -I$(ICE_ROOT)/slice $(InputName).ice --output-dir java

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TsEvents.ICE

!IF  "$(CFG)" == "TianShanIce - Win32 Release"

# Begin Custom Build
InputPath=.\TsEvents.ICE
InputName=TsEvents

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Debug"

# Begin Custom Build
InputPath=.\TsEvents.ICE
InputName=TsEvents

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Java"

# Begin Custom Build
InputPath=.\TsEvents.ICE
InputName=TsEvents

"java/$(InputName).java" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(ICE_ROOT)\bin\slice2java.exe -I$(ICE_ROOT)/slice $(InputName).ice --output-dir java

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TsSite.ICE

!IF  "$(CFG)" == "TianShanIce - Win32 Release"

# Begin Custom Build
InputPath=.\TsSite.ICE
InputName=TsSite

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Debug"

# Begin Custom Build
InputPath=.\TsSite.ICE
InputName=TsSite

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Java"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TsSRM.ICE

!IF  "$(CFG)" == "TianShanIce - Win32 Release"

# Begin Custom Build
InputPath=.\TsSRM.ICE
InputName=TsSRM

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Debug"

# Begin Custom Build
InputPath=.\TsSRM.ICE
InputName=TsSRM

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Java"

# Begin Custom Build
InputPath=.\TsSRM.ICE
InputName=TsSRM

"java/$(InputName).java" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(ICE_ROOT)\bin\slice2java.exe -I$(ICE_ROOT)/slice $(InputName).ice --output-dir java

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TsStorage.ICE

!IF  "$(CFG)" == "TianShanIce - Win32 Release"

# Begin Custom Build
InputPath=.\TsStorage.ICE
InputName=TsStorage

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Debug"

# Begin Custom Build
InputPath=.\TsStorage.ICE
InputName=TsStorage

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Java"

# Begin Custom Build
InputPath=.\TsStorage.ICE
InputName=TsStorage

"java/$(InputName).java" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(ICE_ROOT)\bin\slice2java.exe -I$(ICE_ROOT)/slice $(InputName).ice --output-dir java

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TsStreamer.ICE

!IF  "$(CFG)" == "TianShanIce - Win32 Release"

# Begin Custom Build
InputPath=.\TsStreamer.ICE
InputName=TsStreamer

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Debug"

# Begin Custom Build
InputPath=.\TsStreamer.ICE
InputName=TsStreamer

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Java"

# Begin Custom Build
InputPath=.\TsStreamer.ICE
InputName=TsStreamer

"java/$(InputName).java" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(ICE_ROOT)\bin\slice2java.exe -I$(ICE_ROOT)/slice $(InputName).ice --output-dir java

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TsTransport.ICE

!IF  "$(CFG)" == "TianShanIce - Win32 Release"

# Begin Custom Build
InputPath=.\TsTransport.ICE
InputName=TsTransport

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Debug"

# Begin Custom Build
InputPath=.\TsTransport.ICE
InputName=TsTransport

BuildCmds= \
	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "TianShanIce - Win32 Java"

# Begin Custom Build
InputPath=.\TsTransport.ICE
InputName=TsTransport

"java/$(InputName).java" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(ICE_ROOT)\bin\slice2java.exe -I$(ICE_ROOT)/slice $(InputName).ice --output-dir java

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\TianShanSDK.dox
# End Source File
# End Target
# End Project
