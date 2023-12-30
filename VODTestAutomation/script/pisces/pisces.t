[ ] //*****************************************************************************
[ ] //  NAME:					  PISCES.T
[ ] //
[ ] //  DESCRIPTION:		Test script for for Pisces component of Zodiac project.
[ ] //									Also contains test cases to test my functions that I create.
[ ] //
[ ] //  HISTORY:
[ ] //
[ ] //  Date						Developer					Description
[ ] //  ********				***********				*****************************************
[ ] //  12/16/02        K. Sullivan       Initial design and development
[ ] //	02/07/03				K. Sullivan				tcTestCase0001 is the Pisces sunny day test
[ ] //																		case. Other test cases were used to test
[ ] //																		various functions that I wrote. 
[ ] //
[ ] //*****************************************************************************
[ ] 
[ ] 
[ ] use "piscesfunc.inc"
[ ] use "piscesdefs.inc"
[ ] use "zodiacdatabase.inc"
[ ] use "zodiacutil.inc"
[ ] use "zodiacdefs.inc"
[ ] use "zodiacfiles.inc"
[ ] use "zodiacmsgs.inc"
[ ] use "zodiacfunc.inc"
[ ] use "zodiacnt.inc"
[ ] use "mswconst.inc"
[ ] 
[ ] // Store the current script name.
[ ] const string gsScriptName = Trim(StrTran(GetProgramName( ), ".t", " "))
[ ] 
[ ]  // Read the project ini file to determine where the data drive is.
[ ] const string gsDataDrive = fReadWriteIni(SYS_GetDrive() + ":\autotest\zodiac\" +
    														 						gsScriptName + "\" + gsScriptName + ".ini", csRead,
    																				"ITVQA","DataDrive","")
[ ] // Build the PassFailLog string
[ ] const string gsPassFailLog = gsDataDrive + csMainLogPath + "\"  + gsScriptName + "\" +
    											 			gsScriptName + "log"
[ ] 
[+] main()
	[ ] 
	[ ] //Create the Pisces Pass/Fail log.
	[ ] fCreatePassFailLog()
	[ ] 
	[ ] //Call the testcases
	[ ] // tcTestCase0001()
	[ ] // tcTestCase0002()
	[ ] // tcTestCase0003()
	[ ] // tcTestCase0004()
	[ ] // tcTestCase0005()
	[ ] // tcTestCase0006()
	[ ] // tcTestCase0007()
	[ ] // tcTestCase0008()
	[ ] // tcTestCase0009()
	[ ] // tcTestCase0010()
	[ ] // tcTestCase0011()
	[ ] // tcTestCase0015()
	[ ] 
