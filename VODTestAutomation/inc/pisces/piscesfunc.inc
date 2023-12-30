[ ] //*****************************************************************************
[ ] //  NAME:					  PISCESFUNC.INC
[ ] //
[ ] //  DESCRIPTION:		Function library for Pisces component of Zodiac project.
[ ] //
[ ] //  HISTORY:
[ ] //
[ ] //  Date						Developer					Description
[ ] //  ********				***********				*****************************************
[ ] //  12/11/02        K. Sullivan       Initial design and development.
[ ] //	01/07/03				K. Sullivan				Created fVerifyReportContents function.
[ ] //	01/08/03				K. Sullivan				Created fRunReport function.
[ ] //	01/31/03				K. Sullivan				Created more functions.
[ ] //
[ ] //*****************************************************************************************************
[ ] 
[ ] use "piscesdefs.inc"
[ ] 
[ ] //*****************************************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fControlServices()
[ ] //
[ ] //   ARGUMENTS	  :	-string sServerName is the machine name where you want to control the services.
[ ] //									-string sCommand is the command that you wish to perform on the service, depends
[ ] //									on the acceptable commands from fManageService.  Right now they are start, stop
[ ] //									and status.
[ ] //									-list of string lsITVServices is the list of the ITV Services that you want to 
[ ] //									control.  Right now for Pisces, this list is defined in the piscesdefs.inc file.
[ ] //  
[ ] //   RETURNS     : 	boolean -
[ ] //									TRUE = if all of the services performed the requested operation successfully.
[ ] //									FALSE = if any of the services did not peform the requested operation successfully.
[ ] //
[ ] //   DESCRIPTION	:	This function uses the fManageService function from the zodiacnt.inc file.  I wrote 
[ ] //									this function so that I would only have to perform a command on a long list of services
[ ] //									by calling a function once rather than calling fManageService for each service.
[ ] //
[ ] //*********************************************************************************************************
[ ] 
[+] boolean fControlServices (string sServerName, string sCommand, list of string lsITVServices)
	[+] switch (lsITVServices)
			[+] case lsITVServices1
				[+] for each sITVService1 in lsITVServices1
					[+] if (fManageService(sITVService1, sCommand, sServerName))
						[ ] print ("The service {sITVService1} {sCommand}ed successfully.")
					[+] else
						[ ] print ("The service {sITVService1} did not {sCommand} successfully.")
						[ ] return FALSE
			[+] case lsITVServices2
				[+] for each sITVService2 in lsITVServices2
					[+] if (fManageService(sITVService2, sCommand, sServerName))
						[ ] print ("The service {sITVService2} {sCommand}ed successfully.")
					[+] else
						[ ] print ("The service {sITVService2} did not {sCommand} successfully.")
						[ ] return FALSE
	[ ] return TRUE
[ ] 
[ ] //*****************************************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fBackupDatabase()
[ ] //
[ ] //   ARGUMENTS	  :	-string sLogOnInfo is a string consisting of the DSN Name, the Database 
[ ] //									Server name, the User Id and the Password of the database under test.
[ ] //									An example of the string: "DSN=ZFRED1;SRVR=ZFRED1;UID=sa;PWD=".
[ ] //									-string sDBName is the database name you want to backup,
[ ] //									such as IdsData, IcmData, etc.
[ ] //									-string sBackupDir is the directory where SQL Server puts the backup file.
[ ] //									-out string sTestDBBackupFile is an optional parameter that, when specified, enables
[ ] //									the function to return the name of the backup file with the .test extension.
[ ] //	
[ ] //   RETURNS     : 	boolean -
[ ] //									TRUE = if the backup was peformed successfully and the backup file was renamed
[ ] //									to have a .test extension.
[ ] //									FALSE = if the backup was not peformed successfully or the backup file was not
[ ] //									renamed to have a .test extension.
[ ] //
[ ] //   DESCRIPTION	:	This function saves the current database and renames it to have a .test extension. 
[ ] 
[ ] //*********************************************************************************************************
[ ] 
[+] boolean fBackupDatabase (string sLogOnInfo, string sDBName, string sBackupDir, out string sTestDBBackupFile)
	[ ] STRING sDBBackupFile
	[ ] //Perform the database backup by calling the fDBBackup function.
	[ ] //Verify the backup file returned from fDBBackup exists.
	[ ] //Rename the backup file to have a .test extension.
	[+] if (fDBBackup(sLogOnInfo, sDBName, sBackupDir, sDBBackupFile))
		[ ] print("The backup file name is {sDBBackupFile}, renaming it to have a .test extension.")
		[ ] SYS_MoveFile (sBackupDir +sDBBackupFile, sBackupDir +sDBBackupFile +".test")
		[ ] sTestDBBackupFile = sDBBackupFile +".test"
		[ ] sleep (3)
		[ ] return TRUE
	[+] else
		[ ] Print ("Backup of {sDBName} failed!")
		[ ] return FALSE
		[ ] 
