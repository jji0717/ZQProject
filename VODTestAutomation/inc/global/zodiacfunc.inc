[ ] //*****************************************************************************
[ ] //  NAME:					  ZODIACFUNC.INC
[ ] //
[ ] //  DESCRIPTION:		Includes functions that implement the Zodiac framework:
[ ] //                  Create andwrite to the pass/fail log.  In the future could
[ ] //                  include functions to write a Zodiac style trace log, and 
[ ] //                  other functions that implement the framework.
[ ] //
[ ] //  HISTORY:
[ ] //
[ ] //  Date						Developer					Description
[ ] //  ********				***********				*****************************************
[ ] //  12/11/02        M. Albanese       Initial design and development
[ ] //	12/11/02				M. Albanese				Added fCreateDateTimeStamp(),
[ ] //																		fReadWriteIni()
[ ] //	12/12/02				M. Albanese				Unit test fixes to fReadWriteIni, add
    // 																		fLogPassFail() and fCreatePassFailLog()
[ ] //	12/13/02				M. Albanese				Added fDateTimeStamp() call in 
[ ] //																		fCreatePassFailLog for use in backup log
[ ] //																 		name. Added multi format support to
[ ] //																		fCreateDateTimeStamp
[ ] //  12/18/02        C. Callahan       removed functions that were moved to
[ ] //                                    ZodiacUtil.inc
[ ] //	12/20/02				M. Albanese				Replaced fCreatePassFailLog() and
[ ] //																		fLogPassFailLog() with latest versions.
[ ] //*****************************************************************************
[ ] //
[ ] // Module contains these functions:
[ ] //  NAME                    STATUS (peer reviewed or not)
[ ] //  fCreatePassFailLog()    Reviewed
[ ] //  fLogPassFail()          Reviewed
[ ] //	ZodiacTestCaseExit()
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION    	:		fCreatePassFailLog
[ ] //
[ ] //   ARGUMENTS		:  	None
[ ] //
[ ] //   RETURNS			: 	boolean -
[ ] //										true = if the operation was successful.
[ ] //										false = if we could not backup the previous
[ ] //														version of the log file.
[ ] //
[ ] //   DESCRIPTION 	: 	This function creates the pass/fail log by
[ ] //										by first checking for a previous instance of it 
[ ] //										and backing it up. If the backup fails, we return
[ ] //										a false value to the calling function. If we fail to
[ ] //										create the new log due to a bad path/filename Silk
[ ] //										throws an exception as it is should.
[ ] //
[ ] //*****************************************************************************
[ ] 
[-] public boolean fCreatePassFailLog ()
	[ ] 
	[ ] boolean bRetVal
	[ ] handle hPassFailLog
	[ ] 
	[ ] // Check for an existing PassFailLog.
	[-] if SYS_FileExists(gsPassFailLog + ".txt")
		[ ] // Backup the previous pass/fail log.
		[ ] bRetVal = fCreateBackupFile(gsPassFailLog + ".txt",gsPassFailLog + "_" + (fCreateDateTimeStamp(1)) + ".txt")
		[-] if ! bRetVal
			[ ] // The specified source file to be backed up was not found by fCreateBackupFile.
			[ ] return false
	[ ] 
	[ ] // Create the new log file and fill in the banner. Silk throws an exception if
	[ ] // FileOpen fails. If the file does not exist it is created, but if the path is
	[ ] // bad it will not be (e.g., the path specifies a non-existent "m:\" drive).
	[ ] hPassFailLog=FileOpen(gsPassFailLog + ".txt", FM_WRITE)
	[ ] FileWriteLine(hPassFailLog, Replicate("*", 95))
	[ ] FileWriteLine(hPassFailLog,"*")
	[ ] FileWriteLine(hPassFailLog,"*  " + gsScriptName + " Automated Test Results")
	[ ] FileWriteLine(hPassFailLog,"*  Test Machine: " + 
                  SYS_GetRegistryValue(HKEY_LOCAL_MACHINE,
                  "SYSTEM\ControlSet001\Control\ComputerName\ActiveComputerName",
                  "ComputerName"))
	[ ] FileWriteLine(hPassFailLog, "*  " + DateStr() + " " + TimeStr())
	[ ] FileWriteLine(hPassFailLog,"*")
	[ ] FileWriteLine(hPassFailLog, Replicate("*", 95))
	[ ] FileWriteLine(hPassFailLog,"")
	[ ] FileClose(hPassFailLog)
	[ ] 
	[ ] // Return the success code
	[ ] return true
