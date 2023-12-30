[ ] // test zodiacFiles.inc functions
[ ] // 
[ ] // Revisions
[ ] // 01/06/2003 CAC Test new search file function interface and covert 
[ ] //    tests to testcases
[ ] // 01/07/2003 CAC Add tests for optional argument to fFindInFile[A]
[ ] // 01/24/2003 CAC Fix/Add tests for fFileLineCount(), fFileSize(), fFileTimeStamp()
[ ] //    and fSearchSectionInFile(), FWildSearchFile()
[ ] 
[ ] 
[ ] use "..\..\inc\global\zodiacfiles.inc"
[ ] use "..\..\inc\global\zodiacdefs.inc"
[ ] use "..\..\inc\global\zodiacmsgs.inc"
[ ] use "..\..\inc\global\zodiacfunc.inc"
[ ] use "..\..\inc\global\zodiacnt.inc"
[ ] use "..\..\inc\global\zodiacutil.inc"
[ ] use "mswconst.inc"
[ ] 
[ ] boolean bReturn
[ ] integer iLineNum 
[ ] string sLine
[ ] 
[ ] const string csLocal  = "Local"
[ ] const string csMyLocalDrive = "d"
[ ] const string csMyLocalPath = ""
[ ] // functions assume caller has IPC access to specified server
[ ] const string csMyServer = "cathyc"
[ ] const string csMyServerIP = "192.168.21.51"
[ ] const string csMyDrive = "c"
[ ] const string csMyPath = "\temp\testresults"
[ ] const string csMyFile = "readfile.txt"
[ ] 
[+] private VOID fSearchFileInit()
	[ ] bReturn = NULL
	[ ] iLineNum = NULL
	[ ] sLine = "!!! NOT FOUND or SET !!!"
[+] private VOID fSearchFileResults()
	[ ] // bReturn not set when converted to testcases.
	[ ] // fTrace ("String found in file?  {bReturn}")
	[ ] fTrace ("Found on line: {iLineNum}")
	[ ] fTrace ("Matched line: {sLine}")
	[ ] Print ("{chr(10)}")
[ ] 
[+] VOID fTrace (string sString)
	[ ] Print ("TRACE: {sString}")
[ ] 
[ ] // Store the current script name.
[ ] const string gsScriptName = Trim(StrTran(GetProgramName( ), ".t", " "))
[ ] 
[ ] // Read the project ini file to determine where the data drive is.
[ ] // const string gsDataDrive = fReadWriteIni(SYS_GetDrive() + ":\autotest\zodiac\" +
    // 														 						gsScriptName + "\" + gsScriptName + ".ini", csRead,
    // 																				"ITVQA","DataDrive","")
[ ] const string gsDataDrive = "D:"
[ ] 
[ ] // Build the PassFailLog string
[ ] const string gsPassFailLog = gsDataDrive + csMainLogPath + "\"  + "global" + "\" +
    											 			gsScriptName + "log"
[ ] 
[+] main ()
	[ ] 
	[ ] string sDTStamp
	[ ] integer iOink
	[ ] 
	[ ] list of string lsCmdOutput 
	[ ] SYS_EXECUTE("set PATH", lsCmdOutput)
	[ ] fTrace(lsCmdOutput[1])
	[ ] 
	[ ] fTrace(gsPassFailLog)
	[+] do
		[ ] Verify(fCreatePassFailLog(), TRUE)
	[+] except
		[ ] ExceptLog()
		[ ] AppError("Error on fCreatePassFailLog")
	[ ] 
	[ ] // Calling all testcases
	[ ] fFileLineCount0001()
	[ ] fFileSize0002()
	[ ] fFileTimeStamp0003()
	[ ] fGetLineFromFileA0004()
	[ ] fGetLineFromFileA0005()
	[ ] fGetLineFromFile0006()
	[ ] fGetLineFromFileA0007()
	[ ] fFindInFileA0008()
	[ ] fFindInFile0009()
	[ ] fFindInFileA0010()
	[ ] fFindInFileA0011()
	[ ] fSearchFileA0012()
	[ ] fSearchFileA0013()
	[ ] fSearchFileA0014()
	[ ] fSearchFileA0015()
	[ ] fSearchFile0016()
	[ ] fSearchFileA0017()
	[ ] fSearchFileA0018()
	[ ] fSearchFileA0019()
	[ ] fSearchFileA0020()
	[ ] fSearchFileA0021()
	[ ] fSearchFileA0022()
	[ ] fSearchFileA0023()
	[ ] fSearchFile0024()
	[ ] fGetLineFromFile0025()
	[ ] fFindInFile0026()
	[ ] fSearchFileA0027()
	[ ] fSearchFileA0028()
	[ ] fFindInFile0029()
	[ ] fFindInFile0030()
	[ ] fFileTimeStamp0031()
	[ ] fFileTimeStamp0032()
	[ ] fFileTimeStamp0033()
	[ ] fFileTimeStamp0034()
	[ ] fSearchSectionOfFile0035()
	[ ] fSearchSectionOfFile0036()
	[ ] fSearchSectionOfFile0037()
	[ ] fSearchSectionOfFile0038()
	[ ] fSearchSectionOfFile0039()
	[ ] fWildSearchFile0040()
	[ ] fWildSearchFile0041()
	[ ] fWildSearchFile0042()
	[ ] fFindInFile0043()
