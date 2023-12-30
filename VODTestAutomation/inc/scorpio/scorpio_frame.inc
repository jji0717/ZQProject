[ ] //*****************************************************************************
[ ] //  NAME:					  SCORPIO_FRAME.INC
[ ] //
[ ] //  DESCRIPTION:		Frame file for the scorpio component.
[ ] //
[ ] //  HISTORY:
[ ] //
[ ] //  Date						Developer					Description
[ ] //  ********				***********				*****************************************
[ ] //  01/16/03        M. Albanese       Initial design and development
[ ] //*****************************************************************************
[ ] 
[ ] const wMainWindow = SeaChangeITVAssetManager
[ ] 
[-] window MainWin SeaChangeITVAssetManager
	[+] multitag "SeaChange ITV Asset Manager*"
		[ ] "*?Asset View?"
		[ ] // "$D:\ITV\Exe\AMApp_d.exe" by rommy followed
		[ ] // "\\192.168.12.11\c$\ITV\Exe\AMApp.exe"
		[ ] "$C:\ITV\Exe\AMApp.exe"
	[ ] 
	[ ] // The working directory of the application when it is invoked
	[ ] // const sDir = "D:\ITV\Exe" by rommy followed
	[ ] // const sDir = "\\192.168.12.11\c$\ITV\Exe"
	[ ] const sDir = "C:\ITV\Exe"
	[ ] 
	[ ] // The command line used to invoke the application
	[ ] // const sCmdLine = "D:\ITV\Exe\AMApp_d.exe " by rommy followed
	[ ] // const sCmdLine = "\\192.168.12.11\c$\ITV\Exe\AMApp.exe "
	[ ] const sCmdLine = sDir+"\AMApp.exe "
	[ ] 
	[ ] // The first window to appear when the application is invoked
	[ ] const wStartup = Connect
	[ ] 
	[-] Invoke () 
		[ ] derived::Invoke ()
		[ ] // Connect.SiteServerName.SetText("ITVNODE101") by rommy followed
		[ ] Connect.SiteServerName.SetText("MDS1_SS_CL")
		[ ] // Connect.SiteServerName.SetText(gsAM1IP)
		[ ] Connect.OK.Click ()
	[ ] 
	[ ] // The list of windows the recovery system is to leave open
	[ ] const lwLeaveOpen = {AssetView, ManUtil}
	[ ] 
	[+] Menu Menu1
		[ ] tag "#1"
		[+] MenuItem Restore
			[+] multitag "Restore"
				[ ] "$61728"
		[+] MenuItem Move
			[+] multitag "Move"
				[ ] "$61456"
		[+] MenuItem Size
			[+] multitag "Size"
				[ ] "$61440"
		[+] MenuItem Minimize
			[+] multitag "Minimize"
				[ ] "$61472"
		[+] MenuItem Maximize
			[+] multitag "Maximize"
				[ ] "$61488"
		[+] MenuItem Close
			[+] multitag "Close"
				[ ] "$61536"
		[+] MenuItem Next
			[+] multitag "Next"
				[ ] "$61504"
	[+] Menu File
		[ ] tag "File"
		[+] MenuItem Connect
			[+] multitag "Connect"
				[ ] "$32780"
		[+] MenuItem Disconnect
			[+] multitag "Disconnect"
				[ ] "$32781"
		[+] MenuItem DoStateUpdateNow
			[+] multitag "Do State Update Now"
				[ ] "$32833"
		[+] MenuItem Import
			[+] multitag "Import"
				[ ] "$32839"
		[+] MenuItem Export
			[+] multitag "Export"
				[ ] "$32840"
		[+] MenuItem SaveAs
			[+] multitag "Save As"
				[ ] "$32827"
		[+] MenuItem Close
			[+] multitag "Close"
				[ ] "$57602"
		[+] MenuItem Exit
			[+] multitag "Exit"
				[ ] "$57665"
	[-] Menu Edit
		[ ] tag "Edit"
		[+] Menu New
			[ ] tag "New"
			[+] MenuItem Site
				[+] multitag "Site"
					[ ] "$32809"
			[+] MenuItem Application
				[+] multitag "Application"
					[ ] "$32782"
			[+] MenuItem Folder
				[+] multitag "Folder"
					[ ] "$32798"
			[+] MenuItem Asset
				[+] multitag "Asset"
					[ ] "$32811"
			[+] MenuItem AssetElement
				[+] multitag "Asset Element"
					[ ] "$32810"
		[-] MenuItem Modify
			[+] multitag "Modify"
				[ ] "$32793"
		[-] MenuItem Delete
			[+] multitag "Delete"
				[ ] "$32794"
		[-] MenuItem AddComponent
			[+] multitag "Add Component"
				[ ] "$32792"
		[+] MenuItem CompleteWorkQueueEntry
			[+] multitag "Complete Work Queue Entry"
				[ ] "$32822"
		[+] MenuItem Copy
			[+] multitag "Copy"
				[ ] "$57634"
		[+] MenuItem Paste
			[+] multitag "Paste"
				[ ] "$57637"
		[+] MenuItem MoveUp
			[+] multitag "Move Up"
				[ ] "$32801"
		[+] MenuItem MoveDown
			[+] multitag "Move Down"
				[ ] "$32802"
		[+] MenuItem StreamContent
			[+] multitag "Stream Content"
				[ ] "$32812"
		[+] MenuItem AssociatedAsset
			[-] multitag "Associated Asset"
				[ ] "$32843"
		[+] MenuItem ScheduleMetadataChange
			[+] multitag "Schedule Metadata Change"
				[ ] "$32838"
		[-] MenuItem Search
			[-] multitag "Search"
				[ ] "$32797"
		[-] Menu Advanced
			[ ] tag "Advanced"
			[-] MenuItem ChangeAeState
				[+] multitag "Change Ae State"
					[ ] "$32808"
			[-] MenuItem DeleteAe
				[+] multitag "Delete Ae"
					[ ] "$32820"
			[+] MenuItem AddWqe
				[+] multitag "Add Wqe"
					[ ] "$32818"
			[+] MenuItem DeleteWqe
				[+] multitag "Delete Wqe"
					[ ] "$32819"
			[+] MenuItem AbortWqe
				[+] multitag "Abort Wqe"
					[ ] "$32841"
	[+] Menu View
		[ ] tag "View"
		[+] MenuItem Toolbar
			[+] multitag "Toolbar"
				[ ] "$59392"
		[+] MenuItem StatusBar
			[+] multitag "Status Bar"
				[ ] "$59393"
		[+] MenuItem Asset
			[+] multitag "Asset"
				[ ] "$32771"
		[+] MenuItem AssetElement
			[+] multitag "Asset Element"
				[ ] "$32772"
		[+] MenuItem WorkQueue
			[+] multitag "Work Queue"
				[ ] "$32773"
		[+] MenuItem Expand
			[+] multitag "Expand"
				[ ] "$32817"
		[+] MenuItem Refresh
			[+] multitag "Refresh"
				[ ] "$32787"
		[+] MenuItem Filter
			[+] multitag "Filter"
				[ ] "$32788"
	[+] Menu Setup
		[ ] tag "Setup"
		[+] MenuItem Preferences
			[+] multitag "Preferences"
				[ ] "$32779"
		[+] Menu Views
			[ ] tag "Views"
			[+] MenuItem Site
				[+] multitag "Site"
					[ ] "$32813"
			[+] MenuItem Application
				[+] multitag "Application"
					[ ] "$32814"
			[+] MenuItem Folder
				[+] multitag "Folder"
					[ ] "$32831"
			[+] MenuItem AssetComponent
				[+] multitag "Asset Component"
					[ ] "$32776"
			[+] MenuItem AssetElement
				[+] multitag "Asset Element"
					[ ] "$32777"
			[+] MenuItem WorkQueue
				[+] multitag "Work Queue"
					[ ] "$32778"
		[+] MenuItem Application
			[+] multitag "Application"
				[ ] "$32775"
		[+] MenuItem MetadataDescriptor
			[+] multitag "Metadata Descriptor"
				[ ] "$32837"
		[+] MenuItem Propagation
			[+] multitag "Propagation"
				[ ] "$32834"
		[+] MenuItem DefaultValue
			[+] multitag "Default Value"
				[ ] "$32836"
		[+] MenuItem User
			[+] multitag "User"
				[ ] "$32835"
		[+] MenuItem System
			[+] multitag "System"
				[ ] "$32807"
	[+] Menu xWindow
		[ ] tag "Window"
		[+] MenuItem Cascade
			[+] multitag "Cascade"
				[ ] "$57650"
		[+] MenuItem TileHorizontally
			[+] multitag "Tile Horizontally"
				[ ] "$57651"
		[+] MenuItem TileVertically
			[+] multitag "Tile Vertically"
				[ ] "$57652"
		[+] MenuItem N1AssetView
			[+] multitag "1 Asset View"
				[ ] "$65280"
	[-] Menu Help
		[ ] tag "Help"
		[+] MenuItem HelpTopics
			[+] multitag "Help Topics"
				[ ] "$57667"
		[+] MenuItem OnlineUserSGuide
			[+] multitag "Online User's Guide"
				[ ] "$32842"
		[+] MenuItem AboutAssetManager
			[+] multitag "About Asset Manager"
				[ ] "$57664"
	[+] StatusBar StatusBar1
		[ ] tag "$59393"
		[+] DynamicText DynamicText1
			[ ] tag "#1"
		[+] DynamicText DynamicText2
			[ ] tag "#2"
		[+] DynamicText DynamicText3
			[ ] tag "#3"
		[+] DynamicText DynamicText4
			[ ] tag "#4"
	[-] CustomWin AfxControlBar42ud1
		[ ] msw tag "[AfxControlBar42ud]$59419"
		[-] ToolBar ToolBar1
			[ ] tag "$59392"
			[+] PushButton Connect
				[+] multitag "Connect"
					[ ] "$32780"
			[+] PushButton Asset
				[+] multitag "Asset"
					[ ] "$32771"
			[+] PushButton AssetElement
				[+] multitag "Asset Element"
					[ ] "$32772"
			[+] PushButton WorkQueue
				[+] multitag "Work Queue"
					[ ] "$32773"
			[+] PushButton MoveUp
				[+] multitag "Move Up"
					[ ] "$32801"
			[+] PushButton MoveDown
				[+] multitag "Move Down"
					[ ] "$32802"
			[+] PushButton Refresh
				[+] multitag "Refresh"
					[ ] "$32787"
			[-] PushButton Search
				[-] multitag "Search"
					[ ] "$32797"
			[+] PushButton System
				[+] multitag "System"
					[ ] "$32807"
			[+] PushButton Cascade
				[+] multitag "Cascade"
					[ ] "$57650"
			[+] PushButton TileHorizontally
				[+] multitag "Tile Horizontally"
					[ ] "$57651"
			[+] PushButton TileVertically
				[+] multitag "Tile Vertically"
					[ ] "$57652"
			[+] PushButton AboutAssetManager
				[+] multitag "About Asset Manager"
					[ ] "$57664"
			[+] CheckBox CheckBox1
				[ ] tag "$57669"
	[+] CustomWin AfxControlBar42ud4
		[ ] msw tag "[AfxControlBar42ud]$59422"
	[+] CustomWin AfxControlBar42ud2
		[ ] msw tag "[AfxControlBar42ud]$59420"
	[+] CustomWin AfxControlBar42ud3
		[ ] msw tag "[AfxControlBar42ud]$59421"
