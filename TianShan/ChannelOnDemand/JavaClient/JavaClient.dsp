# Microsoft Developer Studio Project File - Name="JavaClient" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Generic Project" 0x010a

CFG=JavaClient - Win32 Java
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "JavaClient.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "JavaClient.mak" CFG="JavaClient - Win32 Java"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "JavaClient - Win32 Java" (based on "Win32 (x86) Generic Project")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
MTL=midl.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Java"
# PROP BASE Intermediate_Dir "Java"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Java"
# PROP Intermediate_Dir "Java"
# PROP Target_Dir ""
# Begin Target

# Name "JavaClient - Win32 Java"
# Begin Source File

SOURCE=.\ant.xml
USERDEP__ANT_X="java\ChannelOnDemand\ChannelPublisher.java"	
# Begin Custom Build
InputPath=.\ant.xml
InputName=ant

BuildCmds= \
	set PATH=$(PATH);$(ANT_HOME)\bin \
	ant -f $(InputName).xml \
	rd /s java \
	

"ChannelOnDeman.jar" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"ChannelOnDemanSrc.jar" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build
# End Source File
# Begin Source File

SOURCE=..\ChannelOnDemand.ICE
# Begin Custom Build
InputPath=..\ChannelOnDemand.ICE
InputName=ChannelOnDemand

"java\ChannelOnDemand\ChannelPublisher.java" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(ICE_ROOT)\bin\slice2java.exe -I$(ICE_ROOT)/slice -I../../Ice ..\$(InputName).ice --output-dir java

# End Custom Build
# End Source File
# End Target
# End Project
