; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CChildFrame
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "CodMan.h"
LastPage=0

ClassCount=11
Class1=CCodManApp
Class2=CCodManDoc
Class3=CCodManView
Class4=CMainFrame

ResourceCount=8
Resource1=IDRM_PropertyView
Resource2=IDD_OpenCodDlg
Resource3=IDR_CODMANTYPE
Class5=CChildFrame
Class6=CAboutDlg
Class7=COpenCodDlg
Class8=CChannelTV
Class9=CPropertyView
Resource4=IDRM_TreeView
Class10=CChannelEditDlg
Resource5=IDD_ABOUTBOX
Class11=CChnlItemEditDlg
Resource6=IDR_MAINFRAME
Resource7=IDD_ChannelEdit
Resource8=IDD_ChannelItemEdit

[CLS:CCodManApp]
Type=0
HeaderFile=CodMan.h
ImplementationFile=CodMan.cpp
Filter=N
LastObject=CCodManApp
BaseClass=CWinApp
VirtualFilter=AC

[CLS:CCodManDoc]
Type=0
HeaderFile=CodManDoc.h
ImplementationFile=CodManDoc.cpp
Filter=N
BaseClass=CDocument
VirtualFilter=DC
LastObject=CCodManDoc

[CLS:CCodManView]
Type=0
HeaderFile=CodManView.h
ImplementationFile=CodManView.cpp
Filter=C
LastObject=IDM_ReplaceItem
BaseClass=CView
VirtualFilter=VWC


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T


[CLS:CChildFrame]
Type=0
HeaderFile=ChildFrm.h
ImplementationFile=ChildFrm.cpp
Filter=M
LastObject=CChildFrame


[CLS:CAboutDlg]
Type=0
HeaderFile=CodMan.cpp
ImplementationFile=CodMan.cpp
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
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_PRINT_SETUP
Command4=ID_APP_EXIT
Command5=ID_VIEW_TOOLBAR
Command6=ID_VIEW_STATUS_BAR
Command7=ID_APP_ABOUT
CommandCount=7

[TB:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
CommandCount=8

[MNU:IDR_CODMANTYPE]
Type=1
Class=CCodManView
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_PRINT
Command7=ID_FILE_PRINT_PREVIEW
Command8=ID_FILE_PRINT_SETUP
Command9=ID_APP_EXIT
Command10=ID_EDIT_UNDO
Command11=ID_EDIT_CUT
Command12=ID_EDIT_COPY
Command13=ID_EDIT_PASTE
Command14=ID_VIEW_TOOLBAR
Command15=ID_VIEW_STATUS_BAR
Command16=ID_WINDOW_NEW
Command17=ID_WINDOW_CASCADE
Command18=ID_WINDOW_TILE_HORZ
Command19=ID_WINDOW_ARRANGE
Command20=ID_APP_ABOUT
CommandCount=20

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

[DLG:IDD_OpenCodDlg]
Type=1
Class=COpenCodDlg
ControlCount=4
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308864
Control4=IDC_Cod_EndPoint,edit,1350631552

[CLS:COpenCodDlg]
Type=0
HeaderFile=OpenCodDlg.h
ImplementationFile=OpenCodDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=COpenCodDlg
VirtualFilter=dWC

[CLS:CChannelTV]
Type=0
HeaderFile=ChannelTV.h
ImplementationFile=ChannelTV.cpp
BaseClass=CTreeView
Filter=C
LastObject=CChannelTV
VirtualFilter=VWC

[CLS:CPropertyView]
Type=0
HeaderFile=PropertyView.h
ImplementationFile=PropertyView.cpp
BaseClass=CListView
Filter=C
LastObject=IDM_RemoveItem
VirtualFilter=VWC

[DLG:IDD_ChannelEdit]
Type=1
Class=CChannelEditDlg
ControlCount=12
Control1=IDC_ChannelName,edit,1350631552
Control2=IDC_OnDemandName,edit,1350631552
Control3=IDC_MaxBitrates,edit,1350631552
Control4=IDC_Description,edit,1350631552
Control5=IDC_NetIds,edit,1350631552
Control6=IDOK,button,1342242817
Control7=IDCANCEL,button,1342242816
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,static,1342308352
Control12=IDC_STATIC,static,1342308352

[CLS:CChannelEditDlg]
Type=0
HeaderFile=ChannelEditDlg.h
ImplementationFile=ChannelEditDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=CChannelEditDlg
VirtualFilter=dWC

[DLG:IDD_ChannelItemEdit]
Type=1
Class=CChnlItemEditDlg
ControlCount=20
Control1=IDC_ItemName,edit,1350631552
Control2=IDC_Broadcast,edit,1350631552
Control3=IDC_Expiration,edit,1350631552
Control4=IDC_Playable,edit,1350631552
Control5=IDC_ForceNormalSpeed,edit,1350631552
Control6=IDC_InTimeOffset,edit,1350631552
Control7=IDC_OutTimeOffset,edit,1350631552
Control8=IDC_SpliceIn,edit,1350631552
Control9=IDC_SpliceOut,edit,1350631552
Control10=IDOK,button,1342242817
Control11=IDCANCEL,button,1342242816
Control12=IDC_STATIC,static,1342308352
Control13=IDC_STATIC,static,1342308352
Control14=IDC_STATIC,static,1342308352
Control15=IDC_STATIC,static,1342308352
Control16=IDC_STATIC,static,1342308352
Control17=IDC_STATIC,static,1342308352
Control18=IDC_STATIC,static,1342308352
Control19=IDC_STATIC,static,1342308352
Control20=IDC_STATIC,static,1342308352

[CLS:CChnlItemEditDlg]
Type=0
HeaderFile=ChnlItemEditDlg.h
ImplementationFile=ChnlItemEditDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CChnlItemEditDlg

[MNU:IDRM_PropertyView]
Type=1
Class=?
Command1=IDM_PushItem
Command2=IDM_InsertItem
Command3=IDM_RemoveItem
Command4=IDM_ReplaceItem
Command5=IDM_ModifyItem
Command6=IDM_NewChannel
Command7=IDM_RemoveChannel
Command8=IDM_ModifyChannel
CommandCount=8

[MNU:IDRM_TreeView]
Type=1
Command1=IDM_NewChannel
Command2=IDM_ModifyChannel
Command3=IDM_NewItem
Command4=IDM_RemoveChannel
Command5=IDM_ModifyItem
Command6=IDM_ReplaceItem
Command7=IDM_RemoveItem
CommandCount=7

