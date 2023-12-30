[ ] //*****************************************************************************
[ ] //  NAME:					  ZodiacDatabase.INC
[ ] //
[ ] //  DESCRIPTION:		Defines global functions that access a SQL database, retrieve
[ ] //                  and set data, backup and restore the database, etc.
[ ] //
[ ] //	NOTES: 					One needs a DSN set up on their Silk Test box for the machine that 
[ ] //									they want to run these database functions for.  This can be set up
[ ] //									via the system's Control Panel, under ODBC Data Sources.
[ ] //  HISTORY:
[ ] //
[ ] //  Date						Developer					Description
[ ] //  ********				***********				*****************************************
[ ] //  12/11/02        K. Sullivan       Initial design and development.
[ ] //  12/17/02        C. Callahan       Renamed to ZodiacDatabase.inc
[ ] //  01/03/03 				K. Sullivan				Added verification of successful backup 
[ ] //																		for fDBBackup.
[ ] //	01/08/03				K. Sullivan				Added a return value for the backup file for
[ ] //																		fDBBackup.
[ ] //	01/15/03				K. Sullivan				Made database name to fDBQuery optional.
[ ] //																		Made fDBQuery return all columns and rows 
[ ] //																		returned from the query.
[ ] //	01/17/03				K. Sullivan				Created fDBExists, fDBDelete and 
[ ] //																		fDBGetDateTime functions.
[ ] //	01/22/03				K. Sullivan				Added verification to fDBRestore and 
[ ] //																		fDBRunJob.
[ ] //	02/07/03				K. Sullivan				Added more logic to fDBBackup and fDBRunJob 
[ ] //																		so that these functions would not take so long
[ ] //																		to run on smaller databases.
[ ] //
[ ] //*****************************************************************************
[ ] // ZQ $History: zodiacdatabase.inc $
[ ] // 
[ ] // *****************  Version 1  *****************
[ ] // User: Admin        Date: 10-11-12   Time: 16:08
[ ] // Created in $/ZQProjs/VODTestAutomation/inc/global
[ ] // Created.
[ ] // 
[ ] // *****************  Version 1  *****************
[ ] // User: Admin        Date: 10-11-12   Time: 15:42
[ ] // Created in $/Pre-Production/ZQProjs/VODTestAutomation/inc/global
[ ] // Created.
[ ] // 
[ ] // *****************  Version 7  *****************
[ ] // User: Bernie.zhao  Date: 04-08-31   Time: 15:34
[ ] // Updated in $/ZQProjs/VODTestAutomation/inc/global
[ ] // 
[ ] // *****************  Version 6  *****************
[ ] // User: Bernie.zhao  Date: 04-08-27   Time: 15:59
[ ] // Updated in $/ZQProjs/VODTestAutomation/inc/global
[ ] // 
[ ] // *****************  Version 5  *****************
[ ] // User: Bernie.zhao  Date: 04-08-26   Time: 17:10
[ ] // Updated in $/ZQProjs/VODTestAutomation/inc/global
[ ] // 
[ ] // *****************  Version 4  *****************
[ ] // User: Bernie.zhao  Date: 04-08-19   Time: 12:19
[ ] // Updated in $/ZQProjs/VODTestAutomation/inc/global
[ ] // Modified VSS Comment format
[ ] // 
[ ] // *****************  Version 3  *****************
[ ] // User: Bernie.zhao  Date: 04-08-19   Time: 12:16
[ ] // Updated in $/ZQProjs/VODTestAutomation/inc/global
[ ] // Fix a big bug in dbRestore method, to make it runnable
[ ] //*****************************************************************************
[ ] //
[ ] // Module contains these functions:
[ ] //  NAME             STATUS (peer reviewed or not)
[ ] //  fDBBackup()      Not Reviewed
[ ] //	fDBConnect()     Not Reviewed
[ ] //	fDBDelete()      Not Reviewed
[ ] //	fDBExists()			 Not Reviewed
[ ] //  fDBQuery()       Not Reviewed
[ ] //  fDBRestore()     Not Reviewed
[ ] //  fDBRunJob()      Not Reviewed
[ ] //
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fDBGetDateTime ()
[ ] //
[ ] //   ARGUMENTS	  :	string sDBLogOnInfo
[ ] //
[ ] //									-sDBLogOnInfo is string consisting of the DSN Name, the Database 
[ ] //									Server name, the User Id and the Password of the database under test.
[ ] //									An example of the string: "DSN=ZFRED1;SRVR=ZFRED1;UID=sa;PWD=".
[ ] //	               		  
[ ] //   RETURNS	    : The date and time on the database machine in DATETIME format.
[ ] //									
[ ] //
[ ] //
[ ] //   DESCRIPTION	:	Gets the time of the specified database machine. 
[ ] //
[ ] //*****************************************************************************
[ ] 
[+] DATETIME fDBGetDateTime (string sDBLogOnInfo)
	[ ] HSQL hstmnt
	[ ] 
	[ ] HDATABASE hdbc
	[ ] DATETIME dtDBDateTime
	[ ] 
	[ ] print ("Starting fDBGetDateTime.")
	[ ] //Log on to specified database.
	[ ] 
	[ ] hdbc = fDBConnect(sDBLogOnInfo)
	[ ] //Execute query to get the current date and time.
	[ ] hstmnt = DB_ExecuteSqL (hdbc,"Select GETDATE()")
	[ ] //Return the date that the query returned.
	[+] if (DB_FetchNext(hstmnt,dtDBDateTime)== TRUE)
		[ ] DB_FinishSQL (hstmnt)
		[ ] DB_Disconnect (hdbc)
		[ ] print ("Disconnected from the database.")
		[ ] print ("End fDBDateTime.")
		[ ] return dtDBDateTime
	[+] else
		[ ] print ("Could not get date and time info from the database.")
		[ ] return NULL
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fDBConnect()
[ ] //
[ ] //   ARGUMENTS	  :	string sDBLogOnInfo
[ ] 
[ ] //									-sDBLogOnInfo is a string consisting of the DSN Name, the Database 
[ ] //									Server name, the User Id and the Password of the database under test.
[ ] //									An example of the string: "DSN=ZFRED1;SRVR=ZFRED1;UID=sa;PWD=".
[ ] //	               		  
[ ] //   RETURNS	    : Returns a handle to the database connection.  Handle is NULL if 
[ ] //									the connection to the database fails.
[ ] //
[ ] //   DESCRIPTION	:	Connects to the specified database using the specified log on 
[ ] //									information.  
[ ] //*****************************************************************************
[ ] 
[+] HDATABASE fDBConnect(string sDBLogOnInfo)
	[ ] HDATABASE hdbc = NULL
	[ ] 
	[ ] print ("Starting fDBConnect.")
	[ ] print("Connecting to the database.")
	[ ] hdbc = DB_Connect(sDBLogOnInfo)
	[ ] print("Connected to the database.")
	[ ] print ("End fDBConnect.")
	[ ] return hdbc
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fDBQuery()
[ ] //
[ ] //   ARGUMENTS	  :	string sDBLogOnInfo string sSQLQuery string sDBName optional
[ ] 
[ ] //									-string sDBLogOnInfo is string consisting of the DSN Name, the 
[ ] //									Database Server name, the User Id and the Password of the database 
[ ] //									under test. An example of the string: "DSN=ZFRED1;SRVR=ZFRED1;UID=sa;PWD=".
[ ] //									-string sSQLQuery is the SQL query you want to run against
[ ] //									the database under test, such as "select * from Asset".
[ ] //									-string sDBName is the database name you want to run the query
[ ] //									against, such as IdsData, IcmData, etc.  This is optional.
[ ] //	               		  
[ ] //   RETURNS	    : Returns a list that includes the items retruned from the SQL query.
[ ] //
[ ] //   DESCRIPTION	:	Connects to the specified database using the specified log on info
[ ] //									and executes the specified query, and returns the results in a list.
[ ] //
[ ] //	 NOTES				: The group (including myself) is finding that the list of anytype that 
[ ] //									the function returns is hard to work with. I would like to look into the
[ ] //									possibility of having the return type optional, so if one wants the query returned
[ ] //									as just a string, for example, then it gets returned as a string.  If they want
[ ] //									it returned as an integer, it gets returned as an integer, etc.
[ ] //
[ ] //*****************************************************************************
[ ] 
[+] LIST OF ANYTYPE fDBQuery(string sDBLogOnInfo, string sSQLQuery, string sDBName optional)
	[ ] HSQL hstmnt
	[ ] HDATABASE hdbc
	[ ] LIST OF ANYTYPE lDbList
	[ ] LIST OF ANYTYPE lDbReturnList
	[ ] 
	[ ] print ("Starting fDBQuery.")
	[ ] hdbc = fDBConnect(sDBLogOnInfo)
	[+] if sDBName != NULL
		[ ] hstmnt = DB_ExecuteSql(hdbc,"use {sDBName}")
	[ ] hstmnt = DB_ExecuteSqL (hdbc,sSQLQuery)
	[+] while (DB_FetchNext(hstmnt,lDbList)== TRUE)
		[ ] //This append statement is what builds the list that the function returns.
		[ ] ListAppend (lDbReturnList, lDbList)
	[ ] DB_FinishSQL (hstmnt)
	[ ] DB_Disconnect (hdbc)
	[ ] print ("Disconnected from the database.")
	[ ] print ("End fDBQuery.")
	[ ] return lDbReturnList
	[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fDBRunJob ()
[ ] //
[ ] //   ARGUMENTS	  :	string sDBLogOnInfo string sDBName string sDBJobName 
[ ] //									string sStepName string sRunDate
[ ] //
[ ] //									-sDBLogOnInfo is string consisting of the DSN Name, the Database 
[ ] //									Server name, the User Id and the Password of the database under test.
[ ] //									An example of the string: "DSN=ZFRED1;SRVR=ZFRED1;UID=sa;PWD=".
[ ] //									-string sDBName is the database name you want to run the job (aka Stored
[ ] //									procedure) against, such as IdsData, IcmData, etc.
[ ] //									-string sDBJobName is the name of the job that you want to run
[ ] //									against the database under test. For example "idw_IADImportJob".
[ ] //									-string StepName is the name ofthe step of the job to see that the
[ ] //									job was successful or not. For example "idw_IADImportJobStep".
[ ] //									-optional integer for Sleep timer is the amount of time that you want to wait
[ ] //									for the job to run before it's job history is checked.  This is useful to use
[ ] //									when one knows that a particular job takes a long time to run.
[ ] //	               		  
[ ] //   RETURNS	    : TRUE = if the job ran successfully.
[ ] //									FALSE = if the job did not run successfully.
[ ] //
[ ] //   DESCRIPTION	:	Connects to the specified database using the specified log on 
[ ] //									information and runs the specified job.  It then checks the job history
[ ] //									in the database for various information to determine if the job ran
[ ] //									successfully or not.  
[ ] //
[ ] //	 NOTES: 			  Only takes one argument for the job step name, but user may want to check
[ ] //									more than one job step's history to verify that indeed the entire
[ ] //									job ran succesfully.  This function can be enhanced if needed.
[ ] //*****************************************************************************
[+] boolean fDBRunJob(string sDBLogOnInfo, string sDBName, string sDBJobName, string sStepName, integer iSleep optional)
	[ ] HSQL hstmnt
	[ ] HDATABASE hdbc
	[ ] DATETIME dtCurrDateTime
	[ ] INTEGER iLength
	[ ] STRING sCurrDateTime
	[ ] STRING sJobStep
	[ ] STRING sDateRun
	[ ] STRING sTimeRun
	[ ] STRING sRunStatus
	[ ] LIST OF STRING lsHistoryData
	[ ] INTEGER iCount
	[ ] STRING sVerifyJobStep
	[ ] STRING sVerifyDateRun
	[ ] STRING sVerifyTimeRun
	[ ] STRING sVerifyRunStatus
	[ ] STRING sFormat = "yyyymmdd"   // date format to compare strings from jobhistory query to
	[ ] DATETIME dtVerifyDateRun = "2003-01-01" // set to dummy value in case parsing
                              // fails and variable is not set by function
	[ ] BOOLEAN bJobRan = FALSE
	[ ] INTEGER iCheckJob = 1
	[ ] 
	[ ] //Call fDBGetDateTime function to get the date and time from the database machine.
	[ ] dtCurrDateTime = fDBGetDateTime (sDBLogOnInfo)
	[ ] //Cast from DATETIME to TIME for later comparison.
	[ ] TIME tCurrTime = [TIME] dtCurrDateTime
	[ ] //Wait one full minute before starting function, so that in case a job runs in a couple of 
	[ ] //seconds, the time value comparisons will not be too close.
	[ ] sleep (60)
	[ ] //Start function.
	[ ] print ("Starting fDBRunJob.")
	[ ] //Connect to the specified database.
	[ ] hdbc = fDBConnect(sDBLogOnInfo)
	[ ] //Execute command to perform database job.
	[ ] hstmnt = DB_ExecuteSql(hdbc,"use {sDBName}")
	[ ] hstmnt = DB_ExecuteSql (hdbc,"exec sp_start_job @job_name = '{sDBJobName}'")
	[ ] //If one passes in a wait time, use it.
	[+] if iSleep != NULL
		[ ] sleep (iSleep)
	[ ] //Else add default wait statement for job to complete before Silk does the FinishSQL.
	[+] else
		[ ] sleep (60)
	[ ] DB_FinishSQL (hstmnt)
	[ ] DB_Disconnect (hdbc)
	[ ] print ("Started job and disconnected from the database. Now we can verify that the job ran successfully.")
	[+] while ((!bJobRan) && (iCheckJob != 20))
		[ ] //Execute query to see the job history.
		[ ] //Connect to the specified database.
		[ ] hdbc = fDBConnect (sDBLogOnInfo)
		[ ] //Run query to get job history information.
		[ ] hstmnt = DB_ExecuteSql(hdbc,"use msdb")
		[ ] hstmnt = DB_ExecuteSqL (hdbc,"SELECT step_name, run_date, run_time, run_status FROM sysjobhistory WHERE step_name = '{sStepName}'")
		[ ] //Save the data returned from the job history into variables for later comparison.
		[+] while (DB_FetchNext(hstmnt, sJobStep, sDateRun, sTimeRun, sRunStatus)== TRUE)
			[ ] sVerifyJobStep = sJobStep
			[ ] sVerifyRunStatus = sRunStatus
			[ ] //Parse date run string into a valid DATETIME format.
			[ ] ParseDateFormat (sDateRun,sFormat,dtVerifyDateRun)
			[ ] //Get the length of the string for the time run.
			[ ] iLength = Len(sTimeRun)
			[ ] //Rewrite the date run string into ISO format so that it can then be cast to a TIME datatype.
			[+] if iLength == 5
				[ ] sVerifyTimeRun = Stuff(sTimeRun, 2, 0, ":")
				[ ] sVerifyTimeRun = Stuff(sVerifyTimeRun, 5, 0, ":")
			[+] if iLength == 6
				[ ] sVerifyTimeRun = Stuff(sTimeRun, 3, 0, ":")
				[ ] sVerifyTimeRun = Stuff(sVerifyTimeRun, 6, 0, ":")
		[ ] //Cast the new time strings into TIME datatype for later comparison.
		[ ] TIME tVerifyTimeRun = [TIME] sVerifyTimeRun
		[ ] //Disconnect from the database.
		[ ] DB_FinishSQL (hstmnt)
		[ ] DB_Disconnect (hdbc)
		[ ] //Compare variables from last DB_FetchNext to expected values to determine if the job ran successfully.
		[+] if ((sVerifyJobStep == sStepName) && (sVerifyRunStatus == "1"))
			[ ] print ("The job step was found in the jobhistory.")
			[+] if (tVerifyTimeRun > tCurrTime)
				[ ] DATETIME dtCurrDate = [DATETIME] tCurrTime
				[ ] DATE dCurrDate = [DATE] dtCurrDate
				[+] if (dtVerifyDateRun == dCurrDate)
					[ ] print ("The job was found in the history and date and time run is current.")
					[ ] bJobRan = TRUE
				[+] else
					[ ] print ("The job was found in the history, but the run date is not current.")
					[ ] bJobRan = FALSE
			[+] else
				[ ] dtCurrDate = NULL
				[ ] print ("The time run is not current.")
				[ ] bJobRan = FALSE
		[+] else 
			[ ] print ("The job was not run.")
			[ ] bJobRan = FALSE
		[ ] iCheckJob++
		[ ] sleep (60)
	[+] if (bJobRan)
		[ ] return TRUE
	[+] else
		[ ] return FALSE
[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fDBBackup ()
[ ] //
[ ] //   ARGUMENTS	  :	string sDBLogOnInfo string sDBName string sDBBackupDir out string sDBBackupFile
[ ] 
[ ] //									-sDBLogOnInfo is a string consisting of the DSN Name, the Database 
[ ] //									Server name, the User Id and the Password of the database under test.
[ ] //									An example of the string: "DSN=ZFRED1;SRVR=ZFRED1;UID=sa;PWD=".
[ ] //									-string sDBName is the database name you want to run the query
[ ] //									against, such as IdsData, IcmData, etc.
[ ] //									-string sDBBackupDir is the directory where SQL Server puts the backup file.
[ ] //									-out string sDBBackupFile is an optional parameter that, when specified, enables
[ ] //									the function to return the name of the backup file.
[ ] //									        		  
[ ] //   RETURNS	    : TRUE = if the database backup was successful - it checks to see that the backup file
[ ] //									was found in the backup directory.
[ ] //									FALSE = if the database backup was not successful, meaning that the backup file was 
[ ] //									not found in the backup directory.
[ ] //
[ ] //   DESCRIPTION	:	Connects to the specified database using the specified log on 
[ ] //									information and based on the database specified, will run
[ ] //									the applicable backup command.  It then checks to see that the backup file 
[ ] //									exists in the backup directory.
[ ] //
[ ] //	 NOTES: 				The strings for the backup commands will be moved to a zodiacdefs.inc file 
[ ] //									(or databasedef.inc) file, whatever is decided by the QA group, instead of being 
[ ] //									hardcoded here.  But they are for now.
[ ] //
[ ] //									Note that one has be careful which machine they are running this function against.  
[ ] //									For example, the Data Warehouse box has a different backup directory than the other
[ ] //									ITV database machines.  The appropriate path should be set to sDBBackupDir.
[ ] //
[ ] //*****************************************************************************
[+] boolean fDBBackup(string sDBLogOnInfo, string sDBName, string sDBBackupDir, out string sDBBackupFile)
		[ ] HSQL hstmnt
		[ ] HDATABASE hdbc
		[ ] FILEINFO_LIST lfFiles
		[ ] INTEGER iCount
		[ ] INTEGER iSleep
		[ ] DATETIME dtCurrDateTime
		[ ] BOOLEAN bBackupSuccess = FALSE
		[ ] INTEGER iCheckFile = 1
		[ ] 
		[ ] //Call fDBGetDateTime function to get the date and time from the database machine.
		[ ] dtCurrDateTime = fDBGetDateTime (sDBLogOnInfo)
		[ ] //Log on to the specified database.
		[ ] print ("Starting fDBBackup.")
		[ ] hdbc = fDBConnect (sDBLogOnInfo)
		[ ] //Execute the backup command based on the name of the database passed into the function.
		[ ] print ("Executing SQL command to backup the database {sDBName}.")
		[ ] hstmnt = DB_ExecuteSql(hdbc,"use {sDBName}")
		[+] switch (sDBName)
			[+] case "IadData"
				[ ] hstmnt = DB_ExecuteSql(hdbc, "exec iad_maint @exempt_days = 7")
			[+] case "IdsData"
				[ ] hstmnt = DB_ExecuteSql(hdbc, "exec ids_maint @exempt_days = 7")
			[+] case "IcmData"
				[ ] hstmnt = DB_ExecuteSql(hdbc, "master..xp_cmdshell 'SQLMAINT.EXE -D {sDBName} -CkDBNoIdx -CkAlNoIdx -Rpt {sDBBackupDir}\IcmData.rpt -BkUpDB {sDBBackupDir} -BkUpMedia DISK -DelBkUps 2'")
			[+] case "IdwData"
				[ ] hstmnt = DB_ExecuteSql(hdbc, "BACKUP DATABASE [IdwData] TO [idwBackupDevice] WITH INIT,UNLOAD, NAME = N'IdwData full backup', NOSKIP, STATS = 10, NOFORMAT")
				[ ] //have to change sDBName so the filename format for the backup file is like the others.
				[ ] sDBName = "Idw"
			[+] case "SubscriberData"
				[ ] hstmnt = DB_ExecuteSql(hdbc, "exec sdb_maint @backup_exempt_days = 3, @asset_exempt_days = 40")
			[+] default
				[ ] print ("This function does not have a backup command for that database.")
		[ ] //Give SQL some time to backup the database before finishing command execution and disconnecting from database.
		[ ] sleep (60)
		[ ] DB_FinishSQL (hstmnt)
		[ ] DB_Disconnect (hdbc)
		[ ] print ("Disconnected from the database.")
		[ ] print ("End fDBBackup.")
		[ ] //Give the system a few seconds to refresh the backup file list.
		[ ] sleep (5)
		[ ] print ("Now let's see if the backup was successful by looking for the backup file.")
		[+] while ((!bBackupSuccess) && (iCheckFile != 20))
			[ ] //Get a list of files from the backup directory.
			[ ] lfFiles = SYS_GetDirContents (sDBBackupDir)
			[ ] //Check each file in the backup directory, look for a file name with containing the name of the database under test and a BAK extension, and if one is found and 
			[ ] //the timestamp on the file is greater than the current time, then assume the backup was successful and return the backup file name and timestamp.
			[+] for iCount = 1 to ListCount (lfFiles)
				[+] if (MatchStr ("*{sDBName}*", lfFiles[iCount].sName))
					[+] if (MatchStr("*BAK*", lfFiles[iCount].sName))
						[+] if (lfFiles[iCount].dtLastModifyTime > dtCurrDateTime)
							[ ] print ("The backup file was found. The name of the file is {lfFiles[iCount].sName}, and the timestamp on the file is {lfFiles[iCount].dtLastModifyTime}")
							[ ] sDBBackupFile = lfFiles[iCount].sName
							[ ] bBackupSuccess = TRUE
							[ ] break
				[+] else
					[ ] bBackupSuccess = FALSE
					[ ] continue
			[ ] iCheckFile++
			[ ] sleep (60)
		[+] if (bBackupSuccess)
			[ ] return TRUE
		[+] else
			[ ] print ("The backup file was not found.")
			[ ] return FALSE
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fDBRestore ()
[ ] //
[ ] //   ARGUMENTS	  :	string sDBLogOnInfo string sDBName string sDBBackupDir string sDBBackupFile
[ ] //
[ ] //									-sDBLogOnInfo is string consisting of the DSN Name, the Database 
[ ] //									Server name, the User Id and the Password of the database under test.
[ ] //									An example of the string: "DSN=ZFRED1;SRVR=ZFRED1;UID=sa;PWD=".
[ ] //									-string sDBName is the database name you want to restore, such as IdsData, 
[ ] //									IcmData, etc.
[ ] //									-sDBBackupDir - the full unc path were the backup file to be restored exists.
[ ] //									-sDBBackupFile - the backup file name.
[ ] //	               		  
[ ] //   RETURNS	    : TRUE = if the restore was successful.
[ ] //									FALSE = if the restore was not successful.
[ ] //
[ ] //   DESCRIPTION	:	Connects to the specified database using the specified log on 
[ ] //									information and restores the database using the backup file specified.
[ ] //
[ ] //	 NOTES				: Note that one has be careful which machine they are running this function against.  
[ ] //									For example, the Data Warehouse box has a different backup directory than the 
[ ] //									other ITV database machines.  The appropriate path should be set to sDBBackupDir 
[ ] //									and the associated backup file that is to be restored is set to sDBBackupFile.
[ ] //
[ ] //*****************************************************************************
[ ] 
[+] boolean fDBRestore(string sDBLogOnInfo, string sDBName, string sDBBackupDir, string sDBBackupFile)
	[ ] INTEGER iCount
	[ ] HSQL hstmnt 
	[ ] HDATABASE hdbc
	[ ] STRING sRestoreDate
	[ ] LIST OF STRING lsRestoreDate
	[ ] DATETIME dtCurrDateTime
	[ ] 
	[ ] //Call fDBGetDateTime function to get the date and time from the database machine.
	[ ] dtCurrDateTime = fDBGetDateTime (sDBLogOnInfo)
	[ ] //Start function.
	[ ] print ("Starting fDBRestore.")
	[ ] hdbc = fDBConnect (sDBLogOnInfo)
	[ ] print ("Executing SQL command to restore the database {sDBName}.")
	[ ] 
	[ ] //Switch to other database
	[ ] hstmnt = DB_ExecuteSql(hdbc, "USE master")
	[ ] 
	[ ] //Execute command to perform database restore.
	[ ] hstmnt = DB_ExecuteSql(hdbc, "RESTORE DATABASE [{sDBName}] FROM DISK = N'{sDBBackupDir}{sDBBackupFile}' WITH FILE = 1, NOUNLOAD, STATS = 10, RECOVERY")
	[ ] //IMPORTANT - need this wait statement.  If restore is not complete before Silk does the FinishSQL,
	[ ] //the restore will hang.  The way to fix it would be to go into Enterprise Manager manually and peform
	[ ] //the restore again.  You will see that the restore is hung with the state of "Loading" in Enterprise Manager.
	[ ] sleep (200)
	[ ] DB_FinishSQL (hstmnt)
	[ ] DB_Disconnect (hdbc)
	[ ] print ("Disconnected from the database. Now we can verify that the restore was successful.")
	[ ] //Execute query to see the restore history.
	[ ] hdbc = fDBConnect (sDBLogOnInfo)
	[ ] hstmnt = DB_ExecuteSql(hdbc,"use msdb")
	[ ] hstmnt = DB_ExecuteSql(hdbc,"select restore_date from restorehistory WHERE destination_database_name = '{sDBName}'")
	[ ] //Save the data returned from the restore history into a list of string.
	[+] while (DB_FetchNext(hstmnt, sRestoreDate)== TRUE)
		[ ] ListAppend (lsRestoreDate, sRestoreDate)
	[ ] DB_FinishSQL (hstmnt)
	[ ] DB_Disconnect (hdbc)
	[+] for iCount = 1 to ListCount (lsRestoreDate)
		[ ] //Convert time of restore to DATETIME format.
		[ ] DATETIME dtRestoreDate = lsRestoreDate[iCount]
		[ ] //Compare dates in list of string from DB_FetchNext to expected values to determine if the restore was successful.
		[+] if (dtRestoreDate > dtCurrDateTime)
			[ ] print ("The database was restored.")
			[ ] return TRUE
		[+] else
			[ ] continue
	[ ] print ("The database was not restored.")
	[ ] return FALSE 
	[ ] 
[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fDBExists ()
[ ] //
[ ] //   ARGUMENTS	  :	string sDBLogOnInfo string sDBName
[ ] //
[ ] //									-sDBLogOnInfo is string consisting of the DSN Name, the Database 
[ ] //									Server name, the User Id and the Password of the database under test.
[ ] //									An example of the string: "DSN=ZFRED1;SRVR=ZFRED1;UID=sa;PWD=".
[ ] //									-string sDBName is the database name you want to check exists from the
[ ] //									master DB.
[ ] //	               		  
[ ] //   RETURNS	    : TRUE = if the database exists.
[ ] //									FALSE = if the database does not exist.
[ ] //
[ ] //   DESCRIPTION	:	Checks to see if a database exists.
[ ] //
[ ] //*****************************************************************************
[ ] 
[+] boolean fDBExists (string sDBLogOnInfo, string sDBName)
	[ ] ANYTYPE aColumns
	[ ] HSQL hstmnt
	[ ] HDATABASE hdbc
	[ ] LIST OF ANYTYPE lDbReturnList
	[ ] INTEGER iCount
	[ ] 
	[ ] print ("Starting fDBExists.")
	[ ] hdbc = fDBConnect(sDBLogOnInfo)
	[ ] //Execute the query to get the list of databases in the master list.
	[ ] hstmnt = DB_ExecuteSql(hdbc,"use master")
	[ ] hstmnt = DB_ExecuteSqL (hdbc,"sp_helpdb")
	[ ] //Create a list of the databases returned.
	[+] while (DB_FetchNext(hstmnt,aColumns)== TRUE)
		[ ] ListAppend (lDbReturnList, aColumns)
	[ ] DB_FinishSQL (hstmnt)
	[ ] DB_Disconnect (hdbc)
	[ ] //Go through the list and see if the specified database is in the master list.
	[+] for iCount = 1 to ListCount (lDbReturnList)
		[+] if (lDbReturnList[iCount] == "{sDBName}")
			[ ] print ("{sDBName} database is in the database list.")
			[ ] return TRUE
		[+] else
			[ ] continue
	[ ] print ("The {sDBName} database is not in the database list.")
	[ ] return FALSE
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fDBDelete ()
[ ] //
[ ] //   ARGUMENTS	  :	string sDBLogOnInfo string sDBName
[ ] //
[ ] //									-sDBLogOnInfo is string consisting of the DSN Name, the Database 
[ ] //									Server name, the User Id and the Password of the database under test.
[ ] //									An example of the string: "DSN=ZFRED1;SRVR=ZFRED1;UID=sa;PWD=".
[ ] //									-string sDBName is the database name you want to delete.
[ ] //	               		  
[ ] //   RETURNS	    : TRUE = if the database was deleted.
[ ] //									FALSE = if the database was not deleted.
[ ] //
[ ] //   DESCRIPTION	:	Deletes the specified database, then calls the fDBExists function
[ ] //									to see that the database exists.  If it does not exist, then the
[ ] //									function returns TRUE, if it still exists, the function returns 
[ ] //									FALSE.
[ ] //
[ ] //	 NOTES				: The tester must make sure that the applicable services are stopped
[ ] //									on the database machine or the command will not work.  Silk will fail
[ ] //									the script at its DB_ExecuteSql call if the database is in use.
[ ] //
[ ] //*****************************************************************************
[ ] 
[+] boolean fDBDelete (string sDBLogOnInfo, string sDBName)
	[ ] HSQL hstmnt
	[ ] HDATABASE hdbc
	[ ] LIST OF ANYTYPE lDbList
	[ ] 
	[ ] print ("Starting fDBDelete.")
	[ ] hdbc = fDBConnect(sDBLogOnInfo)
	[ ] //Execute the command that deletes the database.
	[ ] hstmnt = DB_ExecuteSqL (hdbc,"DROP DATABASE {sDBName}")
	[ ] DB_FinishSQL (hstmnt)
	[ ] DB_Disconnect (hdbc)
	[ ] //Now we call the fDBExists function to see if the database exists - if it's still in the list, then the 
	[ ] //delete function failed.
	[+] if (fDBExists (sDBLogOnInfo, sDBName))
		[ ] print ("The {sDBName} database is still in the list - the delete function failed.")
		[ ] return FALSE
	[+] else if (!fDBExists (sDBLogOnInfo, sDBName))
		[ ] print ("The {sDBName} database is not in the list - the delete function succeeded.")
		[ ] return TRUE
	[+] else
		[ ] print ("An unexepected error occurred.")
		[ ] return FALSE
