[ ] //*****************************************************************************
[ ] //  NAME:					  SCORPIO.T
[ ] //
[ ] //  DESCRIPTION:		Main test script for creating and verifying assets.
[ ] //
[ ] //  HISTORY:
[ ] //
[ ] //  Date						Developer					Description
[ ] //  ********				***********				*****************************************
[ ] //  11/21/02        M. Albanese       Initial design and development
[ ] //  01/03/03				M. Albanese				Initial work on tcCreateAsset0001
[ ] //  01/08/03				M. Albanese				Ongoing development of tcCreateAsset0001
[ ] //  09/09/04				rommy							Add and modify error report
[ ] //*****************************************************************************
[ ] 
[ ] use "..\..\inc\global\zodiacfunc.inc"
[ ] use "..\..\inc\global\zodiacnt.inc"
[ ] use "..\..\inc\global\zodiacfiles.inc"
[ ] use "..\..\inc\global\zodiacutil.inc"
[ ] use "..\..\inc\global\zodiacdefs.inc"
[ ] use "..\..\inc\global\zodiacmsgs.inc"
[ ] use "..\..\inc\global\zodiacdatabase.inc"
[ ] use "mswconst.inc"
[ ] use "..\..\inc\scorpio\scorpiodefs.inc"
[ ] use "..\..\inc\scorpio\scorpiomsgs.inc"
[ ] use "..\..\inc\scorpio\scorpiofunc.inc"
[ ] use "..\..\inc\scorpio\scorpio_frame.inc"
[ ] 
[ ] // for base state
[ ] use "..\..\inc\global\basestate.inc"
[ ] 
[ ] 
[ ] // Store the current script name.
[ ] // const string gsScriptName = Trim(StrTran(GetProgramName( ), ".t", " "))
[ ] 
[ ] // Read the project ini file to determine where the data drive is.
[ ] // const string gsDataDrive = fReadWriteIni(SYS_GetDrive() + ":\autotest\zodiac\" +
    // 														 						gsScriptName + "\" + gsScriptName + ".ini", csRead,
    // 																				"ITVQA","DataDrive","")
[ ] // Build the PassFailLog string
[ ] // const string gsPassFailLog = csInstallPath + csMainLogPath + gsScriptName + "\" + gsScriptName + "log"
[ ] 
[-] main()
	[ ] // Create the Scorpio Pass/Fail log.
	[ ] // fCreatePassFailLog()
	[ ] // Calling all testcases
	[ ] tcCreateAsset0001()