[ ] 
[ ] // Windows/panes within the main application container.
[+] window ChildWin AssetView
	[ ] tag "Asset View"
	[ ] parent SeaChangeITVAssetManager
	[-] CustomWin AfxMDIFrame42ud1
		[ ] msw tag "[AfxMDIFrame42ud]$59648"
		[-] TreeView TreeView1
			[ ] tag "$59648"
		[-] CustomWin AfxMDIFrame42ud1
			[ ] msw tag "[AfxMDIFrame42ud]$59649"
			[-] CustomWin AfxFrameOrView42ud1
				[ ] msw tag "[AfxFrameOrView42ud]$59648"
				[-] ListView ListView1
					[ ] tag "$1"
					[-] Header Header1
						[ ] tag "$0"
						[+] StaticText UidText
							[ ] tag "Uid"
						[+] StaticText NameText
							[ ] tag "Name"
						[+] StaticText ServerText
							[ ] tag "Server"
			[-] DialogBox DialogBox1
				[ ] tag "$59664"
[ ] 
[ ] // Autogen'd by Silk when frame is created. Use to handle multiple
[ ] // generic message boxes that may appear throughout your testing.
[+] window MessageBoxClass MessageBox
	[ ] tag "~ActiveApp/[DialogBox]$MessageBox"
	[+] PushButton OK
		[ ] tag "OK"
	[+] PushButton Cancel
		[ ] tag "Cancel"
	[+] PushButton Yes
		[ ] tag "Yes"
	[+] PushButton No
		[ ] tag "No"
	[+] StaticText Message
		[ ] motif tag "#2"
		[ ] tag "#1"
