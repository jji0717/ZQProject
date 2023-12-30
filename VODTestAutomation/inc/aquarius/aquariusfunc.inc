[ ] //******************************************************************************
[ ] //  NAME:               AQUARIUSFUNC.INC
[ ] //
[ ] //  DESCRIPTION:        Defines Aquarius specific functions/routines
[ ] //
[ ] //  HISTORY:
[ ] //
[ ] //  Date            Developer         Description
[ ] //  ********        ***********       ******************************************
[ ] //  12/13/02        C. Callahan       Created
[ ] //	12/31/02        C. Callahan       Routines to Parse SessionPl.log file
[ ] //  02/05/03        C. Callahan       Initial design and development.  First
[ ] //                                    testcase Aquarius001 runs and use
[ ] //                                    all the functions defined.  Enhancements
[ ] //                                    can be made to what is written. See docs 
[ ] //                                    in QA Lotus Notes db for detailed status.
[ ] //******************************************************************************
[ ] //111111102222222220333333333044444444405555555550666666666077777777708888888880
[ ] //******************************************************************************
[ ] //
[ ] // Module contains these functions:
[ ] //  NAME                    STATUS (peer reviewed or not)
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :		
[ ] //
[ ] //   ARGUMENTS   :    None
[ ] //
[ ] //   RETURNS     :    boolean -
[ ] //                      False	= if the operation failed.
[ ] //                      True = if the operation was successful.
[ ] //   DESCRIPTION :    
[ ] //
[ ] //******************************************************************************
[ ] //******************************************************************************
[ ] // END: FUNCTION 
[ ] //******************************************************************************
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fTrace()
[ ] //
[ ] //   ARGUMENTS   :    String
[ ] //                    boolean optional - string is printed to log file
[ ] //                    only if this optional boolean is passed in and
[ ] //                    TRUE.
[ ] //
[ ] //   RETURNS     :    VOID
[ ] 
[ ] //   DESCRIPTION :    Prints the string to the SilkTest Results file, prefaced
[ ] //                    by the string TRACE:
[ ] //
[ ] //******************************************************************************
[-] VOID fTrace (string sString,boolean bTrace NULL optional)
	[-] if !IsNull(bTrace)
		[ ] if bTrace
		[ ] // Print ("TRACE: {sString}")
		[ ] Print ("{sString}")
[ ] //******************************************************************************
[ ] // END: fTrace() 
[ ] //******************************************************************************
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fGetSiteInfo()	
[ ] //
[ ] //   ARGUMENTS   :    boolean optional TRUE to Trace results in log file,
[ ] //                        results not traced if FALSE or omitted
[ ] //
[ ] //   RETURNS     :    boolean -
[ ] //                      FALSE	= if the operation failed.
[ ] //                      TRUE = if the operation was successful.
[ ] //   DESCRIPTION :    uses global variables to store the IP 
[ ] //                    address of each [logical] Command Center Node, assuming
[ ] //                    an 8 node system and an Asset Manager.  Uses a 
[ ] //                    global list to store the IP address of each
[ ] //                    Video Server. Information is read from the site ini file
[ ] //                    specified by gsITVSiteIni.
[ ] //                    global variables are assumed to exist
[ ] //******************************************************************************
[+] boolean fGetSiteInfo(boolean bTrace optional)
	[ ] boolean bReturn = FALSE
	[ ] string sVideoServerEnt, sNextVS
	[ ] integer iField = 1
	[-] do
		[ ] // get command center IPs
		[ ] gsMDS1IP = fReadWriteIni(gsZodiacIni,csRead,secCommandCenter, entMDS1IP, "")
		[ ] gsMDS2IP = fReadWriteIni(gsZodiacIni,csRead,secCommandCenter, entMDS2IP, "")
		[ ] gsPS1IP = fReadWriteIni(gsZodiacIni,csRead,secCommandCenter, entPS1IP, "")
		[ ] gsPS2IP = fReadWriteIni(gsZodiacIni,csRead,secCommandCenter, entPS2IP, "")
		[ ] gsAPP1IP = fReadWriteIni(gsZodiacIni,csRead,secCommandCenter, entAPP1IP, "")
		[ ] gsAPP2IP = fReadWriteIni(gsZodiacIni,csRead,secCommandCenter, entAPP2IP, "")
		[ ] gsCM1IP = fReadWriteIni(gsZodiacIni,csRead,secCommandCenter, entCM1IP, "")
		[ ] gsCM2IP = fReadWriteIni(gsZodiacIni,csRead,secCommandCenter, entCM2IP, "")
		[ ] gsAM1IP = fReadWriteIni(gsZodiacIni,csRead,secCommandCenter, entAM1IP, "")
		[ ] 
		[ ] // Trace
		[ ] fTrace("gsMDS1IP: {gsMDS1IP}", bTrace)
		[ ] fTrace("gsMDS2IP: {gsMDS2IP}", bTrace)
		[ ] fTrace("gsPS1IP: {gsPS1IP}", bTrace)
		[ ] fTrace("gsPS2IP: {gsPS2IP}", bTrace)
		[ ] fTrace("gsAPP1IP: {gsAPP1IP}", bTrace)
		[ ] fTrace("gsAPP2IP: {gsAPP2IP}", bTrace)
		[ ] fTrace("gsCM1IP: {gsCM1IP}", bTrace)
		[ ] fTrace("gsCM2IP: {gsCM2IP}", bTrace)
		[ ] fTrace("gsAM1IP: {gsAM1IP}", bTrace)
		[ ] 
		[ ] // get video server IPs
		[ ] // get list of Entry names in ini file
		[ ] sVideoServerEnt = fReadWriteIni(gsZodiacIni,csRead,secVideoServers, entVideoServers, "")
		[ ] // Get first in list
		[ ] sNextVS = GetField(sVideoServerEnt,',',iField)
		[ ] // for each entry in list, get the value of the ini file entry
		[-] while sNextVS != ""
			[ ] // Trim any white space
			[ ] sNextVS = Trim (sNextVS)
			[ ] // if there are no video servers list should specify None
			[-] if (Lower(sNextVS) == "none")
				[ ] break
			[ ] fTrace("sNextVS = {sNextVS}", bTrace)
			[ ] ListAppend(lsVideoServersIP, fReadWriteIni(gsZodiacIni,csRead,"VideoServers", sNextVS,""))
			[ ] iField++
			[ ] sNextVS = GetField(sVideoServerEnt,',',iField)
		[ ] fTrace("Number of VideoServer Nodes: {ListCount(lsVideoServersIP)}",bTrace)
		[ ] fTrace("VideoServer IPs: {lsVideoServersIP}", bTrace)
		[ ] bReturn = TRUE
	[-] except
		[ ] reraise
	[ ] 
	[ ] return bReturn