[ ] //*****************************************************************************
[ ] // Testcase execution begins here
[ ] //*****************************************************************************
[ ] 
[ ] //This will be the Pisces sunny day test case.
[+] testcase tcTestCase0001 ()
	[ ] //Declare variables for the test case.
	[ ] STRING sCCLogOnInfo
	[ ] STRING sDWLogOnInfo
	[ ] STRING sDBBackupDir
	[ ] STRING sDWBackupDir
	[ ] STRING sTestFileDir
	[ ] STRING sDWSQLDir
	[ ] STRING sDBBackupFile
	[ ] STRING sTestDBBackupFile
	[ ] STRING sTestBackupFile
	[ ] STRING sCurrDateTime
	[ ] 
	[ ] //Read the ini file to get the test data for ITV Command Center Data.
	[ ] sCCLogOnInfo = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "DatabaseLogOnInfo", "ITVCCLogOn", "")
	[ ] sDBBackupDir = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "FileDirectories", "DBBackupDir", "")
	[ ] sTestFileDir = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "FileDirectories", "TestFileDir", "")
	[ ] 
	[ ] //Read the ini file to get the test data for Data Warehouse machine.
	[ ] sDWLogOnInfo = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "DatabaseLogOnInfo", "DWHLogOn", "")
	[ ] sDWBackupDir = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "FileDirectories", "DWBackupDir", "")
	[ ] sDWSQLDir = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "FileDirectories", "DWSQLDir", "")
	[ ] 
	[ ] //Connect to desired command center.
	[ ] //Not done yet - assumed that Silk box has net use access to the CC and DW boxes.
	[ ] 
	[ ] //Steps 1 & 2. Stop the ITV services on all of the Command Center nodes.
	[ ] //Stop on Command Center 1.
	[+] if (fControlServices ("ZFRED1", "stop", lsITVServices1))
		[ ] print ("The services stopped successfully, continuing with test.")
	[+] else
		[ ] print ("The services did not stop, can't continue with test.")
		[ ] // Fail test case.
	[ ] //Stop on Command Center 2.
	[+] if (fControlServices ("ZFRED2", "stop", lsITVServices2))
		[ ] print ("The services stopped successfully, continuing with test.")
	[+] else
		[ ] print ("The services did not stop, can't continue with test.")
		[ ] // Fail test case.
	[ ] 
	[ ] // Step 3. Save the current time to Pisces.ini.
	[ ] // Call function from zodiacutil.inc to get the current date and time.
	[ ] sCurrDateTime = fCreateDateTimeStamp (2)
	[ ] print ("The current date and time is:  {sCurrDateTime}")
	[ ] 
	[ ] //Call function from zodiacutil.inc to write the time to the ini file.
	[ ] //Should add something here to verify that the file is not open, or this won't work.  
	[ ] fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csWrite, "DateTime", "CurrDateTime", "{sCurrDateTime}")
	[ ] 
	[ ] // Step 4. Save the current MOD Billing Extract files and load known test set of MOD Billing Extract files.
	[ ] SYS_CopyFile (sTestFileDir +"MbsExtract.dat_test", sDBBackupDir +"MbsExtract.dat_"+sCurrDateTime)
	[ ] print ("Copied test MbsExtract file to backup directory.")
	[ ] 
	[ ] //Step 5. Save current IAD database and load known test IAD database.
	[+] if (fBackupDatabase (sCCLogOnInfo, "IadData", sDBBackupDir, sDBBackupFile))
		[ ] print ("The IAD database was backed up and the backup file was renamed succesfully.")
		[ ] fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csWrite, "BackupFiles", "IADBackupFile", "{sDBBackupFile}")
	[+] else
		[ ] print ("***ERROR*** The IAD database was not backed up and the backup file was not renamed succesfully, failing test case.")
	[ ] // Restore IAD database using the known test database backup file.
	[ ] // Restart mssqlserver service and dependent services on the Command Center before performing restore.
	[+] if (fManageService("MSSQLSERVER", "restart","zfred1"))
		[ ] print ("MSSQLSERVER was restarted, so we can go on to the restore.")
	[+] else
		[ ] print ("MSSQLSERVER was not restarted, so we can't go on to the restore.")
		[ ] print ("***ERROR*** The test IAD database cannot be restored - failing test case!")
		[ ] //Have to stop IAD and IDS again since they got restarted with MSSQLSERVER.
	[+] if (fManageService("iad", "stop", "zfred1"))
		[ ] print ("IAD service was stopped, so we can to on to the restore.")
	[+] else
		[ ] print ("IAD service was not stopped, so we can't go on to the restore.")
		[ ] print ("***ERROR*** The test IAD database cannot be restored - failing test case!")
	[+] if (fManageService("ids", "stop", "zfred1"))
		[ ] print ("IDS service was stopped, so we can to on to the restore.")
	[+] else
		[ ] print ("IDS service was not stopped, so we can't go on to the restore.")
		[ ] print ("***ERROR*** The test IAD database cannot be restored - failing test case!")
	[ ] //Get the name of the test file to restore from the ini file.
	[ ] sTestDBBackupFile = (fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "TestBackupFiles", "IadDataTestBackupFile", ""))
	[ ] //Restore the test database.
	[+] if (fDBRestore(sCCLogOnInfo, "IadData", sTestFileDir, sTestDBBackupFile))
		[ ] print ("The restore of IadData was successful.")
	[+] else
		[ ] Print ("Restore of IadData failed!")
	[ ] 
	[ ] //Step 6. Save current IDS database and load known test IDS database.
	[+] if (fBackupDatabase (sCCLogOnInfo, "IdsData", sDBBackupDir, sDBBackupFile))
		[ ] print ("The IDS database was backed up and the backup file was renamed succesfully.")
		[ ] fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csWrite, "BackupFiles", "IDSBackupFile", "{sDBBackupFile}")
	[+] else
		[ ] print ("***ERROR*** The IDS database was not backed up and the backup file was not renamed succesfully, failing test case.")
	[ ] // Restore IDS database using the known test database backup file.
	[ ] // Restart mssqlserver service and dependent services on the Command Center before performing restore.
	[+] if (fManageService("MSSQLSERVER", "restart","zfred1"))
		[ ] print ("MSSQLSERVER was restarted, so we can go on to the restore.")
	[+] else
		[ ] print ("MSSQLSERVER was not restarted, so we can't go on to the restore.")
		[ ] print ("***ERROR*** The test IDS database cannot be restored - failing test case!")
		[ ] //Have to stop IAD and IDS again since they got restarted with MSSQLSERVER.
	[+] if (fManageService("iad", "stop", "zfred1"))
		[ ] print ("IAD service was stopped, so we can to on to the restore.")
	[+] else
		[ ] print ("IAD service was not stopped, so we can't go on to the restore.")
		[ ] print ("***ERROR*** The test IDS database cannot be restored - failing test case!")
	[+] if (fManageService("ids", "stop", "zfred1"))
		[ ] print ("IDS service was stopped, so we can to on to the restore.")
	[+] else
		[ ] print ("IDS service was not stopped, so we can't go on to the restore.")
		[ ] print ("***ERROR*** The test IDS database cannot be restored - failing test case!")
	[ ] //Get the name of the test file to restore from the ini file.
	[ ] sTestDBBackupFile = (fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "TestBackupFiles", "IdsDataTestBackupFile", ""))
	[ ] //Restore the test database.
	[+] if (fDBRestore(sCCLogOnInfo, "IdsData", sTestFileDir, sTestDBBackupFile))
		[ ] print ("The restore of IdsData was successful.")
	[+] else
		[ ] Print ("Restore of IdsData failed!")
	[ ] 
	[ ] //Step 7. Save current ICM database and load known test ICM database.
	[+] if (fBackupDatabase (sCCLogOnInfo, "IcmData", sDBBackupDir, sDBBackupFile))
		[ ] print ("The ICM database was backed up and the backup file was renamed succesfully.")
		[ ] fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csWrite, "BackupFiles", "ICMBackupFile", "{sDBBackupFile}")
	[+] else
		[ ] print ("***ERROR*** The ICM database was not backed up and the backup file was not renamed succesfully, failing test case.")
	[ ] // Restore ICM database using the known test database backup file.
	[ ] // Restart mssqlserver service and dependent services on the Command Center before performing restore.
	[+] if (fManageService("MSSQLSERVER", "restart","zfred1"))
		[ ] print ("MSSQLSERVER was restarted, so we can go on to the restore.")
	[+] else
		[ ] print ("MSSQLSERVER was not restarted, so we can't go on to the restore.")
		[ ] print ("***ERROR*** The test ICM database cannot be restored - failing test case!")
		[ ] //Have to stop IAD and IDS again since they got restarted with MSSQLSERVER.
	[+] if (fManageService("iad", "stop", "zfred1"))
		[ ] print ("IAD service was stopped, so we can to on to the restore.")
	[+] else
		[ ] print ("IAD service was not stopped, so we can't go on to the restore.")
		[ ] print ("***ERROR*** The test ICM database cannot be restored - failing test case!")
	[+] if (fManageService("ids", "stop", "zfred1"))
		[ ] print ("IDS service was stopped, so we can to on to the restore.")
	[+] else
		[ ] print ("IDS service was not stopped, so we can't go on to the restore.")
		[ ] print ("***ERROR*** The test ICM database cannot be restored - failing test case!")
	[ ] //Get the name of the test file to restore from the ini file.
	[ ] sTestDBBackupFile = (fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "TestBackupFiles", "IcmDataTestBackupFile", ""))
	[ ] //Restore the test database.
	[+] if (fDBRestore(sCCLogOnInfo, "IcmData", sTestFileDir, sTestDBBackupFile))
		[ ] print ("The restore of IcmData was successful.")
	[+] else
		[ ] Print ("Restore of IcmData failed!")
	[ ] 
	[ ] //Step 8. Save current SubscriberData database and load known test SubscriberData database.
	[+] if (fBackupDatabase (sCCLogOnInfo, "SubscriberData", sDBBackupDir, sDBBackupFile))
		[ ] print ("The Subscriber database was backed up and the backup file was renamed succesfully.")
		[ ] fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csWrite, "BackupFiles", "SubscriberDataBackupFile", "{sDBBackupFile}")
	[+] else
		[ ] print ("***ERROR*** The Subscriber database was not backed up and the backup file was not renamed succesfully, failing test case.")
	[ ] // Restore Subscriber database using the known test database backup file.
	[ ] // Restart mssqlserver service and dependent services on the Command Center before performing restore.
	[+] if (fManageService("MSSQLSERVER", "restart","zfred1"))
		[ ] print ("MSSQLSERVER was restarted, so we can go on to the restore.")
	[+] else
		[ ] print ("MSSQLSERVER was not restarted, so we can't go on to the restore.")
		[ ] print ("***ERROR*** The test Subscriber database cannot be restored - failing test case!")
		[ ] //Have to stop IAD and IDS again since they got restarted with MSSQLSERVER.
	[+] if (fManageService("iad", "stop", "zfred1"))
		[ ] print ("IAD service was stopped, so we can to on to the restore.")
	[+] else
		[ ] print ("IAD service was not stopped, so we can't go on to the restore.")
		[ ] print ("***ERROR*** The test Subscriber database cannot be restored - failing test case!")
	[+] if (fManageService("ids", "stop", "zfred1"))
		[ ] print ("IDS service was stopped, so we can to on to the restore.")
	[+] else
		[ ] print ("IDS service was not stopped, so we can't go on to the restore.")
		[ ] print ("***ERROR*** The test Subscriber database cannot be restored - failing test case!")
	[ ] //Get the name of the test file to restore from the ini file.
	[ ] sTestDBBackupFile = (fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "TestBackupFiles", "SubscriberDataTestBackupFile", ""))
	[ ] //Restore the test database.
	[+] if (fDBRestore(sCCLogOnInfo, "SubscriberData", sTestFileDir, sTestDBBackupFile))
		[ ] print ("The restore of SubscriberData was successful.")
	[+] else
		[ ] Print ("Restore of SubscriberData failed!")
	[ ] 
	[ ] // Steps 9-12 Save current Data Warehouse database.
	[ ] // Step 9.	Connect to SQL Server on the Data Warehouse box.
	[ ] // Step 10.	Run the backup command to do a FULL backup of IdwData.  
	[ ] // Step 11.	Verify backup file was created.
	[ ] // Step 12.	Move the newly backed up database to the temporary backup directory and write the name of the file to the ini file.and write the name of the file to the ini file.
	[ ] 
	[ ] // Check to see that there isn't already a copy of idwbackup.bak in the temporary backup directory.  If there is,
	[ ] // delete it.
	[+] if SYS_FileExists (sTestFileDir +"idwbackup.bak")
		[ ] SYS_RemoveFile(sTestFileDir +"idwbackup.bak")
	[ ] //Perform the backup and move the backup file to the temporary backup directory.
	[+] if (fDBBackup(sDWLogOnInfo, "IdwData", sDWBackupDir, sDBBackupFile))
		[ ] print("The backup file name is {sDBBackupFile}.")
		[ ] SYS_MoveFile (sDWBackupDir +sDBBackupFile, sTestFileDir +sDBBackupFile)
		[ ] sleep (3)
		[+] if SYS_FileExists (sTestFileDir +sDBBackupFile)
			[ ] Print ("The backup file {sDBBackupFile} was moved to the temporary backup directory {sTestFileDir}")
			[ ] fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csWrite, "BackupFiles", "IdwDataBackupFile", "{sDBBackupFile}")
		[+] else
			[ ] print ("Could not move the backup file to the temp directory.")
	[+] else
		[ ] Print ("Backup of IdwData failed!")
	[ ] 
	[ ] // Steps 13-16 Delete Data Warehouse Database.
	[ ] // Check to see if the database exists before deleting.
	[+] if (fDBExists (sDWLogOnInfo, "IdwData"))
		[ ] print ("IdwData database exists, so we will delete it.")
		[ ] // Restart mssqlserver service and dependent services on Data Warehouse box before deleting database.
		[+] if (fManageService("MSSQLSERVER", "restart","zfred_dw"))
			[ ] print ("MSSQLSERVER was restarted, so we can go on to the delete step.")
		[+] else
			[ ] print ("MSSQLSERVER was not restarted, so we can't go on to the delete step.")
			[ ] print ("***ERROR*** The Data Warehouse database cannot be deleted, failing test case.")
			[ ] // Call function to delete the Data Warehouse Database.
		[+] if (fDBDelete (sDWLogOnInfo, "IdwData"))
			[ ] print ("The IdwData database was deleted, so move onto initialization.")
		[+] else
			[ ] print ("***ERROR*** The IdwData database was not deleted, failing test case.")
	[+] else 
		[ ] print ("IdwData does not exist, so perform initialization.")
		[ ] 
	[ ] // Steps 16-38 Initialize Data Warehouse.
	[ ] // Call function to initialize Data Warehouse, which runs a series of SQL scripts.
	[+] if (fInitDW(sDWLogOnInfo, sDWSQLDir))
		[ ] print ("The Data Warehouse Database has been initialized.")
	[+] else
		[ ] print ("***ERROR*** The Data Warehouse Database has not been initialized. Failing test case.")
	[ ] // Step 39 Verify that the database was created.
	[+] if (fDBExists (sDWLogOnInfo, "IdwData"))
		[ ] print ("Proceed to populate IdwData database with data.")
	[+] else
		[ ] print ("***ERROR*** Since the database was not created, cannot proceed to populate with data, failing test case.")
	[ ] // Start IISAdmin and dependent services on the Data Warehouse box.
	[+] if (fManageService(sIISName , "start", "ZFRED_DW"))
		[ ] print ("IIS Admin Service was started.")
	[+] else 
		[ ] print ("***ERROR*** IIS Admin Service was not started, so reports cannot be run, failing test case.")
	[+] if (fManageService("w3svc" , "start", "ZFRED_DW"))
		[ ] print ("w3svc was started.")
	[+] else 
		[ ] print ("***ERROR*** The w3svc was not started, so reports cannot be run, failing test case.")
		[ ] 
	[ ] // Steps 40-49 Run Data Warehouse Import Jobs to populate data in DW database.
	[+] if (fRunDWJobs (sDWLogOnInfo))
		[ ] print ("All of the import jobs ran successfully, so Reports can now be run.")
	[+] else
		[ ] print ("***ERROR**** Not all of the import jobs ran successfully, failing test case.")
		[ ] 
	[ ] // Steps 50 - 68 Run Accounting Buy Rate Report and Verify Data on report.
	[ ] // Read the ini file to get the test data for validation of the report, for this test case
	[ ] // we are using the Accounting Buy Rate report data.
	[ ] STRING sReport
	[ ] STRING sTitle
	[ ] STRING sDateFrom
	[ ] STRING sDateTo
	[ ] STRING sGrandTotal
	[ ] sReport = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "AcctBuyRateReport", "Report", "")
	[ ] sTitle = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "AcctBuyRateReport", "ReportTitle", "")
	[ ] sDateFrom = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "AcctBuyRateReport", "ReportDateFrom", "")
	[ ] sDateTo = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "AcctBuyRateReport", "ReportDateTo", "")
	[ ] sGrandTotal = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "AcctBuyRateReport", "GrandTotal", "")
	[ ] 
	[ ] // Delete any existing exported reports from previous test cases from the Export Directory.
	[ ] // But first, read ini file to get location of Export files.
	[ ] STRING sExportLocation = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "FileDirectories", "ExportLocation", "")
	[+] if SYS_FileExists (sExportLocation +"{sReport}.txt")
		[ ] SYS_RemoveFile (sExportLocation +"{sReport}.txt")
		[ ] print ("Existing export file was removed, continue with export function.")
	[+] else
		[ ] print ("No existing export files were found in the Export directory, continue with export function.")
	[ ] // Run and export the report.
	[ ] STRING sExportFile =""
	[+] if (fExportReport (sReport, sExportFile))
		[ ] print ("The file was exported.")
		[ ] print ("The name of the file is {sExportFile}.")
	[+] else
		[ ] print ("The file was not exported, failing test case.")
	[ ] // Verify the report contents - compare against known data.
	[+] if (fVerifyExportReportContents (sReport, sTitle, sDateFrom, sDateTo, sGrandTotal))
		[ ] print ("Verification of the report contents passed.  Proceeding to cleanup steps.")
	[+] else
		[ ] print ("***ERROR*** Verification of the report contents failed.  Failing test case and continuing onto cleanup steps.")
		[ ] 
	[ ] // TEST CLEANUP BEGINS HERE.
	[ ] // Steps 69-72 Delete current test data warehouse database and reload saved database.
	[ ] // Check to see if the database exists before deleting.
	[+] if (fDBExists (sDWLogOnInfo, "IdwData"))
		[ ] print ("IdwData database exists, so we will delete it.")
		[ ] // Restart mssqlserver service and dependent services on Data Warehouse box before deleting.
		[+] if (fManageService("MSSQLSERVER", "restart","zfred_dw"))
			[ ] print ("MSSQLSERVER was restarted, so we can go on to the delete.")
		[+] else
			[ ] print ("MSSQLSERVER was not restarted, so we can't go on to the delete.")
			[ ] print ("***ERROR*** The Data Warehouse database cannot be deleted. Failing test case.")
		[ ] // Call function to delete the Data Warehouse Database.
		[+] if (fDBDelete (sDWLogOnInfo, "IdwData"))
			[ ] print ("The IdwData database was deleted, so move onto restore.")
		[+] else
			[ ] // Fail test case here, or try again.
			[ ] print ("The IdwData database was not deleted, failing test case.")
	[+] else 
		[ ] print ("IdwData does not exist, must have been deleted.")
	[ ] 
	[ ] // Steps 73-74 Restore Data Warehouse database back to the way it was before the test.
	[ ] // Delete the test copy of idwbackup.bak in the temporary backup directory.
	[+] if SYS_FileExists (sDWBackupDir +"idwbackup.bak")
		[ ] SYS_RemoveFile(sDWBackupDir +"idwbackup.bak")
	[ ] // Restart mssqlserver service and dependent services on Data Warehouse box before performing restore.
	[+] if (fManageService("MSSQLSERVER", "restart","zfred_dw", sStatus))
		[ ] print ("MSSQLSERVER was restarted, so we can go on to the restore.")
	[+] else
		[ ] print ("MSSQLSERVER was not restarted, so we can't go on to the restore.")
		[ ] print ("***ERROR*** The Data Warehouse database cannot be restored.  You must manually restore this database!")
	[ ] // Get the name of the backup file saved before the test, so the Data Warehouse database can be restored.")
	[ ] sDBBackupFile = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "BackupFiles", "IdwDataBackupFile", "")
	[ ] // Copy the backed up database IdwBackup.bak from the temporary backup directory to the SQL directory.
	[+] if SYS_FileExists (sTestFileDir +sDBBackupFile)
			[ ] print ("Proceeding to copy the backup file {sDBBackupFile} from the temporary backup directory {sTestFileDir} to the SQL directory for restore.")
			[ ] SYS_CopyFile (sTestFileDir +sDBBackupFile, sDWBackupDir +sDBBackupFile)
			[+] if SYS_FileExists (sDWBackupDir +sDBBackupFile)
				[ ] print ("The backup directory was copied and will now be restored.")
				[ ] // Perform the database restore and verify it was restored successfully.
				[+] if (fDBRestore(sDWLogOnInfo, "IdwData", sDWBackupDir, sDBBackupFile))
					[ ] print ("The Data Warehouse database was restored successfully!")
				[+] else
					[ ] print ("***ERROR*** The Data Warehouse database was not restored.  You must manually restore this database!!!!!!!!")
			[+] else
				[ ] print ("Could not move the backup file from the temp directory, so could not restore.")
	[+] else
		[ ] print ("Can't find the backup file for the DataWarehouse database, could not restore.")
	[ ] 
	[ ] // Step 75 Reload saved MOD Billing extract files to ITV Command Center system.
	[ ] // 
	[ ] // Step 76 Reload saved IAD database back to the way it was before the test.
	[ ] // Delete the known test IAD database backup file from the SQL backup directory.
	[ ] // Perform a database restore on the saved IAD database backup file.
	[ ] // Verify that the backed up file exists in the MSSQL directory.
	[ ] sDBBackupFile = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "BackupFiles", "IADBackupFile", "{sDBBackupFile}")
	[+] if (SYS_FileExists (sDBBackupDir +sDBBackupFile))
		[ ] print ("Now we can restore the IAD database back to the way it was before the test.")
		[ ] // Restart mssqlserver service and dependent services on the Command Center before performing restore.
		[+] if (fManageService("MSSQLSERVER", "restart","zfred1"))
			[ ] print ("MSSQLSERVER was restarted, so we can go on to the restore.")
		[+] else
			[ ] print ("MSSQLSERVER was not restarted, so we can't go on to the restore.")
			[ ] print ("***ERROR*** The IAD database cannot be restored - failing test case!")
			[ ] //Have to stop IAD and IDS again since they got restarted with MSSQLSERVER.
		[+] if (fManageService("iad", "stop", "zfred1"))
			[ ] print ("IAD service was stopped, so we can to on to the restore.")
		[+] else
			[ ] print ("IAD service was not stopped, so we can't go on to the restore.")
			[ ] print ("***ERROR*** The IAD database cannot be restored - failing test case!")
		[+] if (fManageService("ids", "stop", "zfred1"))
			[ ] print ("IDS service was stopped, so we can to on to the restore.")
		[+] else
			[ ] print ("IDS service was not stopped, so we can't go on to the restore.")
			[ ] print ("***ERROR*** The IAD database cannot be restored - failing test case!")
		[+] if (fDBRestore(sCCLogOnInfo, "IadData", sDBBackupDir, sDBBackupFile))
			[ ] print ("The restore of IadData was successful.")
		[+] else
			[ ] Print ("Restore of IadData failed!")
	[+] else
		[ ] print ("Can't find the original backup file, can't restore.")
		[ ] 
	[ ] // Step 77 Reload saved IDS database.
	[ ] // Perform a database restore on the saved ICM database backup file.
	[ ] // Verify that the backed up file exists in the MSSQL directory.
	[ ] sDBBackupFile = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "BackupFiles", "IdsBackupFile", "{sDBBackupFile}")
	[+] if (SYS_FileExists (sDBBackupDir +sDBBackupFile))
		[ ] print ("Now we can restore the IDS database back to the way it was before the test.")
		[ ] // Restart mssqlserver service and dependent services on the Command Center before performing restore.
		[+] if (fManageService("MSSQLSERVER", "restart","zfred1"))
			[ ] print ("MSSQLSERVER was restarted, so we can go on to the restore.")
		[+] else
			[ ] print ("MSSQLSERVER was not restarted, so we can't go on to the restore.")
			[ ] print ("***ERROR*** The IDS database cannot be restored - failing test case!")
			[ ] //Have to stop IAD and IDS again since they got restarted with MSSQLSERVER.
		[+] if (fManageService("iad", "stop", "zfred1"))
			[ ] print ("IAD service was stopped, so we can to on to the restore.")
		[+] else
			[ ] print ("IAD service was not stopped, so we can't go on to the restore.")
			[ ] print ("***ERROR*** The IDS database cannot be restored - failing test case!")
		[+] if (fManageService("ids", "stop", "zfred1"))
			[ ] print ("IDS service was stopped, so we can to on to the restore.")
		[+] else
			[ ] print ("IDS service was not stopped, so we can't go on to the restore.")
			[ ] print ("***ERROR*** The IDS database cannot be restored - failing test case!")
		[+] if (fDBRestore(sCCLogOnInfo, "IdsData", sDBBackupDir, sDBBackupFile))
			[ ] print ("The restore of IdsData was successful.")
		[+] else
			[ ] Print ("Restore of IdsData failed!")
	[+] else
		[ ] print ("Can't find the original backup file, can't restore.")
	[ ] 
	[ ] // Step 78 Reload saved ICM database.
	[ ] // Perform a database restore on the saved ICM database backup file.
	[ ] // Verify that the backed up file exists in the MSSQL directory.
	[ ] sDBBackupFile = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "BackupFiles", "IcmBackupFile", "{sDBBackupFile}")
	[+] if (SYS_FileExists (sDBBackupDir +sDBBackupFile))
		[ ] print ("Now we can restore the ICM database back to the way it was before the test.")
		[ ] // Restart mssqlserver service and dependent services on the Command Center before performing restore.
		[+] if (fManageService("MSSQLSERVER", "restart","zfred1"))
			[ ] print ("MSSQLSERVER was restarted, so we can go on to the restore.")
		[+] else
			[ ] print ("MSSQLSERVER was not restarted, so we can't go on to the restore.")
			[ ] print ("***ERROR*** The ICM database cannot be restored - failing test case!")
			[ ] //Have to stop IAD and IDS again since they got restarted with MSSQLSERVER.
		[+] if (fManageService("iad", "stop", "zfred1"))
			[ ] print ("IAD service was stopped, so we can to on to the restore.")
		[+] else
			[ ] print ("IAD service was not stopped, so we can't go on to the restore.")
			[ ] print ("***ERROR*** The ICM database cannot be restored - failing test case!")
		[+] if (fManageService("ids", "stop", "zfred1"))
			[ ] print ("IDS service was stopped, so we can to on to the restore.")
		[+] else
			[ ] print ("IDS service was not stopped, so we can't go on to the restore.")
			[ ] print ("***ERROR*** The ICM database cannot be restored - failing test case!")
		[+] if (fDBRestore(sCCLogOnInfo, "IcmData", sDBBackupDir, sDBBackupFile))
			[ ] print ("The restore of IcmData was successful.")
		[+] else
			[ ] Print ("Restore of IcmData failed!")
	[+] else
		[ ] print ("Can't find the original backup file, can't restore.")
		[ ] 
	[ ] // Step 79 Reload saved Subscriber Data database.
	[ ] // Perform a database restore on the saved Subscriber database backup file.
	[ ] // Verify that the backed up file exists in the MSSQL directory.
	[ ] sDBBackupFile = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "BackupFiles", "SubscriberDataBackupFile", "{sDBBackupFile}")
	[+] if (SYS_FileExists (sDBBackupDir +sDBBackupFile))
		[ ] print ("Now we can restore the Subscriber database back to the way it was before the test.")
		[ ] // Restart mssqlserver service and dependent services on the Command Center before performing restore.
		[+] if (fManageService("MSSQLSERVER", "restart","zfred1"))
			[ ] print ("MSSQLSERVER was restarted, so we can go on to the restore.")
		[+] else
			[ ] print ("MSSQLSERVER was not restarted, so we can't go on to the restore.")
			[ ] print ("***ERROR*** The Subscriber database cannot be restored - failing test case!")
			[ ] //Have to stop IAD and IDS again since they got restarted with MSSQLSERVER.
		[+] if (fManageService("iad", "stop", "zfred1"))
			[ ] print ("IAD service was stopped, so we can to on to the restore.")
		[+] else
			[ ] print ("IAD service was not stopped, so we can't go on to the restore.")
			[ ] print ("***ERROR*** The Subscriber database cannot be restored - failing test case!")
		[+] if (fManageService("ids", "stop", "zfred1"))
			[ ] print ("IDS service was stopped, so we can to on to the restore.")
		[+] else
			[ ] print ("IDS service was not stopped, so we can't go on to the restore.")
			[ ] print ("***ERROR*** The Subscriber database cannot be restored - failing test case!")
		[+] if (fDBRestore(sCCLogOnInfo, "SubscriberData", sDBBackupDir, sDBBackupFile))
			[ ] print ("The restore of SubscriberData was successful.")
		[+] else
			[ ] Print ("Restore of SubscriberData failed!")
	[+] else
		[ ] print ("Can't find the original backup file, can't restore.")
	[ ] 
	[ ] //Steps 80 - 81 Restart the ITV Services on the Command Center nodes.
	[ ] //Start on Command Center 1.
	[+] if (fControlServices ("ZFRED1", "start", lsITVServices1))
		[ ] print ("The services started successfully.")
	[+] else
		[ ] print ("The services did not start, must manually start and check CC node.")
	[ ] //Start on Command Center 2.
	[+] if (fControlServices ("ZFRED2", "start", lsITVServices2))
		[ ] print ("The services started successfully.")
	[+] else
		[ ] print ("The services did not start, must manually start and check CC node.")
	[ ] 