[ ] //*****************************************************************************
[ ] // Testcase definitions 
[ ] //*****************************************************************************
[ ] 
[+] testcase fFileLineCount0001()
	[ ] int iOink = NULL
	[+] do
		[ ] iOink = fFileLineCount(csMyDrive + ":\" + csMyPath + "\" + csMyFile)
		[ ] fTrace ("fFileLineCount = " + Str(iOink))
		[ ] // Write a PASS test line to the Pass/Fail log
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known",Str(iOink))
	[ ] 
[ ] 
[+] testcase fFileSize0002()
	[ ] int iOink = NULL
	[+] do
		[ ] iOink = fFileSize(csMyDrive + ":\" + csMyPath + "\" + csMyFile)
		[ ] fTrace ("fFileSize = " + Str(iOink))
		[ ] // Write a PASS test line to the Pass/Fail log
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known",Str(iOink))
[ ] 
[+] testcase fFileTimeStamp0003()
	[ ] string sOink = NULL
	[+] do
		[ ] sOink = fFileTimeStamp(csMyDrive + ":\" + csMyPath + "\" + csMyFile, 1,1)
		[ ] print ("Create time for {csMyDrive + ":\" + csMyPath + "\" + csMyFile} = " + sOink)
		[ ] // Write a PASS test line to the Pass/Fail log
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known",[string]sOink)
[ ] 
[+] testcase fGetLineFromFileA0004()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Test fGetLineFromFileA()  - it's an empty line")
		[ ] Verify(fGetLineFromFileA (csLocal, csMyDrive, csMyPath, csMyFile, 6, sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fGetLineFromFileA0005()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Test fGetLineFromFileA()")
		[ ] Verify(fGetLineFromFileA (csLocal, csMyDrive, csMyPath, csMyFile, 3, sLine),TRUE)
		[ ] fSearchFileResults()
		[ ] fSearchFileInit()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fGetLineFromFile0006()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Use fGetLineFromFile: Test fGetLineFromFile()")
		[ ] Verify(fGetLineFromFile (csMyDrive + ":\" + csMyPath + "\" + csMyFile, 3, sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fGetLineFromFileA0007()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Test fGetLineFromFileA(); line number is greater than lines in file")
		[ ] Verify(fGetLineFromFileA (csLocal, csMyDrive, csMyPath, csMyFile, 45, sLine), FALSE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fFindInFileA0008()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Test fFindInFileA() Find >?, is in file")
		[ ] Verify(fFindInFileA (csLocal, csMyDrive, csMyPath, csMyFile, ">?", sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fFindInFile0009()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Test fFindInFile() Find >?, is in file")
		[ ] bReturn = fFindInFile (csMyDrive + ":\" + csMyPath + "\" + csMyFile, ">?", sLine)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fFindInFileA0010()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Test fFindInFileA() Find >?, is in file, but don't request the matched line")
		[ ] Verify(fFindInFileA (csLocal, csMyDrive, csMyPath, csMyFile, ">?"), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fFindInFileA0011()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Expect FALSE: Test fFindInFileA() Find rrttyy, is not in file")
		[ ] Verify(fFindInFileA (csLocal, csMyDrive, csMyPath, csMyFile, "rrttyy", sLine), FALSE)
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchFileA0012()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Find the first AAA in the file")
		[ ] Verify(fSearchFileA (csLocal, csMyLocalDrive, csMyLocalPath, csMyFile, 0, "AAA", iLineNum, sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchFileA0013()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Find the first AAA in the file")
		[ ] Verify(fSearchFileA (csLocal, csMyLocalDrive, csMyLocalPath, csMyFile, 0, "AAA", iLineNum, sLine), TRUE)
		[ ] fTrace ("Find the next AAA in the file")
		[ ] Verify(fSearchFileA (csLocal, csMyLocalDrive, csMyLocalPath, csMyFile, iLineNum+1, "AAA", iLineNum, sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchFileA0014()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("FALSE expected: This string is not in the file")
		[ ] Verify(fSearchFileA (csLocal, csMyLocalDrive, csMyLocalPath, csMyFile, 0, "PLOKIJUH", iLineNum, sLine), FALSE)
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchFileA0015()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("FALSE expected: This line is not in the file")
		[ ] Verify(fSearchFileA (csLocal, csMyLocalDrive, csMyLocalPath, csMyFile, 100, "PLOKIJUH", iLineNum, sLine), FALSE)
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchFile0016()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Use fSearchFile: Find a non alphnumeric character")
		[ ] string sFilePath = "{csMyDrive}:{csMyPath}\{csMyFile}"
		[ ] fTrace(sFilePath)
		[ ] Verify(fSearchFile (sFilePath, 0, "()", iLineNum, sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchFileA0017()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Find a non alphnumeric character")
		[ ] Verify(fSearchFileA (csLocal, csMyLocalDrive, csMyLocalPath, csMyFile, 0, "()", iLineNum, sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchFileA0018()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Find the "" character")
		[ ] Verify(fSearchFileA (csLocal, csMyLocalDrive, csMyLocalPath, csMyFile, 0, """", iLineNum, sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchFileA0019()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Find the "" character")
		[ ] Verify(fSearchFileA (csLocal, csMyLocalDrive, csMyLocalPath, csMyFile, 0, """", iLineNum, sLine), TRUE)
		[ ] fTrace ("Find the next "" character")
		[ ] Verify(fSearchFileA (csLocal, csMyLocalDrive, csMyLocalPath, csMyFile, iLineNum+1, """", iLineNum, sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchFileA0020()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Use a path, not at root; Find >?")
		[ ] Verify(fSearchFileA (csLocal, csMyDrive, csMyPath, csMyFile, 0, ">?", iLineNum, sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchFileA0021()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Again, but leave off leading and trailing \")
		[ ] Verify(fSearchFileA (csLocal, csMyDrive, "temp\testresults", csMyFile, 0, ">?", iLineNum, sLine),TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchFileA0022()
	[+] list of STRING lsIPC = {...}
		[ ] ""
	[+] do
		[ ] fSearchFileInit()
		[ ] Verify (fGetIPCAccess(csMyServer,"Administrator", "deadsea",lsIPC), TRUE)
		[ ] fTrace ("fGetIPCAccess returned: {lsIPC}")
		[ ] fTrace ("Again, but file on remote machine - use node name - assume IPC access")
		[ ] Verify(fSearchFileA (csMyServerIP, csMyDrive, csMyPath, "readfile.txt", 0, ">?", iLineNum, sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] ExceptPrint()
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchFileA0023()
	[+] do
		[ ] fSearchFileInit()
		[ ] Verify (fGetIPCAccess(csMyServerIP,"Administrator", "cdci"), TRUE)
		[ ] fTrace ("Again, but file on remote machine - use IP Address - assume IPC access")
		[ ] bReturn = fSearchFileA (csMyServerIP, csMyDrive, csMyPath, "readfile.txt", 0, ">?", iLineNum, sLine)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchFile0024()
	[+] do
		[ ] fSearchFileInit()
		[ ] Verify (fGetIPCAccess(csMyServerIP,"Administrator", "cdci"), TRUE)
		[ ] fTrace ("Again, but Use fSearchFile:  assume IPC access")
		[ ] Verify(fSearchFile ("\\{csMyServerIP}\{csMyDrive}$\{csMyPath}\readfile.txt", 0, ">?", iLineNum, sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fGetLineFromFile0025()
	[+] do
		[ ] fSearchFileInit()
		[ ] Verify (fGetIPCAccess(csMyServerIP,"Administrator", "deadsea"), TRUE)
		[ ] fTrace ("Test fGetLineFromFile()")
		[ ] Verify(fGetLineFromFile ("\\" + csMyServer + "\" + csMyDrive + "$\" + csMyPath + "\" + csMyFile, 3, sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] ExceptPrint()
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fFindInFile0026()
	[+] do
		[ ] fSearchFileInit()
		[ ] Verify (fGetIPCAccess(csMyServerIP,"Administrator", "cdci"), TRUE)
		[ ] fTrace ("Test fFindInFile() Find >?, is in file")
		[ ] Verify(fFindInFile ("\\" + csMyServerIP + "\" + csMyDrive + "$\" + csMyPath + "\" + csMyFile, ">?", sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchFileA0027()
	[+] do
		[ ] fSearchFileInit()
		[ ] Verify (fGetIPCAccess(csMyServerIP,"Administrator", "cdci"), TRUE)
		[ ] fTrace ("FALSE expected: File on remote machine does not exist")
		[ ] Verify(fSearchFileA (csMyServerIP, csMyDrive, "\temp\", "readfile.txt", 0, ">?", iLineNum, sLine), FALSE)
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchFileA0028()
	[ ] fSearchFileInit()
	[+] do
		[ ] fTrace ("FALSE expected: Remote machine does not exist")
		[ ] Verify(fSearchFileA ("199.168.21.20", "c$", "\temp\", "readfile.txt", 0, ">?", iLineNum, sLine), FALSE)
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fFindInFile0029()
	[ ] // if string that is to contain the line with matched string does not need to
	[ ] // be initialized (fFindInFile0043)
	[ ] // if initialized to NULL, then value does get filled in (this test case)
	[ ] // if initialized to other than NULL does get filled in (see fFindInFile0030)
	[ ] string sInitializedNull = NULL
	[ ] fSearchFileInit()
	[+] do
		[ ] fTrace("Before fFindInFile - sInitializedNull:{[string]sInitializedNull}")
		[ ] fTrace ("Test fFindInFile() Find >?, is in file")
		[ ] Verify(fFindInFile (csMyDrive + ":\" + csMyPath + "\" + csMyFile, ">?", sInitializedNull), TRUE)
		[ ] fTrace("After fFindInFile - sInitializedNull:{[string]sInitializedNull}")
		[ ] Verify((sInitializedNull != NULL), TRUE)
		[ ] sLine = sInitializedNull
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] ExceptPrint()
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fFindInFile0030()
	[ ] // if string that is to contain the line with matched string is not initialized; 
	[ ] // function will not fail (see fFindInFile0043)
	[ ] // if initialized to NULL, then value does get filled in (see fFindInFile0030)
	[ ] // if initialized to other than NULL does get filled in (this test case)
	[ ] string sInitializedEmpty = ""
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace("Before fFindInFile - sInitializedEmpty:{[string]sInitializedEmpty}")
		[ ] fTrace ("Test fFindInFile() Find >?, is in file")
		[ ] bReturn = fFindInFile (csMyDrive + ":\" + csMyPath + "\" + csMyFile, ">?", sInitializedEmpty)
		[ ] fTrace("After fFindInFile - sInitializedEmpty:{[string]sInitializedEmpty}")
		[ ] sLine = sInitializedEmpty
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] ExceptPrint()
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fFileTimeStamp0031()
	[ ] string sOink = NULL
	[+] do
		[ ] sOink = fFileTimeStamp(csMyDrive + ":\" + csMyPath + "\" + csMyFile, 1,2)
		[ ] print ("Create time for {csMyDrive + ":\" + csMyPath + "\" + csMyFile} = " + sOink)
		[ ] // Write a PASS test line to the Pass/Fail log
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known",[string]sOink)
[ ] 
[+] testcase fFileTimeStamp0032()
	[ ] string sOink = NULL
	[+] do
		[ ] Verify (fGetIPCAccess(csMyServerIP,"Administrator", "deadsea"), TRUE)
		[ ] sOink = fFileTimeStamp("\\" + csMyServer + "\" + csMyDrive + "$\" + csMyPath + "\" + csMyFile, 2,3)
		[ ] print ("\\" + csMyServer + "\" + csMyDrive + "$\" + csMyPath + "\" + csMyFile)
		[ ] print ("Modify time for {"\\" + csMyServer + "\" + csMyDrive + "$\" + csMyPath + "\" + csMyFile} = " + sOink)
		[ ] // Write a PASS test line to the Pass/Fail log
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known",[string]sOink)
[ ] 
[+] testcase fFileTimeStamp0033()
	[ ] string sOink = NULL
	[+] do
		[ ] sOink = fFileTimeStamp(csMyDrive + ":\" + csMyPath + "\" + csMyFile, 2,4)
		[ ] print ("Modify time for {"\\" + csMyServer + "\" + csMyDrive + "$\" + csMyPath + "\" + csMyFile} = " + sOink)
		[ ] // Write a PASS test line to the Pass/Fail log
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] ExceptLog()
		[ ] fLogPassFail(csFailed, "Not known",[string]sOink)
[ ] 
[+] testcase fFileTimeStamp0034()
	[ ] string sOink
	[ ] string sErr
	[+] do
		[ ] sOink = fFileTimeStamp(csMyDrive + ":\" + csMyPath + "\" + csMyFile, 3,3,sErr)
		[ ] Verify(sOink,NULL)
		[ ] // Write a PASS test line to the Pass/Fail log
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] ExceptLog()
		[ ] print ("Expected to Fail, incorrect parameter value for Create/Modify")
		[ ] fLogPassFail(csFailed, "Modify time", "Error from fFileTimeStamp: {sErr}")
[ ] 
[+] testcase fSearchSectionOfFile0035()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Use fSearchFile: Find 'AAA' between line 6 and 15")
		[ ] Verify(fSearchSectionOfFile (csMyDrive + ":\" + csMyPath + "\" + csMyFile, 6, 15, "AAA", iLineNum, sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchSectionOfFile0036()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Use fSearchFile: Find AAA between line 5 and 15")
		[ ] Verify(fSearchSectionOfFile (csMyDrive + ":\" + csMyPath + "\" + csMyFile, 5, 15, "AAA", iLineNum, sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchSectionOfFile0037()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Use fSearchFile: Find AAA between line 6 and 10 - Not there")
		[ ] Verify(fSearchSectionOfFile (csMyDrive + ":\" + csMyPath + "\" + csMyFile, 6, 10, "AAA", iLineNum, sLine), FALSE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchSectionOfFile0038()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Use fSearchFile: Find () between line 6 and 10 - On line 10")
		[ ] Verify(fSearchSectionOfFile (csMyDrive + ":\" + csMyPath + "\" + csMyFile, 6, 10, "()", iLineNum, sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fSearchSectionOfFile0039()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Use fSearchFile: Find () between line 21 and 30 - Not there")
		[ ] Verify(fSearchSectionOfFile (csMyDrive + ":\" + csMyPath + "\" + csMyFile, 21, 30, "()", iLineNum, sLine), FALSE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fWildSearchFile0040()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Use fWildSearchFile: Use an * to match a string - should find 'aaa AAA'")
		[ ] string sFilePath = "{csMyDrive}:{csMyPath}\{csMyFile}"
		[ ] fTrace(sFilePath)
		[ ] Verify(fWildSearchFile (sFilePath, 0, "aa*AA", iLineNum, sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fWildSearchFile0041()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Use fWildSearchFile: Use a ? to match a string - should find 'aaa AAA'")
		[ ] string sFilePath = "{csMyDrive}:{csMyPath}\{csMyFile}"
		[ ] fTrace(sFilePath)
		[ ] Verify(fWildSearchFile (sFilePath, 0, "aa? ?AA", iLineNum, sLine), TRUE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fWildSearchFile0042()
	[+] do
		[ ] fSearchFileInit()
		[ ] fTrace ("Use fWildSearchFile: Must pattern match the entire string - should fail'")
		[ ] string sFilePath = "{csMyDrive}:{csMyPath}\{csMyFile}"
		[ ] fTrace(sFilePath)
		[ ] Verify(fWildSearchFile (sFilePath, 0, "a? ?AA", iLineNum, sLine), FALSE)
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[+] testcase fFindInFile0043()
	[ ] // if string that is to contain the line with matched string is not initialized; 
	[ ] // function will not fail (this test case)
	[ ] // if initialized to NULL, then value does get filled in (see fFindInFile0029)
	[ ] // if initialized to other than NULL does get filled in (see fFindInFile0030)
	[ ] string sNotInitialized
	[ ] fSearchFileInit()
	[+] do
		[ ] Verify(IsSet(sNotInitialized), FALSE)
		[ ] fTrace ("Test fFindInFile() Find >?, is in file")
		[ ] Verify(fFindInFile (csMyDrive + ":\" + csMyPath + "\" + csMyFile, ">?", sNotInitialized), TRUE)
		[ ] fTrace("After fFindInFile - sInitializedNull:{[string]sNotInitialized}")
		[ ] Verify(IsSet(sNotInitialized),TRUE)
		[ ] sLine = sNotInitialized
		[ ] fSearchFileResults()
		[ ] fLogPassFail(csPassed, "","")
	[+] except
		[ ] ExceptPrint()
		[ ] fLogPassFail(csFailed, "Not known; returned Line:", [string]sLine)
[ ] 
[ ] //*****************************************************************************
[ ] // END: zodiacFileUT.T
[ ] //*****************************************************************************