[ ] 
[ ] // Initial dialog that prompts for a system to conect to.
[+] window DialogBox Connect
	[ ] tag "Connect"
	[ ] parent SeaChangeITVAssetManager
	[+] StaticText SiteServerNameText
		[+] multitag "Site Server Name:"
			[ ] "$-1[1]"
	[+] ComboBox SiteServerName
		[+] multitag "Site Server Name:"
			[ ] "$1575"
	[+] PushButton OK
		[+] multitag "OK"
			[ ] "$1"
	[+] PushButton Cancel
		[+] multitag "Cancel"
			[ ] "$2"
	[+] PushButton Help
		[+] multitag "Help"
			[ ] "$57670"
	[+] StaticText EnterTheServerNameOfTheD
		[+] multitag "Enter the server name of the Directory at the site."
			[ ] "$-1[2]"
[ ] 
[ ] // Used to search for an asset or asset element.
[-] window DialogBox Search
	[ ] tag "Search"
	[ ] parent SeaChangeITVAssetManager
	[+] StaticText SearchSiteText
		[+] multitag "Search Site:"
			[ ] "$65535[1]"
	[+] PopupList SearchSite
		[+] multitag "Search Site:"
			[ ] "$1011"
	[+] StaticText SearchForText
		[+] multitag "Search For:"
			[ ] "$65535[2]"
	[+] PopupList SearchFor
		[+] multitag "Search For:"
			[ ] "$1496"
	[+] StaticText UidText
		[+] multitag "Uid:"
			[ ] "$1184"
	[+] TextField Uid
		[+] multitag "Uid:"
			[ ] "$1183"
	[+] PushButton Go
		[+] multitag "Go"
			[ ] "$1495"
	[-] PushButton Close
		[-] multitag "Close"
			[ ] "$1"
	[+] PushButton Help
		[+] multitag "Help"
			[ ] "$57670"