[ ] // //Test the database query function.
[+] // testcase tcTestCase0002()
		[ ] // STRING sDBLogOnInfo = "DSN=ZFRED1;SRVR=ZFRED1;UID=sa;PWD="
		[ ] // STRING sDBName = "IdsData"
		[ ] // STRING sSQLQuery = "select * from Asset"
		[ ] // STRING sResult
		[ ] // LIST OF ANYTYPE lDbReturn
		[ ] // //Call function fDBQuery and pass desired database logon info to it, as well
		[ ] // //as the SQL database to connect to, and the query.
		[ ] // lDbReturn = fDBQuery(sDBLogOnInfo, sSQLQuery, sDBName)
		[ ] // print (lDbReturn)
		[ ] // //Write a PASS test line to the Pass/Fail log.
		[ ] // fLogPassFail ("Passed","expect","receive")
		[ ] // 
		[ ] // //Add a sleep to demo time increments in log file.
		[ ] // sleep (1)
		[ ] // 
[ ] //Test the database run job function.
[+] // testcase tcTestCase0003()
		[ ] // STRING sDBLogOnInfo = "DSN=ZFRED_DW;"
		[ ] // STRING sDBName = "msdb"
		[ ] // STRING sDBImportJob = "idw_ICMImportJob"
		[ ] // STRING sJobStep = "idw_ProcessICMData"
		[ ] // //Call function fDBRunJob
		[+] // if (fDBRunJob(sDBLogOnInfo, sDBName, sDBImportJob, sJobStep))
			[ ] // print ("The job ran successfully.")
		[+] // else
			[ ] // print ("The job did not run successfully.")
		[ ] //  
		[ ] //  
