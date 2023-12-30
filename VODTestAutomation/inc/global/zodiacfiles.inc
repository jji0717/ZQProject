[ ] //******************************************************************************
[ ] //111111102222222220333333333044444444405555555550666666666077777777708888888880
[ ] // Filename:    zodiacFiles.inc
[ ] // Project:     Zodiac
[ ] // Author:		  CAC
[ ] // Description: Defines global functions that access files on NT system, 
[ ] //    retrieve file information such as timestamps, size inbytes, size in 
[ ] //    lines, read/write attributes, retrieve information from a file, 
[ ] //    make a copy of a file, etc
[ ] //
[ ] // Significant revisions:
[ ] // 12/13/2002 CAC fSearchFile(), fGetLineFromFile(), fFindInFile()
[ ] // 12/16/2002 CAC Don't set the Matched Line if line isn't found for 
[ ] //    fGetLineFromFile() and fFindInFile(). And make returning the matched 
[ ] //    line optional for fFindInFile()
[ ] // 12/16/2002 CAC Add prototypes for File Information calls written in C
[ ] // 12/20/02   MRA Migrate fCreateBackupFile() from zodiacfunc.inc.
[ ] // 12/20/02   MRA Add banner for fFileLineCount.dll and merge in fReadWriteIni()
[ ] // 01/06/02   CAC change function fSearchFile(), fGetLineFromFile, and
[ ] //    fFindInFile to take a full path.  Function fSearchFileA(),
[ ] //    fGetLineFromFileA() and fFindInFile() support getting file name in
[ ] //    pieces (server, drive, path, filename)
[ ] // 01/06/03   MRA Changed fCreateBackupFile to return "false" if the source
[ ] //            file passed in does not exist or cannot be found.
[ ] // 01/09/03   JRN Changed the FileLineCount.dll declaration to load the new
[ ] //            FileProperties.dll
[ ] // 01/13/03   JRN Ripped out the dll stuff and rewrote it in straight silk
[ ] // 01/24/03   CAC Added fSearchSectionOfFile[A]() to search for lines within a
[ ] //            line number range. Added fWildSearchFile[A] to allow the search
[ ] //            of a line using wild cards.  The entire line must match the
[ ] //            search string.
[ ] //******************************************************************************
[ ] //
[ ] // Module contains these functions:
[ ] //  NAME               	      STATUS (peer reviewed or not)
[ ] //  fFindInFile()      	      not Reviewed
[ ] //  fSearchFile()      	      not Reviewed
[ ] //  fGetLineFromFile() 	      not Reviewed
[ ] //  fFindInFileA()            not Reviewed
[ ] //  fSearchFileA()            not Reviewed
[ ] //  fGetLineFromFileA()       not Reviewed
[ ] //  fFileLineCount()   	      not Reviewed
[ ] //  fFileSize()        	      not Reviewed
[ ] //  fFileTimeStamp()   	      not Reviewed
[ ] //  fCreateBackupFile()       Reviewed
[ ] //  fReadWriteIni()		      Reviewed
[ ] //  fSearchSectionOfFileA()   not Reviewed
[ ] //  fSearchSectionOfFile()    not Reviewed
[ ] //  fWildSearchFile()         not Reviewed
[ ] //  fWildSearchFileA()        not Reviewed
[ ] 
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    	:	fCreateBackupFile
[ ] //
[ ] //   REQUIRES    	:	string sSourceFile
[ ] //									- path to and name of the log file to backup.
[ ] //								:	string sBackupFile
[ ] //									- path to and name of the backup file.
[ ] //               :  
[ ] //   RETURNS     : 	boolean -
[ ] //									false	= if the operation failed.
[ ] //									true = if the operation was successful.
[ ] //
[ ] //   DESCRIPTION : 	Takes an original file and moves it to a backup file.
[ ] //									If the source file does not exist or is not found,
[ ] //									we return a "false" value.
[ ] //
[ ] //*****************************************************************************
[ ] 
[ ] use "zodiacdefs.inc"
[ ] use "zodiacfunc.inc"
[ ] use "zodiacutil.inc"
[ ] use "zodiacmsgs.inc"
[ ] 
[ ] use "msw32.inc"
[ ] 
[ ] 
[-] public boolean fCreateBackupFile (string sSourceFile, string sBackupFile)
	[ ] 
	[ ] boolean bExists
	[ ] 
	[ ] // Look for the specified source file
	[ ] bExists = SYS_FileExists(sSourceFile)
	[+] if bExists
		[ ] // Look for an old copy of the file to be backed up and delete it.
		[ ] bExists = SYS_FileExists(sBackupFile)
		[+] if bExists
			[ ] SYS_RemoveFile(sBackupFile)
		[ ] // Rename the source file by "moving" it. 
		[ ] SYS_MoveFile(sSourceFile, sBackupFile)
	[ ] // Did not find the specified source file. Failing the test case.
	[+] else
		[ ] // Show error
		[ ] LogError("File "+sSourceFile+" does not exist or can not be opened")
		[ ] // Source file does not exist or cannot be found. Check name/path.
		[ ] return false
	[ ] 
	[ ] // Return success code
	[ ] return true