[ ] //******************************************************************************
[ ] // END: fGetSiteInfo()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fGetSiteIPCAccess
[ ] //
[ ] //   ARGUMENTS   :    boolean optional if TRUE will Trace results to 
[ ] //                    SilkTest results file.  No tracing if FALSE or ommitted.
[ ] //
[ ] //   RETURNS     :    boolean -
[ ] //                      False	= if IPC access is denied to any site node.
[ ] //                      True = if the operation was successful.
[ ] //   DESCRIPTION :    Acquires IPC access to each Site node via its IP address
[ ] //                    Uses Account name and Password specified in Site ini 
[ ] //                    file.  There is one for the Command Center Nodes
[ ] //                    and one for Video Server nodes.
[ ] //******************************************************************************
[+] boolean fGetSiteIPCAccess(boolean bTrace optional)
	[ ] boolean bReturn = TRUE
	[ ] // get actual values from ini file
	[ ] string sAccount = "SeaChange"
	[ ] string sPassword = "SeaChange"
	[ ] list of string lsCmdOutput
	[ ] string sIP=""
	[ ] 
	[ ] // get Command Center user name and password for IPC access
	[ ] sAccount = fReadWriteIni(gsZodiacIni,csRead,secCommandCenter, entAccount, "")
	[ ] sPassword = fReadWriteIni(gsZodiacIni,csRead,secCommandCenter, entPassword, "")
	[ ] // get IPC access for command center nodes
	[-] do
		[ ] Verify(fGetIPCAccess(gsMDS1IP, sAccount, sPassword, lsCmdOutput), TRUE)
	[-] except
		[ ] bReturn = FALSE
		[ ] LogWarning("*** Warning: Failed to get IPC Access to IP: {gsMDS1IP}")
		[-] if bTrace != NULL
			[-] if bTrace
				[ ] fTrace("net use returned {lsCmdOutput}")
				[ ] lsCmdOutput = {""}
	[-] do
		[ ] Verify(fGetIPCAccess(gsMDS2IP, sAccount, sPassword, lsCmdOutput), TRUE)
	[-] except
		[ ] bReturn = FALSE
		[ ] LogWarning("*** Warning: Failed to get IPC Access to IP: {gsMDS2IP}")
		[+] if bTrace != NULL
			[+] if bTrace
				[ ] fTrace("net use returned {lsCmdOutput}")
				[ ] lsCmdOutput = {""}
	[-] do
		[ ] Verify(fGetIPCAccess(gsPS1IP, sAccount, sPassword, lsCmdOutput), TRUE)
	[-] except
		[ ] bReturn = FALSE
		[ ] LogWarning("*** Warning: Failed to get IPC Access to IP: {gsPS1IP}")
		[+] if bTrace != NULL
			[+] if bTrace
				[ ] fTrace("net use returned {lsCmdOutput}")
				[ ] lsCmdOutput = {""}
	[-] do
		[ ] Verify(fGetIPCAccess(gsPS2IP, sAccount, sPassword, lsCmdOutput), TRUE)
	[-] except
		[ ] bReturn = FALSE
		[ ] LogWarning("*** Warning: Failed to get IPC Access to IP: {gsPS2IP}")
		[+] if bTrace != NULL
			[+] if bTrace
				[ ] fTrace("net use returned {lsCmdOutput}")
				[ ] lsCmdOutput = {""}
	[-] do
		[ ] Verify(fGetIPCAccess(gsAPP1IP, sAccount, sPassword, lsCmdOutput), TRUE)
	[-] except
		[ ] bReturn = FALSE
		[ ] LogWarning("*** Warning: Failed to get IPC Access to IP: {gsAPP1IP}")
		[+] if bTrace != NULL
			[+] if bTrace
				[ ] fTrace("net use returned {lsCmdOutput}")
				[ ] lsCmdOutput = {""}
	[-] do
		[ ] Verify(fGetIPCAccess(gsAPP2IP, sAccount, sPassword, lsCmdOutput), TRUE)
	[-] except
		[ ] bReturn = FALSE
		[ ] LogWarning("*** Warning: Failed to get IPC Access to IP: {gsAPP2IP}")
		[+] if bTrace != NULL
			[+] if bTrace
				[ ] fTrace("net use returned {lsCmdOutput}")
				[ ] lsCmdOutput = {""}
	[-] do
		[ ] Verify(fGetIPCAccess(gsCM1IP, sAccount, sPassword, lsCmdOutput), TRUE)
	[-] except
		[ ] bReturn = FALSE
		[ ] LogWarning("*** Warning: Failed to get IPC Access to IP: {gsCM1IP}")
		[+] if bTrace != NULL
			[+] if bTrace
				[ ] fTrace("net use returned {lsCmdOutput}")
				[ ] lsCmdOutput = {""}
	[-] do
		[ ] Verify(fGetIPCAccess(gsCM2IP, sAccount, sPassword, lsCmdOutput), TRUE)
	[-] except
		[ ] bReturn = FALSE
		[ ] LogWarning("*** Warning: Failed to get IPC Access to IP: {gsCM2IP}")
		[+] if bTrace != NULL
			[+] if bTrace
				[ ] fTrace("net use returned {lsCmdOutput}")
				[ ] lsCmdOutput = {""}
	[-] do
		[ ] Verify(fGetIPCAccess(gsAM1IP, sAccount, sPassword, lsCmdOutput), TRUE)
	[-] except
		[ ] bReturn = FALSE
		[ ] LogWarning("*** Warning: Failed to get IPC Access to IP: {gsAM1IP}")
		[+] if bTrace != NULL
			[+] if bTrace
				[ ] fTrace("net use returned {lsCmdOutput}")
				[ ] lsCmdOutput = {""}
	[ ] 
	[ ] // get Video Server user name and password for IPC access
	[ ] sAccount = fReadWriteIni(gsZodiacIni,csRead,secVideoServers, entAccount, "")
	[ ] sPassword = fReadWriteIni(gsZodiacIni,csRead,secVideoServers, entPassword, "")
	[ ] // get IPC access for Video Server nodes
	[-] for each sIP in lsVideoServersIP
		[-] do
			[ ] Verify(fGetIPCAccess(sIP, sAccount, sPassword, lsCmdOutput), TRUE)
		[-] except
			[ ] bReturn = FALSE
			[ ] LogWarning("*** Warning: Failed to get IPC Access to IP: {sIP}")
			[-] if bTrace != NULL
				[-] if bTrace
					[ ] fTrace("net use returned {lsCmdOutput}")
					[ ] lsCmdOutput = {""}
	[ ] 
	[ ] return bReturn