[ ] // //Test the database backup function.
[+] // testcase tcTestCase0004()
		[ ] // STRING sDBLogOnInfo = "DSN=ZFRED1;SRVR=ZFRED1;UID=sa;PWD="
		[ ] // STRING sDBName = "IcmData"
		[ ] // STRING sDBBackupDir = "\\ZFRED1\D$\SQL2000\mssql\backup\"
		[ ] // STRING sDBBackupFile =""
		[ ] // // //Call function fDBBackup
		[+] // if (fDBBackup(sDBLogOnInfo, sDBName, sDBBackupDir, sDBBackupFile))
			[ ] // print ("DB was backed up.")
			[ ] // print (sDBBackupFile)
		[+] // else
			[ ] // print ("DB was not backed up.") 
		[ ] // // //Write a PASS test line to the Pass/Fail log.
		[ ] // fLogPassFail ("Passed","expect","receive")
		[ ] // // 
		[ ] // // //Add a sleep to demo time increments in log file.
		[ ] // sleep (1)
		[ ] // 
		[ ] // 
[ ] // //Test the database connect function.
[+] // testcase tcTestCase0005() 
		[ ] // STRING sDBLogOnInfo = "DSN=ZFRED1;SRVR=ZFRED1;UID=sa;PWD="
		[ ] // 
		[ ] // //Call function fDBConnect and pass desired database logon info to it.
		[ ] // fDBConnect(sDBLogOnInfo)
		[ ] // 
		[ ] // //Write a PASS test line to the Pass/Fail log.
		[ ] // fLogPassFail ("Passed","expect","receive")
		[ ] // 
		[ ] // //Add a sleep to demo time increments in log file.
		[ ] // sleep (1)
		[ ] // 
