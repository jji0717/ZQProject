[ ] //*****************************************************************************
[ ] //  NAME:					  TAURUS.T
[ ] //
[ ] //  DESCRIPTION:		Main test script for verifying billing system activity
[ ] //
[ ] //  HISTORY:
[ ] //
[ ] //  Date				    Developer					Description
[ ] //  ********				***********				*****************************************
[ ] //  1/2/03          J. Money          Initial design and development
[ ] //  1/7/03          J. Money					Added Oooodles of code.
[ ] //  1/8/03          J. Money					Added Oooodles more code
[ ] //	ZQ modification
[ ] //	08/18/04				Bernie.Zhao				Extracted common variables for all zodiac tests
[ ] //
[ ] // $Archive: /ZQProjs/VODTestAutomation/script/taurus/taurus.t $
[ ] // $Author: Admin $
[ ] // $Date: 10-11-12 16:08 $
[ ] // $History: taurus.t $
[ ] // 
[ ] // *****************  Version 1  *****************
[ ] // User: Admin        Date: 10-11-12   Time: 16:08
[ ] // Created in $/ZQProjs/VODTestAutomation/script/taurus
[ ] // Created.
[ ] // 
[ ] // *****************  Version 1  *****************
[ ] // User: Admin        Date: 10-11-12   Time: 15:42
[ ] // Created in $/Pre-Production/ZQProjs/VODTestAutomation/script/taurus
[ ] // Created.
[ ] // 
[ ] // *****************  Version 10  *****************
[ ] // User: Bernie.zhao  Date: 04-09-08   Time: 14:47
[ ] // Updated in $/ZQProjs/VODTestAutomation/script/taurus
[ ] // 
[ ] // *****************  Version 9  *****************
[ ] // User: Bernie.zhao  Date: 04-09-08   Time: 11:39
[ ] // Updated in $/ZQProjs/VODTestAutomation/script/taurus
[ ] // modified Error and Warning styles
[ ] // 
[ ] // *****************  Version 8  *****************
[ ] // User: Bernie.zhao  Date: 04-09-07   Time: 16:59
[ ] // Updated in $/ZQProjs/VODTestAutomation/script/taurus
[ ] // half error modified
[ ] // 
[ ] // *****************  Version 7  *****************
[ ] // User: Bernie.zhao  Date: 04-08-31   Time: 15:34
[ ] // Updated in $/ZQProjs/VODTestAutomation/script/taurus
[ ] // 
[ ] // *****************  Version 6  *****************
[ ] // User: Bernie.zhao  Date: 04-08-30   Time: 19:23
[ ] // Updated in $/ZQProjs/VODTestAutomation/script/taurus
[ ] // 
[ ] // *****************  Version 5  *****************
[ ] // User: Bernie.zhao  Date: 04-08-30   Time: 14:24
[ ] // Updated in $/ZQProjs/VODTestAutomation/script/taurus
[ ] // 
[ ] // *****************  Version 4  *****************
[ ] // User: Bernie.zhao  Date: 04-08-30   Time: 14:04
[ ] // Updated in $/ZQProjs/VODTestAutomation/script/taurus
[ ] // 
[ ] // *****************  Version 3  *****************
[ ] // User: Bernie.zhao  Date: 04-08-26   Time: 12:24
[ ] // Updated in $/ZQProjs/VODTestAutomation/script/taurus
[ ] // $Revision: 1 $
[ ] // 
[ ] //*****************************************************************************
[ ] //
[ ] //use "SourceDisk.inc"
[ ] 
[ ] use "mswconst.inc"
[ ] //use "..\..\inc\global\zodiacfunc.inc"
[ ] use "..\..\inc\global\zodiacnt.inc"
[ ] use "..\..\inc\global\zodiacfiles.inc"
[ ] use "..\..\inc\global\zodiacutil.inc"
[ ] use "..\..\inc\global\zodiacdefs.inc"
[ ] use "..\..\inc\global\zodiacmsgs.inc"
[ ] use "..\..\inc\taurus\taurusdefs.inc"
[ ] use "..\..\inc\taurus\taurusfunc.inc"
[ ] use "..\..\inc\taurus\taurusmsgs.inc"
[ ] // for base state
[ ] use "..\..\inc\global\basestate.inc"
[ ] 
[-] main()
	[ ] 
	[ ] tcModBillingReports()
	[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] // Testcase execution begins here