[ ] //******************************************************************************
[ ] // END: fGetSiteIPCAccess()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fSearchSessionPlLog()	
[ ] //
[ ] //   ARGUMENTS   :    in string - name of service of log records to search
[ ] //                    in string - the full path to the Session pl file to search
[ ] //                    in boolean optional - print the Trace statements or not
[ ] //
[ ] //   RETURNS     :    boolean -
[ ] //                      False	= if the operation failed.
[ ] //                      True = if the operation was successful.
[ ] //   DESCRIPTION :    Search the Session Pl log file for the strings associated
[ ] //                      with a given service's log file. Looks for strings
[ ] //                      within the log messages pulled from one log file.
[ ] //                      The strings are specified in aquariusdefs.inc.  They
[ ] //                      are searched for in the order defined.
[ ] //******************************************************************************
[ ] 
[+] boolean fSearchSessionPlLog (string sService, string sFile, inout rtSessionInfo rSession, boolean bTrace optional)
	[ ] boolean bReturn = TRUE
	[ ] boolean bFound = FALSE
	[ ] list of rtLogMessage lrLogStrings
	[ ] rSessionLogMarker rLogMarker
	[ ] rtLogMessage rKey
	[ ] 
	[ ] integer iStart = 0
	[ ] integer iCurStart = 0
	[ ] integer iStop = 0
	[ ] integer iCurStop = 0
	[ ] integer iCurLine = 0
	[ ] string sCurLine = ""
	[ ] string sKey = ""
	[ ] 
	[ ] // if file does not exist, just return
	[-] do
		[ ] Verify (SYS_FileExists(sFile), TRUE)
	[-] except
		[ ] bReturn = FALSE
		[ ] // LogError("Could not access Session Pl Output file {sFile}")
		[ ] // return bReturn
		[ ] fLogPassFail(csFailed,"Can access Session Pl output file ","No access to {sFile}")
	[ ] 
	[-] switch sService
		[-] case "SRM"
			[ ] lrLogStrings = clrSRMLog
			[ ] rLogMarker = crSRMMarker
		[+] case "CM"
			[ ] lrLogStrings = clrCMLog
			[ ] rLogMarker = crCMMarker
		[+] case "DMN"
			[ ] lrLogStrings = clrDMNLog
			[ ] rLogMarker = crDMNMarker
		[+] case "SSAPI"
			[ ] lrLogStrings = clrSSApiLog
			[ ] rLogMarker = crSSApiMarker
		[+] case "SS"
			[ ] lrLogStrings = clrSSLog
			[ ] rLogMarker = crSSMarker
		[+] case "MOD"
			[ ] lrLogStrings = clrMODLog
			[ ] rLogMarker = crMODMarker
		[+] case "MCACLIENT"
			[ ] lrLogStrings = clrMCAClientLog
			[ ] rLogMarker = crMCAClientMarker
		[+] case "MCASERVER"
			[ ] lrLogStrings = clrMCAServerLog
			[ ] rLogMarker = crMCAServerMarker
		[+] default
			[ ] bReturn = FALSE
			[ ] LogError ("Unexpected Log type: {sService}")
			[ ] return bReturn
	[ ] fTrace("***************Serarching {sService}'s log file***************",true)
	[ ] 
	[ ] // Search the Session Pl log file for the strings associated with a given
	[ ] // log file
	[ ] 
	[ ] // while haven't found a match
	[-] while !bFound
		[ ] // Find start and stop marker for given service in session pl log file
		[+] do 
			[ ] boolean bSearch = FALSE
			[ ] bSearch = fSearchFile(sFile,iCurLine,rLogMarker.sStart,iStart,sCurLine)
			[ ] Verify(bSearch,TRUE)
			[ ] iCurLine = iStart+1
		[+] except
			[ ] // no more start/stop blocks to choose from
			[ ] bReturn = FALSE
			[ ] LogWarning ("*** Warning: Didn't find start marker {rLogMarker.sStart} in {sFile}")
			[ ] return bReturn
		[ ] 
		[ ] // found start,now find stop
		[+] do 
			[ ] Verify(fSearchFile(sFile,iCurLine,rLogMarker.sStop,iStop,sCurLine),TRUE)
			[ ] iCurLine = iStop+1
		[+] except
			[ ] // if found start but no stop, search to end of file
			[ ] LogWarning ("*** Warning: Didn't find stop marker {rLogMarker.sStop} in {sFile}")
		[ ] 
		[-] for each rKey in lrLogStrings
			[ ] sKey = rKey.sPostSessionId
			[ ] fTrace("Searching section from {iStart+1} to {iStop}", bTrace)
			[ ] iCurLine = iStart+1
			[-] do
				[ ] // find the messages in the order specified, so change iStart
				[ ] Verify(fSearchSectionOfFile(sFile,iCurLine,iStop, sKey,iCurLine, sCurLine), TRUE)
				[ ] fTrace ("Found string {sKey} at Line {iCurLine}", bTrace)
				[ ] fTrace ("Matched line: {sCurLine} {csNL}", bTrace)
				[ ] bFound = TRUE
				[-] if rKey.sParseFunction != ""
					[ ] // call the function named with the session data structure, the string and a trace flag
					[-] do
						[ ] rSession = ArgListCall(rKey.sParseFunction, {rSession, sCurLine, bTrace})
						[ ] fTrace("{rSession.sId} {rSession.sAssetId} {rSession.bStatus} {rSession.eSledgehammerState} {rSession.sPurchaseTime}", bTrace)
						[ ] Verify(rSession.bStatus, TRUE)
					[-] except
						[ ] LogWarning("*** Warning: Not able to parse msg for needed info: {sCurLine}")
			[-] except
				[ ] // not finding message is okay if haven't found any yet - so ignore
				[ ] // but if found one should be concerned about the others
				[-] if bFound
					[ ] LogWarning ("*** Warning: Did not find {sKey} in {sService} Log between {iStart} and {iStop}")
		[ ] 
		[ ] // TRACE that Key messages were not found for this start stop block
		[ ] // there may be more start stop blocks to choose from
		[-] if !bFound
			[ ] fTrace("Did not find any {sService} messages between lines {iStart} and {iStop}", bTrace)
	[ ] 
	[ ] return bReturn
[ ] 
[ ] //******************************************************************************
[ ] // END: fSearchSessionPlLog()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fGetServices()
[ ] //
[ ] //   ARGUMENTS   :    in string:  name of test case so can look up info 
[ ] //                    for test case in the aquariuscfg.ini file
[ ] //                    out list of string: the list of strings of the
[ ] //                    service logs to look at for this test case
[ ] //                    optional boolean: trace the steps or NOT
[ ] //
[ ] //   RETURNS     :    boolean -
[ ] //                      False	= if the operation failed.
[ ] //                      True = if the operation was successful.
[ ] //   DESCRIPTION :    reads the entry entServiceLogs in the test case
[ ] //                    section of the ini file and sets a list of strings
[ ] //                    to match the comma seperated list specified
[ ] //                    The list can be "none" or "default". If none, then no log
[ ] //                    message willbe checked.  If default, then a default list
[ ] //                    is used.
[ ] //******************************************************************************
[-] boolean fGetServices(in string sTestCase, out list of STRING lsServices,
              in boolean bTrace optional)
	[ ] boolean bReturn = FALSE
	[ ] string sServices
	[ ] string sNext
	[ ] integer iField = 1
	[ ] 
	[-] do
		[ ] // get list of services from aquarius ini file
		[ ] sServices = fReadWriteIni(gsAquariusIni,csRead,sTestCase,entITVServices, "")
		[ ] // Get first in list
		[ ] sNext = GetField(sServices,',',iField)
		[+] while Trim(sNext) != ""
			[ ] // Trim any white space, and Upper Case
			[ ] sNext = Upper (Trim (sNext))
			[ ] // if there are no services list should specify None
			[+] if (sNext == "NONE")
				[ ] bReturn = TRUE
				[ ] break
			[ ] // if should use default, list should specify Default
			[+] if (sNext == "DEFAULT")
				[ ] break
			[ ] fTrace("sNext = {sNext}", bTrace)
			[ ] ListAppend(lsServices, sNext)
			[ ] iField++
			[ ] sNext = GetField(sServices,',',iField)
		[ ] fTrace("Number of Services: {ListCount(lsServices)}",bTrace)
		[ ] fTrace("Services: {lsServices}", bTrace)
		[ ] bReturn = TRUE
	[-] except
		[ ] // just return FALSE
		[ ] // just log where failure occurred - no error logged for testcase
		[ ] ExceptPrint()
	[ ] return bReturn
[ ] //******************************************************************************
[ ] // END: fGetServices()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fBuildSledgehammerXML
[ ] //
[ ] //   ARGUMENTS   :    in string:  name of test case so can look up info for test case
[ ] //                    in the aquariuscfg.ini file
[ ] //                    inout rtSledgehammer: a data structure that keeps
[ ] //                     info about the Sledgehammer run
[ ] //                    optional boolean: trace the steps or NOT
[ ] //
[ ] //   RETURNS     :    boolean -
[ ] //                      False	= if the operation failed.
[ ] //                      True = if the operation was successful.
[ ] //   DESCRIPTION :    
[ ] //
[ ] //******************************************************************************
[-] boolean fBuildSledgehammerXML(in string sTestCaseName, inout rtSledgehammer rInfo, boolean bTrace optional)
	[ ] boolean bReturn = FALSE
	[ ] 
	[ ] rInfo.eState = initialized
	[ ] rInfo.sXMLFile = fReadWriteIni(gsAquariusIni,csRead,sTestCaseName, entSledgehammerXML, "")
	[ ] rInfo.sXMLFile = "{csInstallPath}{dirAquariusIni}{rInfo.sXMLFile}"
	[ ] rInfo.sLog = "{gsTestFileRoot}{filSledgehammerLog}"
	[ ] fTrace("rInfo.sXMLFile: {rInfo.sXMLFile}",bTrace)
	[ ] fTrace("rInfo.sLog: {rInfo.sLog}", bTrace)
	[ ] bReturn = TRUE
	[ ] 
	[ ] return bReturn