[ ] // Comes up when a searched for asset is not located.
[+] window DialogBox AM_FailedToFind
	[ ] tag "Asset Manager"
	[ ] parent Search
	[-] PushButton OK
		[-] multitag "OK"
			[ ] "$2"
	[-] CustomWin Icon1
		[ ] msw tag "[Icon]$10"
	[-] StaticText FailedToFindAssetWithUid
		[-] multitag "Failed to find asset with uid 00081481. (IDS) The specified object does not exist.. (Error code 0xc1020004, 0)"
			[ ] "$65535"
[ ] // Comes up when a searched for asset is located.
[+] window DialogBox SearchAssetResult
	[ ] tag "Search Asset Result"
	[ ] parent Search
	[+] StaticText NameText
		[+] multitag "Name:"
			[ ] "$65535[1]"
	[+] TextField Name
		[+] multitag "Name:"
			[ ] "$1017"
	[+] StaticText StateText
		[+] multitag "State:"
			[ ] "$65535[2]"
	[+] TextField State
		[+] multitag "State:"
			[ ] "$1018"
	[+] StaticText UidText
		[+] multitag "Uid:"
			[ ] "$65535[3]"
	[+] TextField Uid
		[+] multitag "Uid:"
			[ ] "$1565"
	[+] StaticText InFoldersText
		[+] multitag "In Folders:"
			[ ] "$65535[4]"
	[+] ListView InFolders
		[+] multitag "In Folders:"
			[ ] "$1206"
		[+] Header Header1
			[ ] tag "$0"
			[+] StaticText FullPathText
				[ ] tag "Full Path"
	[+] PushButton OK
		[+] multitag "OK"
			[ ] "$1"
	[+] PushButton Modify
		[+] multitag "Modify"
			[ ] "$1012"
	[+] PushButton Delete
		[+] multitag "Delete"
			[ ] "$1014"
	[+] PushButton Copy
		[+] multitag "Copy"
			[ ] "$1443"
	[+] PushButton Help
		[+] multitag "Help"
			[ ] "$57670"
[ ] 
[ ] // Comes up when a searched for asset element is located.
[+] window DialogBox SearchAssetElementResult
	[ ] tag "Search Asset Element Result"
	[ ] parent Search
	[+] StaticText NameText
		[+] multitag "Name:"
			[ ] "$65535[1]"
	[+] TextField Name
		[+] multitag "Name:"
			[ ] "$1017"
	[+] StaticText StateText
		[+] multitag "State:"
			[ ] "$65535[2]"
	[+] TextField State
		[+] multitag "State:"
			[ ] "$1018"
	[+] StaticText UidText
		[+] multitag "Uid:"
			[ ] "$65535[3]"
	[+] TextField Uid
		[+] multitag "Uid:"
			[ ] "$1565"
	[+] StaticText ElementOfAssetsText
		[+] multitag "Element of Assets:"
			[ ] "$65535[4]"
	[+] ListView ElementOfAssets
		[+] multitag "Element of Assets:"
			[ ] "$1195"
		[+] Header Header1
			[ ] tag "$0"
			[+] StaticText UidText
				[ ] tag "uid"
			[+] StaticText NameText
				[ ] tag "Name"
			[+] StaticText StateText
				[ ] tag "State"
	[+] PushButton OK
		[+] multitag "OK"
			[ ] "$1"
	[+] PushButton Modify
		[+] multitag "Modify"
			[ ] "$1012"
	[+] PushButton Copy
		[+] multitag "Copy"
			[ ] "$1443"
	[+] PushButton Help
		[+] multitag "Help"
			[ ] "$57670"