[ ] 
[ ] //*****************************************************************************
[ ] // END: fCreateBackupFile
[ ] //*****************************************************************************
[ ] 
[ ] //*****************************************************************************
[ ] // fSearchFileA(), fSearchFile()
[ ] // Description:
[ ] //   Finds the specified string in the specified file.  Can give a starting
[ ] //   line number as the line number to start the search.  If string is found,
[ ] //   routine returns the line number and line with matching string.   
[ ] // Returns:
[ ] //   Boolean: TRUE if string is found in file, FALSE otherwise
[ ] // Arguments:
[ ] //  in string sServerIP (fSearchFileA())
[ ] //     name or IP address of server on which file is located; 
[ ] //     assume caller has IPC access to server
[ ] //  in string sDrive (fSearchFileA())
[ ] //     drive letter on which file exists
[ ] //  in string sPath (fSearchFileA())
[ ] //     directory path to file
[ ] //  in string sFilename (fSearchFileA())
[ ] //     name of file
[ ] //  in string sFilePath (fSearchFile())
[ ] //  in integer iStartLineNum
[ ] //     the line number to start the search on, if 0 start at beginning
[ ] //  in string sSearchString
[ ] //     the string to look for, if empty string, the Line on iStartLineNum
[ ] //     is returned
[ ] //  out integer iFoundLineNum
[ ] //     the line number of the line with the string
[ ] //  out sMatchedLine
[ ] //     the full line that matched the string.  NULL if matched line not found.
[ ] //******************************************************************************
[+] boolean fSearchFileA(in string sServerIP, in string sDrive, in string sPath,
        in string sFilename, in integer iStartLineNum, in string sSearchString,
        out integer iFoundLineNum, out string sMatchedLine)
	[ ] 
	[ ] boolean bReturn = FALSE
	[ ] string sFullPath = ""
	[ ] 
	[ ] // create sPath from arguments
	[+] if (sServerIP == "Local")
		[ ] sFullPath = "{sDrive}:\{sPath}\{sFilename}"
	[+] else
		[ ] sFullPath = "\\{sServerIP}\{sDrive}$\{sPath}\{sFilename}"
	[ ] 
	[ ] // call fSearchFile
	[ ] bReturn = fSearchFile(sFullPath, iStartLineNum, sSearchString,iFoundLineNum,
        sMatchedLine)
	[ ] return bReturn
[ ] 
[-] boolean fSearchFile(in string sFilePath,in integer iStartLineNum, 
        in string sSearchString, out integer iFoundLineNum, out string sMatchedLine)
	[ ] 
	[ ] boolean bReturn = FALSE
	[ ] string sCurrentLine = ""
	[ ] iFoundLineNum = 0
	[ ] HFILE hFile = NULL
	[ ] 
	[-] do
		[ ] hFile = FileOpen (sFilePath,FM_READ,FS_DENY_NONE)
	[+] except
		[ ] // Could not open file
		[ ] // TBD add error handling
		[ ] // LogError ("ERROR: could not open file {sFilePath}")
		[ ] // ExceptPrint()
		[ ] reraise
		[ ] return bReturn
	[ ] 
	[ ] // read each line looking for string
	[-] while (FileReadLine (hFile, sCurrentLine))
		[ ] // keep track of the line number were on
		[ ] iFoundLineNum++
		[ ] 
		[-] if (iStartLineNum > 0 && (iFoundLineNum < iStartLineNum))
			[ ] // skip to starting line
			[ ] // done with this line; just read next line
		[-] else
			[-] if (sSearchString != "")
				[ ] // were looking for the string sSearchString
				[-] if (StrPos (sSearchString, sCurrentLine) != 0)
					[ ] // found a match
					[ ] bReturn = TRUE
					[ ] // don't need to read anymore lines
					[ ] break
			[-] else
				[ ] // just looking for a line by its line number
				[ ] // found line number wanted, just return full line
				[ ] // test that current line number (iFoundLineNum) ==
				[ ] // iStartLineNumber, logic error if not.
				[-] do
					[ ] Verify (iFoundLineNum, iStartLineNum, "Logic Error")
					[ ] Verify (sSearchString, "", "Logic Error")
					[ ] bReturn = TRUE
					[ ] // don't need to read anymore lines
					[ ] break
				[-] except
					[ ] // TBD handle error
					[ ] ExceptPrint()
	[ ] 
	[ ] // close file
	[ ] FileClose (hFile)
	[-] if bReturn
		[ ] sMatchedLine = sCurrentLine
	[-] else 
		[ ] sMatchedLine = NULL
	[ ] return bReturn