[ ] 
[ ] //******************************************************************************
[ ] // END: fBuildSledgehammerXML
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fRunSledgehammer
[ ] //
[ ] //   ARGUMENTS   :    in string:  name of test case so can look up info for test case
[ ] //                    in the aquariuscfg.ini file
[ ] //                    inout rtSledgehammer: a data structure that keeps
[ ] //                     info about the Sledgehammer run
[ ] //                    optional boolean: trace the steps or NOT
[ ] //
[ ] //   RETURNS     :    boolean -
[ ] //                      False	= if the operation failed.
[ ] //                      True = if the operation was successful.
[ ] //   DESCRIPTION :    
[ ] //
[ ] //******************************************************************************
[-] boolean fRunSledgehammer(in string sTestCaseName, inout rtSledgehammer rInfo, boolean bTrace optional)
	[ ] boolean bReturn = FALSE
	[ ] string sExe
	[ ] boolean bLog = FALSE
	[ ] 
	[ ] // Backup the Sledgehammer log file.  If it doesn't exist - then no
	[ ] // need to back it up.
	[-] do
		[ ] Verify(SYS_FileExists(rInfo.sLog), FALSE)
	[-] except
		[ ] // Backup the previous pass/fail log.
		[ ] bLog = fCreateBackupFile(rInfo.sLog,"{rInfo.sLog}_{(fCreateDateTimeStamp(2))}")
		[-] if !bLog
			[ ] // can not back it up -- so delete it
			[-] if SYS_FileExists(rInfo.sLog)
				[ ] SYS_RemoveFile(rInfo.sLog)
			[-] do
				[ ] Verify(SYS_FileExists(rInfo.sLog), FALSE)
			[-] except
				[ ] // need a clean log file to start - if can't get one then fail
				[ ] bReturn = FALSE
				[ ] ExceptLog()
				[ ] return bReturn
	[ ] 
	[ ] rInfo.eState = starting
	[ ] // get executable path and name 
	[ ] sExe = fReadWriteIni(gsAquariusIni,csRead,secSledgehammer, entExecutable, "")
	[ ] // run using Sys_Execute().  Control does not return to the script until
	[ ] // this finishes.  Want to change to use MainWinn.Start() method so control
	[ ] // returns to Silk sooner.
	[-] do
		[ ] // by rommy modified begin
		[ ] // SYS_SetDrive("c")
		[ ] gsLocalITVRoot = fReadWriteIni(gsZodiacIni,csRead,secITVRoot, entLocalITVRoot, "")
		[ ] SYS_SetDir(gsLocalITVRoot+"exe")
		[ ] // by rommy modified end
		[ ] Verify(SYS_Execute("start {sExe} -l -f {rInfo.sXMLFile}"),0)
		[ ] rInfo.eState = running
		[ ] bReturn = TRUE
	[-] except
		[ ] reraise
	[ ] 
	[ ] return bReturn
[ ] //******************************************************************************
[ ] // END: fRunSledgehammer()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fSledgehammerSuccess
[ ] //
[ ] //   ARGUMENTS   :    in string:  name of test case so can look up info for test case
[ ] //                    in the aquariuscfg.ini file
[ ] //                    inout rtSledgehammer: a data structure that keeps
[ ] //                     info about the Sledgehammer run
[ ] //                    optional boolean: trace the steps or NOT
[ ] //
[ ] //   RETURNS     :    boolean -
[ ] //                      False	= if Sledgehammer run failed.
[ ] //                      True = if Sledgehammer run is successful.
[ ] //   DESCRIPTION :    Determine if the Sledgehammer run was successful by
[ ] //                    parsing the log file for a"well-known" message. The
[ ] //                    Sledgehammer testcase name is used in this messages.
[ ] //
[ ] //******************************************************************************
[ ] 
[-] boolean fSledgehammerSuccess(in string sTestCaseName,inout rtSledgehammer rInfo, boolean bTrace optional)
	[ ] boolean bReturn = FALSE
	[ ] integer iCurLine = 0
	[ ] string sCurLine
	[ ] string sStop
	[ ] 
	[-] do
		[ ] fTrace("Searching '{csSledgehammerStart}' in Sledgehammer.log", bTrace)
		[ ] Verify(fSearchFile(rInfo.sLog,iCurLine, csSledgehammerStart, iCurLine, sCurLine), TRUE)
		[ ] sStop = fReadWriteIni(gsAquariusIni,csRead,sTestCaseName, entSledgehammerTestName, "")
		[ ] sStop = "{csSledgehammerStopPre}{sStop}{csSledgehammerStopPost}"
		[ ] fTrace("Searching '{sStop}' in Sledgehammer.log", bTrace)
		[ ] Verify(fSearchFile(rInfo.sLog,iCurLine, sStop, iCurLine, sCurLine), TRUE)
		[ ] bReturn = TRUE
		[ ] rInfo.eState = complete
	[-] except
		[ ] reraise
	[ ] 
	[ ] return bReturn
[ ] //******************************************************************************
[ ] // END: fSledgehammerSuccess()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fGetSledgehammerSessionIds
[ ] //
[ ] //   ARGUMENTS   :    in string:  name of test case so can look up info for test case
[ ] //                    in the aquariuscfg.ini file
[ ] //                    inout rtSledgehammer: a data structure that keeps
[ ] //                     info about the Sledgehammer run
[ ] //                    optional boolean: trace the steps or NOT
[ ] //
[ ] //   RETURNS     :    boolean -
[ ] //                      False	= if no sessions are found
[ ] //                      True = if found at least one session
[ ] //   DESCRIPTION :    Parse the sledgehammer log file to get the session ids.
[ ] //                    Build a list - one for each session - of info about the
[ ] //                    session.  Currently only 1 session has been used
[ ] //                    in a testcase.  May not work for multiple sessions.
[ ] //
[ ] //******************************************************************************
[-] boolean fGetSledgehammerSessionIds(in string sTestCaseName, inout rtSledgehammer rInfo, boolean bTrace optional)
	[ ] boolean bReturn = FALSE
	[ ] integer iLPos
	[ ] integer iRPos
	[ ] string sSessionId
	[ ] integer iCurLine = 0
	[ ] string sCurLine = ""
	[ ] rtSessionInfo rSessionInfo
	[ ] 
	[-] do
		[ ] // 01/17 18:13:20:086 HAMMER             900  	MSME:0x00305750:[00/00] - AC:AC:AC:AC:AC:00/0111500038 ActInit(): created set-top AC:AC:AC:AC:AC:00 with Session ID <AC:AC:AC:AC:AC:00/0111500038> and NSAP <Nsap(AFI (45)Idi(0000000000000000) HoDSP(00000000) Esi(AC:AC:AC:AC:AC:00) Sel(0))>
		[-] while (fSearchFile(rInfo.sLog,iCurLine, csSledgehammerSettop, iCurLine, sCurLine))
			[ ] iCurLine++
			[ ] //find session id in string <AC:AC:AC:AC:AC:00/0111500038>
			[ ] iLPos = StrPos("<",sCurLine)
			[ ] iRPos = StrPos(">",sCurLine)
			[ ] rSessionInfo.sId = SubStr(sCurLine, iLPos+1,iRPos-iLPos-1)
			[ ] rSessionInfo.bStatus = TRUE
			[ ] rSessionInfo.eSledgehammerState = created
			[ ] ListAppend(rInfo.lrSessions,rSessionInfo)
			[ ] bReturn = TRUE
	[-] except
		[ ] // bReturn is initialized to FALSE
		[ ] // reraise
	[ ] return bReturn
