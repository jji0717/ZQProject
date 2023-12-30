# Microsoft Developer Studio Project File - Name="ZQRTFLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ZQRTFLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ZQRTFLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ZQRTFLib.mak" CFG="ZQRTFLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ZQRTFLib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ZQRTFLib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ZQRTFLib - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZQRTFLIB_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /I "." /I ".\VVXLib" /I ".\VV2Lib" /D "WIN32" /D "NDEBUG" /D "_LIB" /D "_UNICODE" /D "UNICODE" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 /nologo /dll /incremental:yes /debug /machine:I386
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=xcopy               /fy                                  Release\ZQRTFLib.dll                                   ..\..\TianShan\bin\    	xcopy               /fy                                  Release\ZQRTFLib.pdb                                   ..\..\TianShan\bin\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ZQRTFLib - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZQRTFLIB_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I ".\VVXLib" /I ".\VV2Lib" /D "WIN32" /D "_DEBUG" /D "_LIB" /D "_UNICODE" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "ZQRTFLib - Win32 Release"
# Name "ZQRTFLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\RTFAcd.c
# End Source File
# Begin Source File

SOURCE=.\RTFAcdAC3.c
# End Source File
# Begin Source File

SOURCE=.\RTFAcdMPEG2.c
# End Source File
# Begin Source File

SOURCE=.\RTFBuf.c
# End Source File
# Begin Source File

SOURCE=.\RTFCas.c
# End Source File
# Begin Source File

SOURCE=.\RTFCasPan.c
# End Source File
# Begin Source File

SOURCE=.\RTFCasVmx.c
# End Source File
# Begin Source File

SOURCE=.\RTFCat.c
# End Source File
# Begin Source File

SOURCE=.\RTFFlt.c
# End Source File
# Begin Source File

SOURCE=.\RTFGop.c
# End Source File
# Begin Source File

SOURCE=.\RTFIdx.c
# End Source File
# Begin Source File

SOURCE=.\RTFIdxVV2.c
# End Source File
# Begin Source File

SOURCE=.\RTFIdxVVX.c
# End Source File
# Begin Source File

SOURCE=.\RTFLib.c
# End Source File
# Begin Source File

SOURCE=.\RTFObj.c
# End Source File
# Begin Source File

SOURCE=.\RTFOut.c
# End Source File
# Begin Source File

SOURCE=.\RTFPat.c
# End Source File
# Begin Source File

SOURCE=.\RTFPes.c
# End Source File
# Begin Source File

SOURCE=.\RTFPic.c
# End Source File
# Begin Source File

SOURCE=.\RTFPkt.c
# End Source File
# Begin Source File

SOURCE=.\RTFPmt.c
# End Source File
# Begin Source File

SOURCE=.\RTFSeq.c
# End Source File
# Begin Source File

SOURCE=.\RTFSes.c
# End Source File
# Begin Source File

SOURCE=.\RTFSys.c
# End Source File
# Begin Source File

SOURCE=.\RTFVcd.c
# End Source File
# Begin Source File

SOURCE=.\RTFVcdH264.c
# End Source File
# Begin Source File

SOURCE=.\RTFVcdMPEG2.c
# End Source File
# Begin Source File

SOURCE=.\RTFVcdVC1.c
# End Source File
# Begin Source File

SOURCE=.\RTFWin.c
# End Source File
# Begin Source File

SOURCE=.\VV2LIB\VV2Lib.c
# End Source File
# Begin Source File

SOURCE=.\VVXLib\VVXLib.c
# End Source File
# Begin Source File

SOURCE=.\ZQRTFLib.rc
# ADD BASE RSC /l 0x804
# ADD RSC /l 0x804 /i "$(ZQPROJSPATH)/build"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\RTFAcd.h
# End Source File
# Begin Source File

SOURCE=.\RTFAcdAC3.h
# End Source File
# Begin Source File

SOURCE=.\RTFAcdMPEG2.h
# End Source File
# Begin Source File

SOURCE=.\RTFAcdPrv.h
# End Source File
# Begin Source File

SOURCE=.\RTFBuf.h
# End Source File
# Begin Source File

SOURCE=.\RTFCas.h
# End Source File
# Begin Source File

SOURCE=.\RTFCasPan.h
# End Source File
# Begin Source File

SOURCE=.\RTFCasPrv.h
# End Source File
# Begin Source File

SOURCE=.\RTFCasVmx.h
# End Source File
# Begin Source File

SOURCE=.\RTFCat.h
# End Source File
# Begin Source File

SOURCE=.\RTFFlt.h
# End Source File
# Begin Source File

SOURCE=.\RTFGop.h
# End Source File
# Begin Source File

SOURCE=.\RTFIdx.h
# End Source File
# Begin Source File

SOURCE=.\RTFIdxPrv.h
# End Source File
# Begin Source File

SOURCE=.\RTFIdxVV2.h
# End Source File
# Begin Source File

SOURCE=.\RTFIdxVVX.h
# End Source File
# Begin Source File

SOURCE=.\RTFLib.h
# End Source File
# Begin Source File

SOURCE=.\RTFObj.h
# End Source File
# Begin Source File

SOURCE=.\RTFOut.h
# End Source File
# Begin Source File

SOURCE=.\RTFPat.h
# End Source File
# Begin Source File

SOURCE=.\RTFPes.h
# End Source File
# Begin Source File

SOURCE=.\RTFPic.h
# End Source File
# Begin Source File

SOURCE=.\RTFPkt.h
# End Source File
# Begin Source File

SOURCE=.\RTFPmt.h
# End Source File
# Begin Source File

SOURCE=.\RTFPrv.h
# End Source File
# Begin Source File

SOURCE=.\RTFSeq.h
# End Source File
# Begin Source File

SOURCE=.\RTFSes.h
# End Source File
# Begin Source File

SOURCE=.\RTFSys.h
# End Source File
# Begin Source File

SOURCE=.\RTFVcd.h
# End Source File
# Begin Source File

SOURCE=.\RTFVcdH264.h
# End Source File
# Begin Source File

SOURCE=.\RTFVcdMPEG2.h
# End Source File
# Begin Source File

SOURCE=.\RTFVcdPrv.h
# End Source File
# Begin Source File

SOURCE=.\RTFVcdVC1.h
# End Source File
# Begin Source File

SOURCE=.\RTFWin.h
# End Source File
# Begin Source File

SOURCE=.\SeaResource.h
# End Source File
# Begin Source File

SOURCE=.\VV2LIB\VV2Lib.h
# End Source File
# Begin Source File

SOURCE=.\VVXLib\Vvx.h
# End Source File
# Begin Source File

SOURCE=.\VVXLib\VVXDef.h
# End Source File
# Begin Source File

SOURCE=.\VVXLib\VVXLib.h
# End Source File
# Begin Source File

SOURCE=.\ZQResource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