[ ] 
[ ] //******************************************************************************
[ ] // END: fSearchFileA() fSearchFile()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] // fGetLineFromFileA(), fGetLineFromFile()
[ ] // Description:
[ ] //   Caller provides a line number and routine 
[ ] //   returns the line at that line number
[ ] // Returns:
[ ] //   Boolean: TRUE if string is found in file, FALSE otherwise
[ ] // Arguments:
[ ] //  in string sServerIP (fGetLineFromFileA())
[ ] //     name or IP address of server on which file is located; 
[ ] //     assume caller has IPC access to server
[ ] //  in string sDrive (fGetLineFromFileA())
[ ] //     drive letter on which file exists
[ ] //  in string sPath (fGetLineFromFileA())
[ ] //     directory path to file
[ ] //  in string sFilename (fGetLineFromFileA())
[ ] //     name of file
[ ] //  in string sFilePath (fGetLineFromFile())
[ ] //     full path to file
[ ] //  in integer iLineNum
[ ] //     the line number to retrieve
[ ] //  out sMatchedLine
[ ] //     the full line that matched the line number, set to NULL if line number
[ ] //     does not exist
[ ] //******************************************************************************
[+] boolean fGetLineFromFileA(in string sServerIP, in string sDrive, in string sPath,
        in string sFilename, in integer iLineNum, out string sMatchedLine)
	[ ] 
	[ ] // Call fSearchFileA(), just set the string to match to "", and don't return
	[ ] // the line number 
	[ ] integer iFoundLineNum
	[ ] string sLocalLine = ""
	[ ] boolean bReturn = FALSE
	[ ] 
	[ ] bReturn = fSearchFileA(sServerIP, sDrive,sPath,sFilename,
      iLineNum,"",iFoundLineNum,sLocalLine)
	[ ] // if found a match return the line; otherwise return NULL
	[+] if (bReturn)
			[ ] sMatchedLine = sLocalLine
		[-] else 
			[ ] sMatchedLine = NULL
	[ ] return bReturn
[ ] 
[+] boolean fGetLineFromFile(in string sFilePath,in integer iLineNum, 
        out string sMatchedLine)
	[ ] 
	[ ] // Call fSearchFile(), just set the string to match to "", and don't return
	[ ] // the line number 
	[ ] integer iFoundLineNum
	[ ] string sLocalLine = ""
	[ ] boolean bReturn = FALSE
	[ ] 
	[ ] bReturn = fSearchFile(sFilePath,iLineNum,"",iFoundLineNum,sLocalLine)
	[ ] // if found a match return the line; otherwise return NULL
	[+] if (bReturn)
			[ ] sMatchedLine = sLocalLine
		[+] else 
			[ ] sMatchedLine = NULL
	[ ] return bReturn
[ ] 
[ ] //******************************************************************************
[ ] // END: fGetLineFromFile(), fGetLineFromFileA()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] // fFindInFileA() fFindInFile()
[ ] // Description:
[ ] //   Finds the specified string in the specified file.  Searches the entire
[ ] //   file. If string is found, routine can return the line with matching string
[ ] //   (optional return argument)   
[ ] // Returns:
[ ] //   Boolean: TRUE if string is found in file, FALSE otherwise
[ ] // Arguments:
[ ] //  in string sServerIP
[ ] //     name or IP address of server on which file is located; 
[ ] //     assume caller has IPC access to server
[ ] //  in string sDrive
[ ] //     drive letter on which file exists
[ ] //  in string sPath
[ ] //     directory path to file
[ ] //  in string sFilename 
[ ] //     name of file
[ ] //  in string sSearchString
[ ] //     the string to look for
[ ] //  out sMatchedLine optional
[ ] //     the full line that matched the string, if the argument is supplied
[ ] //******************************************************************************
[+] boolean fFindInFileA(in string sServerIP, in string sDrive, in string sPath,
        in string sFilename, in string sSearchString, out string sMatchedLine optional)
	[ ] 
	[ ] // Call fSearchFileA(), just set the line number to 0, and don't return the
	[ ] // line number it was found on
	[ ] integer iFoundLineNum = 0
	[ ] string sLocalLine = ""
	[ ] boolean bReturn = FALSE
	[ ] 
	[ ] bReturn = fSearchFileA(sServerIP, sDrive,sPath,sFilename,0,sSearchString,
        iFoundLineNum,sLocalLine)
	[ ] 
	[ ] //if a match was found and caller wants the entire line return it
	[+] if (bReturn)
		[ ] sMatchedLine = sLocalLine
	[ ] 
	[ ] return bReturn
[ ] 
[+] boolean fFindInFile(in string sFilePath,in string sSearchString, 
        out string sMatchedLine optional)
	[ ] 
	[ ] // Call fSearchFile(), just set the line number to 0, and don't return the
	[ ] // line number it was found on
	[ ] integer iFoundLineNum = 0
	[ ] string sLocalLine = ""
	[ ] boolean bReturn = FALSE
	[ ] 
	[ ] bReturn = fSearchFile(sFilePath,0,sSearchString,
        iFoundLineNum,sLocalLine)
	[ ] 
	[ ] //if a match was found and caller wants the entire line return it
	[+] if (bReturn)
		[ ] sMatchedLine = sLocalLine
	[ ] 
	[ ] return bReturn