[ ] 
[ ] //*****************************************************************************
[ ] // END: fCreatePassFailLog
[ ] //*****************************************************************************
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION    :	fLogPassFail
[ ] //
[ ] //   REQUIRES    :	string sPassOrFail
[ ] //									-	The Pass or Fail state of the testcase
[ ] //									string sExpected
[ ] //									- The expected result in the form of a message string.
[ ] //										Use only if reporting a failure.
[ ] //									string sReceived
[ ] //									-	The result received in the form of a message string. 
[ ] //										Use only if reporting a failure.
[ ] //
[ ] //   RETURNS     : 	boolean -
[ ] //									false	= if the pass/fail log could not be written to.
[ ] //									true = if the operation was successful.
[ ] //
[ ] //   DESCRIPTION : 	Writes the testcase name and PASS or FAIL status
[ ] //                  to the current pass/fail log. If logging a failure,
[ ] //									the Expected and Received strings are also written.
[ ] //
[ ] //*****************************************************************************
[ ] 
[-] public boolean fLogPassFail (string sPassOrFail, string sExpected, string sReceived)
	[ ] 
	[ ] boolean bRetVal
	[ ] handle hCurrLog
	[ ] string sPassFailLog
	[ ] string sTestCaseName
	[ ] 
	[ ] // Get the name of the currently executing test case.
	[ ] sTestCaseName = GetTestCaseName()
	[ ] 
	[ ] // If the Pass/Fail log doesn't exist, try to create it.
	[-] if ! SYS_FileExists(gsPassFailLog + ".txt")
		[ ] bRetVal = fCreatePassFailLog()
		[-] if !bRetVal
			[ ] // Couldn't create the pass/fail log, so we have nothing to
			[ ] // write to. Return false to the caller.
			[ ] return false
	[ ] 
	[ ] // Once the log is located, write the pass or fail entry into it.
	[-] if SYS_FileExists(gsPassFailLog + ".txt")
		[ ] hCurrLog=FileOpen(gsPassFailLog  + ".txt", FM_APPEND)
		[ ] FileWriteLine(hCurrLog, sTestCaseName + "  " + sPassOrFail + "  [" + DateStr() + " " + TimeStr() + "]")
		[ ] // If the test case failed, we add a detail entry.
		[-] if sPassOrFail == csFailed
			[ ] FileWriteLine(hCurrLog, "***ERROR:  " + "Expected: " + sExpected + (Replicate(" ", 3))
    							+ "Received: " + sReceived)
			[ ] FileWriteLine(hCurrLog, "")
			[ ] FileClose(hCurrLog)
			[ ] // Stop test case execution.
			[ ] // AppError("Expected: " + sExpected + (Replicate(" ", 3))+ "Received: " + sReceived)
			[ ] LogError("*** Error: Expected: " + sExpected + (Replicate(" ", 3))+ "Received: " + sReceived)
			[ ] ZodiacTestCaseExit(True)
		[-] else // Test case passed. Add a new line to the log file.
			[ ] FileWriteLine(hCurrLog, "")
			[ ] FileClose(hCurrLog)
	[ ] 
	[ ] // Return the success code.
	[ ] return true
[ ] 
[ ] //*****************************************************************************
[ ] // END: fLogPassFail
[ ] //*****************************************************************************
[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION    :	TestCaseExit
[ ] //
[ ] //	 ARGUMENTS	 :	pExcep - the exception which cause testcase to exit
[ ] //									use TRUE to exit a testcase
[ ] //*****************************************************************************
[ ] 
[-] void ZodiacTestCaseExit(BOOLEAN pExcep)
	[ ] Print()
	[ ] Print("Base state activated")
	[ ] Print()
	[ ] //DefaultTestCaseExit(pExcep)
	[ ] SetBaseState()
	[ ] exit
[ ] //*****************************************************************************
[ ] // END: TestCaseExit
[ ] //*****************************************************************************
[ ] 
[ ] //*****************************************************************************
[ ] // END: ZODIACFUNC.INC
[ ] //*****************************************************************************
[ ] 
