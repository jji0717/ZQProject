; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CNPVRSMSUIView
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "NPVRSMSUI.h"
LastPage=0

ClassCount=6
Class1=CNPVRSMSUIApp
Class2=CNPVRSMSUIDoc
Class3=CNPVRSMSUIView
Class4=CMainFrame

ResourceCount=3
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Class5=CAboutDlg
Class6=DefineTime
Resource3=IDD_DEFINE

[CLS:CNPVRSMSUIApp]
Type=0
HeaderFile=NPVRSMSUI.h
ImplementationFile=NPVRSMSUI.cpp
Filter=N

[CLS:CNPVRSMSUIDoc]
Type=0
HeaderFile=NPVRSMSUIDoc.h
ImplementationFile=NPVRSMSUIDoc.cpp
Filter=N

[CLS:CNPVRSMSUIView]
Type=0
HeaderFile=NPVRSMSUIView.h
ImplementationFile=NPVRSMSUIView.cpp
Filter=C
BaseClass=CListView
VirtualFilter=VWC
LastObject=ID_REFRESH


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
LastObject=ID_DEFINE




[CLS:CAboutDlg]
Type=0
HeaderFile=NPVRSMSUI.cpp
ImplementationFile=NPVRSMSUI.cpp
Filter=D
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_DEFINE
Command2=ID_REFRESH
Command3=ID_APP_ABOUT
CommandCount=3

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
CommandCount=14

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
CommandCount=8

[DLG:IDD_DEFINE]
Type=1
Class=DefineTime
ControlCount=2
Control1=IDOK,button,1342242817
Control2=IDC_DATETIMEPICKER1,SysDateTimePick32,1342242852

[CLS:DefineTime]
Type=0
HeaderFile=DefineTime.h
ImplementationFile=DefineTime.cpp
BaseClass=CDialog
Filter=D
LastObject=DefineTime
VirtualFilter=dWC