[ ] 
[ ] //*****************************************************************************
[ ] // END: fFindInFile(), fFindInFileA()
[ ] //*****************************************************************************
[ ] //*****************************************************************************
[ ] // fSearchSectionOfFileA(), fSearchSectionOfFile()
[ ] // Description:
[ ] //   Finds the specified string in the specified file.  Must give a starting
[ ] //   line number as the line number to start the search and an ending line
[ ] //   number to end the search.  If string is found,
[ ] //   routine returns the line number and line with matching string. 
[ ] //   Caller should use fSearchFile() or fSearchFileA() if not searching a 
[ ] //   section of the file.
[ ] // Returns:
[ ] //   Boolean: TRUE if string is found in file, FALSE otherwise
[ ] // Arguments:
[ ] //  in string sServerIP (fSearchSectionOfFileA())
[ ] //     name or IP address of server on which file is located; 
[ ] //     assume caller has IPC access to server
[ ] //  in string sDrive (fSearchSectionOfFileA())
[ ] //     drive letter on which file exists
[ ] //  in string sPath (fSearchSectionOfFileA())
[ ] //     directory path to file
[ ] //  in string sFilename (fSearchSectionOfFileA())
[ ] //     name of file
[ ] //  in string sFilePath (fSearchSectionOfFile())
[ ] //  in integer iStartLineNum
[ ] //     the line number to start the search on, if 0 start at beginning
[ ] //  in integer iEndLineNum
[ ] //     the line number to end the search on, should be greater than
[ ] //     iStartLineNum
[ ] //  in string sSearchString
[ ] //     the string to look for, if empty string, the Line on iStartLineNum
[ ] //     is returned
[ ] //  out integer iFoundLineNum
[ ] //     the line number of the line with the string
[ ] //  out sMatchedLine
[ ] //     the full line that matched the string.  NULL if matched line not found.
[ ] //******************************************************************************
[+] boolean fSearchSectionOfFileA(in string sServerIP, in string sDrive,
      in string sPath, in string sFilename, in integer iStartLineNum,
      in integer iEndLineNum, in string sSearchString, out integer iFoundLineNum NULL,
      out string sMatchedLine NULL)
	[ ] 
	[ ] boolean bReturn = FALSE
	[ ] string sFullPath = ""
	[ ] 
	[ ] // create sPath from arguments
	[+] if (sServerIP == "Local")
		[ ] sFullPath = "{sDrive}:\{sPath}\{sFilename}"
	[+] else
		[ ] sFullPath = "\\{sServerIP}\{sDrive}$\{sPath}\{sFilename}"
	[ ] 
	[ ] // call fSearchFile
	[ ] // bReturn = fSearchSectionOfFile(sFullPath, iStartLineNum, iEndLineNum, sSearchString, iFoundLineNum, sMatchedLine)
	[ ] return bReturn
[ ] 
[-] boolean fSearchSectionOfFile(in string sFilePath,in integer iStartLineNum,
    in integer iEndLineNum, string sSearchString, inout integer iFoundLineNum NULL,
    inout string sMatchedLine NULL)
	[ ] 
	[ ] boolean bReturn = FALSE
	[ ] string sCurrentLine = ""
	[ ] iFoundLineNum = 0
	[ ] HFILE hFile = NULL
	[ ] 
	[+] do
		[ ] hFile = FileOpen (sFilePath,FM_READ,FS_DENY_NONE)
	[-] except
		[ ] // Could not open file
		[ ] // TBD add error handling
		[ ] LogError ("*** ERROR: could not open file {sFilePath}")
		[ ] ExceptPrint()
		[ ] return bReturn
	[ ] 
	[ ] // read each line looking for string
	[-] while (FileReadLine (hFile, sCurrentLine))
		[ ] // keep track of the line number we're on
		[ ] iFoundLineNum++
		[ ] 
		[+] if (iStartLineNum > 0 && (iFoundLineNum < iStartLineNum))
			[ ] // skip to starting line
			[ ] // done with this line; just read next line
		[+] else if (iFoundLineNum > iEndLineNum)
			[ ] // haven't found a match and beyond the end of the section to
			[ ] // search
			[ ] // don't need to read anymore lines
			[ ] break
		[-] else
				[ ] // were looking for the string sSearchString
				[+] if (StrPos (sSearchString, sCurrentLine) != 0)
					[ ] // found a match
					[ ] bReturn = TRUE
					[ ] // don't need to read anymore lines
					[ ] break
			[-] else
				[ ] // just looking for a line by its line number
				[ ] // found line number wanted, just return full line
				[ ] // test that current line number (iFoundLineNum) ==
				[ ] // iStartLineNumber, logic error if not.
				[-] do
					[ ] Verify (iFoundLineNum, iStartLineNum, "Logic Error")
					[ ] Verify (sSearchString, "", "Logic Error")
					[ ] bReturn = TRUE
					[ ] // don't need to read anymore lines
					[ ] break
				[-] except
					[ ] // TBD handle error
					[ ] ExceptPrint()
	[ ] 
	[ ] // close file
	[ ] FileClose (hFile)
	[+] if bReturn
		[ ] sMatchedLine = sCurrentLine
	[+] else 
		[ ] sMatchedLine = NULL
	[ ] return bReturn