[ ] 
[ ] //******************************************************************************
[ ] // END: fGetSledgehammerSessionIds()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fSledgehammerSessionSuccess()
[ ] //
[ ] //   ARGUMENTS   :    in string:  name of test case so can look up info for test case
[ ] //                    in the aquariuscfg.ini file
[ ] //                    inout rtSessionInfo: a data structure that keeps
[ ] //                     info about a Sledgehammer session
[ ] //                    optional boolean: trace the steps or NOT
[ ] //
[ ] //   RETURNS     :    boolean -
[ ] //                      False	= if the operation failed.
[ ] //                      True = if the operation was successful.
[ ] //   DESCRIPTION :    for each session in the sledgehammer run, search
[ ] //                    the sledgehammer log file for key entries.
[ ] //                    If further parsing is requried, then call the 
[ ] //                    specified parse function.
[ ] //
[ ] //******************************************************************************
[-] boolean fSledgehammerSessionSuccess(in string sTestCaseName,inout rtSledgehammer rInfo, boolean bTrace optional)
	[ ] boolean bReturn = TRUE
	[ ] rtLogMessage rLog
	[ ] integer iCurLine = 0
	[ ] string sCurLine
	[ ] rtSessionInfo rSession
	[ ] integer iCurSession = 0
	[ ] 
	[ ] // for each session in the sledgehammer run; parse the sledgehammer log file.
	[-] for each rSession in rInfo.lrSessions
		[ ] iCurLine = 0
		[ ] iCurSession++
		[ ] // look for specific log messages, and further parse the line if a parse function is specified.
		[-] for each rLog in clrSledgehammerLog
			[-] do
				[ ] fTrace("Searching {rInfo.sLog} for {rLog.sPreSessionId}{rSession.sId}{rLog.sPostSessionId}", bTrace)
				[-] if rLog.bWildCard
					[ ] Verify(fWildSearchFile(rInfo.sLog,iCurLine, "{rLog.sPreSessionId}{rSession.sId}{rLog.sPostSessionId}", iCurLine, sCurLine), TRUE)
				[-] else
					[ ] Verify(fSearchFile(rInfo.sLog,iCurLine, "{rLog.sPreSessionId}{rSession.sId}{rLog.sPostSessionId}", iCurLine, sCurLine), TRUE)
				[ ] // parse the line if there is a function to call
				[-] if rLog.sParseFunction != ""
					[ ] // call the function named with the session data structure, the string and a trace flag
					[ ] //ArgListCall ("Min", {1,2}) is equivalent to Min (1,2)
					[-] do
						[ ] rSession = ArgListCall(rLog.sParseFunction, {rSession, sCurLine, bTrace})
						[ ] fTrace("{rSession.sId} {rSession.sAssetId} {rSession.bStatus} {rSession.eSledgehammerState}", bTrace)
						[ ] Verify(rSession.bStatus, TRUE)
					[-] except
						[ ] LogWarning("Not able to parse msg for needed info: {sCurLine}")
			[+] except
				[ ] LogWarning("*** Warning: Did not find expected Sledgehammer Log msgs for; {rSession.sId}.  Missing message: {rLog.sPreSessionId}{rSession.sId}{rLog.sPostSessionId}")
				[ ] iCurLine = 0
		[ ] rSession.eSledgehammerState = terminated
		[ ] // update rInfo record
		[ ] rInfo.lrSessions[iCurSession] = rSession
		[ ] fTrace("{rInfo.lrSessions[iCurSession].sId} {rInfo.lrSessions[iCurSession].sAssetId} {rInfo.lrSessions[iCurSession].bStatus} {rInfo.lrSessions[iCurSession].eSledgehammerState}", bTrace)
		[ ] 
		[ ] bReturn = TRUE
	[ ] 
	[ ] return bReturn
[ ] //******************************************************************************
[ ] // END: fSledgehammerSessionSuccess()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fGenerateCfgSessionPl
[ ] //
[ ] //   ARGUMENTS   :    in string:  list of services of interest for this 
[ ] //                    test case
[ ] //                    optional boolean: trace the steps or NOT
[ ] //
[ ] //   RETURNS     :    boolean -
[ ] //                      False = if could not generate a cfg file
[ ] //                      True = if created a configuration file for session.pl
[ ] //   DESCRIPTION :    
[ ] //
[ ] //******************************************************************************
[-] private void fWriteLogFilenames(HFILE hFile,string sServer1, string sServer2,string sLog1, string sLog2)
	[ ] // write the path to the log file for the Server1 service
	[ ] FileWriteLine(hFile,"\\{sServer1}\{csITVRootShared}\log\{sLog1}")
	[ ] // if a vinca pair then look for 2nd log file on 1 box
	[ ] // and log files on 2nd box
	[-] if (sServer1 != sServer2)
		[ ] FileWriteLine(hFile,"\\{sServer1}\{gsRemoteITVRoot}\log\{sLog2}")
		[ ] FileWriteLine(hFile,"\\{sServer2}\{gsRemoteITVRoot}\log\{sLog2}")
		[ ] FileWriteLine(hFile,"\\{sServer2}\{gsRemoteITVRoot}\log\{sLog1}")
[ ] 
[-] boolean fGenerateCfgSessionPl(list of string lsServices, boolean bTrace optional)
	[ ] boolean bReturn = FALSE
	[ ] string sTestCaseName = GetTestCaseName()
	[ ] string sFile = "{gsTestFileRoot}{filSessionPlCfg}"
	[ ] string sService
	[ ] HFILE hFile
	[ ] string sVS
	[ ] 
	[-] do
		[ ] // delete existing cfg file if it exists
		[ ] // filSessionPlCfg 	= "SessionPlLogs.cfg"
		[-] if (SYS_FileExists(sFile))
			[ ] SYS_RemoveFile(sFile)
		[ ] 
		[ ] // create a new file
		[ ] hFile = FileOpen(sFile,FM_WRITE, FS_DENY_WRITE)
		[ ] 
		[ ] // for each service in the list of services
		[ ] // write a header to the SessionPlLogs.cfg file and
		[ ] // write a line for each log file to be considered
		[ ] // use the function fWriteLogFilename() to avoid lots of
		[ ] // duplicate code, just changing names
		[-] for each sService in lsServices
			[ ] // handle on service by service basis
			[-] switch Upper(sService)
				[-] case "SRM"
					[ ] // log header
					[ ] FileWriteLine(hFile,"[{sService}]")
					[ ] fWriteLogFilenames(hFile,gsCM1IP, gsCM2IP, csSRM1Log, csSRM2Log)
				[-] case "DMN"
					[ ] // log header
					[ ] FileWriteLine(hFile,"[{sService}]")
					[ ] fWriteLogFilenames(hFile,gsCM1IP, gsCM2IP, csDMN1Log, csDMN2Log)
				[-] case "CM"
					[ ] // log header
					[ ] FileWriteLine(hFile,"[{sService}]")
					[ ] fWriteLogFilenames(hFile,gsCM1IP, gsCM2IP, csCM1Log, csCM2Log)
				[-] case "SSAPI"
					[ ] // log header
					[ ] FileWriteLine(hFile,"[{sService}]")
					[ ] fWriteLogFilenames(hFile,gsCM1IP, gsCM2IP, csSSAPI_CM1Log, csSSAPI_CM2Log)
				[-] case "SS"
					[ ] // log header
					[ ] FileWriteLine(hFile,"[{sService}]")
					[+] for each sVS in lsVideoServersIP
						[ ] fWriteLogFilenames(hFile, sVS, sVS, csSSLog, csSSLog)
				[-] case "MOD"
					[ ] // log header
					[ ] FileWriteLine(hFile,"[{sService}]")
					[ ] fWriteLogFilenames(hFile,gsAPP1IP, gsAPP2IP, csMOD1Log, csMOD2Log)
				[-] case "MCACLIENT"
					[ ] // log header
					[ ] FileWriteLine(hFile,"[{sService}]")
					[+] for each sVS in lsVideoServersIP
						[ ] fWriteLogFilenames(hFile, sVS, sVS, csMCAClientLog, csMCAClientLog)
				[-] case "MCASERVER"
					[ ] // log header
					[ ] FileWriteLine(hFile,"[{sService}]")
					[+] for each sVS in lsVideoServersIP
						[ ] fWriteLogFilenames(hFile, sVS, sVS, csMCAServerLog, csMCAServerLog)
				[-] default
					[ ] LogWarning("*** Warning: {sService} not handled by fGenerateCfgSessionPl()")
		[ ] 
		[ ] // close file
		[ ] FileClose(hFile)
		[ ] 
		[ ] // write file name to aquarius ini file
		[-] if(fReadWriteIni(gsAquariusIni,csWrite,sTestCaseName,entSessionPlCfg, sFile) != "true")
			[ ] LogError("*** Error: fail to write cfg file name to aquarius ini file")
			[ ] ZodiacTestCaseExit(True)
		[ ] 
		[ ] bReturn = TRUE
	[-] except
		[ ] ExceptPrint()
		[ ] // function will return false
	[ ] 
	[ ] return bReturn
	[ ] 