[ ] 
[ ] //*****************************************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fRunReport()
[ ] //
[ ] //   ARGUMENTS	  :	-string sReport is the name of the report you want to run.  The 
[ ] //									list of available names for the report are in the case statements.
[ ] //
[ ] //  
[ ] //   RETURNS     : 	boolean -
[ ] //									TRUE = if the correct report was displayed.
[ ] //									FALSE = if the requested report was not displayed.
[ ] //
[ ] //   DESCRIPTION	:	This function takes the requested report that one wants to run. Based on
[ ] //									the name of the report, the Report Parameters page is displayed, the desired
[ ] //									parameter data is entered, the report is run and verification is done to verify
[ ] //									that the correct report is displayed on the screen.  No data is verified on this
[ ] //									displayed report - only the title to check that the report was run.  Verification of 
[ ] //									data is done with fVerifyExportReportContents, as the displayed report window's data
[ ] //									is recoginzed by Silk Test as custom windows and verification of data is difficult.
[ ] //					
[ ] //	 NOTES				: Case statements need to be added for the other reports - only two are defined right now.
[ ] //									Also, the only parameters that can be passed into the report parameters page is currently
[ ] //									a date range.  More development is required to enhance this function.
[ ] //
[ ] //*********************************************************************************************************
[ ] 
[+] boolean fRunReport (string sReport)
	[ ] BROWSERLOCATION blLocation = "http://192.168.21.237/itvreport"
	[ ] STRING sUser = "administrator"
	[ ] STRING sPassword = "deadsea"
	[ ] //Declare a ReportData variable for ReportData record.
	[ ] ReportData recReportData
	[ ] //Use the fReadWriteIni function to get the report dates under test.
	[ ] // recReportData.sDateFrom = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "ReportDates", "ReportDateFrom", "")
	[ ] // recReportData.sDateTo = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "ReportDates", "ReportDateTo", "")
	[ ] //Based on the name of the report passed in, set the WINDOW types.
	[+] switch (sReport)
			[+] case "AccountingBuyRateReport"
				[ ] recReportData.sDateFrom = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "AcctBuyRateReport", "ReportDateFrom", "")
				[ ] recReportData.sDateTo = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "AcctBuyRateReport", "ReportDateTo", "")
				[ ] recReportData.wReport = ITVReports.AccountingBuyRateReport
				[ ] recReportData.wParam = AcctBuyRateRptParam
				[ ] recReportData.wDisplay = AcctBuyRateRptDisplay.ExportReport
				[ ] recReportData.wExport = ExportReport.ExportTable.Link.AcctBuyRateRptTXT2
			[+] case "SubscriptionAssetViewsReport"
				[ ] recReportData.wReport = ITVReports.SubscriptionAssetViewsReport
				[ ] recReportData.wParam = SubsAssetViewsRptParam
				[ ] recReportData.wDisplay = SubsAssetViewsRptDisplay.ExportReport
				[ ] recReportData.wExport = ExportReport.ExportTable.Link.SubsAssetViewsRptTXT2
	[ ] //Navigate to the ITV Reports main page.
	[ ] Browser.LoadPage(blLocation, sUser, sPassword)
	[ ] //Select the desired report to run from the ITV Reports main page.
	[ ] ITVReports.SetActive( )
	[ ] ITVReports.ExpandAll.Click ()
	[ ] recReportData.wReport.Click ()
	[ ] //Enter the desired parameters for the report.  These will come from the ini file.
	[ ] recReportData.wParam.DateRangeFromField.SetText(recReportData.sDateFrom)
	[ ] recReportData.wParam.DateRangeToField.SetText(recReportData.sDateTo)
	[ ] recReportData.wParam.RunReport.Click ()
	[ ] //Verify the correct report ran and is displayed.
	[+] if (recReportData.wDisplay.Exists())
		[ ] print ("The correct report was displayed.")
		[ ] return TRUE
	[+] else if (NotEnoughStorage.NotEnoughStorageIsAvail.Exists())
		[ ] print ("An error that not enough storage is availabe returned from SQL.")
		[ ] return FALSE
	[+] else
		[ ] print ("The report requested was not displayed.")
		[ ] return FALSE