[ ] 
[ ] //*****************************************************************************
[ ] // END: fSearchSectionOfFileA() fSearchSectionOfFile()
[ ] //*****************************************************************************
[ ] 
[ ] //*****************************************************************************
[ ] // fWildSearchFileA(), fWildSearchFile()
[ ] // Description:
[ ] //   Finds the specified string in the specified file. If the string contains
[ ] //   the character * or ? it is treated as a "wildcard" with * matching any
[ ] //   string of 0 or more characters and ? matching any single character.
[ ] //   The string to search for must "pattern" the entire line.  So if
[ ] //   looking for 'MyArray[*] in the line 'The value of MyArray[2] is 34'
[ ] //   The string to search for is "*MyArray[*]*".
[ ] //   Also the match is not case sensitive.  Can give a starting
[ ] //   line number as the line number to start the search.  If string is found,
[ ] //   routine returns the line number and line with matching string.  
[ ] //   function is the same as fSearchFileA(), fSearchFile() except 
[ ] //   the silk call MatchStr() is used instead of StrPos() 
[ ] // Returns:
[ ] //   Boolean: TRUE if string is found in file, FALSE otherwise
[ ] // Arguments:
[ ] //  in string sServerIP (fWildSearchFileA())
[ ] //     name or IP address of server on which file is located; 
[ ] //     assume caller has IPC access to server
[ ] //  in string sDrive (fWildSearchFileA())
[ ] //     drive letter on which file exists
[ ] //  in string sPath (fWildSearchFileA())
[ ] //     directory path to file
[ ] //  in string sFilename (fWildSearchFileA())
[ ] //     name of file
[ ] //  in string sFilePath (fWildSearchFile())
[ ] //  in integer iStartLineNum
[ ] //     the line number to start the search on, if 0 start at beginning
[ ] //  in string sSearchString
[ ] //     the string to look for, if empty string, the Line on iStartLineNum
[ ] //     is returned
[ ] //  out integer iFoundLineNum
[ ] //     the line number of the line with the string
[ ] //  out sMatchedLine
[ ] //     the full line that matched the string.  NULL if matched line not found.
[ ] //******************************************************************************
[+] boolean fWildSearchFileA(in string sServerIP, in string sDrive, in string sPath,
        in string sFilename, in integer iStartLineNum, in string sSearchString,
        out integer iFoundLineNum NULL, out string sMatchedLine NULL)
	[ ] 
	[ ] boolean bReturn = FALSE
	[ ] string sFullPath = ""
	[ ] 
	[ ] // create sPath from arguments
	[+] if (sServerIP == "Local")
		[ ] sFullPath = "{sDrive}:\{sPath}\{sFilename}"
	[+] else
		[ ] sFullPath = "\\{sServerIP}\{sDrive}$\{sPath}\{sFilename}"
	[ ] 
	[ ] // call fSearchFile
	[ ] bReturn = fWildSearchFile(sFullPath, iStartLineNum, sSearchString,iFoundLineNum,
        sMatchedLine)
	[ ] return bReturn
[ ] 
[-] boolean fWildSearchFile(in string sFilePath,in integer iStartLineNum, 
        in string sSearchString, out integer iFoundLineNum NULL, 
        out string sMatchedLine NULL)
	[ ] 
	[ ] boolean bReturn = FALSE
	[ ] string sCurrentLine = ""
	[ ] iFoundLineNum = 0
	[ ] HFILE hFile = NULL
	[ ] 
	[+] do
		[ ] hFile = FileOpen (sFilePath,FM_READ,FS_DENY_NONE)
	[-] except
		[ ] // Could not open file
		[ ] // TBD add error handling
		[ ] LogError ("*** ERROR: could not open file {sFilePath}")
		[ ] ExceptPrint()
		[ ] return bReturn
	[ ] 
	[ ] // read each line looking for string
	[-] while (FileReadLine (hFile, sCurrentLine))
		[ ] // keep track of the line number were on
		[ ] iFoundLineNum++
		[ ] 
		[+] if (iStartLineNum > 0 && (iFoundLineNum < iStartLineNum))
			[ ] // skip to starting line
			[ ] // done with this line; just read next line
		[-] else
			[+] if (sSearchString != "")
				[ ] // were looking for the string sSearchString
				[+] if (MatchStr (sSearchString, sCurrentLine))
					[ ] // found a match
					[ ] bReturn = TRUE
					[ ] // don't need to read anymore lines
					[ ] break
			[-] else
				[ ] // just looking for a line by its line number
				[ ] // found line number wanted, just return full line
				[ ] // test that current line number (iFoundLineNum) ==
				[ ] // iStartLineNumber, logic error if not.
				[-] do
					[ ] Verify (iFoundLineNum, iStartLineNum, "Logic Error")
					[ ] Verify (sSearchString, "", "Logic Error")
					[ ] bReturn = TRUE
					[ ] // don't need to read anymore lines
					[ ] break
				[-] except
					[ ] // TBD handle error
					[ ] ExceptPrint()
	[ ] 
	[ ] // close file
	[ ] FileClose (hFile)
	[+] if bReturn
		[ ] sMatchedLine = sCurrentLine
	[+] else 
		[ ] sMatchedLine = NULL
	[ ] return bReturn