[ ] //******************************************************************************
[ ] // END: fGenerateCfgSessionPl()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fRunSessionPl
[ ] //
[ ] //   ARGUMENTS   :    in string:  name of test case so can look up info for 
[ ] //                    test case in the aquariuscfg.ini file
[ ] //                    optional boolean: trace the steps or NOT
[ ] //
[ ] //   RETURNS     :    boolean -
[ ] //                      False = if starting the program "session.pl" failed.
[ ] //                      True = if starting the prgram "session.pl" was successful.
[ ] //   DESCRIPTION :    Runs the session pl program using SYS_Execute().  The
[ ] //                    path and name of the executable is read from the
[ ] //                    test server ini file.  The configuration file path and
[ ] //                    name is read from the ITV site ini file.  This should be
[ ] //                    change to be generated from the site and test case
[ ] //                    information.  The comd window starts and control returns
[ ] //                    to the SilkTest script, so caller must wait for
[ ] //                    the window to close, before attempting to use the
[ ] //                    results from session.pl.  To do this the window is 
[ ] //                    given a caption "{csSessionPlWindow}{sSessionId}"
[ ] //
[ ] //******************************************************************************
[-] boolean fRunSessionPl(in string sTestCaseName, in string sSessionId, boolean bTrace optional)
	[ ] boolean bReturn = FALSE
	[ ] string sExe
	[ ] string sCfg
	[ ] string sLog
	[ ] string sCmdCaption
	[ ] 
	[-] do
		[ ] sExe = fReadWriteIni(gsAquariusIni,csRead,secSessionPl, entExecutable, "")
		[ ] // write file name to aquarius ini file
		[ ] // sCfg = fReadWriteIni(gsAquariusIni,csRead,sTestCaseName,entSessionPlCfg, "")
		[ ] sCfg = "{gsTestFileRoot}{filSessionPlCfg}"
		[ ] sLog = "{gsTestFileRoot}{filSessionPlLog}"
		[ ] sCmdCaption = "{Chr(34)}{csSessionPlWindow}{sSessionId}{Chr(34)}"
		[ ] fTrace("Session Pl Exe: {sExe}", bTrace)
		[ ] fTrace("Session Pl Cfg: {sCfg}", bTrace)
		[ ] fTrace("Session Pl Log: {sLog}", bTrace)
		[ ] Verify(SYS_Execute("start {sCmdCaption} {sExe} -c {sCfg} -f {sLog} -v {sSessionId}"),0)
		[ ] bReturn = TRUE
	[-] except
		[ ] ExceptLog()
	[ ] 
	[ ] return bReturn
[ ] 
[ ] //******************************************************************************
[ ] // END: fRunSessionPl()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fSledgehammerSessionSetup
[ ] //
[ ] //   ARGUMENTS   :    in rtSessionInfo - data structure to populate with info
[ ] //                      from parsing the supplied line
[ ] //                    in string the line to parse.  Its the session setup request 
[ ] //                      from sledgehammer
[ ] //                    optional boolean: trace the steps or NOT
[ ] //
[ ] //   RETURNS     :    rtSessionInfo - data structure to populate with info
[ ] //                      from parsing the supplied line
[ ] //
[ ] //   DESCRIPTION :    get the AssetUID from the session setup request sent by
[ ] //                    sledgehammer. This 
[ ] //                    function is called using ArgListCall() so all the
[ ] //                    arguments are 'in' only.  The data structure
[ ] //                    we're trying to update -- information about the
[ ] //                    session -- is therefore passed in, copied, then used
[ ] //                    as the return argument.
[ ] //                    Below is sample of string that is being parsed.
[ ] //MSME:0x00305750:[01/05] - AC:AC:AC:AC:AC:00/0511774612 sending session setup with AssetUID=0x1bc00a53, AppType=0x1bc00001, HomeID=32, SmartCardID=48, PurchaseID=0, PackageID=0
[ ] //******************************************************************************
[ ] 
[-] rtSessionInfo fSledgehammerSessionSetup(in rtSessionInfo rInfo, in string sLine, in boolean bTrace optional)
	[ ] integer iPos
	[ ] string sWork
	[ ] rtSessionInfo rWork = rInfo
	[ ] fTrace("Entering fSledgehammerSessionSetup()", bTrace)
	[-] do
		[ ] // find the AssetUID in the string
		[ ] iPos = StrPos(csAssetTag,sLine)
		[ ] sWork = Substr(sLine, iPos+Len(csAssetTag))
		[ ] //line now starts with value of AssetUid, 0x1bc00a53
		[ ] //strip the 0x by starting at 3 with the Substr func
		[ ] // find the ', ' that separates the tag=values
		[ ] iPos = StrPos(", ", sWork)
		[ ] rWork.sAssetId= Substr(sWork,3,iPos - 3)
		[ ] rWork.bStatus = TRUE
	[-] except
		[ ] rWork.sAssetId = ""
		[ ] rWork.bStatus = FALSE
	[ ] 
	[ ] fTrace ("AssetId: {rWork.sAssetId}")
	[ ] return rWork
[ ] 
[ ] //******************************************************************************
[ ] // END: fSledgehammerSessionSetup()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fGetPurchaseTimeFromModLog
[ ] //
[ ] //   ARGUMENTS   :    in rtSessionInfo - data structure to populate with info
[ ] //                      from parsing the supplied line
[ ] //                    in string the line to parse.  Its the session setup request 
[ ] //                      from sledgehammer
[ ] //                    optional boolean: trace the steps or NOT
[ ] //
[ ] //   RETURNS     :    rtSessionInfo - data structure to populate with info
[ ] //                      from parsing the supplied line
[ ] //
[ ] //   DESCRIPTION :    get the purchase time from a MOD log message. This                   sledgehammer. This 
[ ] //                    function is called using ArgListCall() so all the
[ ] //                    arguments are 'in' only.  The data structure
[ ] //                    we're trying to update -- information about the
[ ] //                    session -- is therefore passed in, copied, then used
[ ] //                    as the return argument.
[ ] //                    Below is sample of string that is being parsed.
[ ] // Entering ActionStartStream - Sid=2139.00040001/1,PurchaseTime=1043441792
[ ] //******************************************************************************
[ ] 
[+] rtSessionInfo fGetPurchaseTimeFromModLog(in rtSessionInfo rInfo, in string sLine, in boolean bTrace optional)
	[ ] integer iPos
	[ ] string sWork
	[ ] rtSessionInfo rWork = rInfo
	[ ] fTrace("Entering fGetPurchaseTimeFromModLog()", bTrace)
	[+] do
		[ ] // find the tag in the string
		[ ] iPos = StrPos(csTag,sLine)
		[ ] sWork = Substr(sLine, iPos+Len(csTag))
		[ ] //line now starts with value of PurchaseTime 1043441792
		[ ] //it is at the end of the line, but Trim it to be sure
		[ ] rWork.sPurchaseTime= Trim(sWork)
		[ ] rWork.bStatus = TRUE
	[+] except
		[ ] rWork.sPurchaseTime = ""
		[ ] rWork.bStatus = FALSE
	[ ] 
	[ ] fTrace ("PurchaseTime: {rWork.sPurchaseTime}", bTrace)
	[ ] return rWork
[ ] 
[ ] //******************************************************************************
[ ] // END: fGetPurchaseTimeFromModLog()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fCheckREQLOGEntry
[ ] //
[ ] //   ARGUMENTS   :    in string testcase name, so can look up info in
[ ] //                      aquariuscfg file
[ ] //                    in rtSessionInfo - data structure populated with info
[ ] //                      from parsing the log files, .sId comes in as 
[ ] //                      AC:AC:AC:AC:AC:00/0003956469
[ ] //                    optional boolean: trace the steps or NOT
[ ] //   RETURN      :    boolean -
[ ] //                      False	= if the operation failed.
[ ] //                      True = if the operation was successful.
[ ] //   DESCRIPTION :    compare information in CMREQLOG table to expected 
[ ] //                    information for test case found in ini file.  
[ ] //                    Currently verifies:
[ ] //                      AssetID - expected value if from sledgehammer session
[ ] //                        setup request
[ ] //                      Nodegroup - expected value in test case specification
[ ] //                      Status - expected value in test case specification
[ ] //                      Extended status - expected value in test case 
[ ] //                        specification
[ ] //
[ ] // Sample of SQL Queries to get information
[ ] // select ASSET,NODEGRP from CMREQLOG where SESSION = 'acacacacac001fb30aa7'
[ ] // SELECT NAME from CMREQLOG JOIN CMREQLOG_STATUS_CODES ON
    // CMREQLOG.STATUS = CMREQLOG_STATUS_CODES.STATUS where
    // CMREQLOG.SESSION = 'acacacacac001fb30aa7'