[ ] 
[ ] // Comes up when you click the Modify... button in the
[ ] // SearchAssetElementResult dialog.
[+] window DialogBox ModifyAE
	[ ] tag "Modify AE"
	[ ] parent SearchAssetElementResult
	[+] DialogBox General
		[+] multitag "General"
			[ ] "$0"
		[+] StaticText NameText
			[+] multitag "Name:"
				[ ] "$-1[2]"
		[+] TextField Name
			[+] multitag "Name:"
				[ ] "$1017"
		[+] CheckBox IsContentAvailable
			[+] multitag "Is content available?"
				[ ] "$1024"
		[+] StaticText DescriptionText
			[+] multitag "Description:"
				[ ] "$-1[3]"
		[+] TextField Description
			[+] multitag "Description:"
				[ ] "$1021"
		[+] StaticText PriorityText
			[+] multitag "Priority:"
				[ ] "$-1[4]"
		[+] StaticText N0TheLowestPriority100Th
			[+] multitag "(0 the lowest priority, 100 the highest)"
				[ ] "$-1[5]"
		[+] TextField Priority
			[+] multitag "Priority:"
				[ ] "$1044"
		[+] UpDown Spin1
			[+] multitag "Spin1"
				[ ] "$1042"
		[+] StaticText UidText
			[+] multitag "Uid:"
				[ ] "$1098"
		[+] TextField Uid
			[+] multitag "Uid:"
				[ ] "$1565"
		[+] StaticText StateText
			[+] multitag "State:"
				[ ] "$1070"
		[+] TextField State
			[+] multitag "State:"
				[ ] "$1018"
		[+] StaticText SiteNameText
			[+] multitag "Site Name:"
				[ ] "$-1[1]"
		[+] TextField SiteName
			[+] multitag "Site Name:"
				[ ] "$1004"
	[+] DialogBox Content
		[+] multitag "Content"
			[ ] "$0"
		[+] StaticText ProviderText
			[+] multitag "Provider:"
				[ ] "$-1[1]"
		[+] ComboBox Provider
			[+] multitag "Provider:"
				[ ] "$1525"
		[+] StaticText ContentTypeText
			[+] multitag "Content Type:"
				[ ] "$-1[2]"
		[+] PopupList ContentType
			[+] multitag "Content Type:"
				[ ] "$1047"
		[+] StaticText ConversionText
			[+] multitag "Conversion"
				[ ] "$-1[3]"
		[+] StaticText MethodText
			[+] multitag "Method:"
				[ ] "$-1[4]"
		[+] PopupList Method
			[+] multitag "Method:"
				[ ] "$1091"
		[+] StaticText AssignTo1Text
			[+] multitag "Assign To:[1]"
				[ ] "$-1[5]"
		[+] PopupList AssignTo1
			[+] multitag "Assign To:[1]"
				[ ] "$1571"
		[+] StaticText VerificationText
			[+] multitag "Verification"
				[ ] "$-1[6]"
		[+] StaticText PreloadText
			[+] multitag "Preload:"
				[ ] "$-1[7]"
		[+] PopupList Preload
			[+] multitag "Preload:"
				[ ] "$1092"
		[+] StaticText AssignTo2Text
			[+] multitag "Assign To:[2]"
				[ ] "$-1[8]"
		[+] PopupList AssignTo2
			[+] multitag "Assign To:[2]"
				[ ] "$1096"
		[+] StaticText PostloadText
			[+] multitag "Postload:"
				[ ] "$-1[9]"
		[+] PopupList Postload
			[+] multitag "Postload:"
				[ ] "$1094"
		[+] StaticText AssignTo3Text
			[+] multitag "Assign To:[3]"
				[ ] "$-1[10]"
		[+] PopupList AssignTo3
			[+] multitag "Assign To:[3]"
				[ ] "$1569"
	[-] DialogBox Dates
		[+] multitag "Dates"
			[ ] "$0"
		[+] StaticText ReleaseText
			[+] multitag "Release:"
				[ ] "$-1[2]"
		[-] CustomWin N11011
			[-] msw multitag "[SysDateTimePick32]1?1?01[1]"
				[ ] "[SysDateTimePick32]$1053"
		[-] CustomWin N120000AM1
			[-] msw multitag "[SysDateTimePick32]12:00:00 AM[1]"
				[ ] "[SysDateTimePick32]$1059"
			[+] UpDown UpDown1
				[ ] tag "$1000"
		[+] StaticText UploadText
			[+] multitag "Upload:"
				[ ] "$-1[3]"
		[-] CustomWin N11012
			[-] msw multitag "[SysDateTimePick32]1?1?01[2]"
				[ ] "[SysDateTimePick32]$1116"
		[-] CustomWin N120000AM2
			[-] msw multitag "[SysDateTimePick32]12:00:00 AM[2]"
				[ ] "[SysDateTimePick32]$1576"
			[+] UpDown UpDown1
				[ ] tag "$1000"
		[+] StaticText ActivateText
			[+] multitag "Activate:"
				[ ] "$-1[4]"
		[-] CustomWin N11013
			[-] msw multitag "[SysDateTimePick32]1?1?01[3]"
				[ ] "[SysDateTimePick32]$1054"
		[-] CustomWin N120000AM3
			[-] msw multitag "[SysDateTimePick32]12:00:00 AM[3]"
				[ ] "[SysDateTimePick32]$1060"
			[+] UpDown UpDown1
				[ ] tag "$1000"
		[+] StaticText DeactivateText
			[+] multitag "Deactivate:"
				[ ] "$-1[5]"
		[-] CustomWin N123010
			[-] msw multitag "[SysDateTimePick32]12?30?10"
				[ ] "[SysDateTimePick32]$1056"
		[-] CustomWin N120000AM4
			[-] msw multitag "[SysDateTimePick32]12:00:00 AM[4]"
				[ ] "[SysDateTimePick32]$1062"
			[+] UpDown UpDown1
				[ ] tag "$1000"
		[+] StaticText DeleteText
			[+] multitag "Delete:"
				[ ] "$-1[6]"
		[-] CustomWin N123110
			[-] msw multitag "[SysDateTimePick32]12?31?10"
				[ ] "[SysDateTimePick32]$1057"
		[-] CustomWin N120000AM5
			[-] msw multitag "[SysDateTimePick32]12:00:00 AM[5]"
				[ ] "[SysDateTimePick32]$1064"
			[+] UpDown UpDown1
				[ ] tag "$1000"
		[+] StaticText DateTimeText
			[+] multitag "Date Time"
				[ ] "$-1[1]"
	[+] DialogBox EncodedFile
		[+] multitag "Encoded File"
			[ ] "$0"
		[+] StaticText SourceFileText
			[+] multitag "Source File"
				[ ] "$-1[1]"
		[+] StaticText PathText
			[+] multitag "Path:"
				[ ] "$-1[2]"
		[+] TextField Path
			[+] multitag "Path:"
				[ ] "$1051"
		[+] StaticText NameText
			[+] multitag "Name:"
				[ ] "$-1[3]"
		[+] TextField Name
			[+] multitag "Name:"
				[ ] "$1017"
		[+] PushButton Browse
			[+] multitag "Browse"
				[ ] "$1007"
		[+] CheckBox DeleteSourceFileAfterItIs
			[+] multitag "Delete source file after it is uploaded"
				[ ] "$1025"
		[+] CheckBox SourceFileHasBeenTransferr
			[+] multitag "Source file has been transferred to the uploading area"
				[ ] "$1380"
	[+] DialogBox Advanced
		[+] multitag "Advanced"
			[ ] "$0"
		[+] StaticText MetadataText
			[+] multitag "Metadata:"
				[ ] "$-1"
		[+] ListView Metadata
			[+] multitag "Metadata:"
				[ ] "$1010"
			[+] Header Header1
				[ ] tag "$0"
				[+] PushButton Name
					[ ] tag "Name"
				[+] PushButton Value
					[ ] tag "Value"
		[+] PushButton Add
			[+] multitag "Add"
				[ ] "$1002"
		[+] PushButton Modify
			[+] multitag "Modify"
				[ ] "$1012"
		[+] PushButton Delete
			[+] multitag "Delete"
				[ ] "$1014"
	[ ] 
	[+] PushButton OK
		[+] multitag "OK"
			[ ] "$1"
	[+] PushButton Cancel
		[+] multitag "Cancel"
			[ ] "$2"
	[+] PushButton Help
		[+] multitag "Help"
			[ ] "$9"
	[+] PageList PageList
		[ ] tag "$12320"
	[ ] 