[ ] 
[ ] //*****************************************************************************
[ ] // END: fWildSearchFileA() fWildSearchFile()
[ ] //*****************************************************************************
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION    :  fReadWriteIni
[ ] //
[ ] //   ARGUMENTS   :  string sIniFile
[ ] //                  - Identifies the path/name of the initialization
[ ] //                    file (.ini) to read or write to.
[ ] //                  string sOpType
[ ] //                  - Identifies whether the requested operation
[ ] //                    involves reading ot writing to the .ini file.
[ ] //                  string sSection
[ ] //                  - Identifies the section in the .ini file that contains
[ ] //                    the entry to read from or write to.
[ ] //                  string sEntry
[ ] //                  - Identifies the name of entry in the .ini file to read
[ ] //                    from or write into.
[ ] //                  string sWhatToWrite
[ ] //                  - On a write operation, the string to write to the .ini
[ ] //                    file. On a read any passed-in value is ignored.
[ ] //
[ ] //   RETURNS     :  string -
[ ] //									On a read operation, the function returns the requested
[ ] //                  string. On a write operation, it returns the standard
[ ] //									success code "true" as a string rather than a boolean.
[ ] //   DESCRIPTION :  Used to read and write values from the specified initiali-
[ ] //                  zation file.
[ ] //									
[ ] //*****************************************************************************
[ ] 
[-] public string fReadWriteIni (string sIniFile, string sOpType, string sSection,
    string sEntry, string sWhatToWrite)
	[ ] 
	[ ] HANDLE hIni
	[ ] string sRetStr
	[ ] boolean bTest
	[ ] list of string lsContents
	[ ] 
	[ ] // If the specified INI file does not exist, log the err and exit the testcase.
	[-] if ! SYS_FileExists(sIniFile)
		[ ] // Fail the testcase and log the error in the PF log.
		[ ] LogError("*** ERROR: File "+sIniFile+" does not exist")
		[ ] fLogPassFail(csFailed,sIniFile,cerrIniNotFound)
		[ ] 
	[ ] 
	[ ] // Check the OpType. Must be sRead or sWrite or log the err and exit the testcase.
	[-] if (sOpType != csRead && sOpType != csWrite)
		[ ] // Fail the testcase and log the error in the PF log.
		[ ] fLogPassFail(csFailed,cmsgExpReadOrWrite,cerrBadOpType)
	[ ] 
	[ ] // Get a handle for the specified .ini file. If ini file does not exists, 4Test
    // creates an empty one by default. Not what we want.
	[ ] hIni = IniFileOpen(sIniFile)
	[ ] // If we passed in the name of a non-existent ini file and 4Test
	[ ] // made an empty one, we need to zap that and exit the test case.
	[ ] lsContents = SYS_GetFileContents (sIniFile)
	[-] if lsContents == NULL
		[ ] // Close the ini file.
		[ ] IniFileClose(hIni)
		[ ] // Kill the empty ini file.
		[ ] SYS_RemoveFile(sIniFile)
		[ ] // Show error
		[ ] // LogError("**************** ERROR *****************")
		[ ] LogError("*** File "+sIniFile+" is empty")
		[ ] // Fail the testcase and log the error in the PF log.
		[ ] fLogPassFail(csFailed,sIniFile,cerrAutoGenIni)
	[ ] 
	[ ] // Process the 'read' or 'write' accordingly.
	[-] switch (sOpType)
		[ ] 
		[ ] // Read in the value, close the .ini and return the value to the calling
		[ ] // program.
		[-] case csRead
			[ ] // Read in the value.
			[ ] sRetStr = RTrim(IniFileGetValue(hIni,sSection,sEntry))
			[ ] 
			[ ] //If the value or the section was not found, IniFileGetValue returns an empty string.
			[ ] // Fail the testcase.
			[-] if sRetStr == ""
				[ ] // Close the ini file.
				[ ] IniFileClose(hIni)
				[ ] // Show error
				[ ] LogError("*** ERROR: File "+sIniFile+" does not contain section or value: "+sSection+"/"+sEntry)
				[ ] // Fail the testcase and log the error in the PF log.
				[ ] fLogPassFail(csFailed,cmsgExpValidRetVal,cerrBadSectionOrValue)
			[ ] 
			[ ] // Close the ini file.
			[ ] IniFileClose(hIni)
			[ ] 
			[ ] // Return the requested value.
			[ ] return (sRetStr)
		[ ] 
		[ ] // Write the value, close the .ini and return sPass to calling program.
		[ ] // If the specified section or value doesn’t exist, IniFileSetValue adds it.
		[-] case csWrite
			[ ] IniFileSetValue(hIni,sSection,sEntry,sWhatToWrite)
			[ ] // Close the ini file
			[ ] IniFileClose(hIni)
			[ ] // Return the success code.
			[ ] return ("true")