[ ] 
[ ] //*****************************************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fExportReport()
[ ] //
[ ] //   ARGUMENTS	  :	-string sReport is the name of the report you want to export.  The 
[ ] //									list of available names for the report are in the case statements in the 
[ ] //									fRunReport function from above.
[ ] //									-string sExportFile is the filename of the Export File.
[ ] //
[ ] //  
[ ] //   RETURNS     : 	boolean -
[ ] //									TRUE = if the file exported successfully.
[ ] //									FALSE = if the file did not export successfully.
[ ] //
[ ] //   DESCRIPTION	:	This function takes the requested report that one wants to export, calls the
[ ] //									fRunReport function to run the report and then exports the report into a tab
[ ] //									Tab-Separated Text format.  This format was chosen as it is fairly easy and 
[ ] //									reliable to verify the data using Silk Test.  The fVerifyReportContents
[ ] //									function below will verify the data in this type of file against data that one
[ ] //									gives it. 
[ ] //					
[ ] //	 NOTES				: This function can later be expanded to include exporting in other file formats 
[ ] //									if the need to do so arises.
[ ] //
[ ] //*********************************************************************************************************
[ ] 
[+] boolean fExportReport(string sReport, out string sExportFile)
	[ ] //Declare variables.
	[ ] STRING sExportLocation
	[ ] boolean bRunReturn
	[ ] FILEINFO_LIST lfFiles
	[ ] DATETIME dtCurrDateTime
	[ ] INTEGER iCount
	[ ] // Declare a ReportData variable for ReportData record.
	[ ] ReportData recReportData
	[ ] //Read ini file to get location that will contain the export files under test.
	[ ] sExportLocation = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "FileDirectories", "ExportLocation", "")
	[ ] //Call function to get current time to be used in export file check.
	[ ] dtCurrDateTime = fCreateDateTimeStamp (3)
	[ ] //Call fRunReport to run the desired report to be exported.
	[ ] bRunReturn = (fRunReport (sReport))
	[+] if (bRunReturn)
		[ ] print ("The Run Report function displayed the correct report. The report will now be exported.")
	[+] else
		[ ] print ("The Run Report function did not display the correct report.  The report cannot be exported.")
		[ ] return FALSE
	[ ] //Based on the name of the report passed in, set the WINDOW types.
	[+] switch (sReport)
			[+] case "AccountingBuyRateReport"
				[ ] recReportData.wReport = ITVReports.AccountingBuyRateReport
				[ ] recReportData.wParam = AcctBuyRateRptParam
				[ ] recReportData.wDisplay = AcctBuyRateRptDisplay.ExportReport
				[ ] recReportData.wExport = ExportReport.ExportTable.Link.AcctBuyRateRptTXT2
			[+] case "SubscriptionAssetViewsReport"
				[ ] recReportData.wReport = ITVReports.SubscriptionAssetViewsReport
				[ ] recReportData.wParam = SubsAssetViewsRptParam
				[ ] recReportData.wDisplay = SubsAssetViewsRptDisplay.ExportReport
				[ ] recReportData.wExport = ExportReport.ExportTable.Link.SubsAssetViewsRptTXT2
	[ ] //Export the applicable report.
	[ ] recReportData.wDisplay.Click()
	[ ] recReportData.wExport.Click()
	[ ] sleep (2)
	[ ] //Save the export file to a tab separated text file.  
	[ ] Browser.MenuSelect (20, 14, "Save As")
	[ ] SaveWebPage.SetActive ()
	[+] if (SaveWebPage.ThisWebPageMayNotSaveText.bExists)
		[ ] SaveWebPage.Yes.Click()
		[ ] // SaveWebPage.NeverAskMeAgain.Check()
	[ ] SaveWebPage.SaveInPopup.Select (" (D:)")
	[+] if (DriveNotAccessible.bExists)
		[ ] DriveNotAccessible.Cancel.Click ()
	[ ] SaveWebPage.SaveInPopup.Select (" (D:)")
	[ ] SaveWebPage.SaveInList.DoubleSelect ("Pisces")
	[ ] SaveWebPage.SaveInList.DoubleSelect ("ExportFiles")
	[ ] SaveWebPage.FileName.SetText ("{sReport}.txt")
	[ ] SaveWebPage.Save.Click ()
	[+] if (FileAlreadyExists.bExists)
			[ ] FileAlreadyExists.Yes.Click()
			[ ] Browser.SetActive ()
			[ ] Browser.Close ()
	[ ] sleep (10)
	[ ] //Check that the report was exported.
	[ ] lfFiles = SYS_GetDirContents (sExportLocation)
	[ ] //Check each file in the temp directory.
	[ ] //If the timestamp on the file is greater than the current time, then assume the 
	[ ] //export was successful and return the export file name and timestamp.
	[+] for iCount = 1 to ListCount (lfFiles)
		[+] if (MatchStr ("*{sReport}*", lfFiles[iCount].sName))
			[+] if (lfFiles[iCount].dtLastModifyTime > dtCurrDateTime)
				[ ] print ("The report was exported succesfully. The name of the file is {lfFiles[iCount].sName}, and the timestamp on the file is {lfFiles[iCount].dtLastModifyTime}")
				[ ] sExportFile = lfFiles[iCount].sName
				[ ] return TRUE
			[+] // else
				[ ] // continue
		[+] else
			[ ] continue
			[ ] print ("The export file was not found.")
	[ ] sExportFile = NULL
	[ ] return FALSE
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fVerifyExportReportContents()
[ ] //
[ ] //   ARGUMENTS	  :	string sFilePathAndName, string sTitle, string sDateFrom, 
[ ] //									string sDateTo, string sTotal
[ ] //
[ ] //									-string sFilePathAndName is a string consisting of the machine name and 
[ ] //									file path and name where the exported report was saved.  For example,
[ ] //									"\\KARYN\C$\TEMP\AcctBuyRate_TXT.txt".
[ ] //									-string sTitle is the name of the report you want to verify the contents of.
[ ] //									For example, "Accounting Buy Rate Report"
[ ] //	               	-string sDateFrom is the From Date that you entered on the report parameters
[ ] //									web page of the Report Generator when you requested the report.
[ ] //									For example, "12/1/02".
[ ] //									-string sDateTo is the To Date that you entered on the report parameters
[ ] //									web page of the Report Generator when you requested the report.
[ ] //									-string sTotal is the Grand Total that you are expecting on the report.  Note
[ ] //									that as of now, this will only pull data from the first tab that it sees in the 
[ ] //									row of the report for Grand Total.  If more data is requested from that line,
[ ] //									then enhancements need to be made to this function.
[ ] //  
[ ] //   RETURNS     : 	boolean -
[ ] //									TRUE = if the report displays the expected data.
[ ] //									FALSE = if the report does not display the expected data.
[ ] //
[ ] //   DESCRIPTION	:	This function assumes a Tab-Separated Text formatted file for the report exists somewhere.  
[ ] //									This type of file is an option on the Export Report web page of the Report Generator.  
[ ] //									The report generated by the Report Generator must be exported to this type of format, and then
[ ] //									saved to a file path with a .txt file extension.  The function takes the arguments
[ ] //									and verifies that the data is displayed on the report.  This function will return TRUE
[ ] //									if the values on the report were what were expected.  
[ ] //
[ ] //*****************************************************************************
[ ] 
[+] boolean fVerifyExportReportContents (string sReport, string sTitle, string sDateFrom, string sDateTo, string sTotal)
	[ ] 
	[ ] //Work to be done - if file isn't found, fail function, if data is wrong, fail function.
	[ ] 
	[ ] //Verify the report title.
	[ ] //Call the fFindInFile function to look for the line with the expected title, then verify the 
	[ ] //correct title is displayed on the report.
	[ ] STRING sFoundTitle = ""
	[ ] STRING sExportLocation
	[ ] //Read ini file to get location that will contain the export files under test.
	[ ] sExportLocation = fReadWriteIni (SYS_GetDrive()+":"+filPiscesIni, csRead, "FileDirectories", "ExportLocation", "")
	[+] if (fFindInFile (sExportLocation +sReport + ".txt", sTitle, sFoundTitle))
		[ ] print ("The {sTitle} was displayed as expected.")
	[+] else
		[ ] print ("The {sTitle} was NOT displayed, the title on the report read {sFoundTitle}")
		[ ] return FALSE
	[ ] 
	[ ] //Next verify the dates on the report.
	[ ] //Call the fFindInFile function to look for the line with the dates, and verify the dates
	[ ] //are the expected dates for the report.
	[ ] STRING sFoundPeriod = ""
	[ ] STRING sMatchStr = "For the period"
	[ ] STRING sDate1
	[ ] STRING sDate2
	[ ] fFindInFile (sExportLocation +sReport + ".txt", sMatchStr, sFoundPeriod)
	[ ] //Work needs to be done here.  If the day or month has two digits in Date1, then this won't work.
	[ ] sDate1 = SubStr (sFoundPeriod, 17, 6 )
	[ ] sDate2 = SubStr (sFoundPeriod, 27, 8)
	[+] if ((sDate1 == sDateFrom) && (sDate2 == sDateTo))
		[ ] print ("The correct period for the report was displayed, {sDateFrom} to {sDateTo}.")
	[+] else
		[ ] print ("The wrong period for the report was displayed.  The dates displayed were {sDate1} to {sDate2}")
		[ ] return FALSE
	[ ] 
	[ ] //Finally, verify the Grand Total on the report.
	[ ] //Call the fFindInFile function to look for the line with the grand total, then verify the 
	[ ] //expected grand total is displayed on the report.
	[ ] STRING sFoundTotal = ""
	[ ] STRING sMatchTotal = "Grand"
	[ ] STRING sTotalFound
	[ ] fFindInFile (sExportLocation +sReport + ".txt", sMatchTotal, sFoundTotal)
	[ ] sTotalFound = (GetField (sFoundTotal, '	', 2) ) // print the second tab of the Grand Total line to get the number of buys.
	[+] if (sTotalFound == sTotal)
		[ ] print ("The total number of buys on the report was what was expected {sTotal}")
	[+] else 
		[ ] print ("The total number of buys on the report was not what was expected.  The value displayed on the report was {sTotalFound}.")
		[ ] return FALSE
	[ ] return TRUE
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fInitDW()
[ ] //
[ ] //	 ARGUMENTS			-sDBLogOnInfo is string consisting of the DSN Name, the Database 
[ ] //									Server name, the User Id and the Password of the Data Warehouse database under test.
[ ] //									An example of the string: "DSN=ZFRED_DW"
[ ] //  
[ ] //   RETURNS     : 	boolean -
[ ] //									TRUE = if the function ran.
[ ] //									FALSE = if the function did not run.
[ ] //
[ ] //   DESCRIPTION	:	This function runs a series of sql scripts to initialize a Data Warehouse database.
[ ] //									The sql files must reside in the directory specified in string sDWSQLDir.
[ ] //								  The files SetLinkedServer.sql and TestSettings.sql must be edited manually with 
[ ] //									the correct server names for the system under test.
[ ] //
[ ] //*****************************************************************************
[ ] 
[+] boolean fInitDW(string sDWLogOnInfo, string sDWSQLDir)
	[ ] LIST OF ANYTYPE lDbReturn
	[ ] 
	[ ] print ("Running IdwConfigTablesFunctions.sql")
	[ ] lDbReturn = fDBQuery(sDWLogOnInfo, "master..xp_cmdshell 'isql -E -n -i {sDWSQLDir}IdwConfigTablesFunctions.sql'")
	[ ] print (lDbReturn)
	[ ] 
	[ ] print ("Running IdwSettings.sql")
	[ ] lDbReturn = fDBQuery(sDWLogOnInfo, "master..xp_cmdshell 'isql -E -n -i {sDWSQLDir}IdwSettings.sql'")
	[ ] print (lDbReturn)
	[ ] 
	[ ] print ("IdwReportProcs.sql")
	[ ] lDbReturn = fDBQuery(sDWLogOnInfo, "master..xp_cmdshell 'isql -E -n -i {sDWSQLDir}IdwReportProcs.sql'")
	[ ] print (lDbReturn)
	[ ] 
	[ ] print ("Running IdwConvergysReports.sql")
	[ ] lDbReturn = fDBQuery(sDWLogOnInfo, "master..xp_cmdshell 'isql -E -n -i {sDWSQLDir}IdwConvergysReports.sql'")
	[ ] print (lDbReturn)
	[ ] 
	[ ] print ("Running IdwConfigBackup.sql")
	[ ] lDbReturn = fDBQuery(sDWLogOnInfo, "master..xp_cmdshell 'isql -E -n -i {sDWSQLDir}IdwConfigBackup.sql'")
	[ ] print (lDbReturn)
	[ ] 
	[ ] print ("Running SetLinkedServer.sql")
	[ ] lDbReturn = fDBQuery(sDWLogOnInfo, "master..xp_cmdshell 'isql -E -n -i {sDWSQLDir}SetLinkedServer.sql'")
	[ ] print (lDbReturn)
	[ ] 
	[ ] print ("Running IdwConfigIds.sql")
	[ ] lDbReturn = fDBQuery(sDWLogOnInfo, "master..xp_cmdshell 'isql -E -n -i {sDWSQLDir}IdwConfigIds.sql'")
	[ ] print (lDbReturn)
	[ ] 
	[ ] print ("Running IdwConfigIcm.sql")
	[ ] lDbReturn = fDBQuery(sDWLogOnInfo, "master..xp_cmdshell 'isql -E -n -i {sDWSQLDir}IdwConfigIcm.sql'")
	[ ] print (lDbReturn)
	[ ] 
	[ ] print ("Running IdwConfigSub.sql")
	[ ] lDbReturn = fDBQuery(sDWLogOnInfo, "master..xp_cmdshell 'isql -E -n -i {sDWSQLDir}IdwConfigSub.sql'")
	[ ] print (lDbReturn)
	[ ] 
	[ ] print ("Running IdwConfigIad.sql")
	[ ] lDbReturn = fDBQuery(sDWLogOnInfo, "master..xp_cmdshell 'isql -E -n -i {sDWSQLDir}IdwConfigIad.sql'")
	[ ] print (lDbReturn)
	[ ] 
	[ ] print ("Running IdwConfigScheduledTasks.sql")
	[ ] lDbReturn = fDBQuery(sDWLogOnInfo, "master..xp_cmdshell 'isql -E -n -i {sDWSQLDir}IdwConfigScheduledTasks.sql'")
	[ ] print (lDbReturn)
	[ ] 
	[ ] print ("Running IdwUpgrade.sql")
	[ ] lDbReturn = fDBQuery(sDWLogOnInfo, "master..xp_cmdshell 'isql -E -n -i {sDWSQLDir}IdwUpgrade.sql'")
	[ ] print (lDbReturn)
	[ ] 
	[ ] print ("Running TestSettings.sql")
	[ ] lDbReturn = fDBQuery(sDWLogOnInfo, "master..xp_cmdshell 'isql -E -n -i {sDWSQLDir}TestSettings.sql'")
	[ ] print (lDbReturn)
	[ ] 
	[ ] return TRUE
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fRunDWJobs()
[ ] //
[ ] //	 ARGUMENTS			-sDBLogOnInfo is string consisting of the DSN Name, the Database 
[ ] //									Server name, the User Id and the Password of the Data Warehouse database under test.
[ ] //									An example of the string: "DSN=ZFRED_DW"
[ ] //  
[ ] //   RETURNS     : 	boolean -
[ ] //									TRUE = if the function ran all of the DW jobs successfully.
[ ] //									FALSE = if any of the jobs did not run successfully.
[ ] //
[ ] //   DESCRIPTION	:	This function runs a series of sql server jobs to import data from all
[ ] //									applicable databases for creation of data in a Data Warehouse database. The 
[ ] //									order is important, so don't change it.
[ ] //
[ ] //	 NOTES				: There is a current bug with the DW which is preventing the Subscriber 
[ ] //									Data import job to run, so I commented it out for now.  This will become
[ ] //									necessary to have if someone wants to run Subscription reports.
[ ] //
[ ] //*****************************************************************************
[ ] 
[+] boolean fRunDWJobs (string sDBLogOnInfo)
	[ ] INTEGER iCount
	[ ] INTEGER iSleep
	[+] LIST OF STRING lsImportJobs = {...}
		[ ] "idw_MBSImportJob"
		[ ] "idw_IDSImportJob"
		[ ] "idw_IADImportJob"
		[ ] "idw_ICMImportJob"
		[ ] // "idw_SUBImportJob"
	[ ] STRING sImportJob
	[+] LIST OF STRING lsJobSteps = {...}
		[ ] "idw_MBSImportJobstep"
		[ ] "idw_IDSImportJobStep"
		[ ] "idw_IADImportJobStep" 
		[ ] "idw_ProcessICMData"
		[ ] // "idw_SUBImportJobStep"
	[ ] STRING sJobStep
	[ ] 
	[+] for iCount = 1 to ListCount (lsImportJobs)
		[ ] sImportJob = lsImportJobs[iCount]
		[ ] sJobStep = lsJobSteps[iCount]
		[ ] print ("Running {sImportJob}.")
		[ ] //Call the fDBRunJob function for each job in the list above.
		[+] if (fDBRunJob(sDBLogOnInfo, "msdb", "{sImportJob}", "{sJobStep}"))
			[ ] print ("The {sImportJob} ran successfully.")
		[+] else
			[ ] print ("The {sImportJob} did not run successfully - failing fRunDWjobs function.")
			[ ] return FALSE
	[ ] return TRUE