[ ] //Test the fRunReport function.
[+] // testcase tcTestCase0007 ()
	[ ] // fRunReport ("AccountingBuyRateReport")
	[ ] // 
[ ] // //Test the fVerifyExportReportContents function.
[+] // testcase tcTestCase0008 ()
	[ ] // fVerifyExportReportContents ("AccountingBuyRateReport", "Accounting Buy Rate Report", "12/1/02", "12/31/02", "34")
[ ] // //Test the fExportReport function.
[+] // testcase tcTestCase0009 ()
	[ ] // STRING sExportFile =""
	[+] // if (fExportReport ("SubscriptionAssetViewsReport", sExportFile))
		[ ] // print (sExportFile)
	[+] // else
		[ ] // print ("The file was not exported.")
	[ ] // 
[ ] // //Test the fDBGetDateTime function.
[+] // testcase tcTestCase0010 ()
	[ ] // DATETIME dtDateTime
	[ ] // STRING sDWLogOnInfo
	[ ] // sDWLogOnInfo = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "DatabaseLogOnInfo", "DWHLogOn", "")
	[ ] // dtDateTime = fDBGetDateTime (sDWLogOnInfo)
	[ ] // print (dtDateTime)
	[ ] // 
[ ] // //Test the database restore function.
[+] // testcase tcTestCase0011 ()
		[ ] // STRING sDBLogOnInfo = "DSN=ZFRED_DW"
		[ ] // STRING sDBName = "IdwData"
		[ ] // STRING sDBBackupDir = "\\KARYN\D$\Temp\DBTestFiles\"
		[ ] // STRING sDBBackupFile ="IdwBackup.BAK"
		[ ] // 
		[+] // if (fDBRestore(sDBLogOnInfo, sDBName, sDBBackupDir, sDBBackupFile))
			[ ] // print ("DW DB restore was successful!")
		[+] // else 
			[ ] // print ("DW DB restore was not succesful, manually restore this database!!!!!!!!")
		[ ] // 