[ ] 
[ ] //*****************************************************************************
[ ] // END: fReadWriteIni
[ ] //*****************************************************************************
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION    :  fFileLineCount
[ ] //
[ ] //   ARGUMENTS   :  string sFileAndPathName
[ ] //                  - Identifies the path/name of the file.
[ ] //
[ ] //   RETURNS     :  integer -
[ ] //									The number of lines in the file. On error, 0 is returned.
[ ] //
[ ] //   OPTIONAL
[ ] //   RETURNS		 :	string - 
[ ] //									On error, the error string gets set.  It's up to the caller
[ ] //									to determine whether or not it would like to receive the error
[ ] //									string back.
[ ] //
[ ] //   DESCRIPTION :  Opens the specified file and counts the number of lines
[ ] //                  contained within the file.  If an error is encountered, the
[ ] // 									function returns a 0 and sets the error string for display
[ ] //									by the caller.
[ ] //									
[ ] //*****************************************************************************
[ ] 
[-] integer fFileLineCount(in string sFilePathAndName, out string sError optional)
	[ ] 
	[ ] HFILE hFile
	[ ] INTEGER iLineCount = 0
	[ ] STRING sCurrentLine = ""
	[ ] 
	[ ] // Determine if the file actually exists at the specified path
	[-] if (!SYS_FileExists (sFilePathAndName))
		[ ] iLineCount = 0
		[ ] sError = "Error: "+sFilePathAndName+" does not exist at the specified location."
		[ ] return iLineCount
	[ ] 
	[ ] // Create a READ handle to the file 
	[ ] hFile = FileOpen (sFilePathAndName, FM_READ)
	[ ] 
	[ ] // Loop over the contents of the file reading line-by-line
	[+] while (FileReadLine (hFile, sCurrentLine))
		[ ] iLineCount++
		[ ] 
	[ ] // close the open file handle.
	[ ] FileClose(hFile)
	[ ] 
	[ ] 
	[ ] return iLineCount
	[ ] 