[ ] 
[ ] //*****************************************************************************
[ ] // Testcase execution begins here
[ ] //*****************************************************************************
[ ] 
[ ] //*****************************************************************************
[ ] //  NAME:					  tcCreateAsset0001 ()
[ ] //
[ ] //  DESCRIPTION:		Baseline test case for creating and verifying an asset
[ ] //									(end-to-end).
[ ] //
[ ] //  HISTORY:
[ ] //
[ ] //  Date						Developer					Description
[ ] //  ********				***********				*****************************************
[ ] //  01/03/03				M. Albanese				Initial design and development.
[ ] //*****************************************************************************
[-] testcase tcCreateAsset0001() appstate BaseState
	[ ] 
	[ ] boolean bFoundString = FALSE
	[ ] boolean bImportDirExists = true
	[ ] boolean bRetVal = FALSE
	[ ] HDATABASE hDBHandle
	[ ] HFILE hRptFile
	[ ] integer i = 0
	[ ] integer iBitRate
	[ ] integer iPos = 0
	[ ] integer iCount = 0
	[ ] integer iDBAssetID
	[ ] integer iListIndex = 0
	[ ] integer iManUtilIndex = 0
	[ ] integer iPlayTime = 0
	[ ] integer iPlayTimeFraction = 0
	[ ] integer iRetVal = 0
	[ ] integer iRptFileSize = 0
	[ ] integer iTick = 0
	[ ] list of FILEINFO lfImportedDirConts = {}
	[ ] list of string lsAeMetaData = {}
	[ ] list of string lsScpDirOut = {}
	[ ] string sAMServer = csEmptyString
	[ ] string sAActiveTime = csEmptyString
	[ ] string sADeactiveTime = csEmptyString
	[ ] string sAeActiveDate = csEmptyString
	[ ] string sAeDeactiveDate = csEmptyString
	[ ] string sAeDeleteDate = csEmptyString
	[ ] string sAeContentName1 = csEmptyString
	[ ] string sAeID =	csEmptyString
	[ ] string sAeName1 = csEmptyString
	[ ] string sAeName1DT = csEmptyString
	[ ] string sAeContentPath = csEmptyString
	[ ] string sAeReleaseDate = csEmptyString
	[ ] string sAeUploadDate = csEmptyString
	[ ] string sAssetID = csEmptyString
	[ ] string sAssetIDBase10 = csEmptyString
	[ ] string sAssetName1 = csEmptyString
	[ ] string sAssetName1DT = csEmptyString
	[ ] string sAssetFolder = csEmptyString
	[ ] string sCMServer	=	csEmptyString
	[ ] string sConnectString = csEmptyString
	[ ] string sDBAssetID = csEmptyString
	[ ] string sDTStamp = csEmptyString
	[ ] string sExpRepCount = csEmptyString
	[ ] string sImportedDir = csEmptyString
	[ ] string sImportPath = csEmptyString
	[ ] string sItvFileName = csEmptyString
	[ ] string sMasterFile = csEmptyString
	[ ] string sMasterPath = csEmptyString
	[ ] string sMDItemText	= csEmptyString
	[ ] string sMDItemText2	=	csEmptyString
	[ ] string sNewPathPart = csEmptyString
	[ ] string sQueryString = csEmptyString
	[ ] string sReplicaCount = csEmptyString
	[ ] string sReturnedAeID = csEmptyString
	[ ] string sReturnedAeName = csEmptyString
	[ ] string sReturnedAssetID = csEmptyString
	[ ] string sReturnedAssetName = csEmptyString
	[ ] string sRptFileLine = csEmptyString
	[ ] string sRptFileName = csEmptyString
	[ ] string sRptAssetFolder = csEmptyString
	[ ] string sTruncatedAeName = csEmptyString
	[ ] string sTruncatedAssetName = csEmptyString
	[ ] string sUploadServer = csEmptyString
	[ ] string sWorkingFile = csEmptyString
	[ ] string sWorkingFileNameDT = csEmptyString
	[ ] string sWorkingPath = csEmptyString
	[ ] 
	[ ] gsScriptName = Trim(StrTran(GetProgramName( ), ".t", " "))
	[ ] gsPassFailLog = csInstallPath + csMainLogPath + gsScriptName + "\" +gsScriptName + "log"
	[ ] // Create the Scorpio Pass/Fail log.
	[ ] fCreatePassFailLog()
	[ ] 
	[ ] //*****************************************************************************
	[ ] // Setup Begins Here
	[ ] //*****************************************************************************
	[ ] // start AM
	[ ] SeaChangeITVAssetManager.Invoke ()
	[ ] // Maximize the Asset Manager Application until we need it.
	[ ] SeaChangeITVAssetManager.Maximize()
	[ ] 
	[ ] // Read in the name of the AssetManager server.
	[ ] gsAM1IP = fReadWriteIni(csInstallPath+filZodiacIni,csRead,secCommandCenter,entAM1IP, "")
	[ ] gsLocalITVRoot = fReadWriteIni(csInstallPath+filZodiacIni,csRead,secITVRoot,entLocalITVRoot, "")
	[ ] gsRemoteITVRoot = fReadWriteIni(csInstallPath+filZodiacIni,csRead,secITVRoot,entRemoteITVRoot, "")
	[ ] 
	[ ] // Set up an IPC connection to the AM server.
	[ ] // iRetVal = SYS_Execute("net use \\" + gsAM1IP + "\ipc$ /u:administrator itv")
	[ ] // If we fail the IPC Connect, we log the error and exit the script.
	[-] // if iRetVal != 0
		[ ] // fLogPassFail("Failed",cmsgAMIPCConnect, cerrFailedIPCConnect)
	[ ] 
	[ ] // See if the IAM service is running. If it is, shut it down.
	[ ] // iRetVal = SYS_Execute("d:\itv\exe\instserv iam stop itvnode101")  by rommy followed
	[ ] ResOpenList("Step 1 - Stop iam service")
	[ ] iRetVal = SYS_Execute("\\"+gsAM1IP+"\"+gsRemoteITVRoot+"exe\instserv iam stop "+gsAM1IP) 
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 2 - Create then write a datetime stamp to the scorpiocfg.ini file")
	[ ] // Create then write a datetime stamp to the scorpiocfg.ini file.
	[ ] sDTStamp = fCreateDateTimeStamp(1)
	[ ] Print("datetime stamp = " + sDTStamp)
	[ ] Print("Modify StartDateTime with: " + sDTStamp)
	[ ] fReadWriteIni(csInstallPath+filScorpioIni,csWrite,secGeneric,entStartDateTime, sDTStamp) 
	[ ] ResCloseList()
	[ ] 
	[ ] // Rename iam.log to iam_datetime.bak
	[ ] // fCreateBackupFile("\\itvnode101\c$\itv\log\iam.log", "\\itvnode101\c$\itv\log\iam_"
    // + sDTStamp + ".bak") by rommy followed
	[ ] ResOpenList("Step 3 - Rename iam.log to iam_" + sDTStamp + ".bak")
	[ ] Print()
	[ ] fCreateBackupFile("\\"+gsAM1IP+"\"+gsRemoteITVRoot+"log\iam.log", "\\"+gsAM1IP+"\c$\itv\log\iam_"
    + sDTStamp + ".bak")
	[ ] ResCloseList()
	[ ] 
	[ ] // Restart the iam service.
	[ ] // iRetVal = SYS_Execute("d:\itv\exe\instserv iam start itvnode101")  by rommy followed
	[ ] ResOpenList("Step 4 - Restart the iam service")
	[ ] iRetVal = SYS_Execute("\\"+gsAM1IP+"\"+gsRemoteITVRoot+"exe\instserv iam start "+gsAM1IP)
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 5 - If master ITV data file exists, copy it to the working itv file")
	[ ] // Verify master ITV data file exists.
	[ ] sMasterPath	=	fReadWriteIni(csInstallPath+filScorpioIni,csRead,secImport,entITVMasterDataPath,"")
	[ ] sMasterFile = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secImport,entITVMasterDataFile,"")
	[ ] sMasterFile = sMasterPath + sMasterFile
	[ ] Print("Expected master ITV data file path = " + sMasterFile)
	[-] if SYS_FileExists(sMasterFile)
		[ ] Print("Master ITV data file exists")
		[ ] // Piece together the path to and name of the working version of the itv file.
		[ ] sWorkingPath = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secImport,entITVWorkingDataPath,"")
		[ ] sWorkingFile = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secImport,entITVWorkingDataFile,"")
		[ ] sWorkingFileNameDT = sWorkingFile + "_" + sDTStamp + ".itv"
		[ ] sWorkingFile = sWorkingPath + sWorkingFile + "_" + sDTStamp + ".itv"
		[ ] Print("The working itv file path = " + sWorkingFile)
		[ ] // For uniqueness, we tack on the current date and time to the end of the working
		[ ] // itv file name. We need to store this unique name in scorpiocfg.ini for future
		[ ] // use.
		[ ] Print("Modify LastWorkingDataFile in scorpiocfg.ini with: " + sWorkingFileNameDT)
		[ ] fReadWriteIni(csInstallPath+filScorpioIni,csWrite,secImport,entLastWorkingDataFile,sWorkingFileNameDT) 
		[ ] Print("Copy the master data file to the working itv file")
		[ ] // Copy the master data file to the working itv file.
		[ ] SYS_CopyFile(sMasterFile, sWorkingFile)
	[-] else
		[ ] // ERROR*** We need an error handler here.
		[ ] LogError("*** Error: Master ITV data file does not exist")
		[ ] TestCaseExit(True)
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 6 - Retrieve the Asset & Ae data info from scorpiocfg.ini, then modify with date/time stamp appended")
	[ ] // Retrieve the Asset & Ae data info from scorpiocfg.ini.
	[ ] sAssetName1 = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secAssetData,entAssetName1,"")
	[ ] sAeName1 = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secAssetData,entAeName1,"")
	[ ] sAssetName1DT = sAssetName1 + "_" + sDTStamp
	[ ] sAeName1DT = sAeName1 + "_" + sDTStamp
	[ ] Print("AssetName1 = " + sAssetName1)
	[ ] Print("AeName1 = " + sAeName1)
	[ ] Print("AssetName1DT = " + sAssetName1DT)
	[ ] Print("AeName1DT = " + sAeName1DT)
	[ ] // For uniqueness, we tack on the current date and time to the end of the Asset
	[ ] // and Ae object names. We need to store these unique names in scorpiocfg.ini
	[ ] // for future use.
	[ ] Print("Modify AssetName1DT with: " + sAssetName1DT)
	[ ] fReadWriteIni(csInstallPath+filScorpioIni,csWrite,secAssetData,entAssetName1DT,sAssetName1DT)
	[ ] Print("Modify AeName1DT with: " + sAeName1DT)
	[ ] fReadWriteIni(csInstallPath+filScorpioIni,csWrite,secAssetData,entAeName1DT,sAeName1DT)
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 7 - Modify the working itv file with info from scorpiocfg")
	[ ] // Now we need to modify the working itv file with the names of the
	[ ] // Asset and Asset Element that we just created.
	[ ] Print("Modify AssetName1DT with: " + sAssetName1DT)
	[ ] fReadWriteIni(sWorkingFile,csWrite,secUid,ent200080001,sAssetName1DT)
	[ ] Print("Modify AeName1DT with: " + sAeName1DT)
	[ ] fReadWriteIni(sWorkingFile,csWrite,secUid,ent300080001,sAeName1DT)
	[ ] 
	[ ] // Retrieve the ActivateTime and DeactivateTime settings for the test asset.
	[ ] sAActiveTime = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secAssetData,entAssetActivate,"")
	[ ] sADeactiveTime = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secAssetData,entAssetDeactivate,"")
	[ ] // Set the ActivateTime and DeactivateTime settings for the test asset in the working
	[ ] // itv file..
	[ ] fReadWriteIni(sWorkingFile,csWrite,secMD_20008001,entActivateTime,sAActiveTime)
	[ ] Print("Modify ActivateTime with: " + sAActiveTime)
	[ ] fReadWriteIni(sWorkingFile,csWrite,secMD_20008001,entDeactivateTime,sADeactiveTime)
	[ ] Print("Modify DeactivateTime with: " + sADeactiveTime)
	[ ] 
	[ ] // Retrieve the ActivateDate, DeactiveDate, DeleteDate, FileName, FilePath,
	[ ] // ReleaseDate and UploadDate settings for the test asset element.
	[ ] sAeActiveDate = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secAssetData,entAeActivate,"")
	[ ] sAeDeactiveDate = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secAssetData,entAeDeactivate,"")
	[ ] sAeDeleteDate = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secAssetData,entAeDelete,"")
	[ ] sAeContentName1 = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secContent,entContentFileName1,"")
	[ ] sAeContentPath = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secContent,entContentPath,"")
	[ ] sAeReleaseDate = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secAssetData,entAeRelease,"")
	[ ] sAeUploadDate = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secAssetData,entAeUpload,"")
	[ ] 
	[ ] // Set the ActivateDate, DeactiveDate, DeleteDate, FileName, FilePath ReleaseDate and
	[ ] // UploadDate settings for the test asset element in the working itv file.
	[ ] fReadWriteIni(sWorkingFile,csWrite,secMD_30008001,entActiveDate,sAeActiveDate)
	[ ] fReadWriteIni(sWorkingFile,csWrite,secMD_30008001,entDeactiveDate,sAeDeactiveDate)
	[ ] fReadWriteIni(sWorkingFile,csWrite,secMD_30008001,entDeleteDate,sAeDeleteDate)
	[ ] fReadWriteIni(sWorkingFile,csWrite,secMD_30008001,entFileName,"0,4," + sAeContentName1)
	[ ] fReadWriteIni(sWorkingFile,csWrite,secMD_30008001,entFilePath,"0,4," + sAeContentPath)
	[ ] fReadWriteIni(sWorkingFile,csWrite,secMD_30008001,entReleaseDate,sAeReleaseDate)
	[ ] fReadWriteIni(sWorkingFile,csWrite,secMD_30008001,entUploadDate,sAeUploadDate)
	[ ] Print("Modify ActiveDate with: " + sAeActiveDate)
	[ ] Print("Modify DeactiveDate with: " + sAeDeactiveDate)
	[ ] Print("Modify DeleteDate with: " + sAeDeleteDate)
	[ ] Print("Modify FileName with: " + sAeContentName1)
	[ ] Print("Modify FilePath with: " + sAeContentPath)
	[ ] Print("Modify ReleaseDate with: " + sAeReleaseDate)
	[ ] Print("Modify UploadDate with: " + sAeUploadDate)
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 8 - Verify the content file exists in " + sAeContentPath)
	[ ] // Verify the content file exists.
	[-] if ! SYS_FileExists(sAeContentPath + "\" + sAeContentName1)
		[ ] // ERROR*** We need an error handler here.
		[ ] LogError("*** Error: Content file does not exist")
		[ ] TestCaseExit(True)
	[ ] Print()
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 9 - Verify the upload path exists")
	[ ] // Read in and verify the specified upload path exists.
	[ ] sImportPath = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secImport,entImportPath, "") 
	[ ] Print("Expected upload path = " + sImportPath)
	[ ] bImportDirExists = SYS_DirExists(sImportPath)
	[ ] Print("Upload path exists")
	[-] if ! bImportDirExists
		[ ] // ERROR*** We need an error handler here.
		[ ] LogError("*** Error: The upload path does not exist")
		[ ] TestCaseExit(True)
	[ ] ResCloseList()
	[ ] 
	[ ] // *****************************************************************************
	[ ] // Test Execution Begins Here
	[ ] // *****************************************************************************
	[ ] // 
	[ ] ResOpenList("Step 10 - Copy the working data file to the upload path")
	[ ] Print()
	[ ] // Copy the working data file to the upload path.
	[ ] SYS_MoveFile(sWorkingFile, sImportPath + "\" + sWorkingFileNameDT)
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 11 - Verify the working data file exists in the upload path")
	[ ] Print()
	[ ] //Check for the working data file in the upload path.
	[ ] SYS_FileExists(sImportPath + "\" + sWorkingFileNameDT)
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 12 - Wait a few seconds then verify the working data file is removed from the Upload path")
	[ ] Print()
	[ ] // Verify the working data file is removed from the Upload path.
	[+] while SYS_FileExists (sImportPath + "\" + sWorkingFile)
		[ ] SLEEP (1)
	[ ] ResCloseList()
	[ ] 
	[ ] // Insert a short delay before checking the Imported folder for the .itv and
	[ ] // .rpt files.
	[ ] SLEEP (5)
	[ ] 
	[ ] ResOpenList("Step 13 - Verify the working data file and its related .rpt files exist in the Imported directory")
	[ ] // Verify the working data file and report file exist in the Imported directory.
	[ ] sImportedDir = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secImport,entImportedDir, "")
	[ ] lfImportedDirConts = SYS_GetDirContents(sImportedDir)
	[ ] 
	[ ] // Retrieve the names of the .itv and .rpt files.
	[ ] sItvFileName = sImportedDir + "/" + lfImportedDirConts[1].sName
	[ ] sRptFileName	= sImportedDir + "/" + lfImportedDirConts[2].sName
	[ ] Print("ItvFileName = " + sItvFileName)
	[ ] Print("RptFileName = " + sRptFileName)
	[ ] // Make sure the report file has been created and is not
	[ ] // zero length.
	[-] while iRptFileSize == 0
		[ ] SLEEP (1)
		[ ] lfImportedDirConts = SYS_GetDirContents(sImportedDir)
		[ ] iRptFileSize = lfImportedDirConts[2].iSize
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 14 - Verify the import was successful by checking for the ';Metadata:' string in the report file")
	[ ] // Verify the import was successful by checking for the ";Metadata:"
	[ ] // string in the report file.
	[ ] hRptFile = FileOpen (sRptFileName, FM_READ)
	[-] while (FileReadLine (hRptFile, sRptFileLine))
		[ ] bRetVal = MatchStr(csMetadata, sRptFileLine)
		[-] if bRetVal
			[ ] bFoundString = TRUE
			[ ] FileClose (hRptFile)
			[ ] break
	[ ] 
	[-] if ! bFoundString
		[ ] // We didn't find the Metadata string. Fail.
		[ ] LogError("*** Error: Fail to find the Metadata string, import failed!")
		[ ] TestCaseExit(True)
	[ ] Print("Import succeed!")
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 15 - Search the .rtp file for Asset ID&AeID, and write them to scorpiocfg.ini file")
	[ ] // Find the Asset ID and write it to scorpiocfg.ini file.
	[ ] bFoundString = FALSE
	[ ] hRptFile = FileOpen (sRptFileName, FM_READ)
	[-] while (FileReadLine (hRptFile, sRptFileLine))
		[ ] bRetVal = MatchStr("; Asset: " + "*", sRptFileLine)
		[-] if bRetVal
			[ ] bFoundString = TRUE
			[ ] iPos = StrPos("(", sRptFileLine, TRUE)
			[ ] sAssetID = Trim(SubStr(sRptFileLine, iPos + 1))
			[ ] iPos = StrPos(")", sAssetID, TRUE)
			[ ] sAssetID = Stuff(sAssetID, iPos, 1,"")
			[ ] Print("Asset ID in .rpt file = " + sAssetID)
			[-] if Trim(sAssetID) == ""
				[ ] LogError("*** Error: Asset ID shouldn't be null")
				[ ] TestCaseExit(True)
			[ ] Print("Modify Asset ID in scorpiocfg.ini")
			[ ] fReadWriteIni(csInstallPath+filScorpioIni,csWrite,secAssetData,entAssetID,sAssetID)
			[ ] FileClose (hRptFile)
			[ ] break
	[ ] 
	[-] if ! bFoundString
		[ ] // We didn't find the Asset string. Fail.
		[ ] LogWarning("*** Warning: Fail to find the Asset string")
	[ ] 
	[ ] // Find the Asset Element ID and write it to the scorpiocfg.ini file.
	[ ] bFoundString = FALSE
	[ ] hRptFile = FileOpen (sRptFileName, FM_READ)
	[-] while (FileReadLine (hRptFile, sRptFileLine))
		[ ] bRetVal = MatchStr("; Ae: " + "*", sRptFileLine)
		[-] if bRetVal
			[ ] bFoundString = TRUE
			[ ] iPos = StrPos("(", sRptFileLine, TRUE)
			[ ] sAeID = Trim(SubStr(sRptFileLine, iPos + 1))
			[ ] iPos = StrPos(")", sAeID, TRUE)
			[ ] sAeID = Stuff(sAeID, iPos, 1,"")
			[-] if Trim(sAeID) == ""
				[ ] LogError("*** Error: AeID shouldn't be null")
				[ ] TestCaseExit(True)
			[ ] Print("AeID in .rpt file = " + sAeID)
			[ ] Print("Modify AeID in scorpiocfg.ini")
			[ ] fReadWriteIni(csInstallPath+filScorpioIni,csWrite,secAssetData,entAeID,sAeID)
			[ ] FileClose (hRptFile)
			[ ] break
	[ ] 
	[-] if ! bFoundString
		[ ] // We didn't find the Ae string. Fail.
		[ ] LogWarning("*** Warning: Fail to find the Ae string")
		[ ] // TestCaseExit(True)
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 16 - Verify the folder path of the asset and asset element stored actually is the expected")
	[ ] // Read in the folder path we expect the asset and asset element to be
	[ ] // created under, then and verify that's what appears in the .rpt file.
	[ ] sAssetFolder = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secAssetData,entFolder, "")
	[ ] Print("Expected folder = " + sAssetFolder)
	[ ] bFoundString = FALSE
	[ ] hRptFile = FileOpen (sRptFileName, FM_READ)
	[-] while (FileReadLine (hRptFile, sRptFileLine))
		[ ] bRetVal = MatchStr("*" + sAssetFolder + "*", sRptFileLine)
		[-] if bRetVal
			[ ] bFoundString = TRUE
			[ ] iPos = StrPos("\", sRptFileLine)
			[ ] sRptAssetFolder = Trim(SubStr(sRptFileLine,iPos))
			[ ] FileClose (hRptFile)
			[ ] break
	[ ] 
	[-] if bFoundString
		[ ] Print("Received Folder = " + sRptAssetFolder)
		[-] if sAssetFolder != sRptAssetFolder
			[ ] // We have an error condition
			[ ] LogWarning("*** Warning: The received is not the expected")
	[ ] 
	[-] if ! bFoundString
		[ ] // We didn't find the Folder string. Fail.
		[ ] LogWarning("**** Warning: Fail to find the folder string")
		[ ] // TestCaseExit(True)
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 17 - Verify the new asset using AMApp")
	[ ] //*****************************************************************************
	[ ] // Find the new asset using AMApp
	[ ] //*****************************************************************************
	[ ] 
	[ ] // Choose Edit=>Search from the main AMApp menu.
	[ ] gsRemoteITVRoot = fReadWriteIni(csInstallPath+filZodiacIni,csRead,secITVRoot,entRemoteITVRoot, "")
	[ ] SeaChangeITVAssetManager.Edit.Search.Pick()
	[ ] 
	[ ] // Select to search 'Asset by uid'
	[ ] Search.SearchFor.Select("Asset by uid")
	[ ] 
	[ ] // Type in the Asset UID we created and want to find.
	[ ] Search.Uid.SetText(sAssetID)
	[ ] // Start the search.
	[ ] Search.Go.Click()
	[ ] 
	[ ] // If we passed in the wrong asset id or the asset id was not found
	[ ] // we'll get a message dialog that we need to dismiss. This is
	[ ] // a showstopper.
	[+] if AM_FailedToFind.Exists()
		[ ] AM_FailedToFind.Close()
		[ ] Search.Close.Click()
		[ ] LogError("*** Error: Fail to find the Asset: " + sAssetID)
		[ ] TestCaseExit(True)
		[ ] // Log the ERROR
		[ ] // Probably AppError at this point unless we want to do more detailed
		[ ] // investigation like cheking the database, etc.
	[ ] Print("Asset " + sAssetID + " exists")
	[ ] // Strip off the preceding asset type info (# ,) from the Asset Name
	[ ] // so we can compare it against the asset name that gets returned by
	[ ] // the search operation.
	[ ] sTruncatedAssetName = Stuff(sAssetName1DT,1,2,"")
	[ ] Print("AssetName expected = " + sTruncatedAssetName)
	[ ] // Fetch the contents of the name and Uid boxes from the 
	[ ] // Search Asset Result dialog.
	[ ] sReturnedAssetName = SearchAssetResult.Name.GetText()
	[ ] sReturnedAssetID = SearchAssetResult.Uid.GetText()
	[ ] Print("AssetName received = " + sReturnedAssetName)
	[ ] // Verify that what was returned matches what we know these
	[ ] // values to be.
	[-] if sReturnedAssetName == sTruncatedAssetName
			[ ] Print("AssetID expected = " + sAssetID)
			[ ] Print("AssetID received = " + sReturnedAssetID)
		[-] if sReturnedAssetID != sAssetID
			[ ] // Failed to get correct asset ID - pretty unlikely to occur
			[ ] // at this point if we found the asset and the Name is correct, but
			[ ] // cover it anyway.
			[ ] LogWarning("*** Warning: Failed to get correct AssetID")
	[-] else
			[ ] // Failed to get correct asset - pretty unlikely to occur
			[ ] // at this point, if we found the AssetID, but cover it anyway.
			[ ] LogWarning("*** Warning: Failed to get correct asset")
	[ ] 
	[ ] Print("Verify the asset lives in the expected folder: " + sAssetFolder)
	[ ] // See if the asset lives in the folder we think it should by
	[ ] // checking for it in the In Folders list.
	[ ] iListIndex = SearchAssetResult.InFolders.FindItem(sAssetFolder)
	[-] if iListIndex == 0
			[ ] // We failed to find the folder we thought it should be in.
			[ ] // Should probably grab what's in the list and do something
			[ ] // with that.
			[ ] LogWarning("*** Warning: The asset doesn't exist")
	[ ] 
	[ ] // Close the SearchAssetResult window.
	[-] if SearchAssetResult.Exists()
		[ ] SearchAssetResult.OK.Click()
	[ ] 
	[ ] // Close the Search window.
	[-] if Search.Exists()
		[ ] Search.Close.Click()
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 18 - Find the new asset through the IDS database")
	[ ] //*****************************************************************************
	[ ] // Find the new asset through the IDS database
	[ ] //*****************************************************************************
	[ ] 
	[ ] // Convert the Asset ID from hex to decimal. Strip off the leading zeros.
	[ ] // sAssetIDBase10 = Stuff(fHexToDecimal(sAssetID),1,4,"") by rommy followed
	[ ] sAssetIDBase10 = fHexToDecimal(sAssetID)
	[ ] // Check for the asset id in the IDS database.
	[ ] gsDSN = fReadWriteIni(csInstallPath+filZodiacIni,csRead,secODBCDsn,entDSN, "")     
	[ ] gsUid = fReadWriteIni(csInstallPath+filZodiacIni,csRead,secCommandCenter,entSQLUid, "")
	[ ] gsPassword = fReadWriteIni(csInstallPath+filZodiacIni,csRead,secCommandCenter,entSQLPassword, "")
	[-] if Lower(gsPassword) == "none"
		[ ] gsPassword = ""
	[ ] // sConnectString = ("DSN=ITVNODE101;SRVR=ITVNODE101;UID=sa;PWD=") by rommy followed
	[ ] // sConnectString = ("DSN="+sDSNName+";SRVR="+sServerIP+";UID=sa;PWD=")
	[ ] sConnectString = ("DSN="+gsDSN+";SRVR="+gsAM1IP+";UID="+gsUid+";PWD="+gsPassword)
	[ ] sQueryString = "select ASSET_UID from Asset where ASSET_UID = {sAssetIDBase10}"
	[ ] iDBAssetID = fDBQueryForInt(sConnectString,sQueryString,csIdsData)
	[ ] // Compare the asset id from the database to our expected string.
	[-] // if Str(iDBAssetID) != sAssetIDBase10 by rommy followed
		[ ] // // ERROR on the comparison.
	[-] if iDBAssetID == -1
			[ ] // // ERROR on the comparison.
			[ ] LogWarning("*** Warning: " + sAssetIDBase10 + " doesn't exist in the database")
	[ ] Print(sAssetIDBase10 + " exists in the database")
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 19 - Check the iam.log for success indicators")
	[ ] //*****************************************************************************
	[ ] // Check the iam.log for two success indicators in the form of strings.
	[ ] //*****************************************************************************
	[ ] 
	[ ] // Check for two success indicators in the iam.log.
	[ ] iTick =  1
	[ ] Print("Serching iam.log for 'op Add Wqe'")
	[ ] // bFoundString = fFindInFile("\\itvnode101\c$\itv\log\iam.log", "op Add Wqe") by rommy followed
	[ ] bFoundString = fFindInFile("\\"+gsAM1IP+"\"+gsRemoteITVRoot+"log\iam.log", "op Add Wqe")
	[-] while !bFoundString && (iTick < 300)
			[ ] SLEEP (1)
			[ ] // bFoundString = fFindInFile("\\itvnode101\c$\itv\log\iam.log", "op Add Wqe") by rommy followed
			[ ] bFoundString = fFindInFile("\\"+gsAM1IP+"\"+gsRemoteITVRoot+"log\iam.log", "op Add Wqe")
			[ ] iTick++
	[-] if !bFoundString
		[ ] LogWarning("*** Warning: Fail to find 'op Add Wqe'")
	[ ] 
	[ ] iTick = 1
	[ ] Print("Serching iam.log for 'PS completed upload Ae'")
	[ ] // bFoundString = fFindInFile("\\itvnode101\c$\itv\log\iam.log", "PS completed upload Ae") by rommy followed
	[ ] bFoundString = fFindInFile("\\"+gsAM1IP+"\"+gsRemoteITVRoot+"log\iam.log", "PS completed upload Ae")
	[-] while !bFoundString && (iTick < 300)
		[ ] SLEEP (1)
		[ ] // bFoundString = fFindInFile("\\itvnode101\c$\itv\log\iam.log", "PS completed upload Ae") by rommy followed
		[ ] bFoundString = fFindInFile("\\"+gsAM1IP+"\"+gsRemoteITVRoot+"log\iam.log", "PS completed upload Ae")
		[ ] iTick++
	[-] if !bFoundString
		[ ] LogWarning("*** Warning: Fail to find 'PS completed upload Ae'")
	[ ] //*****************************************************************************
	[ ] // Run scp dir to verify the asset element UID exists on the upload cluster.
	[ ] //*****************************************************************************
	[ ] 
	[ ] // // Retrieve the name of the Uplocad cluster server.
	[ ] // sUploadServer = fReadWriteIni(filZodiacIni,csRead,secVideoServers,entUploadClusterName, "")
	[ ] // 
	[ ] // // Set up an IPC connection to the Upload Cluster server.
	[ ] // iRetVal = SYS_Execute("net use \\" + sUploadServer + "\ipc$ /u:administrator deadsea") 
	[ ] // 
	[ ] // // Issue an scp dir command and store the output in a list of string.
	[ ] // iRetVal = SYS_Execute("c:\scp dir", lsScpDirOut)
	[ ] // 
	[ ] // SLEEP (1)
	[ ] 
	[ ] 
	[ ] //*****************************************************************************
	[ ] // Grep the iam.log "Deleted upload wqe data file".
	[ ] //*****************************************************************************
	[ ] 
	[ ] // Check for two success indicators in the iam.log.
	[ ] iTick =  1
	[ ] Print("Serching iam.log for 'Deleted upload wqe data file'")
	[ ] // bFoundString = fFindInFile("\\itvnode101\c$\itv\log\iam.log", "Deleted upload wqe data file") by rommy followed
	[ ] bFoundString = fFindInFile("\\"+gsAM1IP+"\"+gsRemoteITVRoot+"log\iam.log", "Deleted upload wqe data file")
	[-] while !bFoundString && (iTick < 300)
			[ ] SLEEP (1)
			[ ] bFoundString = fFindInFile("\\"+gsAM1IP+"\"+gsRemoteITVRoot+"log\iam.log", "Deleted upload wqe data file")
			[ ] iTick++
	[-] if !bFoundString
		[ ] LogWarning("*** Warning: Fail to find 'Deleted upload wqe data file'")
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 20 - Verify the new asset element using AMApp")
	[ ] //*****************************************************************************
	[ ] // Verify the new asset element using AMApp
	[ ] //*****************************************************************************
	[ ] 
	[ ] // NOTE:
	[ ] // I need to do this disconnect/reconnect here else the Modify button in
	[ ] // the Search Asset Element dialog won't be enabled. Probably a bug in AMApp
	[ ] // 1.4.6 that I will have to report.
	[ ] 
	[ ] SeaChangeITVAssetManager.File.Disconnect.Pick()
	[ ] AMSiteDisconnectPrompt.Yes.Click()
	[ ] SeaChangeITVAssetManager.File.Connect.Pick()
	[ ] // Connect.SiteServerName.SetText(sAMServer) by rommy followed
	[ ] Connect.SiteServerName.SetText("MDS1_SS_CL")
	[ ] Connect.OK.Click()
	[ ] 
	[ ] // Choose Edit=>Search from the main AMApp menu.
	[ ] SeaChangeITVAssetManager.Edit.Search.Pick()
	[ ] 
	[ ] // Select to search 'Asset element by uid'
	[ ] Search.SearchFor.Select("Asset element by uid")
	[ ] 
	[ ] // Type in the Asset element UID we created and want to find.
	[ ] Search.Uid.SetText(sAeID)
	[ ] // Start the search.
	[ ] Search.Go.Click()
	[ ] 
	[ ] // If we passed in the wrong asset id or the asset id was not found
	[ ] // we'll get a message dialog that we need to dismiss. This is
	[ ] // a showstopper.
	[-] if AM_FailedToFind.Exists()
		[ ] AM_FailedToFind.Close()
		[ ] Search.Close.Click()
		[ ] LogError("*** Error: Fail to find the Asset element: " + sAeID)
		[ ] TestCaseExit(True)
		[ ] 
		[ ] // Log the ERROR
		[ ] // Probably AppError at this point unless we want to do more detailed
		[ ] // investigation like cheking the database, etc.
	[ ] 
	[ ] // Strip off the preceding asset type info (# ,) from the Asset Name
	[ ] // so we can compare it against the asset name that gets returned by
	[ ] // the search operation.
	[ ] sTruncatedAeName = Stuff(sAeName1DT,1,2,"")
	[ ] Print("Expected AeName = " + sTruncatedAeName)
	[ ] // Fetch the contents of the name and Uid boxes from the 
	[ ] // Search Asset Result dialog.
	[ ] sReturnedAeName = SearchAssetElementResult.Name.GetText()
	[ ] sReturnedAeID = SearchAssetElementResult.Uid.GetText()
	[ ] Print("Received AeName = " + sReturnedAeName)
	[ ] // Verify that what was returned matches what we know these
	[ ] // values to be.
	[-] if sReturnedAeName == sTruncatedAeName
		[ ] Print("Expected AeID = " + sAeID)
		[ ] Print("Received AeID = " + sReturnedAeID)
		[+] if sReturnedAeID != sAeID
			[ ] // Failed to get correct asset ID - pretty unlikely to occur
			[ ] // at this point if we found the asset and the Name is correct, but
			[ ] // cover it anyway.
			[ ] LogWarning("*** Warning: Failed to get correct AeID")
	[-] else
			[ ] // Failed to get correct asset - pretty unlikely to occur
			[ ] // at this point, if we found the AssetID, but cover it anyway.
			[ ] LogWarning("*** Warning: Failed to get correct AeName")
	[ ] 
	[ ] Print("Verify the BitRate value in the list metadata is nonzero")
	[ ] // Open Modify Ae dialog.
	[-] if SearchAssetElementResult.Exists()
		[ ] SearchAssetElementResult.Modify.Click()
	[ ] 
	[ ] // Choose the Advanced tab.
	[ ] ModifyAE.PageList.Select("Advanced")
	[ ] 
	[ ] // Read in the list of Ae metadata
	[ ] lsAeMetaData = ModifyAE.Advanced.Metadata.GetContents()
	[ ] 
	[ ] // Count the number of metadata returned.
	[ ] iCount = ListCount(lsAeMetaData)
	[ ] 
	[ ] // First, parse for "BitRate" then check for a non-zero
	[ ] // Bitrate value.
	[-] for (i = 1; i <= iCount; i++)
		[ ] sMDItemText = lsAeMetaData[i]
		[ ] // Return the first 7 characters of each metadata string.
		[ ] sMDItemText2 = SubStr(sMDItemText,1,7)
		[ ] // Does the substring equal "BitRate"?
		[-] if sMDItemText2 == "BitRate"
			[ ] // Is the BitRate value 0?
			[ ] // Strip off the leading "BitRate" and ";" to
			[ ] // get at the value.
			[ ] sMDItemText2 = Stuff(sMDItemText,1, 8,"")
			[ ] // Convert the value from a string to an integer.
			[ ] iBitRate = Val(sMDItemText2)
			[-] if iBitRate == 0
				[ ] LogWarning("*** Warning: Ae Bitrate was 0")
				[ ] SLEEP (1)
			[-] else
				[ ] print ("Ae BitRate metadata was A-OK (non-zero)")
				[ ] break
				[ ] 
	[ ] Print("Verify the PlayTime value in the list metadata is non-zero")
	[ ] // Second, parse for "PlayTime" then check for a non-zero
	[ ] // PlayTime value.
	[-] for (i = 1; i <= iCount; i++)
		[ ] sMDItemText = lsAeMetaData[i]
		[ ] // Return the first 8 characters of each metadata string.
		[ ] sMDItemText2 = SubStr(sMDItemText,1,8)
		[ ] // Does the substring equal "BitRate"?
		[-] if sMDItemText2 == "PlayTime"
			[ ] // Is the PlayTime value 0?
			[ ] // Strip off the leading "PlayTime" and ";" to
			[ ] // get at the value.
			[ ] sMDItemText2 = Stuff(sMDItemText,1, 9,"")
			[ ] // Convert the value from a string to an integer.
			[ ] iPlayTime = Val(sMDItemText2)
			[-] if iPlayTime == 0
				[ ] //ERROR - PlayTime should be greater than 0.
				[ ] // Include the expected and returned values in the
				[ ] // error message.
				[ ] LogWarning("*** Warning: Ae PlayTime was 0")
				[ ] SLEEP (1)
			[-] else
				[ ] print ("Ae PlayTime metadata was A-OK (non-zero)")
				[ ] break
	[ ] 
	[ ] Print("Verify the PlayTimeFraction value in the list metadata is non-zero")
	[ ] // Finally, parse for "PlayTimeFraction" then check for a non-zero
	[ ] // PlayTimeFraction value.
	[-] for (i = 1; i <= iCount; i++)
		[ ] sMDItemText = lsAeMetaData[i]
		[ ] // Return the first 16 characters of each metadata string.
		[ ] sMDItemText2 = SubStr(sMDItemText,1,16)
		[ ] // Does the substring equal "BitRate"?
		[-] if sMDItemText2 == "PlayTimeFraction"
			[ ] // Is the PlayTimeFraction value 0?
			[ ] // Strip off the leading "PlayTimeFraction" and ";" to
			[ ] // get at the value.
			[ ] sMDItemText2 = Stuff(sMDItemText,1, 17,"")
			[ ] // Convert the value from a string to an integer.
			[ ] iPlayTimeFraction = Val(sMDItemText2)
			[-] if iPlayTimeFraction == 0
				[ ] //ERROR - PlayTimeFraction should be greater than 0.
				[ ] // Include the expected and returned values in the
				[ ] // error message.
				[ ] LogWarning("*** Warning: Ae PlayTimeFraction was 0")
				[ ] SLEEP (1)
			[-] else
				[ ] print ("Ae PlayTimeFraction metadata was A-OK (non-zero)")
				[ ] break
	[ ] 
	[ ] // Close the ModifyAe and Search windows. 
	[ ] ModifyAE.Cancel.Click()
	[ ] 
	[ ] SearchAssetElementResult.OK.Click()
	[ ] 
	[ ] Search.Close.Click()
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 21 - Find the new asset element through the IDS database")
	[ ] //*****************************************************************************
	[ ] // Find the new asset element through the IDS database
	[ ] //*****************************************************************************
	[ ] 
	[ ] string sAeIDBase10 = csEmptyString
	[ ] integer iDBAeID = 0
	[ ] 
	[ ] // Convert the Asset Element ID from hex to decimal. Strip off the leading zeros.
	[ ] // sAeIDBase10 = Stuff(fHexToDecimal(sAeID),1,4,"") by rommy followed
	[ ] sAeIDBase10 = fHexToDecimal(sAeID)
	[ ] Print("Expected decimal AeID = " + sAeIDBase10)
	[ ] // Check for the asset id in the IDS database.
	[ ] 
	[ ] // sConnectString = ("DSN=ITVNODE101;SRVR=ITVNODE101;UID=sa;PWD=") by rommy followed
	[ ] sConnectString = ("DSN="+gsDSN+";SRVR="+gsAM1IP+";UID="+gsUid+";PWD="+gsPassword)
	[ ] sQueryString = "select UID from AeState where UID = {sAeIDBase10}"
	[ ] iDBAeID = fDBQueryForInt(sConnectString,sQueryString,csIdsData)
	[ ] // Compare the asset id from the database to our expected string.
	[-] // if Str(iDBAeID) != sAeIDBase10 by rommy followed
		[ ] // ERROR on the comparison.
	[-] if iDBAeID == -1
		[-] // ERROR on the comparison.
			[ ] LogWarning("*** Warning: The AeID doesn't exist in the database")
	[-] else
		[ ] Print ("The AeID exists in the database!")
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 22 - Verify the Asset Replica count using ManUtil")
	[ ] //*****************************************************************************
	[ ] // Verify the Asset Replica count using ManUtil
	[ ] //*****************************************************************************
	[ ] 
	[ ] // Minimize the AMApp window
	[ ] SeaChangeITVAssetManager.Minimize()
	[ ] 
	[ ] // Start up Manutil_d.exe
	[ ] // ManUtil.Start("d:\itv\exe\manutil_d.exe") by rommy followed
	[ ] ManUtil.Start(gsLocalITVRoot+"exe\manutil.exe")
	[ ] 
	[-] if ManUtil.Exists()
		[ ] ManUtil.Maximize()
		[ ] ManUtil.File.ManageRemoteApplications.Pick()
	[ ] 
	[ ] // Read in the name of the CM server from scorpiocfg.ini.
	[ ] gsCM1IP = fReadWriteIni(csInstallPath+filZodiacIni,csRead,secCommandCenter,entCM1IP, "")
	[ ] 
	[-] if SelectComputer.Exists()
		[ ] // SelectComputer.Computer.SetText(sCMServer) by rommy followed
		[ ] SelectComputer.Computer.SetText(gsCM1IP)
		[ ] SelectComputer.OK.Click()
	[ ] 
	[ ] // Expand the ICM and IPS API objects, then select our test asset id.
	[ ] ManUtil.SetActive()
	[ ] ManUtilInspectServices.SetActive ()
	[ ] // ManUtilInspectServices.Frame.LeftPane.DoubleSelect ("/ITVNODE101/ICM") by rommy followed
	[ ] ManUtilInspectServices.Frame.LeftPane.DoubleSelect ("/"+gsCM1IP+"/ICM")
	[ ] // ManUtilInspectServices.Frame.LeftPane.DoubleSelect ("/ITVNODE101/ICM/IPS API") by rommy followed
	[ ] ManUtilInspectServices.Frame.LeftPane.DoubleSelect ("/"+gsCM1IP+"/ICM/IPS API")
	[ ] ManUtilInspectServices.Frame.Frame.BottomRightPane.Select (sAssetID)
	[ ] 
	[ ] // Retrieve the replica count reported by manutil
	[ ] iManUtilIndex = ManUtilInspectServices.Frame.Frame.BottomRightPane.GetSelIndex ()
	[ ] sReplicaCount = ManUtilInspectServices.Frame.Frame.BottomRightPane.GetItemText (iManUtilIndex, 3)
	[ ] Print("Received ReplicaCount = " + sReplicaCount)
	[ ] 
	[-] if sReplicaCount == "0"
		[ ] // Something is wrong.
		[ ] LogWarning("*** Warning: ReplicaCount shouldn't be 0")
	[ ] // Read in the expected replica count from the scorpiocfg.ini file.
	[ ] sExpRepCount = fReadWriteIni(csInstallPath+filScorpioIni,csRead,secVideoServers,entExpRepCount,"")
	[ ] Print("Expected ReplicaCount = " + sExpRepCount)
	[ ] // Determine how many replicas we expected versus what came back.
	[-] if sReplicaCount != sExpRepCount
		[ ] // We have an error.
		[ ] LogWarning("*** Warning: ReplicaCount was not correct")
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 23 - Delete the .itv and .rpt files from the Imported directory")
	[ ] Print()
	[ ] // Delete the .itv and .rpt files from the Imported directory in preparation
	[ ] // for next run of the test case.
	[ ] SYS_RemoveFile(sItvFileName)
	[ ] SYS_RemoveFile(sRptFileName)
	[ ] ResCloseList()
	[ ] 
	[ ] // The test case PASSED
	[ ] fLogPassFail("Passed","", "")
	[ ] 
	[ ] //*****************************************************************************
	[ ] // END: tcCreateAsset0001()
	[ ] //*****************************************************************************
[ ] 
[ ] //*****************************************************************************
[ ] // END: SCORPIO.T
[ ] //*****************************************************************************