[ ] // //See what the job history query returns.
[+] // testcase tcTestCase0012 ()
	[ ] // HSQL hstmnt 
	[ ] // HDATABASE hdbc
	[ ] // STRING sDBLogOnInfo = "DSN=ZFRED1;SRVR=ZFRED1;UID=sa;PWD="
	[ ] // STRING sDBName = "msdb"
	[ ] // LIST OF STRING lsHistoryData
	[ ] // INTEGER iCount
	[ ] // INTEGER iLength
	[ ] // STRING sJobStep
	[ ] // STRING sDateRun
	[ ] // STRING sTimeRun
	[ ] // STRING sRunStatus
	[ ] // STRING sStepName = "Step 1"
	[ ] // STRING sVerifyTimeRun
	[ ] // DATETIME dtCurrDateTime
	[ ] // STRING sFormat = "yyyymmdd"   // date format to compare strings from jobhistory query to
	[ ] // DATETIME dtVerifyDateRun = "2003-01-01" // set to dummy value in case parsing
    //                           // fails and variable is not set by function
	[ ] // 
	[ ] // dtCurrDateTime = "2003-01-29 12:37:38"
	[ ] // TIME tCurrTime = [TIME] dtCurrDateTime
	[ ] // //Execute query to see the job history.
	[ ] // //Connect to the specified database.
	[ ] // hdbc = fDBConnect (sDBLogOnInfo)
	[ ] // //Run query to get job history information.
	[ ] // hstmnt = DB_ExecuteSql(hdbc,"use msdb")
	[ ] // hstmnt = DB_ExecuteSqL (hdbc,"SELECT step_name, run_date, run_time, run_status FROM sysjobhistory WHERE step_name = '{sStepName}'")
	[ ] // //Save the data returned from the job history into variables for later comparison.
	[+] // while (DB_FetchNext(hstmnt, sJobStep, sDateRun, sTimeRun, sRunStatus)== TRUE)
		[ ] // ListAppend (lsHistoryData, sJobStep)
		[ ] // ListAppend (lsHistoryData, sDateRun)
		[ ] // ListAppend (lsHistoryData, sTimeRun)
		[ ] // ListAppend (lsHistoryData, sRunStatus)
		[ ] // //Parse date run string into a valid DATETIME format.
		[ ] // ParseDateFormat (sDateRun,sFormat,dtVerifyDateRun)
		[ ] // //Get the length of the string for the time run.
		[ ] // iLength = Len(sTimeRun)
		[ ] // //Rewrite the date run string into ISO format so that it can then be cast to a TIME datatype.
		[+] // if iLength == 5
			[ ] // sVerifyTimeRun = Stuff(sTimeRun, 2, 0, ":")
			[ ] // sVerifyTimeRun = Stuff(sVerifyTimeRun, 5, 0, ":")
		[+] // if iLength == 6
			[ ] // sVerifyTimeRun = Stuff(sTimeRun, 3, 0, ":")
			[ ] // sVerifyTimeRun = Stuff(sVerifyTimeRun, 6, 0, ":")
	[ ] // //Cast the new time strings into TIME datatype for later comparison.
	[ ] // TIME tVerifyTimeRun = [TIME] sVerifyTimeRun
	[ ] // //Disconnect from the database.
	[ ] // DB_FinishSQL (hstmnt)
	[ ] // DB_Disconnect (hdbc)
	[ ] // print ("***sVerifyTimeRun is {sVerifyTimeRun}")
	[ ] // print ("***tVerifyTimeRun is {tVerifyTimeRun}")
	[ ] // print ("LIST = {lsHistoryData}")
	[ ] // //Compare variables from last DB_FetchNext to expected values to determine if the job ran successfully.
	[+] // for iCount = 1 to ListCount (lsHistoryData)
		[+] // if ((sJobStep == sStepName) && (sRunStatus == "1"))
			[ ] // print ("The job step was found in the jobhistory.")
		[+] // else 
			[ ] // print ("The job was not run.")
			[ ] // continue
			[ ] // // return FALSE
		[+] // if (tVerifyTimeRun > tCurrTime)
			[ ] // DATETIME dtCurrDate = [DATETIME] tCurrTime
			[ ] // DATE dCurrDate = [DATE] dtCurrDate
		[+] // else
			[ ] // dtCurrDate = NULL
			[ ] // print ("The time run is not current.")
			[ ] // continue
			[ ] // // return FALSE
		[+] // if (dtVerifyDateRun == dCurrDate)
			[ ] // print ("The job was found in the history and date and time run is current.")
			[ ] // // return TRUE
		[+] // else
			[ ] // print ("The job was found in the history, but the run date is not current.")
			[ ] // continue
			[ ] // // return FALSE
	[ ] // // 