[ ] 
[+] window DialogBox PropagationNetworkIPAddress
	[ ] tag "Propagation Network IP Address"
	[ ] parent ModifyAE
	[+] StaticText ThePropgationNetworkShould
		[+] multitag "The Propgation network should always be used for uploading content files to the ITV MediaClusters. To ensure that the"
			[ ] "$65535[1]"
	[+] StaticText PropagationServiceUsesThis
		[+] multitag "Propagation service uses this network for upload operations, the content file path must specify the machine by its Propagatio*"
			[ ] "$65535[2]"
	[+] StaticText CanMapAMachineNameToIts
		[+] multitag "can map a machine name to its corresponding Propagation network IP address and automatically replace the machine name with it*"
			[ ] "$65535[3]"
	[+] StaticText IfITVNODE101IsOnTheProp
		[+] multitag "If ?ITVNODE101? is on the Propagation network, please specify the Propagation network IP address."
			[ ] "$1549"
	[+] StaticText PropagationNetworkIPText
		[+] multitag "Propagation Network IP:"
			[ ] "$65535[4]"
	[+] CustomWin N0000
		[+] msw multitag "[SysIPAddress32]0.0.0.0"
			[ ] "[SysIPAddress32]$1545"
		[+] TextField TextField4
			[ ] tag "$1545[4]"
		[+] TextField TextField3
			[ ] tag "$1545[3]"
		[+] TextField TextField2
			[ ] tag "$1545[2]"
		[+] TextField TextField1
			[ ] tag "$1545[1]"
	[+] StaticText ClickOKToMapITVNODE101T
		[+] multitag "Click OK to map ?ITVNODE101? to its Propagation network IP address and replace ?ITVNODE101? with this IP address in the curren*"
			[ ] "$1550"
	[+] PushButton OK
		[+] multitag "OK"
			[ ] "$1"
	[+] PushButton Cancel
		[+] multitag "Cancel"
			[ ] "$2"
	[+] PushButton Help
		[+] multitag "Help"
			[ ] "$57670"