[ ] // SELECT NAME from CMREQLOG JOIN CMREQLOG_EXTSTATUS_CODES ON
    // CMREQLOG.EXTENDEDSTATUS = CMREQLOG_EXTSTATUS_CODES.STATUS where
    // CMREQLOG.SESSION = 'acacacacac001fb30aa7'
[ ] 
[ ] // const string entSledgehammerNodegroup= "SledgehammerNodegroup"
[ ] // const string entReqlogStatus = "ReqlogStatus"
[ ] // const string entReqlogExtendedStatus = "ReqlogExtendedStatus"
[ ] //******************************************************************************
[ ] 
[-] boolean fCheckREQLOGEntry(in string sTestCaseName,in rtSessionInfo rInfo, in boolean bTrace optional)
	[ ] boolean bReturn = TRUE
	[ ] string sExpected=""
	[ ] string sActual=""
	[ ] string sSession
	[ ] string sDBLogOn
	[ ] list of ANYTYPE laTemp = {}
	[ ] ANYTYPE aTemp
	[ ] string sQuery = ""
	[ ] 
	[ ] // convert sSessionId to one used in the SQL query
	[ ] sSession = fStripSessionId(rInfo.sId)
	[ ] 
	[ ] // create db logon info from ini file
	[-] do
		[ ] Verify(fGetDBLogOn(entMDSSQLServer, sDBLogOn, bTrace), TRUE)
	[-] except
		[ ] // Cannot do checks without DB Logon info
		[ ] ExceptLog()
		[ ] ZodiacTestCaseExit(True)
		[ ] // return FALSE
	[ ] 
	[ ] // compare Asset Id to one used by Sledgehammer
	[-] do
		[ ] sQuery = "{csGetAssetIdREQLOGPre}{sSession}{csGetAssetIdREQLOGPost}"
		[ ] laTemp = fDBQuery(sDBLogOn, sQuery, "IcmData")
		[ ] // expect one return stmt from query
		[ ] // expect one value
		[ ] Verify(ListCount(laTemp),1)
		[ ] aTemp = laTemp[1]
		[ ] sActual = [string]aTemp[1]
		[ ] fTrace("Asset Id from db query = {sActual}", bTrace)
		[ ] // returned in decimal convert to Hex by rommy followed
		[ ] // sActual = fDecimalToHex(sActual)
		[ ] sExpected = fHexToDecimal(rInfo.sAssetId) 
		[ ] //commented by rommy followed
		[ ] // fTrace("From fDecimal to Hex query: sActual = {sActual}",bTrace)
		[ ] // compare to expected, based on sledgehammer session setup by rommy modified
		[ ] // Verify(lower(rInfo.sAssetId),lower(sActual))
		[ ] Verify(val(sActual),val(sExpected))
	[-] except
		[ ] LogWarning("*** Warning: Expected AssetId: {rInfo.sAssetId}  Actual AssetId: {fDecimalToHex(sActual)} in REQLOG")
		[ ] bReturn = FALSE
	[ ] 
	[ ] // compare Nodegroup in REQ log to one expected based
	[ ] // testcase specification
	[ ] sExpected = ""
	[ ] sActual = ""
	[-] do
		[ ] sExpected = fReadWriteIni(gsAquariusIni,csRead,GetTestCaseName(), entSledgehammerNodegroup, "")
		[ ] sQuery = "{csGetNodegroupREQLOGPre}{sSession}{csGetNodegroupREQLOGPost}"
		[ ] laTemp = fDBQuery(sDBLogOn, sQuery, "IcmData")
		[ ] // expect one return stmt from query
		[ ] // expect one value
		[ ] Verify(ListCount(laTemp),1)
		[ ] aTemp = laTemp[1]
		[ ] sActual = [string]aTemp[1]
		[ ] fTrace("Nodegroup from db query: sActual = {sActual}", bTrace)
		[ ] Verify(val(sExpected),sActual)
	[-] except
		[ ] LogWarning("*** Warning: Expected Nodegroup: {sExpected}  Actual Nodegroup: {sActual} in REQLOG")
		[ ] bReturn = FALSE
	[ ] 
	[ ] // compare Status in REQ log to one expected based
	[ ] // testcase specification
	[ ] sExpected = ""
	[ ] sActual = ""
	[-] do
		[ ] sExpected = fReadWriteIni(gsAquariusIni,csRead,GetTestCaseName(), entReqlogStatus, "")
		[ ] sQuery = "{csGetStatusREQLOGPre}{sSession}{csGetStatusREQLOGPost}"
		[ ] laTemp = fDBQuery(sDBLogOn, sQuery, "IcmData")
		[ ] // expect one return stmt from query
		[ ] // expect one value
		[ ] Verify(ListCount(laTemp),1)
		[ ] aTemp = laTemp[1]
		[ ] sActual = [string]aTemp[1]
		[ ] fTrace("Status from db query: sActual = {sActual}", bTrace)
		[ ] Verify(Upper(sActual),Upper(sExpected))
	[-] except
		[ ] LogWarning("*** Warning: Expected Status: {sExpected}  Actual Status: {sActual} in REQLOG")
		[ ] bReturn = FALSE
	[ ] 
	[ ] // compare extended status in REQ log to one expected based
	[ ] // testcase specification
	[ ] sExpected = ""
	[ ] sActual = ""
	[-] do
		[ ] sExpected = fReadWriteIni(gsAquariusIni,csRead,GetTestCaseName(), entReqlogExtendedStatus, "")
		[ ] sQuery = "{csGetExtStatusREQLOGPre}{sSession}{csGetExtStatusREQLOGPost}"
		[ ] laTemp = fDBQuery(sDBLogOn, sQuery, "IcmData")
		[ ] // expect one return stmt from query
		[ ] // expect one value
		[ ] Verify(ListCount(laTemp),1)
		[ ] aTemp = laTemp[1]
		[ ] sActual = [string]aTemp[1]
		[ ] fTrace("Extended status from db query: sActual = {sActual}", bTrace)
		[ ] Verify(Upper(sActual),Upper(sExpected))
	[-] except
		[ ] LogWarning("*** Warning: Expected Extended Status: {sExpected}  Actual Extended Status: {sActual} in REQLOG")
		[ ] bReturn = FALSE
	[ ] 
	[ ] return bReturn
[ ] //******************************************************************************
[ ] // END: fCheckREQLOGEntry()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fCheckViewingRecord
[ ] //
[ ] //   ARGUMENTS   :    in rtSessionInfo - data structure populated with info
[ ] //                      from parsing the log files, .sId comes in as 
[ ] //                      AC:AC:AC:AC:AC:00/0003956469, sAssetId is in hex
[ ] //                      sPurchaseTime is # sec since 1970
[ ] //
[ ] //   RETURNS     :    boolean -
[ ] //                      False	= if the operation failed.
[ ] //                      True = if the operation was successful.
[ ] //
[ ] //   DESCRIPTION :    Looks up info (currently only Billable) in the
[ ] //                    Viewing tables and compares it to expected values
[ ] //                    Billable - expected value in testcase specification
[ ] //******************************************************************************
[-] boolean fCheckViewingRecord(in rtSessionInfo rInfo,in boolean bTrace optional)
	[ ] boolean bReturn = TRUE
	[ ] string sExpected = ""
	[ ] string sActual = ""
	[ ] string sSession
	[ ] string sDBLogOn
	[ ] list of ANYTYPE laTemp = {}
	[ ] ANYTYPE aTemp
	[ ] string sQuery = ""
	[ ] 
	[ ] // get 'key for Viewing info -- of the form ACACACACAC00-465570387-1043452757
	[ ] // MAC-AssetUid-purchasetime
	[ ] sSession = fGetViewingKey(rInfo)
	[ ] // create db logon info from ini file
	[-] do
		[ ] Verify(fGetDBLogOn(entPSSQLServer, sDBLogOn, bTrace), TRUE)
	[-] except
		[ ] // Cannot do checks without DB Logon info
		[ ] ExceptLog()
		[ ] ZodiacTestCaseExit(True)
		[ ] // return FALSE
	[ ] 
	[ ] // compare Billable Value in Viewing Record to one expected based
	[ ] // testcase specification
	[-] do
		[ ] sExpected = fReadWriteIni(gsAquariusIni,csRead,GetTestCaseName(), entBillable, "")
		[ ] sQuery = "{csGetBillableIadPre}{sSession}{csGetBillableIadPost}"
		[ ] laTemp = fDBQuery(sDBLogOn, sQuery, "IadData")
		[ ] // expect one return stmt from query
		[ ] // expect one value
		[ ] Verify(ListCount(laTemp),1)
		[ ] aTemp = laTemp[1]
		[ ] sActual = [string]aTemp[1]
		[ ] fTrace("Billable from db query: sActual = {sActual}", bTrace)
		[ ] fTrace("Billable from ini file: sExpected = {sExpected}", bTrace)
		[ ] // Take a yes or a 1 as Billable
		[-] if (Upper(sExpected) == "YES")
			[ ] sExpected = "1"
		[-] if (Upper(sExpected) == "NO")
			[ ] sExpected = "0"
		[ ] Verify(sActual,sExpected)
	[-] except
		[ ] LogWarning("*** Warning: Expected Billable: {sExpected}  Actual Billable: {sActual} in REQLOG")
		[ ] bReturn = FALSE
	[ ] 
	[ ] return bReturn