[ ] // //Test the database run job function.
[+] // testcase tcTestCase0013 ()
		[ ] // STRING sDBLogOnInfo = "DSN=ZFRED_DW"
		[ ] // STRING sDBName = "msdb"
		[ ] // STRING sStepName = "idw_IADImportJobStep"
		[ ] // STRING sDBJobName = "idw_IADImportJob"
		[ ] // LIST OF STRING lsRunStatus
		[ ] // 
		[+] // if (fDBRunJob(sDBLogOnInfo, sDBName, sDBJobName, sStepName))
			[ ] // print ("IAD ImportJob ran successfully.")
		[+] // else
			[ ] // print ("Didn't work.")
		[ ] // 
[ ] // //Test restarting a service.
[+] // testcase tcTestCase0014 ()
	[ ] // STRING sStatus
	[ ] // 
	[+] // if (fManageService("MSSQLSERVER", "restart","zfred_dw", sStatus))
		[ ] // print ("The service was restarted.")
		[ ] // print (sStatus)
	[+] // else
		[ ] // print ("The service was not restarted.")
		[ ] // print (sStatus)
[ ] // //Test that a restore was successful.
[+] // testcase tcTestCase0015 ()
	[ ] // STRING sDBLogOnInfo = 
	[ ] // STRING sDBName = "IadData"
	[ ] // HSQL hstmnt 
	[ ] // HDATABASE hdbc
	[ ] // STRING sRestoreDate
	[ ] // ANYTYPE aRestoreDate
	[ ] // LIST OF STRING lsRestoreDate
	[ ] // INTEGER iCount
	[ ] // DATETIME dtCurrDateTime
	[ ] // STRING sRestoreID
	[ ] // STRING sRestoreName
	[ ] // 
	[ ] // dtCurrDateTime = "2003-01-30 12:37:38"
	[ ] // print ("The current time is {dtCurrDateTime}")
	[ ] // //Execute query to see the restore history.
	[ ] // hdbc = fDBConnect (sDBLogOnInfo)
	[ ] // hstmnt = DB_ExecuteSql(hdbc,"use msdb")
	[ ] // hstmnt = DB_ExecuteSql(hdbc,"select restore_date from restorehistory WHERE destination_database_name = '{sDBName}'")
	[ ] // // hstmnt = DB_ExecuteSql(hdbc,"select * from restorehistory")
	[ ] // //Save the data returned from the restore history into variables.
	[ ] // // while (DB_FetchNext(hstmnt, sRestoreID, sRestoreDate, sRestoreName)== TRUE)
	[+] // while (DB_FetchNext(hstmnt, sRestoreDate)== TRUE)
		[ ] // ListAppend (lsRestoreDate, sRestoreDate)
	[ ] // DB_FinishSQL (hstmnt)
	[ ] // DB_Disconnect (hdbc)
	[ ] // print (lsRestoreDate)
	[+] // for iCount = 1 to ListCount (lsRestoreDate)
		[ ] // //Convert time of restore to DATETIME format.
		[ ] // DATETIME dtRestoreDate = lsRestoreDate[iCount]
		[ ] // //Compare variables from last DB_FetchNext to expected values to determine if the restore was successful.
		[+] // if (dtRestoreDate > dtCurrDateTime)
			[ ] // print ("The database was restored.")
		[+] // else
			[ ] // continue
	[ ] // print ("The database was not restored.")
	[ ] // 
[ ] // //Test the DW ImportJobs function.
[+] // testcase tcTestCase0016 ()
	[ ] // STRING sDWLogOnInfo = "DSN=ZFRED_DW"
	[+] // if (fRunDWJobs (sDWLogOnInfo))
		[ ] // print ("All of the import jobs ran successfully, so Reports can now be run.")
	[+] // else
		[ ] // print ("***ERROR**** Not all of the import jobs ran successfully, failing test case.")
		[ ] // 