[ ] 
[-] window DialogBox AMSiteDisconnectPrompt
	[ ] tag "Asset Manager"
	[ ] parent SeaChangeITVAssetManager
	[+] PushButton Yes
		[+] multitag "Yes"
			[ ] "$6"
	[+] PushButton No
		[+] multitag "No"
			[ ] "$7"
	[+] CustomWin Icon1
		[ ] msw tag "[Icon]$10"
	[+] StaticText DisconnectingFromTheSiteWi
		[+] multitag "Disconnecting from the site will close all the views. Continue?"
			[ ] "$65535"
[ ] 
[ ] // Manutil
[-] window MainWin ManUtil
	[-] multitag "SeaChange Service Management Utility"
		[ ] // "$d:\Itv\exe\manutil_d.exe" by rommy followed
		[ ] "*Inspect Services on *"
		[ ] // "$\\192.168.12.11\c$\itv\Exe\ManUtil.exe"
		[ ] "$C:\ITV\Exe\ManUtil.exe"
		[ ] // "$\\"+gsAM1IP+"\"+gsITVRemoteRoot+"Exe\ManUtil.exe"
	[ ] 
	[+] Menu File
		[ ] tag "File"
		[-] MenuItem ManageLocalApplications
			[-] multitag "Manage Local Applications"
				[ ] "$32773"
		[-] MenuItem ManageRemoteApplications
			[-] multitag "Manage Remote Applications"
				[ ] "$32772"
		[+] MenuItem ControlLocalServices
			[+] multitag "Control Local Services"
				[ ] "$32796"
		[+] MenuItem ControlRemoteServices
			[+] multitag "Control Remote Services"
				[ ] "$32791"
		[+] MenuItem Save
			[+] multitag "Save"
				[ ] "$32828"
		[+] MenuItem CloseWindow
			[+] multitag "Close Window"
				[ ] "$57602"
		[+] MenuItem Exit
			[+] multitag "Exit"
				[ ] "$57665"
	[+] Menu Services
		[ ] tag "Services"
		[+] MenuItem Start
			[+] multitag "Start"
				[ ] "$32798"
		[+] MenuItem StartDependents
			[+] multitag "Start Dependents"
				[ ] "$32799"
		[+] MenuItem Stop
			[+] multitag "Stop"
				[ ] "$32820"
		[+] MenuItem Restart
			[+] multitag "Restart"
				[ ] "$32803"
		[+] MenuItem AddService
			[+] multitag "Add Service"
				[ ] "$32814"
		[+] MenuItem RemoveService
			[+] multitag "Remove Service"
				[ ] "$32815"
		[+] MenuItem ChangeDependencies
			[+] multitag "Change Dependencies"
				[ ] "$32813"
		[+] Menu Startup
			[ ] tag "Startup"
			[+] MenuItem Automatic
				[+] multitag "Automatic"
					[ ] "$32800"
			[+] MenuItem Manual
				[+] multitag "Manual"
					[ ] "$32801"
			[+] MenuItem Disabled
				[+] multitag "Disabled"
					[ ] "$32802"
		[+] Menu Process
			[ ] tag "Process"
			[+] MenuItem OwnProcess
				[+] multitag "Own Process"
					[ ] "$32810"
			[+] MenuItem SharedProcess
				[+] multitag "Shared Process"
					[ ] "$32811"
			[+] MenuItem InteractiveProcess
				[+] multitag "Interactive Process"
					[ ] "$32812"
		[+] MenuItem ShowAllServices
			[+] multitag "Show All Services"
				[ ] "$32821"
		[+] MenuItem Properties
			[+] multitag "Properties"
				[ ] "$32817"
	[+] Menu View
		[ ] tag "View"
		[+] MenuItem Toolbar
			[+] multitag "Toolbar"
				[ ] "$59392"
		[+] MenuItem StatusBar
			[+] multitag "Status Bar"
				[ ] "$59393"
		[+] MenuItem AutoExpand
			[+] multitag "Auto Expand"
				[ ] "$32778"
		[+] MenuItem SortByType
			[+] multitag "Sort by Type"
				[ ] "$32775"
		[+] MenuItem SortByName
			[+] multitag "Sort by Name"
				[ ] "$32776"
		[+] MenuItem Refresh
			[+] multitag "Refresh"
				[ ] "$32774"
	[+] Menu xWindow
		[ ] tag "Window"
		[+] MenuItem Cascade
			[+] multitag "Cascade"
				[ ] "$57650"
		[+] MenuItem Tile
			[+] multitag "Tile"
				[ ] "$57651"
		[+] MenuItem ArrangeIcons
			[+] multitag "Arrange Icons"
				[ ] "$57649"
		[+] MenuItem Split
			[+] multitag "Split"
				[ ] "$57653"
	[+] Menu Help
		[ ] tag "Help"
		[+] MenuItem HelpTopics
			[+] multitag "Help Topics"
				[ ] "$57667"
		[+] MenuItem AboutManagementUtility
			[+] multitag "About Management Utility"
				[ ] "$57664"
	[+] StatusBar StatusBar1
		[ ] tag "$59393"
		[+] DynamicText DynamicText1
			[ ] tag "#1"
		[+] DynamicText DynamicText2
			[ ] tag "#2"
	[+] CustomWin AfxControlBar42ud1
		[ ] msw tag "[AfxControlBar42ud]$59419"
		[+] ToolBar ToolBar1
			[ ] tag "$59392"
			[+] PushButton ManageRemoteApplications
				[+] multitag "Manage Remote Applications"
					[ ] "$32772"
			[+] PushButton ControlRemoteServices
				[+] multitag "Control Remote Services"
					[ ] "$32791"
			[+] PushButton Save
				[+] multitag "Save"
					[ ] "$32828"
			[+] PushButton Refresh
				[+] multitag "Refresh"
					[ ] "$32774"
			[+] PushButton Cascade
				[+] multitag "Cascade"
					[ ] "$57650"
			[+] PushButton Tile
				[+] multitag "Tile"
					[ ] "$57651"
			[+] PushButton Split
				[+] multitag "Split"
					[ ] "$57653"
			[+] PushButton AboutManagementUtility
				[+] multitag "About Management Utility"
					[ ] "$57664"
			[+] CheckBox CheckBox1
				[ ] tag "$57669"
	[+] CustomWin AfxControlBar42ud4
		[ ] msw tag "[AfxControlBar42ud]$59422"
	[-] CustomWin AfxControlBar42ud2
		[ ] msw tag "[AfxControlBar42ud]$59420"
	[+] CustomWin AfxControlBar42ud3
		[ ] msw tag "[AfxControlBar42ud]$59421"