[ ] //******************************************************************************
[ ] // END: fCheckViewingRecord()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fStripSessionId
[ ] //
[ ] //   ARGUMENTS   :    in string session id, comes in as 
[ ] //                      AC:AC:AC:AC:AC:00/0003956469
[ ] //
[ ] //   RETURNS     :    string of the form acacacacac001fb30aa7
[ ] //
[ ] //   DESCRIPTION :    convert session id of form AC:AC:AC:AC:AC:00/0003956469
[ ] //                      to form  acacacacac001fb30aa7
[ ] //******************************************************************************
[+] string fStripSessionId(in string sSessionId)
	[ ] string sMac
	[ ] string sId
	[ ] integer iPos
	[ ] 
	[ ] // find the /
	[ ] iPos = StrPos("/",sSessionId)
	[ ] sMac = SubStr(sSessionId,1,iPos-1)
	[ ] sId = SubStr(sSessionId, iPos+1)
	[ ] 
	[ ] // get rid of leading 0's
	[ ] //convert the decimal session id to hex and put in lower case
	[ ] sId = Str(Val(sId))
	[ ] sId = lower(fDecimalToHex(sId))
	[ ] 
	[ ] //get the ':" out of the MAC address
	[ ] sMac = lower(StrTran(sMac,":",""))
	[ ] 
	[ ] // return the strings appended to each other
	[ ] return "{sMac}{sId}"
[ ] 
[ ] //******************************************************************************
[ ] // END: fStripSessionId()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fGetViewingKey
[ ] //
[ ] //   ARGUMENTS   :    in string session id, comes in as 
[ ] //                      AC:AC:AC:AC:AC:00/0003956469
[ ] //
[ ] //   RETURNS     :    string of the form ACACACACAC00-465570387-1043452757
[ ] //                    mac-assetuid-purchasetime
[ ] //
[ ] //   DESCRIPTION :    get the viewing tables key of the form 
[ ] //                    ACACACACAC00-465570387-1043452757
[ ] //                    from the info about the session
[ ] //******************************************************************************
[+] string fGetViewingKey(in rtSessionInfo rInfo)
	[ ] string sMac
	[ ] string sAssetId
	[ ] integer iPos
	[ ] integer iVal
	[ ] 
	[ ] // find the /
	[ ] iPos = StrPos("/",rInfo.sId)
	[ ] sMac = SubStr(rInfo.sId,1,iPos-1)
	[ ] 
	[ ] //get the ':" out of the MAC address
	[ ] sMac = UPPER(StrTran(sMac,":",""))
	[ ] 
	[ ] //convert the hex uid to decimal
	[ ] sAssetId = fHexToDecimal(rInfo.sAssetId)
	[ ] //get rid of leading 0's
	[ ] sAssetId = Str(Val(sAssetId))
	[ ] 
	[ ] // return the strings appended to each other
	[ ] return "{sMac}-{sAssetId}-{rInfo.sPurchaseTime}"
[ ] 
[ ] //******************************************************************************
[ ] // END: fStripSessionId()
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] //
[ ] //   FUNCTION    :    fGetDBLogOn
[ ] //
[ ] //   ARGUMENTS   :    in DB to get logon string for either MDS or PS, use 
[ ] //                    the strings entMDSSQLServer or entPSSQLServer
[ ] //                    out string to use for the the fDB...() functions that get 
[ ] //                      access to a SQL Server database. 
[ ] //                    in boolean optional trace the steps or not
[ ] //
[ ] //   RETURNS     :    boolean -
[ ] //                      False	= if the operation failed.
[ ] //                      True = if the operation was successful.
[ ] //
[ ] //   DESCRIPTION :    get from ini files the db logon for a SQL Server DB.  
[ ] //                    Create a string of the form DSN=ZFRED1;SRVR=ZFRED1;UID=sa;PWD=
[ ] //
[ ] //******************************************************************************
[-] boolean fGetDBLogOn(in string sNode, out string sDBLogOn, in boolean bTrace optional)
	[ ] boolean bReturn = FALSE
	[ ] string sSQLServer
	[ ] string sDSN
	[ ] string sUid
	[ ] string sPassword
	[ ] 
	[-] do
		[ ] // Look up the site SQL server name
		[ ] sSQLServer = fReadWriteIni(gsZodiacIni,csRead,secCommandCenter,sNode, "")
		[ ] // Look up the DSN name for this site on the test server
		[ ] sDSN = fReadWriteIni(gsZodiacIni, csRead,secODBCDsn,entDSN,"")
		[ ] // Get the UID  and Password from the Site file
		[ ] sUid = fReadWriteIni(gsZodiacIni,csRead,secCommandCenter,entSQLUid, "")
		[ ] sPassword = fReadWriteIni(gsZodiacIni,csRead,secCommandCenter,entSQLPassword, "")
		[-] if Lower(sPassword) == "none"
			[ ] sPassword = ""
		[ ] //form string
		[ ] sDBLogOn = "DSN={sDSN};SRVR={sSQLServer};UID={sUid};PWD={sPassword}"
		[ ] bReturn =TRUE
	[-] except
		[ ] reraise
	[ ] return bReturn
[ ] //******************************************************************************
[ ] // END: fGetDBLogOn()
[ ] //******************************************************************************
[ ] 
[-] boolean fRunxmlMODIF(inout rtSledgehammer rInfo, in string sXMLTagSeries, in string sAttrName, in string sAttrValue)
	[ ] boolean bReturn = FALSE
	[ ] 
	[ ] // get xml path and name 
	[ ] // sXML = fReadWriteIni(gsAquariusIni,csRead,sTestCaseName, entSledgehammerXML, "")
	[ ] // sXML = "{csInstallPath}{dirAquariusIni}{sXML}"
	[ ] // run using Sys_Execute().  Control does not return to the script until
	[ ] // this finishes.  Want to change to use MainWinn.Start() method so control
	[ ] // returns to Silk sooner.
	[-] do
		[ ] //gsLocalITVRoot = fReadWriteIni(gsZodiacIni,csRead,secITVRoot, entLocalITVRoot, "")
		[ ] SYS_SetDir(csInstallPath+csxmlMODIFYRoot)
		[ ] print("xmlMODIF {rInfo.sXMLFile} {sXMLTagSeries} {sAttrName} {sAttrValue}")
		[ ] SYS_Execute("xmlMODIF {rInfo.sXMLFile} {sXMLTagSeries} {sAttrName} {sAttrValue}")
		[ ] bReturn = TRUE
	[-] except
		[ ] reraise
	[ ] 
	[ ] return bReturn
[ ] 
[ ] //******************************************************************************
[ ] // END: AQUARIUSFUNC.INC
[ ] //******************************************************************************
[ ] 