[ ] //*****************************************************************************
[ ] // END: fFileLineCount
[ ] //*****************************************************************************
[ ] 
[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION    :  fFileSize
[ ] //
[ ] //   ARGUMENTS   :  string sFileAndPathName
[ ] //                  - Identifies the path/name of the file.
[ ] //
[ ] //   RETURNS     :  integer -
[ ] //									The size of the file, in bytes.  On error, 0 is returned.
[ ] //
[ ] //   OPTIONAL
[ ] //   RETURNS		 :	string - 
[ ] //									On error, the error string gets set.  It's up to the caller
[ ] //									to determine whether or not it would like to receive the error
[ ] //									string back.
[ ] //
[ ] //   DESCRIPTION :  First, the variable sFileAndPathName is broken down into
[ ] //									the path of the file and the filename itself.  Using
[ ] //									SYS_GetDirContents() we are able to use the FILEINFO structure
[ ] //									provided by silk to extract the information required.
[ ] //									
[ ] //*****************************************************************************
[ ] 
[+] integer fFileSize(in string sFilePathAndName, out string sError optional)
	[ ] 
	[ ] LIST OF FILEINFO lfFiles
	[ ] INTEGER iCount = 0
	[ ] INTEGER iTempInt = 0
	[ ] STRING sFilePath = ""
	[ ] STRING sFileName = ""
	[ ] 
	[ ] 
	[ ] // Determine if the file exists before we continue...
	[+] if (!SYS_FileExists (sFilePathAndName))
		[ ] sError = "Error: "+sFilePathAndName+" does not exist at the specified location."
		[ ] return 0
	[ ] 
	[ ] // Determine where the first backslash exists in the string and break it here
	[ ] iTempInt = strPos("\", sFilePathAndName, TRUE)
	[ ] // Parse the File Path
	[ ] sFilePath = SubStr (sFilePathAndName, 1, iTempInt)
	[ ] // Parse the File Name
	[ ] sFileName = SubStr (sFilePathAndName, (iTempInt+1))
	[ ] 
	[ ] // Get the contents of the directory in the path
	[ ] lfFiles = SYS_GetDirContents (sFilePath)
	[+] for iCount = 1 to ListCount (lfFiles)
		[+] if((lfFiles[iCount].bIsDir == FALSE) && (lfFiles[iCount].sName == sFileName))
			[ ] return lfFiles[iCount].iSize
	[ ] 
	[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] // END: fFileSize
[ ] //*****************************************************************************
[ ] 
[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION    :  fFileTimeStamp
[ ] //
[ ] //   ARGUMENTS   :  string sFileAndPathName
[ ] //                  - Identifies the path/name of the file.
[ ] 
[ ] //									integer iTimeType
[ ] //									- Determines wether or not the caller wants to receive
[ ] //										Creation time or modified time.
[ ] //										1 = Creation time is returned
[ ] // 										2 = Modified time is returned
[ ] //
[ ] //									integer iReturnTimeFormat
[ ] //									- The format the caller wants the time returned as:
[ ] //										1 = Date and time returned as mm-dd-yy@hhmmss.
[ ] //												Use this to create unique backup filenames.
[ ] // 										2 = Date and time returned as yyyymmddhhmmss
[ ] //										3 = Date and time returned as yyyy-mm-dd hh.nn.ss.fff.
[ ] //										4 = Default; Date and time returned as
[ ] //												MM/DD/YY HH:MM:SS AM/PM
[ ] //
[ ] //   RETURNS     :  string -
[ ] //									The timestamp of the file in the specified format.  On Error,
[ ] //									NULL is returned.
[ ] //
[ ] //   OPTIONAL
[ ] //   RETURNS		 :	string - 
[ ] //									On error, the error string gets set.  It's up to the caller
[ ] //									to determine whether or not it would like to receive the error
[ ] //									string back.
[ ] //
[ ] //   DESCRIPTION :  First, the variable sFileAndPathName is broken down into
[ ] //									the path of the file and the filename itself.  Using
[ ] //									SYS_GetDirContents() we are able to use the FILEINFO structure
[ ] //									provided by silk to extract the information required.
[ ] //									
[ ] //*****************************************************************************
[ ] 
[-] string fFileTimeStamp(in string sFilePathAndName, in integer iTimeType, in integer iReturnTimeFormat, out string sError optional)
	[ ] 
	[ ] LIST OF FILEINFO lfFiles
	[ ] DATETIME dtFileDateTime
	[ ] INTEGER iCount = 0
	[ ] INTEGER iTempInt = 0
	[ ] STRING sFilePath = ""
	[ ] STRING sFileName = ""
	[ ] STRING sFileTimeStamp = ""
	[ ] 
	[ ] // Determine if the file exists before we continue...
	[+] if (!SYS_FileExists (sFilePathAndName))
		[ ] sError = "Error: "+sFilePathAndName+" does not exist at the specified location."
		[ ] return NULL
	[ ] 
	[ ] // Determine where the first backslash exists in the string and break it here
	[ ] iTempInt = strPos("\", sFilePathAndName, TRUE)
	[ ] // Parse the File Path
	[ ] sFilePath = SubStr (sFilePathAndName, 1, iTempInt)
	[ ] // Parse the File Name
	[ ] sFileName = SubStr (sFilePathAndName, (iTempInt+1))
	[ ] 
	[ ] // Get the contents of the directory in the path
	[ ] lfFiles = SYS_GetDirContents (sFilePath)
	[-] for iCount = 1 to ListCount (lfFiles)
		[-] if((lfFiles[iCount].bIsDir == FALSE) && ( lower(lfFiles[iCount].sName) == lower(sFileName) ))
			[+] if(iTimeType == 1)
				[ ] dtFileDateTime = lfFiles[iCount].dtCreationTime
			[+] else if(iTimeType == 2)
				[ ] dtFileDateTime = lfFiles[iCount].dtLastModifyTime
			[+] else
				[ ] sError = "Error: Incorrect File Time Format Type"
				[ ] return NULL
	[ ] 
	[ ] // Return the time in the requested format
	[+] switch (iReturnTimeFormat)
		[ ] // A format that can be used to create a unique DOS file name.
		[+] case 1  
			[ ] sFileTimeStamp = FormatDateTime(dtFileDateTime, "mm-dd-yy@hhmmss")
		[ ] // No symbols
		[+] case 2 
			[ ] sFileTimeStamp = FormatDateTime(dtFileDateTime,"yyyymmddhhmmss")
		[ ] // A format the same as the format returned from Silk's SYS_GetDirContents function.
		[+] case 3 
			[ ] sFileTimeStamp = FormatDateTime(dtFileDateTime,"yyyy-mm-dd hh.nn.ss.fff")
		[ ] // default format = MM/DD/YY HH:MM:SS AM/PM
		[+] default
			[ ] sFileTimeStamp = FormatDateTime(dtFileDateTime)
	[ ] 
	[ ] return sFileTimeStamp
	[ ] 
	[ ] 
[ ] //*****************************************************************************
[ ] // END: fFileTimeStamp
[ ] //*****************************************************************************
[ ] 