[ ] 
[+] window ChildWin ManUtilInspectServices
	[ ] // tag "Inspect Services on *" by rommy followed
	[ ] tag "Inspect Services on *"
	[ ] parent ManUtil
	[-] CustomWin Frame
		[ ] // msw tag "[AfxMDIFrame42ud]$59648" by rommy followed
		[ ] msw tag "[AfxMDIFrame42u]$59648"
		[-] TreeView LeftPane
			[ ] tag "$59648"
		[-] CustomWin Frame
			[ ] // msw tag "[AfxMDIFrame42ud]$59649" by rommy followed
			[ ] msw tag "[AfxMDIFrame42u]$59649"
			[-] ListView TopRightPane
				[ ] tag "$59648"
				[+] Header Header1
					[ ] tag "$0"
					[+] PushButton Variable
						[ ] tag "Variable"
					[+] PushButton Value
						[ ] tag "Value"
			[-] ListView BottomRightPane
				[ ] tag "$59664"
				[-] Header Header1
					[ ] tag "$0"
					[-] PushButton AssetID
						[ ] tag "Asset ID"
					[-] PushButton BitRateKbps
						[ ] tag "BitRate (Kbps)"
					[-] PushButton ReplicaCount
						[ ] tag "ReplicaCount"
[ ] 
[+] window DialogBox SelectComputer
	[ ] tag "Select Computer"
	[ ] parent ManUtil
	[+] StaticText ComputerText
		[+] multitag "Computer:"
			[ ] "$-1"
	[+] TextField Computer
		[+] multitag "Computer:"
			[ ] "$1002"
	[+] PushButton OK
		[+] multitag "OK"
			[ ] "$1"
	[+] PushButton Cancel
		[+] multitag "Cancel"
			[ ] "$2"
	[+] PushButton Browse
		[+] multitag "Browse"
			[ ] "$1003"
	[+] TreeView SelectComputer
		[+] multitag "Select Computer:"
			[ ] "$1000"
	[+] StaticText SelectComputerText
		[+] multitag "Select Computer:"
			[ ] "$1001"
	[+] PushButton Help
		[+] multitag "Help"
			[ ] "$9"
[ ] 