[ ] //*****************************************************************************
[-] testcase tcModBillingReports() appstate BaseState
	[ ] 
	[ ] // Store the current script name.
	[ ] gsScriptName = Trim(StrTran(GetProgramName( ), ".t", " "))
	[ ] 
	[ ] // // Read the project ini file to determine where the data drive is.
	[ ] // const string gsDataDrive = fReadWriteIni(csInstallPath + "zodiac\" +
    // gsScriptName + "\" + gsScriptName + ".ini", csRead,
    // "ITVQA","DataDrive","")
	[ ] 
	[ ] // Build the PassFailLog string
	[ ] gsPassFailLog = csInstallPath + csMainLogPath + gsScriptName + "\" +
    gsScriptName + "log"
	[ ] 
	[ ] // Create the Scorpio Pass/Fail log.
	[ ] fCreatePassFailLog()
	[ ] 
	[ ] //ITVMACHINEIPS recMachineIP
	[ ] //DBLOGONINFO recDBLOGONINFO
	[ ] 
	[ ] STRING sUsePreGenerated = "FALSE"
	[ ] STRING sViewingCount
	[ ] STRING sInProgressCount
	[ ] STRING sReportedRecordCount
	[ ] STRING sNotReportedRecordCount
	[ ] STRING sViewingCount_Expected
	[ ] STRING sInProgressCount_Expected
	[ ] STRING sReportedRecordCount_Expected
	[ ] STRING sNotReportedRecordCount_Expected
	[ ] 
	[ ] STRING sTestExecutionBegin
	[ ] STRING sTestExecutionEnd
	[ ] STRING sCurrStepTime
	[ ] 
	[ ] STRING sBackedUpDatabaseName
	[ ] STRING sTaurusDatabaseName
	[ ] 
	[ ] STRING sPreGeneratedDatabaseLocation
	[ ] STRING sPreGeneratedDatabaseName
	[ ] 
	[ ] STRING sSQLBackupDirectory
	[ ] 
	[ ] INTEGER iNumMbsExtractDatFiles = 0
	[ ] INTEGER iNumMbsExtractErrFiles = 0
	[ ] LIST OF FILEINFO lfFiles
	[ ] 
	[ ] STRING sDstOutputPath 
	[ ] STRING sVDLCount_Expected
	[ ] STRING sTimeToCompare
	[ ] 
	[ ] STRING sVDLFileName
	[ ] 
	[ ] INTEGER iVDLCount
	[ ] 
	[ ] STRING sMbsDstRptCount_Expected
	[ ] STRING sMbsDstRptFile
	[ ] BOOLEAN bMbsDstFileExist = FALSE
	[ ] 
	[ ] INTEGER iMbsDstRptCount
	[ ] 
	[ ] STRING sModBillingArchivePath
	[ ] STRING sModBillingUploadPath
	[ ] STRING sMbsDatFile
	[ ] STRING sMbsDatIdxFile
	[ ] 
	[ ] BOOLEAN bMbsDatIdxFileExist = FALSE
	[ ] BOOLEAN bMbsDatFileExist = FALSE
	[ ] BOOLEAN bCheckforMbsDstDatError = FALSE
	[ ] BOOLEAN bFoundMbsDstDatError = FALSE
	[ ] 
	[ ] STRING sMbsExtractCount_Expected
	[ ] 
	[ ] STRING sExtendedStatus
	[ ] 
	[ ] STRING sPathToRegExe
	[ ] //STRING sPathToRegTools
	[ ] 
	[ ] STRING sTestValue
	[ ] INTEGER iTestValue
	[ ] BOOLEAN bTestValue
	[ ] 
	[ ] STRING sDBLogOnInfo
	[ ] DATETIME dtCurrDatabaseDateTime
	[ ] STRING sDatabaseDateTime
	[ ] 
	[ ] STRING sNETUSEcommand
	[ ] 
	[ ] sCurrStepTime= fCreateDateTimeStamp (2)
	[ ] Print()
	[ ] Print("***************************************************************************************")
	[ ] Print(SubStr(sCurrStepTime,1,4)+"/"+SubStr(sCurrStepTime,5,2)+"/"+SubStr(sCurrStepTime,7,2)+" "+SubStr(sCurrStepTime,9,2)+":"+SubStr(sCurrStepTime,11,2)+":"+SubStr(sCurrStepTime,13,2))
	[ ] Print("Pre-Requisite Steps:                                                                ")
	[ ] Print("***************************************************************************************")
	[ ] Print()
	[ ] 
	[ ] 
	[ ] // Step 1
	[ ] // Write Current Local System time to Tauruscfg.ini in the value "TestExecutionBegin"
	[ ] ResOpenList("Step 1 - Write Current Local System time to 'TestExecutionBegin' in Tauruscfg.ini")
	[ ] 
	[ ] sTestExecutionBegin = fCreateDateTimeStamp(2)
	[ ] sTestValue = fReadWriteIni(csInstallPath +filTaurusIni, csWrite, "TimeInformation", "TestExecutionBegin", sTestExecutionBegin)
	[ ] 
	[-] if (Upper(sTestValue) == "TRUE")
		[ ] Print (sTestExecutionBegin+" was successfully written to Tauruscfg.ini")
	[-] else
		[ ] LogWarning ("*** Warning: Couldn't write to Tauruscfg.ini")
	[ ] ResCloseList()
	[ ] //TestCaseExit(TRUE)	// just for test use
	[ ] 
	[ ] // Step 2
	[ ] // Read in the IP Addresses from the Zodiac.ini file
	[ ] ResOpenList("Step 2 - Reading in IPs from Zodiac.ini file")
	[ ] gsAPP1IP = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter, entAPP1IP, "Booyah") 
	[ ] Print ("Retreived value for APP1 is - "+gsAPP1IP)
	[ ] gsAPP2IP = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter, entAPP2IP, "Booyah") 
	[ ] Print ("Retreived value for APP2 is - "+gsAPP2IP)
	[ ] gsCM1IP = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter, entCM1IP, "Booyah") 
	[ ] Print ("Retreived value for CM1 is - "+gsCM1IP)
	[ ] gsCM2IP = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter, entCM2IP, "Booyah") 
	[ ] Print ("Retreived value for CM2 is - "+gsCM2IP)
	[ ] gsMDS1IP = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter,entMDS1IP , "Booyah") 
	[ ] Print ("Retreived value for MDS1 is - "+gsMDS1IP)
	[ ] gsMDS2IP = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter, entMDS2IP, "Booyah") 
	[ ] Print ("Retreived value for MDS2 is - "+gsMDS2IP)
	[ ] gsPS1IP = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter, entPS1IP, "Booyah") 
	[ ] Print ("Retreived value for PS1 is - "+gsPS1IP)
	[ ] gsPS2IP = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter, entPS2IP, "Booyah") 
	[ ] Print ("Retreived value for PS2 is - "+gsPS2IP)
	[ ] ResCloseList()
	[ ] 
	[ ] // Step 3
	[ ] // Fetch the SQL Database logon info from Tauruscfg.ini
	[ ] ResOpenList("Step 3 - Fetch SQL Database logon info from Tauruscfg.ini and intialize logon data structure")
	[ ] 
	[ ] Print("Fetching SQL Database info from Tauruscfg.ini...")
	[ ] // Retrieve SQL Logon info from Tauruscfg.ini
	[ ] gsDSN = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secODBCDsn, entDSN, "Booyah")
	[ ] gsMDSSQLServer = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter, entMDSSQLServer, "Booyah")
	[ ] gsUid = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter, entSQLUid, "Booyah")
	[ ] gsPassword = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter, entSQLPassword, "Booyah")
	[-] if gsPassword == "none"
		[ ] gsPassword = ""
	[ ] 
	[ ] Print("Retrieved SQL Database logon info:")
	[ ] Print("----------------------------------")
	[ ] Print("DSN - "+gsDSN)
	[ ] Print("SRVR - "+gsMDSSQLServer)
	[ ] Print("UID - "+gsUid)
	[ ] Print("PWD - "+gsPassword)
	[ ] 
	[ ] // Build Logon String
	[ ] sDBLogOnInfo = "DSN="+gsDSN+";SRVR="+gsMDSSQLServer+";UID="+gsUid+";PWD="+gsPassword
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] // Step 3a
	[ ] // Get Current Database Time for timestamp comparisons
	[ ] ResOpenList("Step 3a - Get Current Database Time for TimeStamp Comparisons")
	[ ] 
	[ ] //Call fDBGetDateTime function to get the date and time from the database machine.
	[ ] dtCurrDatabaseDateTime = fDBGetDateTime (sDBLogOnInfo)
	[ ] sDatabaseDateTime = FormatDateTime(dtCurrDatabaseDateTime,"yyyymmddhhmmss")
	[ ] 
	[ ] Print(sDatabaseDateTime)
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] // Step 4
	[ ] // Stop the following services...
	[ ] ResOpenList("Step 4 - Stopping necessary services")
	[ ] Print()
	[ ] 
	[-] if (!fManageService(csMODName, csStop, gsAPP1IP, sExtendedStatus))        // Movies on Demand
		[ ] LogError ("*** Error: Cannot Stop "+csMODName+" on "+gsAPP1IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[-] if (!fManageService(csMOD2Name, csStop, gsAPP2IP, sExtendedStatus))       // Movies on Demand2
		[ ] LogError ("*** Error: Cannot Stop "+csMOD2Name+" on "+gsAPP2IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[-] if (!fManageService(csICMName, csStop, gsCM1IP, sExtendedStatus))         // ICM
		[ ] LogError ("*** Error: Cannot Stop "+csICMName+" on "+gsCM1IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[-] if (!fManageService(csICM2Name, csStop, gsCM2IP, sExtendedStatus))        // ICM2
		[ ] LogError ("*** Error: Cannot Stop "+csICM2Name+" on "+gsCM2IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[-] if (!fManageService(csSCTPName, csStop, gsAPP1IP, sExtendedStatus))       // SCTP
		[ ] LogError ("*** Error: Cannot Stop "+csSCTPName+" on "+gsAPP1IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[-] if (!fManageService(csSCTP2Name, csStop, gsAPP2IP, sExtendedStatus))      // SCTP2
		[ ] LogError ("*** Error: Cannot Stop "+csSCTP2Name+" on "+gsAPP2IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[-] if (!fManageService(csMODBillingName, csStop, gsPS1IP, sExtendedStatus))  // MOD Billing
		[ ] LogError ("*** Error: Cannot Stop "+csMODBillingName+" on "+gsPS1IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[-] if (!fManageService(csIADName, csStop, gsPS1IP, sExtendedStatus))         // IAD
		[ ] LogError ("*** Error: Cannot Stop "+csIADName+" on "+gsPS1IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[-] if (!fManageService(csSysmonName, csStop, gsMDS2IP, sExtendedStatus))     // Sysmon
		[ ] LogError ("*** Error: Cannot Stop "+csSysmonName+" on "+gsMDS2IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 5
	[ ] // On PS1 in c$\itv\log rename Mbs.log to Mbs.log.currentdatetime
	[ ] ResOpenList("Step 5 - Rename Mbs.log to Mbs.log.currentdatetime")
	[ ] 
	[ ] STRING sModBillingLogPath
	[ ] 
	[ ] sModBillingLogPath = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "CommandCenterLogInfo", "ModBillingLogPath", "Booyah")
	[ ] 
	[-] if (!SYS_FileExists("\\"+gsPS1IP+"\"+sModBillingLogPath+"Mbs.log"))
		[ ] LogWarning ("*** Warning: Mbs.log file does not exist.")
	[-] else
		[ ] Print ("Renaming Mbs.Log to Mbs_"+sTestExecutionBegin+".log")
		[-] if(!fCreateBackupFile("\\"+gsPS1IP+"\"+sModBillingLogPath+"mbs.log", "\\"+gsPS1IP+"\"+sModBillingLogPath+"mbs_"+sTestExecutionBegin+".log"))
			[ ] TestCaseExit(TRUE)
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 6
	[ ] // Back up the MOD Billing Registry
	[ ] ResOpenList("Step 6 - Back up the MOD Billing registry")
	[ ] 
	[ ]  STRING sWhereToPutBillingBackup
	[ ]  STRING sBillingRegistryBackupName
	[ ]  
	[ ] // Determine where the backup file should be placed from Tauruscfg.ini
	[ ] sWhereToPutBillingBackup = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "BillingRegistry", "WhereToPutBillingBackup", "Booyah") 
	[ ] // Build the name of the backup file
	[ ] 
	[ ] sBillingRegistryBackupName = "BillingBackup_"+sTestExecutionBegin+".ini"
	[ ] sTestValue = fReadWriteIni(csInstallPath+filTaurusIni, csWrite, "BillingRegistry", "BillingRegistryBackupName", sBillingRegistryBackupName) 
	[ ] sPathToRegExe = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "BillingRegistry", "PathToRegExe", "Booyah") 
	[-] if(!fRegExport(sPathToRegExe, gsPS1IP, csBillingRegistryPath, sWhereToPutBillingBackup+sBillingRegistryBackupName))
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] // Step 7
	[ ] // Backup the IAD Database
	[ ] // a.	Connect to the SQL Server.
	[ ] // b.	select iadData
	[ ] // c.	exec iad_maint @exempt_days = 7
	[ ] 
	[ ] ResOpenList("Step 7 - Backup the IAD Database")
	[ ] Print()
	[ ] sBackedUpDatabaseName = "Taurus_backup_Iad_db.BAK"
	[ ] sSQLBackupDirectory = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "SQLParameters", "SQLBackupDirectory", "Booyah")
	[ ] 
	[ ] bTestValue = fDBBackup(sDBLogOnInfo, csIadData, sSQLBackupDirectory, sBackedUpDatabaseName)
	[-] if (!bTestValue)
		[ ] LogError ("*** Error: Backup of IadData failed!")
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] Print("File Name is - "+sBackedUpDatabaseName)
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 8
	[ ] // Copy the IAD Database backup
	[ ] // a.	Rename iadData_db_yyymmddhhmm.bak to iadData_db_TaurusBackup.bak and write name of backup to Taurus.ini
	[ ] ResOpenList("Step 8 - Copy the IAD Database backup to iadData_TaurusBackup_<dateTime>.back")
	[ ] 
	[ ] sTaurusDatabaseName = "IadData_db_temp_TaurusBackup_"+sTestExecutionBegin+".bak"
	[ ] SYS_CopyFile (sSQLBackupDirectory+sBackedUpDatabaseName, sSQLBackupDirectory+sTaurusDatabaseName)
	[ ] Print ("IadData file copied to "+sTaurusDatabaseName+" sucessfully!")
	[ ] Print ("Note: The original file ("+sBackedUpDatabaseName+") has remained intact")
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] sCurrStepTime= fCreateDateTimeStamp (2)
	[ ] Print()
	[ ] Print("***************************************************************************************")
	[ ] Print(SubStr(sCurrStepTime,1,4)+"/"+SubStr(sCurrStepTime,5,2)+"/"+SubStr(sCurrStepTime,7,2)+" "+SubStr(sCurrStepTime,9,2)+":"+SubStr(sCurrStepTime,11,2)+":"+SubStr(sCurrStepTime,13,2))
	[ ] Print("Test setup begins here:                                                                  ")
	[ ] Print("***************************************************************************************")
	[ ] Print()
	[ ] 
	[ ] // Step 1
	[ ] // Configure Mod Billing Service Registry Values
	[ ] // 1.	Configure Billing service HKEY_LOCAL_MACHINE\Software\seachange\ITV Applications\CurrentVersion\Services\MOD Billing
	[ ] // a.	ITVExternalRoot: REG_EXPAND_SZ: D:\itv
	[ ] // b.	BillingDllName: REG_SZ: MbsDst
	[ ] // c.	BillingDLLInterval: REG_DWORD: 1 (1 unit of Type)
	[ ] // d.	BillingDLLIntervalType: REG_DWORD: 2 (hour)
	[ ] // e.	BillingDLLIntervalStart: REG_DWORD: xxxx (seconds)
	[ ] // f.	ReportFreeMovie: REG_DWORD: 1
	[ ] // g.	UploadDirectory: RED_EXPAND_SZ: \mod\billing\upload (default)
	[ ] // h.	UploadFileDuration: REG_DWORD: 432000 (default)
	[ ] ResOpenList("Step 1 - Configure MOD Billing Service registry values")
	[ ] 
	[ ] STRING sLocationOfTestRegistry
	[ ] STRING sTestRegistryName
	[ ] 
	[ ] //Determine where PreGeneratedRegistry file is
	[ ] sLocationOfTestRegistry = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "BillingRegistry", "LocationOfTestRegistry", "Booyah") 
	[ ] //Determine the Name of the Test Registry
	[ ] sTestRegistryName = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "BillingRegistry", "TestRegistryName", "Booyah") 
	[ ] 
	[ ] sPathToRegExe = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "BillingRegistry", "PathToRegExe", "Booyah") 
	[ ] 
	[-] if(!fRegImport(sPathToRegExe, gsPS1IP, sLocationOfTestRegistry+sTestRegistryName))
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] // Step 2
	[ ] // Configure MOD Billing plug-in
	[ ] // 2.	Configure Billing plug-in HKEY_LOCAL_MACHINE\Software\seachange\ITV Applications\CurrentVersion\Services\MOD Billing\BillingDLL
	[ ] // a.	DstType: REG_DWORD: 1
	[ ] // b.	DstOutputPath: REG_EXPAND_SZ: \mod\billing\upload
	[ ] ResOpenList("Step 2 - Configure MOD Billing Plug-in - Rolled into step 1 for now")
	[ ] Print()
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] // Step 3
	[ ] // Restore Known Good IAD database:
	[ ] // a.	Retrieve name and location of Pre-Generated database from the Taurus.ini
	[ ] // b.	Copy Pre-Generated IAD locally to the PS1 machine running SQL.
	[ ] // c.	Verify database is copied to the SQL machine (PS1).
	[ ] // d.	Connect to SQL server
	[ ] // e.	RESTORE DATABASE [IadData] FROM  DISK = N'C:\temp\IadData_db_200211170300.BAK' WITH  FILE = 1,  NOUNLOAD ,  STATS = 10,  RECOVERY
	[ ] // f.	Check that the IAD database was successful
	[ ] // i.	Query Analyzer returns progress and success statements.
	[ ] ResOpenList("Step 3 - Restore Known Good IAD Database")
	[ ] 
	[ ] Print ("Restarting SQL Server to clear out connections to the database")
	[-] if (!fManageService("MSSQLServer", csRestart, gsPS1IP, sExtendedStatus))         // SQLServer
		[ ] LogError ("*** Error: Backup of IadData failed! Cannot Stop MSSQLServer on "+gsPS1IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] Print ("Stopping IAD if it restarted due to the SQL restart")
	[-] if (!fManageService(csIADName, csStop, gsPS1IP, sExtendedStatus))         // IAD
		[ ] LogError ("*** Error: Cannot Stop "+csIADName+" on "+gsPS1IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] Print ("Reading in location of known IADDatabase from Tauruscfg.ini")
	[ ] 
	[ ] // Retrieve Database Location and Name from Tauruscfg.ini
	[ ] sPreGeneratedDatabaseLocation = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "PreGeneratedDatabase", "PreGeneratedDatabaseLocation", "Booyah") 
	[ ] sPreGeneratedDatabaseName = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "PreGeneratedDatabase", "PreGeneratedDatabaseName", "Booyah") 
	[ ] 
	[-] if (!fDBRestore(sDBLogOnInfo, csIadData , sPreGeneratedDatabaseLocation, sPreGeneratedDatabaseName))
		[ ]  LogError ("*** Error: Database backup not successful")
		[ ]  TestCaseExit(TRUE)
	[-] else
		[ ]  Print ("Database Restored")
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] // Step 5
	[ ] // Determine if "Viewing" records match known values:
	[ ] // a.	Connect to the IAD database on PS1.
	[ ] // b.	"select count(*) from "Viewing" and store the return value in memory in the variable "ViewingCount".
	[ ] // c.	Retrieve the value "ViewingCount_Expected" from Taurus.ini and store it locally in a variable of the same name.
	[ ] // d.	Compare return value in "ViewingCount" to "ViewingCount_Expected" (from Taurus.ini) for equality.
	[ ] 
	[ ] ResOpenList("Step 5 - Determine if 'Viewing' records in IADData match known values")
	[ ] 
	[ ] // Retrieve Known Value for Viewing Count from Tauruscfg.ini
	[ ] sViewingCount_Expected = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "PreGeneratedDatabase", "ViewingCount_Expected", "Booyah")
	[ ] 
	[ ] sViewingCount = fDBQueryForString(sDBLogOnInfo, csViewingQuery, csIadData)
	[ ] 
	[-] if (sViewingCount == sViewingCount_Expected)
		[ ] Print ("IAD 'Viewing' Records match")
		[ ] Print ("Actual = "+sViewingCount)
		[ ] Print ("Expected = "+sViewingCount_Expected)
	[-] else
		[ ] LogWarning ("*** Warning: IADData 'Viewing' record counts DO NOT match. Actual = {sViewingCount}. Expected = {sViewingCount_Expected}.")
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] // Step 6
	[ ] // Determine if "InProgress" records match known values:
	[ ] // a.	Connect to the IAD database on PS1.
	[ ] // b.	"select count(*) from InProgress" and store the return value in memory in the variable "InProgressCount".
	[ ] // c.	Retrieve the value "InProgressCount_Expected" from Taurus.ini and store it locally in a variable of the same name.
	[ ] // d.	Compare the return value in "InProgressCount" to "InProgressCount_Expected" (from Taurus.ini) for equality.
	[ ] 
	[ ] ResOpenList("Step 6 - Determine if 'InProgress' records in IADData match known values")
	[ ] 
	[ ] sInProgressCount_Expected = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "PreGeneratedDatabase", "InProgressCount_Expected", "Booyah")
	[ ] 
	[ ] sInProgressCount = fDBQueryForString(sDBLogOnInfo, csInProgressQuery, csIadData)
	[ ] 
	[-] if (sInProgressCount == sInProgressCount_Expected)
		[ ] Print ("IAD 'In Progress' Records match")
		[ ] Print ("Actual = "+sInProgressCount)
		[ ] Print ("Expected = "+sInProgressCount_Expected)
	[-] else
		[ ] LogWarning ("*** Warning: IADData 'InProgress' record counts DO NOT match. Actual = {sInProgressCount}. Expected = {sInProgressCount_Expected}.")
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] // Step 7
	[ ] // Determine the number of records that are marked as being "reported" in the database.
	[ ] // a.	Connect to the IAD database on PS1.
	[ ] // b.	"select count(*) from Viewing_Reported where Reported = 1" and store the return value in memory in the variable "ReportedRecordCount".
	[ ] // c.	Retrieve the value "ReportedRecordCount_Expected" from the Taurus.ini file and store it locally in a variable of the same name.
	[ ] // d.	Compare the return value in "ReportedRecordCount" to "ReportedRecordCount_Expected" for equality.
	[ ] 
	[ ] ResOpenList("Step 7 - Determine if 'Reported' records in IADData match known values")
	[ ] sReportedRecordCount_Expected = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "PreGeneratedDatabase", "ReportedRecordCount_Expected", "Booyah")
	[ ] 
	[ ] sReportedRecordCount = fDBQueryForString(sDBLogOnInfo, csReportedQuery, csIadData)
	[ ] 
	[-] if (sReportedRecordCount == sReportedRecordCount_Expected)
		[ ] Print ("IAD 'Reported' Records match")
		[ ] Print ("Actual = "+sReportedRecordCount)
		[ ] Print ("Expected = "+sReportedRecordCount_Expected)
	[-] else
		[ ] LogWarning ("*** Warning: IADData 'Reported' record counts DO NOT match. Actual = {sReportedRecordCount}. Expected = {sReportedRecordCount_Expected}.")
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] // Step 8	
	[ ] // Determine the number of records that are marked as being "not reported" in the database.
	[ ] // a.	Connect to the IAD database on PS1.
	[ ] // b.	"select count(*) from Viewing_Reported where Reported = 0" and store the return value in memory in the variable "NotReportedRecordCount".
	[ ] // c.	Retrieve the value "NotReportedCount_Expected from the Taurus.ini file and store it locally in a variable of the same name.
	[ ] // d.	Compare the return value in "NotReportedRecordCount" to "NotReportedRecordCount_Expected" for equality.
	[ ] 
	[ ] ResOpenList("Step 8 - Determine if 'Not Reported' records in IADData match known values")
	[ ] sNotReportedRecordCount_Expected = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "PreGeneratedDatabase", "NotReportedRecordCount_Expected", "Booyah")
	[ ] 
	[ ] sNotReportedRecordCount = fDBQueryForString(sDBLogOnInfo, csNotReportedQuery, csIadData)
	[ ] 
	[-] if (sNotReportedRecordCount == sNotReportedRecordCount_Expected)
		[ ] Print ("IAD 'Not Reported' Records match")
		[ ] Print ("Actual = "+sNotReportedRecordCount)
		[ ] Print ("Expected = "+sNotReportedRecordCount_Expected)
	[-] else
		[ ] LogWarning ("*** Warning: IADData 'Not Reported' record counts DO NOT match. Actual = {sNotReportedRecordCount}. Expected = {sNotReportedRecordCount_Expected}.")
		[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] // Step 4	
	[ ] // Starting the IAD service on PS1.
	[ ] ResOpenList("Modified Step 4 - Starting IAD on PS1 machine")
	[ ] Print("The start of IAD service had been moved here!")
	[ ] Print()
	[-] if (!fManageService(csIADName, csStart, gsPS1IP, sExtendedStatus))
		[ ] LogError ("*** Error: Cannot Start "+csIADName+" on "+gsPS1IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[ ] // sleep for 1 min to make sure IAD update the IadData database
	[ ] //sleep(60)
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] sCurrStepTime= fCreateDateTimeStamp (2)
	[ ] Print()
	[ ] Print("***************************************************************************************")
	[ ] Print(SubStr(sCurrStepTime,1,4)+"/"+SubStr(sCurrStepTime,5,2)+"/"+SubStr(sCurrStepTime,7,2)+" "+SubStr(sCurrStepTime,9,2)+":"+SubStr(sCurrStepTime,11,2)+":"+SubStr(sCurrStepTime,13,2))
	[ ] Print("Test Executing Begins Here:                                                                   ")
	[ ] Print("***************************************************************************************")
	[ ] Print()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 1
	[ ] // Set Mod Billing Service to report records shortly after the service is started:
	[ ] // a.	Retrieve current system time and store it locally in a variable.
	[ ] // b.	Set "MOD Billing" level registry value BillingDllIntervalStart: REG_DWORD: xxxx to exactly 1 minute ahead of current system time that we got from step (a).
	[ ] // c.	Record the value of BillingDllIntervalStart to a variable.
	[ ] // Steps
	[ ] // 1. Create get the latest time
	[ ] // 2. Grab the minutes portion and convert to int
	[ ] // 3. Take the minutes and add 2
	[ ] // 4. multiply by 60
	[ ] // 5. set the regval BillingDLLIntervalStart
	[ ] 
	[ ] ResOpenList("Step 1 - Set the Mod Billing Service Reporting Time")
	[ ] 
	[ ] STRING sCurrentMinutes
	[ ] INTEGER iConvertedMinutes
	[ ] Print()
	[ ] 
	[ ] //Call fDBGetDateTime function to get the date and time from the database machine.
	[ ] dtCurrDatabaseDateTime = fDBGetDateTime (sDBLogOnInfo)
	[ ] sDatabaseDateTime = FormatDateTime(dtCurrDatabaseDateTime,"yyyymmddhhmmss")
	[ ] 
	[ ] // Get the minutes time
	[ ] sCurrentMinutes = SubStr (sDatabaseDateTime, 11, 2)
	[ ] iConvertedMinutes = Val(sCurrentMinutes)
	[ ] Print ("Number of minutes before conversion - "+str(iConvertedMinutes))
	[ ] iConvertedMinutes = ((iConvertedMinutes + 2) * 60)
	[ ] Print ("Number of minutes after seconds conversion - "+str(iConvertedMinutes))
	[ ] 
	[ ] sPathToRegExe = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "BillingRegistry", "PathToRegExe", "Booyah") 
	[ ] 
	[ ] // Writing to registry
	[ ] fRegCreateValue(sPathToRegExe, gsPS1IP, "SOFTWARE\SeaChange\Itv Applications\CurrentVersion\Services\Mod Billing", "BillingDLLIntervalStart", "2", str(iConvertedMinutes))
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 2
	[ ] // Start Mod Billing Service on PS1.
	[ ] ResOpenList("Step 2 - Start Mod Billing Service on PS1")
	[ ] Print()
	[-] if (!fManageService(csMODBillingName, csStart, gsPS1IP, sExtendedStatus))
		[ ] LogError ("*** Error: Cannot Start "+csMODBillingName+" on "+gsPS1IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] // Step 2a
	[ ] // Let's sleep so that the records will report and the service will fully start
	[ ] ResOpenList("Step 2a - Wait while Mod Billing fully starts and reports records")
	[ ] Print("Sleeping for 30 seconds")
	[ ] sleep(15)
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 3
	[ ] // Verify Mod Billing started:
	[ ] // a.	 Check the \\PS1\c$\itv\log\mbs.log for the following statements
	[ ] // i.	11/27 10:24:42:166 MBS                900  (MOD) Movies on Demand Service started 
	[ ] // ii.	11/27 10:24:42:246 MBS                900  (MOD) Established connection to ITV DS
	[ ] // iii.	11/27 10:24:47:254 MBS                900  (MOD) Established connection to ITV AD
	[ ] // iv.	11/27 10:24:52:241 MBS                900  Entering CModBill::LoadMbsDll for C:\ITV\Exe\\MbsDst
	[ ] ResOpenList("Step 3 - Verify Mod Billing Service started")
	[ ] Print()
	[ ] 
	[ ] HFILE hFile
	[ ] STRING sFileLine
	[ ] STRING sModBillingLogFilePath
	[ ] 
	[ ] BOOLEAN bIsMbsStartedLogCheck1 = FALSE
	[ ] BOOLEAN bIsMbsStartedLogCheck2 = FALSE
	[ ] BOOLEAN bIsMbsStartedLogCheck3 = FALSE
	[ ] BOOLEAN bIsMbsStartedLogCheck4 = FALSE
	[ ] 
	[ ] BOOLEAN bIsMbsDstInitialized = FALSE
	[ ] 
	[ ] // Let the service get started...
	[ ] 
	[ ] sModBillingLogFilePath = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "CommandCenterLogInfo", "ModBillingLogPath", "Booyah") 
	[ ] 
	[ ] Print ("Looking for Log File at : \\"+gsPS1IP+"\"+sModBillingLogFilePath+"Mbs.log")
	[ ] 
	[ ] // check if file exists
	[-] if (!SYS_FileExists("\\"+gsPS1IP+"\"+sModBillingLogFilePath+"Mbs.log"))
		[ ] LogError ("*** Error: \\"+gsPS1IP+"\"+sModBillingLogFilePath+"Mbs.log DOES NOT EXIST")
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] hFile = FileOpen ("\\"+gsPS1IP+"\"+sModBillingLogFilePath+"Mbs.log", FM_READ)
	[-] while (FileReadLine (hFile, sFileLine))
		[ ] // Checking the file for certain key strings to determine if MOD Billing has started
		[ ] 
		[ ] // (MOD) Error on IdsBind
		[ ] 
		[ ] // Looking for "(MOD) Movies on Demand Service started"
		[-] if (StrPos (csModBillingLogMessage1, sFileLine) > 0)
			[ ] bIsMbsStartedLogCheck1 = TRUE
			[ ] Print ("Found - (MOD) Movies on Demand Service started")
		[ ] 
		[ ] // Looking for "(MOD) Established connection to ITV DS"
		[-] if (StrPos (csModBillingLogMessage2, sFileLine) > 0)
			[ ] bIsMbsStartedLogCheck2 = TRUE
			[ ] Print ("Found - (MOD) Established connection to ITV DS")
		[ ] 
		[ ] // Looking for "(MOD) Established connection to ITV AD"
		[-] if (StrPos (csModBillingLogMessage3, sFileLine) > 0)
			[ ] bIsMbsStartedLogCheck3 = TRUE
			[ ] Print ("Found - (MOD) Established connection to ITV AD")
		[ ] 
		[ ] // Looking for "Entering CModBill::LoadMbsDll"
		[-] if (StrPos (csModBillingLogMessage4, sFileLine) > 0)
			[ ] bIsMbsStartedLogCheck4 = TRUE
			[ ] Print ("Found - Entering CModBill::LoadMbsDll")
		[ ] 
	[ ] // Closing the file handle
	[ ] FileClose (hFile)
	[ ] 
	[-] if (!bIsMbsStartedLogCheck1)
		[ ] LogError ("*** Error: Log Message : '"+csModBillingLogMessage1+"' can't be found in Mbs.log")
		[ ] TestCaseExit(TRUE)
	[-] if (!bIsMbsStartedLogCheck2)
		[ ] LogError ("*** Error: Log Message : '"+csModBillingLogMessage2+"' can't be found in Mbs.log")
		[ ] TestCaseExit(TRUE)
	[-] if (!bIsMbsStartedLogCheck3)
		[ ] LogError ("*** Error: Log Message : '"+csModBillingLogMessage3+"' can't be found in Mbs.log")
		[ ] TestCaseExit(TRUE)
	[-] if (!bIsMbsStartedLogCheck4)
		[ ] LogError ("*** Error: Log Message : '"+csModBillingLogMessage4+"' can't be found in Mbs.log")
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 4
	[ ] // Verify Billing DLL initialized with the proper values:
	[ ] // a.	Check the \\PS1\c$\itv\log\mbsDst.log for the following statements
	[ ] // i.	11/27 10:24:52:251 MOD Billing        900  CConfig::GetInteger: Using database for BillingDLL\DstType value - 1
	[ ] // ii.	More T.B.D.
	[ ] ResOpenList("Step 4 - Verify Billing DLL initialized with correct values")
	[ ] 
	[ ] // Check if file exists
	[-] if (!SYS_FileExists("\\"+gsPS1IP+"\"+sModBillingLogFilePath+"MbsDST.log"))
		[ ] LogError ("*** Error: \\"+gsPS1IP+"\"+sModBillingLogFilePath+"MbsDst.log DOES NOT EXIST")
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] hFile = FileOpen ("\\"+gsPS1IP+"\"+sModBillingLogFilePath+"MbsDST.log", FM_READ)
	[-] while (FileReadLine (hFile, sFileLine))
		[ ] 
		[-] if (StrPos (csMbsDstLogMessage1, sFileLine) > 0)
			[ ] bIsMbsDstInitialized = TRUE
		[ ] 
	[-] if (!bIsMbsDstInitialized)
		[ ] LogError ("*** Error: Log Message : '"+csMbsDstLogMessage1+"' can't be found in MbsDST.log")
		[ ] TestCaseExit(TRUE)
	[-] else
		[ ] Print ("Found: '"+csMbsDstLogMessage1+"'")
	[ ] FileClose(hFile)
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] // Step 5
	[ ] // Verify that Mod Billing Service generated the "Billable Events File" on the PS1 box.
	[ ] // a.	Retrieve the value from "BillableEventsGen_GoToSleep" from Taurus.ini
	[ ] // b.	Retrieve the current system time from the Taurus.ini field "TestExecutionBegin".
	[ ] // c.	Check the directory %ItvExternalRoot%\itv\mod\billing\archive for two files:
	[ ] // i.	MbsDat_yyyymmddhhmm.dat
	[ ] // ii.	MbsDat_yyyymmddhhmm.idx
	[ ] // d.	If the files are found determine if their timestamp is greater than the time from step (a).
	[ ] // e.	If a file exist in %ItvExternalRoot%\itv\mod\billing\upload plug-in encountered an error or if there are no records to report.
	[ ] 
	[ ] 
	[ ] ResOpenList("Step 5 - Verify Mod Billing Service generated 'Billable Events File'")
	[ ] 
	[ ] sleep(180)
	[ ] // Get the Archive Path for the billing service
	[ ] sModBillingArchivePath = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "BillingDLLInfo", "ModBillingArchivePath", "Booyah")
	[ ] // Get the Upload Path for the billing service
	[ ] sModBillingUploadPath = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "BillingDLLInfo", "ModBillingUploadPath", "Booyah")
	[ ] 
	[ ] // scan directory for MbsDat.dat and MbsDat.idx
	[ ] // if they don't exist check upload for error
	[ ] 
	[ ] // We gots to first get the list of files
	[ ] lfFiles = SYS_GetDirContents (sModBillingArchivePath)
	[ ] 
	[ ] // Scanning for MbsDat.dat
	[+] for iTestValue = 1 to ListCount (lfFiles)
		[-] if (MatchStr ("*MbsDst_*", lfFiles[iTestValue].sName))
			[-] if (MatchStr("*dat*", lfFiles[iTestValue].sName))
				[-] if (fFileTimeStamp(sModBillingArchivePath+lfFiles[iTestValue].sName,2,2) > sDatabaseDateTime)
					[ ] print ("MbsDst.bak file was found. The name of the file is "+lfFiles[iTestValue].sName)
					[ ] sMbsDatFile = lfFiles[iTestValue].sName
					[ ] bMbsDatFileExist = TRUE
					[ ] 
	[ ] 
	[ ] // Scanning for MbsDat.idx
	[+] for iTestValue = 1 to ListCount (lfFiles)
		[+] if (MatchStr ("*MbsDst_*", lfFiles[iTestValue].sName))
			[+] if (MatchStr("*idx*", lfFiles[iTestValue].sName))
				[+] if (fFileTimeStamp(sModBillingArchivePath+lfFiles[iTestValue].sName,2,2) > sDatabaseDateTime)
					[ ] print ("Mbsdat.idx file was found. The name of the file is "+lfFiles[iTestValue].sName)
					[ ] sMbsDatIdxFile = lfFiles[iTestValue].sName
					[ ] bMbsDatIdxFileExist = TRUE
					[ ] 
	[ ] 
	[ ] 
	[ ] 
	[ ] // If the MbsDat.dat  as generated let's do our work
	[-] if (bMbsDatFileExist && bMbsDatIdxFileExist)
		[ ] Print ("Both files were generated...Success!")
		[ ] bCheckforMbsDstDatError = FALSE
	[-] else if (!bMbsDatFileExist && bMbsDatIdxFileExist)
		[ ] LogWarning ("*** Warning: MbsDst.dat file was not generated, but the MbsDst.idx file was")
		[ ] bCheckforMbsDstDatError = TRUE
	[-] else if (bMbsDatFileExist && !bMbsDatIdxFileExist)
		[ ] LogWarning ("*** Warning: MbsDst.dat file was not generated, but the MbsDst.idx file was")
		[ ] bCheckforMbsDstDatError = TRUE
	[-] else
		[ ] LogWarning ("*** Warning: Neither MbsDst.dat or MbsDat.idx were generated")
		[ ] bCheckforMbsDstDatError = TRUE
	[ ] 
	[-] if (bCheckforMbsDstDatError)
		[ ] // We gots to first get the list of files
		[ ] lfFiles = SYS_GetDirContents (sModBillingUploadPath)
		[ ] 
		[-] if (ListCount(lfFiles) == 0)
			[ ] LogError ("*** Error: No files in MbsDst DLL related folders")
			[ ] TestCaseExit(TRUE)
		[ ] 
		[ ] // Scanning for MbsDat.dat in the upload directory signifying an error
		[-] for iTestValue = 1 to ListCount (lfFiles)
			[-] if (MatchStr ("*mbsdst*", lfFiles[iTestValue].sName))
				[-] if (MatchStr("*dat*", lfFiles[iTestValue].sName))
					[-] if (fFileTimeStamp(sModBillingUploadPath+lfFiles[iTestValue].sName,2,2) > sDatabaseDateTime)
						[ ] Print ("mbsdst.dat file was found. The name of the file is "+lfFiles[iTestValue].sName)
						[ ] bFoundMbsDstDatError = TRUE
						[ ] TestCaseExit(TRUE)
			[ ] 
		[-] if (!bFoundMbsDstDatError)
			[ ] LogError ("*** Error: Catastrophic failure - The error file wasn't even generated")
			[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 6
	[ ] // Verify number of records in MbsDst.dat matches known figure stored in Taurus.ini.
	[ ] // a.	Open file in Perl using binary mode.
	[ ] // b.	Method of Verifying TBD.
	[ ] ResOpenList("Step 6 - Verify Number of records in MbsDat.dat matches expected value - NOT IMPLEMENTED")
	[ ] Print()
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 8
	[ ] // Verify number of records in VDL file matches expected number:
	[ ] // a.	Retrieve the "TestExecutionBegin" value from Taurus.ini
	[ ] // b.	Search \\PS1\d$\itv\mod\billing for a VDLxxxxx.txt file with a timestamp newer than the time value read in from Taurus.ini.
	[ ] // c.	Retrieve expected value from "VDL_Expected" in Taurus.ini
	[ ] // d.	Count the number of lines in \\PS1\d$\itv\mod\billing\DstOutput\VDLxxxxx.txt
	[ ] // e.	Store the count locally in a variable.
	[ ] // f.	Compare the return value in from (c) to "VDL_Expected" for equality.
	[ ] 
	[ ] ResOpenList("Step 8 - Verify number of records in VDLxxxxx.txt matches expected value")
	[ ] 
	[ ] // Retrieve Path to DST Ouput log files
	[ ] sDstOutputPath = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "BillingDLLInfo", "DstOutputPath", "Booyah")
	[ ] // Retrieve number of expected records to be contained within VDLxxxxx.txt file
	[ ] sVDLCount_Expected = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "BillingDLLInfo", "VDL_Expected", "Booyah")
	[ ] 
	[ ] // Retrieve VDL file name from ini file
	[ ] sVDLFileName = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "BillingDLLInfo", "VDL_FileName", "Booyah")
	[ ] 
	[ ] // Check that file exists
	[-] if (!SYS_FileExists(sDstOutputPath+sVDLFileName))
		[ ] LogError ("*** Error: "+sDstOutputPath+sVDLFileName+" DOES NOT EXIST")
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] // Determine if the file is fresh
	[ ] sTimeToCompare = fFileTimeStamp(sDstOutputPath+sVDLFileName, 2, 2)
	[ ] 
	[-] if (sTimeToCompare > sDatabaseDateTime)
		[ ] // Get the Number of records/lines in the file
		[ ] iVDLCount = fFileLineCount(sDstOutputPath+sVDLFileName)
		[ ] 
		[-] if(str(iVDLCount-1) == sVDLCount_Expected)
			[ ] Print("Success! VDL Counts match")
		[-] else
			[ ] LogWarning ("*** Warning:  VDL counts DO NOT match. Lines Found in file - {str(iVDLCount-1)}. Lines Expected - {sVDLCount_Expected}.")
	[-] else
		[ ] LogError ("*** Error: A new VDL File Not Generated - Bomb Test")
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 9
	[ ] // Verify number of records in MbsDst.rpt matches expected number: 
	[ ] // a.	Retrieve the "TestExecutionBegin" value from Taurus.ini
	[ ] // b.	Search \\PS1\d$\itv\mod\billing for a MbsDst_yymmddhhmmss.rpt file with a timestamp newer than the time value read in from Taurus.ini.
	[ ] // c.	Retrieve expected value from "MbsDstRpt_Expected" in Taurus.ini
	[ ] // d.	Count the number of lines (and subtract one to get the real total) in \\PS1\d$\itv\mod\billing\MbsDst_yymmddhhmmss.rpt
	[ ] // e.	Store the count locally in a variable.
	[ ] // f.	Compare the return value in (c) to "MbsDstRpt_Expected" for equality.
	[ ] ResOpenList("Step 9 - Verify number of records in MbsDst*.rpt matches expected value")
	[ ] 
	[ ] // Retrieve Path to DST Ouput log files
	[ ] sDstOutputPath = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "BillingDLLInfo", "DstOutputPath", "Booyah")
	[ ] // Retrieve number of expected records to be contained within VDLxxxxx.txt file
	[ ] sMbsDstRptCount_Expected = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "BillingDLLInfo", "MbsDstRptCount_Expected", "Booyah")
	[ ] 
	[ ] 
	[ ] // We gots to first get the list of files
	[ ] lfFiles = SYS_GetDirContents (sDstOutputPath)
	[ ] 
	[ ] //Check each file in the backup directory, look for a file name with containing the name of the database under test and a BAK extension, and if one is found and 
	[ ] //the timestamp on the file is greater than the current time, then assume the backup was successful and return the backup file name and timestamp.
	[-] for iTestValue = 1 to ListCount (lfFiles)
		[-] if (MatchStr ("*MbsDst_*", lfFiles[iTestValue].sName))
			[-] if (fFileTimeStamp(sDstOutputPath+lfFiles[iTestValue].sName,2,2) > sDatabaseDateTime)
				[ ] print ("MbsDst.rpt file was found. The name of the file is "+lfFiles[iTestValue].sName)
				[ ] sMbsDstRptFile = lfFiles[iTestValue].sName
				[ ] bMbsDstFileExist = TRUE
	[ ] 
	[ ] // If the MbsDst.rpt was generated let's do our work
	[-] if (bMbsDstFileExist)
		[ ] 
		[ ] 
		[ ] // convert the Unicode rpt file to Ascii rpt file
		[ ] sPathToRegExe = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "BillingRegistry", "PathToRegExe", "Booyah") 
		[ ] LIST OF STRING lsConvertOut
		[ ] Print("Converting "+sMbsDstRptFile+" To "+sMbsDstRptFile+".asc")
		[ ] SYS_Execute(sPathToRegExe+"convertor.exe "+chr(34)+sDstOutputPath+sMbsDstRptFile+chr(34)+" "+chr(34)+sDstOutputPath+sMbsDstRptFile+".asc"+chr(34),lsConvertOut)
		[ ] Print(lsConvertOut)
		[ ] sMbsDstRptFile = sMbsDstRptFile+".asc"
		[ ] 
		[ ] // Get the Number of records/lines in the file
		[ ] iMbsDstRptCount = fFileLineCount(sDstOutputPath+sMbsDstRptFile)
		[ ] 
		[-] if(str(iMbsDstRptCount) == sMbsDstRptCount_Expected)
			[ ] Print("Success! MbsDstRpt Counts match")
		[-] else
			[ ] LogWarning("*** Warning: MbsDstRpt Counts do not match. Lines Found in file - {str(iMbsDstRptCount)}. Lines Expected - {sMbsDstRptCount_Expected}.")
			[ ] 
	[-] else
		[ ] LogError ("*** Error: A new MbsDst File Not Generated - Bomb Test")
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 10
	[ ] // Verify X number of fields in files to known values stored in Taurus.ini
	[ ] // a.	VDLxxxxx.txt
	[ ] // i.	Starttime
	[ ] // ii.	Title
	[ ] // iii.	Minutes
	[ ] // iv.	Price
	[ ] // b.	MbsDst*.rpt
	[ ] // i.	Mac address
	[ ] // ii.	Asset UID
	[ ] // iii.	Starttime
	[ ] // iv.	Title
	[ ] // v.	Minutes
	[ ] // vi.	Price
	[ ] ResOpenList("Step 10 - Verify fields in VDLxxxxx.txt and MbsDst*.rpt")
	[ ] 
	[ ] // Read lines into LIST OF String
	[ ] // 1. Search for "What Lies beneath", increase "WhatLiesCount", check price is "$4.99" 
	[ ] // Do for both files.
	[ ] 
	[ ] STRING sLine
	[ ] STRING sTitleToVerify
	[ ] STRING sTitleCountToVerify
	[ ] STRING sTitlePriceToVerify
	[ ] HFILE HandleToVDLFile
	[ ] HFILE HandleToMbsDstFile
	[ ] 
	[ ] 
	[ ] INTEGER iTitleRecordCount=0
	[ ] LIST OF STRING lsFoundTitles = {}
	[ ] 
	[ ] // Retrieve Path to DST Ouput log files
	[ ] sDstOutputPath = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "BillingDLLInfo", "DstOutputPath", "Booyah")
	[ ] // Retrieve the title of the movie to check within VDLxxxxx.txt file and MbsDst*.rpt
	[ ] sTitleToVerify = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "PreGeneratedDatabase", "TitleToVerify", "Booyah")
	[ ] // Retrieve number of expected records for the title to be contained within VDLxxxxx.txt file and MbsDst*.rpt
	[ ] sTitleCountToVerify = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "PreGeneratedDatabase", "TitleCountToVerify", "Booyah")
	[ ] // Retrieve Price of the title
	[ ] sTitlePriceToVerify = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "PreGeneratedDatabase", "TitlePriceToVerify", "Booyah")
	[ ] 
	[ ] // Check that file exists - Just in case...
	[-] if (!SYS_FileExists(sDstOutputPath+sVDLFileName))
		[ ] LogError ("*** Error: "+sDstOutputPath+sVDLFileName+" DOES NOT EXIST")
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] // Determine if the file is fresh - Just in Case...
	[ ] sTimeToCompare = fFileTimeStamp(sDstOutputPath+sVDLFileName, 2, 2)
	[ ] 
	[-] if (sTimeToCompare > sDatabaseDateTime)
		[ ] 
		[ ] HandleToVDLFile = FileOpen (sDstOutputPath+sVDLFileName, FM_READ)
		[ ] 
		[-] while(FileReadLine (HandleToVDLFile, sLine))
			[-] if (strPos(sTitleToVerify, sLine) > 1)
				[ ] ListAppend (lsFoundTitles, sLine)
				[-] if(strPos(sTitlePriceToVerify, sLine) > 1)
					[ ] iTitleRecordCount++
					[ ] ListPrint(lsFoundTitles)
		[ ] FileClose (HandleToVDLFile)
		[ ] 
		[ ] // Wrap up and print results
		[-] if (ListCount(lsFoundTitles) == iTitleRecordCount)
			[ ] Print ("Results after scanning "+sVDLFileName+" for "+sTitleToVerify+" with price "+sTitlePriceToVerify)
			[ ] Print ("Total number of occurences of "+sTitleToVerify+" Found: "+str(ListCount(lsFoundTitles)))
			[ ] Print ("Total number of occurences of "+sTitleToVerify+" with price "+sTitlePriceToVerify+"= "+str(iTitleRecordCount))
			[ ] Print ("We Expected to find "+sTitleCountToVerify+" occurences of "+sTitleToVerify)
		[-] if (str(iTitleRecordCount) == sTitleCountToVerify)
			[ ] Print ("SUCCESS!!!! - We found the correct number of titles")
		[-] else
			[ ] LogWarning ("*** Warning: Counts do not match...something went wrong")
			[ ] Print ("Results after scanning "+sVDLFileName+" for "+sTitleToVerify+" with price "+sTitlePriceToVerify)
			[ ] Print ("Total number of occurences of "+sTitleToVerify+" Found: "+str(ListCount(lsFoundTitles)))
			[ ] Print ("Total number of occurences of "+sTitleToVerify+" with price "+sTitlePriceToVerify+"= "+str(iTitleRecordCount))
			[ ] Print ("We Expected to find "+sTitleCountToVerify+" occurences of "+sTitleToVerify)
			[ ] 
		[ ] 
		[ ] 
	[-] else
		[ ] LogError ("*** Error: A new VDL File Not Generated - Bomb Test")
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] 
	[ ] lsFoundTitles = {}
	[ ] 
	[ ] // Let's double check to ensure MbsDst*.rpt is still there
	[ ] lfFiles = SYS_GetDirContents (sDstOutputPath)
	[ ] 
	[ ] // // Expand the sTitleToVerify string to Unicode string
	[ ] // sTitleToVerify = fExpand2Unicode(sTitleToVerify)
	[ ] 
	[ ] //Check each file in the backup directory, look for a file name with containing the name of the database under test and a BAK extension, and if one is found and 
	[ ] //the timestamp on the file is greater than the current time, then assume the backup was successful and return the backup file name and timestamp.
	[-] for iTestValue = 1 to ListCount (lfFiles)
		[-] if (MatchStr ("*MbsDst_*", lfFiles[iTestValue].sName))
			[-] if (fFileTimeStamp(sDstOutputPath+lfFiles[iTestValue].sName,2,2) > sDatabaseDateTime)
				[ ] print ("MbsDst.rpt file was found. The name of the file is "+lfFiles[iTestValue].sName)
				[ ] sMbsDstRptFile = lfFiles[iTestValue].sName
				[ ] bMbsDstFileExist = TRUE
	[ ] 
	[ ] // If the MbsDst.rpt was generated let's do our work
	[-] if (bMbsDstFileExist)
		[ ] HandleToMbsDstFile = FileOpen (sDstOutputPath+sMbsDstRptFile, FM_READ)
		[ ] 
		[-] while(FileReadLine (HandleToMbsDstFile, sLine))
			[-] if (strPos(sTitleToVerify, sLine) > 1)
				[ ] ListAppend (lsFoundTitles, sLine)
				[-] if(strPos(sTitlePriceToVerify, sLine) > 1)
					[ ] iTitleRecordCount++
		[ ] FileClose (HandleToMbsDstFile)
		[ ] 
		[ ] // Wrap up and print results
		[-] if (ListCount(lsFoundTitles) == iTitleRecordCount)
			[ ] Print ("Results after scanning "+sMbsDstRptFile+" for "+sTitleToVerify+" with price "+sTitlePriceToVerify)
			[ ] Print ("Total number of occurences of "+sTitleToVerify+" Found: "+str(ListCount(lsFoundTitles)))
			[ ] Print ("Total number of occurences of "+sTitleToVerify+" with price "+sTitlePriceToVerify+"= "+str(iTitleRecordCount))
			[ ] Print ("We Expected to find "+sTitleCountToVerify+" occurences of "+sTitleToVerify)
		[-] if (str(iTitleRecordCount) == sTitleCountToVerify)
			[ ] Print ("SUCCESS!!!! - We found the correct number of titles")
		[-] else
			[ ] LogWarning ("*** Warning: Counts do not match...something went wrong")
			[ ] Print ("Results after scanning "+sMbsDstRptFile+" for "+sTitleToVerify+" with price "+sTitlePriceToVerify)
			[ ] Print ("Total number of occurences of "+sTitleToVerify+" Found: "+str(ListCount(lsFoundTitles)))
			[ ] Print ("Total number of occurences of "+sTitleToVerify+" with price "+sTitlePriceToVerify+"= "+str(iTitleRecordCount))
			[ ] Print ("We Expected to find "+sTitleCountToVerify+" occurences of "+sTitleToVerify)
			[ ] 
		[ ] 
		[ ] 
	[-] else
		[ ] LogError ("*** Error: A new MbsDst File Not Generated - Bomb Test")
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 11
	[ ] // Determine the number of records that are marked as being "reported" in the database.
	[ ] // a.	Connect to the IAD database on PS1.
	[ ] // b.	"select count(*) from Viewing_Reported where Reported = 1" and store the return value in memory in the variable "ReportedRecordCount"
	[ ] // c.	Retrieve the value "ReportedRecordCount_Expected" from the Taurus.ini file and store it locally in a variable of the same name.
	[ ] // d.	Compare the return value in "ReportedRecordCount" to "ReportedRecordCount_Expected" for equality.
	[ ] 
	[ ] ResOpenList("Step 11 - Determine if 'Reported' records in IADData match known values")
	[ ] sReportedRecordCount_Expected = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "PreGeneratedDatabase", "ReportedRecordCount_Expected_After", "Booyah")
	[ ] 
	[ ] sReportedRecordCount = fDBQueryForString(sDBLogOnInfo, csReportedQuery, csIadData)
	[ ] 
	[-] if (sReportedRecordCount == sReportedRecordCount_Expected)
		[ ] Print ("IAD 'Reported' Records match")
		[ ] Print ("Actual = "+sReportedRecordCount)
		[ ] Print ("Expected = "+sReportedRecordCount_Expected)
	[-] else
		[ ] LogWarning ("*** Warning: IADData 'Reported' record counts DO NOT match. Actual = {sReportedRecordCount}. Expected = {sReportedRecordCount_Expected}.")
		[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 12
	[ ] // Determine the number of records that are marked as being "not reported" in the database.
	[ ] // a.	Connect to the IAD database on PS1.
	[ ] // b.	"select count(*) from Viewing_Reported where Reported = 0" and store the return value in memory in the variable "NotReportedRecordCount".
	[ ] // c.	Retrieve the value "NotReportedCount_Expected from the Taurus.ini file and store it locally in a variable of the same name.
	[ ] // d.	Compare the return value in "NotReportedRecordCount" to "NotReportedRecordCount_Expected" for equality.
	[ ] 
	[ ] ResOpenList("Step 12 - Determine if 'Not Reported' records in IADData match known values")
	[ ] 
	[ ] sNotReportedRecordCount_Expected = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "PreGeneratedDatabase", "NotReportedRecordCount_Expected_After", "Booyah")
	[ ] 
	[ ] sNotReportedRecordCount = fDBQueryForString(sDBLogOnInfo, csNotReportedQuery, csIadData)
	[ ] 
	[-] if (sNotReportedRecordCount == sNotReportedRecordCount_Expected)
		[ ] Print ("IAD 'Not Reported' Records match")
		[ ] Print ("Actual = "+sNotReportedRecordCount)
		[ ] Print ("Expected = "+sNotReportedRecordCount_Expected)
	[-] else
		[ ] LogWarning ("*** Warning: IADData 'Not Reported' record counts DO NOT match. Actual = {sNotReportedRecordCount}. Expected = {sNotReportedRecordCount_Expected}.")
		[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] // Step 13
	[ ] // Determine if "InProgress" records match known values:
	[ ] // a.	Connect to the IAD database on PS1.
	[ ] // b.	"select count(*) from InProgress" and store the return value in memory in the variable "InProgressCount".
	[ ] // c.	Retrieve the value "InProgressCount_Expected" from Taurus.ini and store it locally in a variable of the same name.
	[ ] // d.	Compare the return value in "InProgressCount" to "InProgressCount_Expected" (from Taurus.ini) for equality.
	[ ] ResOpenList("Step 13 - Determine if 'InProgress' records in IADData match known values")
	[ ] 
	[ ] sInProgressCount_Expected = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "PreGeneratedDatabase", "InProgressCount_Expected_After", "Booyah")
	[ ] 
	[ ] sInProgressCount = fDBQueryForString(sDBLogOnInfo, csInProgressQuery, csIadData)
	[ ] 
	[-] if (sInProgressCount == sInProgressCount_Expected)
		[ ] Print ("IAD 'In Progress' Records match")
		[ ] Print ("Actual = "+sInProgressCount)
		[ ] Print ("Expected = "+sInProgressCount_Expected)
	[-] else
		[ ] LogWarning ("*** Warning: IADData 'InProgress' record counts DO NOT match. Actual = {sInProgressCount}. Expected = {sInProgressCount_Expected}.")
		[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 14
	[ ] // Check that each "not reported" record matches expected set of not reported records if count > 0
	[ ] ResOpenList("Step 14 - Check that each 'not reported' record matches expected set of not reported records if count > 0 - NOT IMPLEMENTED")
	[ ] Print()
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] // Step 15
	[ ] // Purge billable reported and non-billable events
	[ ] // a.	Perform an iad_purge
	[ ] // i.	SQLUTIL.EXE -Usa -x -s\t -D5 -b30 -i %ITVROOT%\Exe\MbsExtract.sql -o d:\sql2000\MSSQL\BACKUP\MbsExtract.dat -e d:\sql2000\MSSQL\BACKUP\MbsExtract.err
	[ ] 
	[ ] // Use Karyn's Run JOb thing
	[ ] 
	[ ] ResOpenList("Step 15 - Purge all billable and non-billable events")
	[ ] 
	[-] if (!fDBRunJob(sDBLogOnInfo, "msdb", csIadDataPurge, "Step 1"))
		[ ] LogError ("*** Error: IadData_purge failed")
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 16
	[ ] // Verify Mbs extract file is generated in \\PS1\d$\SQL2000\mssql\backup\mbsextract.dat_yyyymmddhhmm
	[ ] // a.	Retrieve the test execution begin time from "TestExecutionBegin" in Taurus.ini
	[ ] // b.	Search for an MbsExtract.dat file with a timestamp greater than the "TestExecutionBegin" value.
	[ ] // c.	If the files are found determine if their timestamp is greater than the time from step (a).
	[ ] ResOpenList("Step 16 - Verify MbsExtract file is generated")
	[ ] 
	[ ] // Reset Control variable
	[ ] bTestValue = FALSE
	[ ] 
	[ ] // Get the location of the MSSQL backup directory from Tauruscfg.ini
	[ ] sSQLBackupDirectory = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "SQLParameters", "SQLBackupDirectory", "Booyah")
	[ ] 
	[ ] // // Retrieve contents of the backup directory
	[ ] lfFiles = SYS_GetDirContents (sSQLBackupDirectory)
	[ ] 
	[ ] // Scan the directory for MbsExtract.dat files
	[-] for iTestValue = 1 to ListCount (lfFiles)
		[-] if (MatchStr ("*MbsExtract.dat*", lfFiles[iTestValue].sName))
			[-] if (lfFiles[iTestValue].sName > "MbsExtract.dat_"+sDatabaseDateTime)
				[ ] Print ("MbsExtract.dat file has been generated")
				[ ] Print ("Found - "+lfFiles[iTestValue].sName)
				[ ] bTestValue = TRUE
	[ ] 
	[-] if (!bTestValue)
		[ ] LogError ("*** Error: MbsExtract.dat file has not been generated")
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 17
	[ ] // Verify Mbs extract error file (MbsExtract.err_yyymmddhhmmss) is 0 bytes
	[ ] // a.	Search for an MbsExtract.err file in \\PS1\d$\SQL2000\mssql\backup\ with a timestamp greater than the “TestExecutionBeing” value.
	[ ] // b.	Determine the file size of the MbsExtract.err and store it locally in a variable.
	[ ] // c.	Compare the value in the variable from (b) and the number zero for equality.
	[ ] ResOpenList("Step 17 - Verify MbsExtract error file is 0 bytes")
	[ ] 
	[ ] // Reset Control variable
	[ ] bTestValue = FALSE
	[ ] 
	[ ] // No need to redo a SYS_GetDirContents for files (see step 16 for the scan code)
	[ ] 
	[ ] // Scan the directory for MbsExtract.err files
	[-] for iTestValue = 1 to ListCount (lfFiles)
		[-] if (MatchStr ("*MbsExtract.err*", lfFiles[iTestValue].sName))
			[-] if (lfFiles[iTestValue].sName > "MbsExtract.err_"+sDatabaseDateTime)
				[ ] Print ("MbsExtract.err file has been generated and it is 0 bytes")
				[ ] Print ("Found - "+lfFiles[iTestValue].sName)
				[ ] bTestValue = TRUE
	[ ] 
	[-] if (!bTestValue)
		[ ] LogError ("*** Error: MbsExtract.err file has not been generated")
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 18
	[ ] // Verify Mbs extract files are properly pruned by the system:
	[ ] // a.	Check that no more than 30 MbsExtract files exist in D:\SQL2000\mssql\backup
	[ ] ResOpenList("Step 18 - Verify MbsExtract files are being properly pruned at 30 by system")
	[ ] 
	[ ] // No need to redo a SYS_GetDirContents for files (see step 16 for the scan code)
	[ ] 
	[ ] // Scan the directory for MbsExtract.dat and MbsExtract.err files
	[-] for iTestValue = 1 to ListCount (lfFiles)
		[-] if (MatchStr ("*MbsExtract.dat*", lfFiles[iTestValue].sName))
			[ ] iNumMbsExtractDatFiles++
		[-] else if (MatchStr ("*MbsExtract.err*", lfFiles[iTestValue].sName))
			[ ] iNumMbsExtractErrFiles++
	[ ] 
	[-] if (iNumMbsExtractDatFiles > 30)
		[ ] LogWarning ("*** Warning: MbsExtract.dat files are not being properly pruned")
		[ ] Print ("Pruning should take place when 30 files are reached")
		[ ] Print ("The number of MbsExtract.dat Files currently present is - "+str(iNumMbsExtractDatFiles))
	[-] else if (iNumMbsExtractErrFiles > 30)
		[ ] LogWarning ("*** Warning: MbsExtract.err files are not being properly pruned")
		[ ] Print ("Pruning should take place when 30 files are reached")
		[ ] Print ("The number of MbsExtract.dat Files currently present is - "+str(iNumMbsExtractErrFiles))
	[ ] 
	[-] else
		[ ] Print ("The number of MbsExtract.dat Files currently present is - "+str(iNumMbsExtractDatFiles))
		[ ] Print ("The number of MbsExtract.err Files currently present is - "+str(iNumMbsExtractErrFiles))
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 19	
	[ ] // Verify number of records in MbsExtract file match expected number
	[ ] // a.	Retrieve expected value from “MbsExtractRecords_Expected” in Taurus.ini
	[ ] // b.	Count Number of lines in \\PS1\d$\SQL2000\mssql\backup\mbsextract.dat_yyyymmddhhmm
	[ ] // c.	Store the count locally in a variable.
	[ ] // d.	Compare the return value in from (c) to “MbsExtractRecords_Expected” for equality.
	[ ] ResOpenList("Step 19 - Verify number of records in MbsExtract file")
	[ ] Print()
	[ ] 
	[ ] STRING sMbsExtractFile
	[ ] BOOLEAN bMbsExtractFileExist = FALSE
	[ ] INTEGER iMbsExtractCount = 0
	[ ] 
	[ ] // Retrieve Path to SQL Backup folder
	[ ] sSQLBackupDirectory = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "SQLParameters", "SQLBackupDirectory", "Booyah")
	[ ] // Retrieve number of expected records to be contained within the MbsExtract file
	[ ] sMbsExtractCount_Expected = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "BillingDLLInfo", "MbsExtractCount_Expected", "Booyah")
	[ ] 
	[ ] 
	[ ] // We gots to first get the list of files
	[ ] lfFiles = SYS_GetDirContents (sSQLBackupDirectory)
	[ ] 
	[ ] //Check each file in the backup directory, look for a file name with containing the name of the database under test and a BAK extension, and if one is found and 
	[ ] //the timestamp on the file is greater than the current time, then assume the backup was successful and return the backup file name and timestamp.
	[-] for iTestValue = 1 to ListCount (lfFiles)
		[-] if (MatchStr ("*MbsExtract*", lfFiles[iTestValue].sName))
			[+] if (fFileTimeStamp(sSQLBackupDirectory+lfFiles[iTestValue].sName,2,2) > sDatabaseDateTime)
				[ ] print ("MbsExtract file was found. The name of the file is "+lfFiles[iTestValue].sName)
				[ ] sMbsExtractFile = lfFiles[iTestValue].sName
				[ ] bMbsExtractFileExist = TRUE
	[ ] 
	[ ] // If the MbsDst.rpt was generated let's do our work
	[-] if (bMbsExtractFileExist)
		[ ] // Get the Number of records/lines in the file
		[ ] iMbsExtractCount = fFileLineCount(sDstOutputPath+sMbsDstRptFile)
		[ ] 
		[-] if(str(iMbsExtractCount) == sMbsExtractCount_Expected)
			[ ] Print("Success! MbsExtract Counts match")
			[ ] Print("Records Found - "+str(iMbsExtractCount))
			[ ] Print("Records Expected - "+sMbsExtractCount_Expected)
		[-] else
			[ ] LogWarning("*** Warning: MbsExtract Counts do not match. Records Found - {str(iMbsExtractCount)}. Records Expected - {sMbsExtractCount_Expected}.")
			[ ] 
	[-] else
		[ ] LogError ("*** Error: A new MbsExtract File Not Generated - Bomb Test")
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // // Step 20
	[ ] // // Verify Fields in MbsExtract file
	[ ] // ResOpenList("Step 20 - Verify Fields in MbsExtract File - NOT IMPLEMENTED")
	[ ] // Print()
	[ ] // ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 21
	[ ] // Verify that IAD is now purged of all "Reported" records
	[ ] // Connect on PS1 to IadData
	[ ] // "select count(*) from Viewing_Reported where Reported = 1" should == 0
	[ ] ResOpenList("Step 21 - Verify IADData is purged of all 'reported' records")
	[ ] 
	[ ] sReportedRecordCount = fDBQueryForString(sDBLogOnInfo, csReportedQuery, csIadData)
	[ ] 
	[-] if (sReportedRecordCount == "0")
		[ ] Print ("IADData 'Reported' record counts match")
		[ ] Print ("Actual = "+sReportedRecordCount)
		[ ] Print ("Expected = 0")
	[-] else
		[ ] LogWarning ("*** Warning: IADData 'Reported' record counts DO NOT match. Actual = {sReportedRecordCount}. Expected = 0")
		[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 22
	[ ] // Verify that all "Not Reported" records have been purged
	[ ] // Connect on PS1 to IadData
	[ ] // "select count(*) from Viewing_Reported where Reported = 0" should == 0
	[ ] ResOpenList("Step 22 - Verify IADData is purged of all 'Not Reported' records")
	[ ] 
	[ ] sNotReportedRecordCount = fDBQueryForString(sDBLogOnInfo, csNotReportedQuery, csIadData)
	[ ] 
	[-] if (sNotReportedRecordCount == "0")
		[ ] Print ("IADData 'Not Reported' record counts match")
		[ ] Print ("Actual = "+sReportedRecordCount)
		[ ] Print ("Expected = 0")
	[-] else
		[ ] LogWarning ("*** Warning: IADData 'Not Reported' record counts DO NOT match. Actual = {sNotReportedRecordCount}. Expected = 0")
		[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 23
	[ ] // Verify that all "In Progress" records have been purged
	[ ] // Connect on PS1 to IadData
	[ ] // "select count(*) from InProgress" should == 0
	[ ] ResOpenList("Step 22 - Verify IADData is purged of all 'InProgress' records")
	[ ] 
	[ ] sInProgressCount = fDBQueryForString(sDBLogOnInfo, csInProgressQuery, csIadData)
	[ ] 
	[-] if (sInProgressCount == "0")
		[ ] Print ("IADData 'InProgress' record counts match")
		[ ] Print ("Actual = "+sInProgressCount)
		[ ] Print ("Expected = 0")
	[-] else
		[ ] LogWarning ("*** Warning: IADData 'InProgress' record counts DO NOT match. Actual = {sInProgressCount}. Expected = 0")
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] sCurrStepTime= fCreateDateTimeStamp (2)
	[ ] Print()
	[ ] Print("***************************************************************************************")
	[ ] Print(SubStr(sCurrStepTime,1,4)+"/"+SubStr(sCurrStepTime,5,2)+"/"+SubStr(sCurrStepTime,7,2)+" "+SubStr(sCurrStepTime,9,2)+":"+SubStr(sCurrStepTime,11,2)+":"+SubStr(sCurrStepTime,13,2))
	[ ] Print("Test Case Cleanup:                                                                     ")
	[ ] Print("***************************************************************************************")
	[ ] Print()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 1
	[ ] // Stop IAD in preparation for database restore
	[ ] ResOpenList("Step 1 - Stop IAD for saved IAD database restore")
	[ ] Print()
	[-] if (!fManageService(csIADName, csStop, gsPS1IP, sExtendedStatus))
		[ ] LogError ("*** Error: Cannot Stop "+csIADName+" on "+gsPS1IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 2
	[ ] // Restore saved IAD Database
	[ ] // Print ("Restoring Saved IAD database now...")
	[ ] ResOpenList("Step 2 - Restore saved IAD database")
	[ ] 
	[ ] Print ("Restarting SQL Server to clear out connections to the database")
	[-] if (!fManageService("MSSQLServer", csRestart, gsPS1IP, sExtendedStatus))         // SQLServer
		[ ] LogError ("*** Error: Cannot Stop MSSQLServer on "+gsPS1IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] Print ("Stopping IAD if it restarted due to the SQL restart")
	[-] if (!fManageService(csIADName, csStop, gsPS1IP, sExtendedStatus))         // IAD
		[ ] LogError ("*** Error: Cannot Stop "+csIADName+" on "+gsPS1IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] Print ("Reading in location of saved IADDatabase from Tauruscfg.ini")
	[ ] sSQLBackupDirectory = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "SQLParameters", "SQLBackupDirectory", "Booyah")
	[ ] 
	[-] if (!fDBRestore(sDBLogOnInfo, csIadData , sSQLBackupDirectory, sBackedUpDatabaseName))
		[ ] LogError ("*** Error: Database backup not successful")
		[ ] TestCaseExit(TRUE)
	[-] else
		[ ] Print ("Database Restored")
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 3
	[ ] // Delete Backup Database File
	[ ] ResOpenList("Step 3 - Delete Copied Backup Database File")
	[ ] 
	[-] if SYS_FileExists(sSQLBackupDirectory+sBackedUpDatabaseName)
		[ ] Print("Deleting "+sTaurusDatabaseName)
		[ ] SYS_RemoveFile(sSQLBackupDirectory+sTaurusDatabaseName)
		[ ] Print(sTaurusDatabaseName+" Deleted")
	[-] else 
		[ ] LogError("*** Error: The IAD Database backup "+sTaurusDatabaseName+" can't be found!!")
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] 
	[ ] // Step 4
	[ ] // Restore MOD Billing Registry
	[ ] ResOpenList("Step 4 - Restore Mod Billing Registry")
	[ ] 
	[ ] sPathToRegExe = fReadWriteIni(csInstallPath+filTaurusIni, csRead, "BillingRegistry", "PathToRegExe", "Booyah") 
	[ ] 
	[ ] 
	[ ] // First, Delete the current Registry Key
	[ ] fRegDeleteKey(sPathToRegExe, gsPS1IP, csBillingRegistryPath)
	[ ] // Second, Import our backup
	[-] if(!fRegImport(sPathToRegExe, gsPS1IP, sWhereToPutBillingBackup+sBillingRegistryBackupName))
		[ ] TestCaseExit(TRUE)
	[ ] // If Import is successful we need to delete backup :)
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] // Step 5
	[ ] // Start the remaining services...
	[ ] ResOpenList("Step 5 - Starting necessary services for cleanup")
	[ ] Print()
	[-] if (!fManageService(csMODName, csStart, gsAPP1IP, sExtendedStatus))        // Movies on Demand
		[ ] LogError ("*** Error: Cannot Start "+csMODName+" on "+gsAPP1IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[-] if (!fManageService(csMOD2Name, csStart, gsAPP2IP, sExtendedStatus))       // Movies on Demand2
		[ ] LogError ("*** Error: Cannot Start "+csMOD2Name+" on "+gsAPP2IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[-] if (!fManageService(csICMName, csStart, gsCM1IP, sExtendedStatus))         // ICM
		[ ] LogError ("*** Error: Cannot Start "+csICMName+" on "+gsCM1IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[-] if (!fManageService(csICM2Name, csStart, gsCM2IP, sExtendedStatus))        // ICM2
		[ ] LogError ("*** Error: Cannot Start "+csICM2Name+" on "+gsCM2IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[-] if (!fManageService(csSCTPName, csStart, gsAPP1IP, sExtendedStatus))       // SCTP
		[ ] LogError ("*** Error: Cannot Start "+csSCTPName+" on "+gsAPP1IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[-] if (!fManageService(csSCTP2Name, csStart, gsAPP2IP, sExtendedStatus))      // SCTP2
		[ ] LogError ("*** Error: Cannot Start "+csSCTP2Name+" on "+gsAPP2IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[-] if (!fManageService(csIADName, csStart, gsPS1IP, sExtendedStatus))         // IAD
		[ ] LogError ("*** Error: Cannot Start "+csIADName+" on "+gsPS1IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[-] if (!fManageService(csSysmonName, csStart, gsMDS2IP, sExtendedStatus))     // Sysmon
		[ ] LogError ("*** Error: Cannot Start "+csSysmonName+" on "+gsMDS2IP+" "+sExtendedStatus)
		[ ] TestCaseExit(TRUE)
	[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] 
	[ ] // Step 6
	[ ] // Write Current System time to Tauruscfg.ini in the value "TestExecutionEnd"
	[ ] ResOpenList("Step 6 - Write Current System time to 'TestExecutionEnd' in Tauruscfg.ini")
	[ ] 
	[ ] sTestExecutionEnd = fCreateDateTimeStamp (2)
	[ ] sTestValue = fReadWriteIni(csInstallPath+filTaurusIni, csWrite, "TimeInformation", "TestExecutionEnd", sTestExecutionEnd)
	[ ] 
	[-] if (Upper(sTestValue) == "TRUE")
		[ ] Print (sTestExecutionEnd+" was successfully written to Tauruscfg.ini")
	[-] else
		[ ] LogWarning ("*** Warning: Couldn't write to Tauruscfg.ini")
	[ ] ResCloseList()
	[ ] 
	[ ] // The test case PASSED
	[ ] fLogPassFail("Passed","", "")
	[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] // END: TAURUS.T
[ ] //*****************************************************************************
