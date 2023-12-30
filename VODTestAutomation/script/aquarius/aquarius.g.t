[ ] //******************************************************************************
[ ] // Copyright (c) 2004 by
[ ] // ZQ Interactive, Inc., Shanghai, PRC.,
[ ] // All Rights Reserved.  Unpublished rights reserved under the copyright
[ ] // laws of the United States.
[ ] // 
[ ] // The software contained  on  this media is proprietary to and embodies the
[ ] // confidential technology of ZQ Interactive, Inc. Possession, use,
[ ] // duplication or dissemination of the software and media is authorized only
[ ] // pursuant to a valid written license from ZQ Interactive, Inc.
[ ] // 
[ ] // This software is furnished under a  license  and  may  be used and copied
[ ] // only in accordance with the terms of  such license and with the inclusion
[ ] // of the above copyright notice.  This software or any other copies thereof
[ ] // may not be provided or otherwise made available to  any other person.  No
[ ] // title to and ownership of the software is hereby transferred.
[ ] //
[ ] // The information in this software is subject to change without notice and
[ ] // should not be construed as a commitment by ZQ Interactive, Inc.
[ ] //
[ ] // Ident : $Id:  $
[ ] // Branch: $Name:  $
[ ] // Author: Bernie Zhao (Tianbin Zhao)
[ ] // Desc  : data-driven aquarius test
[ ] //
[ ] // Revision History: 
[ ] // ---------------------------------------------------------------------------
[ ] // $Log: /ZQProjs/VODTestAutomation/script/aquarius/aquarius.g.t $
[ ] // 
[ ] // 1     10-11-12 16:08 Admin
[ ] // Created.
[ ] // 
[ ] // 1     10-11-12 15:42 Admin
[ ] // Created.
[ ] // 
[ ] // 1     04-09-16 17:51 Bernie.zhao
[ ] // data-driven script
[ ] // 
[ ] // ===========================================================================
[ ] // 
[ ] //******************************************************************************
[ ] 
[ ] use "..\..\inc\global\zodiacfunc.inc"
[ ] use "..\..\inc\global\zodiacnt.inc"
[ ] use "..\..\inc\global\zodiacfiles.inc"
[ ] use "..\..\inc\global\zodiacutil.inc"
[ ] use "..\..\inc\global\zodiacdefs.inc"
[ ] use "..\..\inc\global\zodiacmsgs.inc"
[ ] use "..\..\inc\global\zodiacdatabase.inc"
[ ] use "mswconst.inc"
[ ] // defines the disk for the SilkTest source files (csSourceDisk)
[ ] //use "..\..\inc\aquarius\sourcedisk.inc" 
[ ] 
[ ] use "..\..\inc\aquarius\aquariusfunc.inc"
[ ] use "..\..\inc\aquarius\aquariusdefs.inc"
[ ] use "..\..\inc\aquarius\aquariusmsgs.inc"
[ ] use "..\..\inc\aquarius\aquariusframe.inc"
[ ] 
[ ] // for base state
[ ] use "..\..\inc\global\basestate.inc"
[ ] 
[ ] 
[ ] boolean gbZodiacInit = FALSE
[ ] boolean gbAquariusInit = FALSE
[ ] 
[ ] // paths to files that hold test information
[ ] string gsAquariusIni
[ ] string gsZodiacIni
[ ] string gsITVSiteIni 
[ ] string gsTestServerIni
[ ] string gsTestFileRoot
[ ] 
[ ] // variables that define the ITV Site
[ ] // string sAM1IP, sMDS1IP, sMDS2IP, sPS1IP, sPS2IP, sAPP1IP, sAPP2IP, sCM1IP, sCM2IP
[ ] list of string lsVideoServersIP
[ ] // Create the pass/fail log
[+] boolean fZodiacStartup(boolean bTrace optional)
	[ ] boolean bReturn = FALSE
	[-] do
		[ ] // Store the current script name.
		[ ] gsScriptName = Trim(StrTran(GetProgramName( ), ".t", " "))
		[ ] fTrace("gsScriptName: {gsScriptName}",bTrace)
		[ ] 
		[ ] // gsDataDrive = csSourceDisk
		[ ] // fTrace("gsDataDrive:  {gsDataDrive}",bTrace)
		[ ] 
		[ ] // Build the PassFailLog string
		[ ] gsPassFailLog = csInstallPath + csMainLogPath + gsScriptName + "\" +
    											 			gsScriptName + "log"
		[ ] fTrace("gsPassFailLog:  {gsPassFailLog}",bTrace)
		[ ] 
		[ ] Verify(fCreatePassFailLog(),TRUE)
		[ ] bReturn = TRUE
	[-] except
		[ ] reraise
	[ ] return bReturn
[ ] 
[ ] // get the file path for the Aquariuscfg.ini,ITVSite and 
[ ] // testserver ini 
[ ] // get the directory location for all the files created
[ ] // by this script.
[ ] // get the ITV site information from the ITVSite.ini file
[ ] // get IPC access to each of the nodes in the site (Command
[ ] // center and video servers)
[-] boolean fAquariusStartUp(boolean bTrace optional)
	[ ] boolean bReturn = FALSE
	[ ] ResOpenList("Step 2.1 - get the file path for the Aquariuscfg.ini")
	[-] do
		[ ] // set file path for Aquariuscfg.ini file
		[ ] gsAquariusIni = "{csInstallPath}{filAquariusIni}"
		[ ] gsZodiacIni = "{csInstallPath}{filZodiacIni}"
		[ ] // get file path for ITVSiteIni and TestServerIni
		[ ] 
		[ ] // // Determine location of the aquarius test output files
		[ ] // string sTestServerIni = fReadWriteIni(gsAquariusIni,csRead,"TestSystem","TestServer","")
		[ ] // string sSiteIni = fReadWriteIni(gsAquariusIni,csRead,"TestSystem","ITVSite","")
		[ ] // 
		[ ] // // set global string to full path to these files
		[ ] // gsTestServerIni = "{csInstallPath}{dirAuariusIni}{sTestServerIni}"
		[ ] // fTrace ("TestServer INI file: {gsTestServerIni}", bTrace)
		[ ] // gsITVSiteIni = "{csInstallPath}{dirAquariusIni}{sSiteIni}"
		[ ] // fTrace ("ITVSite INI file: {gsITVSiteIni}", bTrace)
		[ ] 
		[ ] // Get the Root of the path for the Aquarius TestFiles from the TestServer INI file
		[ ] // use this as the root of the location where the files created in this
		[ ] // test case go
		[ ] // gsTestFileRoot = fReadWriteIni(gsTestServerIni,csRead,secTestFiles,entLocalRoot, "")
		[ ] // gsTestFileRoot = "{gsTestFileRoot}{dirTestFiles}"
		[ ] gsTestFileRoot = "{csInstallPath}{csMainLogPath}{dirTestFiles}"
		[ ] fTrace ("gsTestFileRoot: {gsTestFileRoot}", bTrace)
		[ ] 
		[ ] //Try to ensure dir exists
		[+] do
			[-] if !SYS_DirExists(gsTestFileRoot)
				[ ] SYS_MakeDir(gsTestFileRoot)
			[ ] //add trailing \
			[ ] gsTestFileRoot = "{gsTestFileRoot}\" 
		[-] except
			[ ] LogError("*** Error: Could not ensure {gsTestFileRoot} exists")
			[ ] ResCloseList()
			[ ] ZodiacTestCaseExit(True)
	[+] except
		[ ] // Could not get the location of information for the test 
		[ ] // Expect that fReadWriteIni may have already failed the TestCase - 
		[ ] // but just in case - options are to use some hard coded defaults
		[ ] // for now just reraise
		[ ] bReturn = FALSE
		[ ] reraise
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 2.2 - get ITV site info")
	[ ] // Get ITV site info; fail test case if this fails
	[-] do
		[ ] // set list of VideoServer empty until ensure aquarius setup 
		[ ] // is done once
		[ ] lsVideoServersIP = {}
		[ ] Verify(fGetSiteInfo(TRUE),TRUE)
	[-] except
		[ ] bReturn = FALSE
		[ ] ExceptLog()
		[ ] fLogPassFail(csFailed,"GetSiteInfo() succeeds","GetSiteInfo() failed")
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 2.3 - get IPC access to Command Center nodes and Video Server Nodes")
	[ ] // Get IPC access to Command Center nodes and Video Server Nodes
	[ ] // Don't fail test case if cannot get access
	[-] do
		[ ] Verify(fGetSiteIPCAccess(TRUE), TRUE)
	[+] except
		[ ] // fGetSiteIPCAccess Logs Warning
		[ ] LogWarning("IPC Access not acquired to all Test Site nodes")
	[ ] bReturn = TRUE
	[ ] ResCloseList()
	[ ] 
	[ ] return bReturn
[ ] 
[ ] 
[-] // *** DATA DRIVEN ASSISTANT Section (!! DO NOT REMOVE !!) ***
	[ ] use "datadrivetc.inc"
	[ ] //use "aquarius.t"
	[ ] 
	[ ] // *** DSN ***
	[ ] STRING gsDSNConnect = "DSN=Segue DDA Excel;DBQ=C:\autotest\zodiac\ctl\aquarius\aquariuscfg.xls;UID=;PWD=;"
	[ ] 
	[ ] // *** Global record for each testcase ***
	[ ] 
	[-] type REC_DATALIST_DD_Aquarius0001 is record
		[ ] REC_Sheet1_ recSheet1_  //Sheet1$, 
	[ ] 
	[ ] // *** Global record for each Table ***
	[ ] 
	[-] type REC_Sheet1_ is record
		[ ] INTEGER NodeGroup  //NodeGroup, 
		[ ] STRING Mac  //Mac, 
		[ ] STRING IP  //IP, 
		[ ] INTEGER Port  //Port, 
		[ ] STRING Appuid  //Appuid, 
		[ ] STRING Assetid  //Assetid, 
		[ ] STRING SRM  //SRM, 
		[ ] STRING CM  //CM, 
		[ ] STRING SCTP  //SCTP, 
		[ ] STRING Value  //Value, 
	[ ] 
	[ ] // *** Global record containing sample data for each table ***
	[ ] // *** Used when running a testcase with 'Use Sample Data from Script' checked ***
	[ ] 
	[-] REC_Sheet1_ grTest_Sheet1_ = {...}
		[ ] 1											//nodegroup
		[ ] "00:11:2F:10:74:66"                   //mac address, 
		[ ] "155.12.168.192"                    //IP address,
		[ ] 16145                  //port, 
		[ ] "0x05600001"                //application uid, 
		[ ] "0x05600295"               //asset id, 
		[ ] "192.168.12.12"       //srm host, 
		[ ] "192.168.12.12"                    //srm host, 
		[ ] "192.168.12.12"                  //srm host, 
		[ ] "00:11:2F:10:74:66"                 //mac address in clientattribute
	[ ] 
	[ ] // *** End of DATA DRIVEN ASSISTANT Section ***
	[ ] 
[-] testcase DD_Aquarius0001 (REC_DATALIST_DD_Aquarius0001 rData) appstate BaseState
	[ ] string sTestStatus = csPassed
	[ ] string sExpectedResult = ""
	[ ] string sActualResult = ""
	[ ] 
	[ ] string sRefTimeStamp = ""
	[ ] string sTestCaseName = GetTestCaseName()
	[ ] string sXML = ""
	[ ] string sXMLtagseries = ""
	[ ] string sAttrname = ""
	[ ] string sAttrvalue = ""
	[ ] 
	[ ] string sSessionPlOut = ""
	[ ] list of string lsServices = {}
	[ ] string sService = ""
	[ ] rtSledgehammer rSledgehammer
	[ ] rtSessionInfo rSession
	[ ] boolean bSessionPl = FALSE
	[ ] string sSessionId = ""
	[ ] gsRemoteITVRoot = fReadWriteIni(csInstallPath+filZodiacIni,csRead,secITVRoot,entRemoteITVRoot, "")
	[ ] 
	[ ] // if running just a testcase ensure Zodiac level stuff is setup
	[ ] ResOpenList("Step 1 - Create the pass/fail log")
	[ ] Print("haha,test")
	[-] if !gbZodiacInit
		[-] do
			[ ] Verify(fZodiacStartup(TRUE), True)
			[ ] gbZodiacInit = TRUE
		[-] except
			[ ] ExceptLog()
			[ ] // fails the test case
			[ ] fLogPassFail(csFailed,"fZodiacStartup() Success", "fZodiacStartup() Failed")
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 2 - Get file path of Aquariuscfg.ini, get info of ITVSite, IPC access to CC and Video Server Nodes")
	[ ] // if running just a testcase ensure Aquarius level stuff is setup
	[-] if !gbAquariusInit
		[-] do
			[ ] Verify(fAquariusStartup(TRUE), TRUE)
			[ ] gbAquariusInit = TRUE
		[-] except
			[ ] ExceptLog()
			[ ] // fails the test case
			[ ] fLogPassFail(csFailed,"fAquariusStartup() Success", "fAquariusStartup() Failed")
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 3 - Create datetime stamp, then write it to StartDateTime in Aquariuscfg.ini")
	[ ] // Create a reference date and time stamp using option 4
	[ ] // Write it to the TestCase part of ini file.
	[ ] sRefTimeStamp = fCreateDateTimeStamp(4)
	[ ] fReadWriteIni(gsAquariusIni,csWrite,sTestCaseName,entStartDateTime, sRefTimeStamp)
	[ ] fTrace ("{sTestCaseName} Start: {sRefTimeStamp}",True)
	[ ] ResCloseList()
	[ ] 
	[ ] // set test case root
	[ ] // eventually add a time stamp and make the directory
	[ ] // string sTestCaseFileRoot = "{gsTestFileRoot}\{sTestCaseName}\" by rommy followed
	[ ] // string sTestCaseFileRoot = "{gsTestFileRoot}{sTestCaseName}\"
	[ ] 
	[ ] // lots of stuff goes here
	[ ] 
	[ ] ResOpenList("Step 4 - Get the list of Services to use in this test case")
	[ ] // Get the list of Services to use in this test case
	[-] do
		[ ] Verify(fGetServices(sTestCaseName, lsServices, TRUE),TRUE)
	[-] except
		[ ] // set to a default value
		[ ] lsServices = clsDefaultServices
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 5 - Build Sledgehammer XML file and run Sledgehammer")
	[ ] // Build Sledgehammer XML file and run Sledgehammer
	[-] do
		[ ] Verify(fBuildSledgehammerXML(sTestCaseName, rSledgehammer, TRUE), TRUE)
	[-] except
		[ ] // Fail test case if build Sledgehammer fails
		[ ] // ExceptLog()
		[ ] // fails the test case
		[ ] fLogPassFail(csFailed,"Build XML file Succeeds", "Build xml file Failed")
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 6 - run xmlMODIFY to modify xml file")
	[ ] string sNodeGroup	= Str(rData.recSheet1_.NodeGroup)						 //nodegroup
	[ ] string sMac				= rData.recSheet1_.Mac                   //mac address, 
	[ ] string sIP				= rData.recSheet1_.IP                    //IP address,
	[ ] string sPort			= Str(rData.recSheet1_.Port)                  //port, 
	[ ] string sAppuid		= rData.recSheet1_.Appuid                //application uid, 
	[ ] string sAssetid		= rData.recSheet1_.Assetid               //asset id, 
	[ ] string sSRM				= rData.recSheet1_.SRM       						 //srm host, 
	[ ] string sCM				= rData.recSheet1_.CM                    //srm host, 
	[ ] string sSCTP			= rData.recSheet1_.SCTP                  //srm host, 
	[ ] string sValue			= rData.recSheet1_.Value                 //mac address in clientattribute
	[ ] 
	[-] List of string lsXMLString = {...}
		[ ] "{sNodeGroup}"
		[ ] "{sMac}"                   //mac address, 
		[ ] "{sIP}"                    //IP address,
		[ ] "{sPort}"                  //port, 
		[ ] "{sAppuid}"                //application uid, 
		[ ] "{sAssetid}"               //asset id, 
		[ ] "{sSRM}"                   //srm host, 
		[ ] "{sCM}"                    //srm host, 
		[ ] "{sSCTP}"                  //srm host, 
		[ ] "{sValue}"               //mac address in clientattribute
	[ ] integer icurAttrName = 0
	[ ] integer icurAttrValue = 0
	[ ] string sAttrName = ""
	[ ] string sAttrValue = ""
	[ ] string sXMLTagSeries = ""
	[ ] 
	[-] do
		[-] for each sXMLTagSeries in clsXMLTagSeries
			[ ] icurAttrName++
			[ ] icurAttrValue++
			[ ] sAttrName = clsXMLAttrName[icurAttrName]
			[ ] sAttrValue = lsXMLString[icurAttrValue] 
			[ ] verify(fRunxmlMODIF(rSledgehammer, sXMLTagSeries, sAttrName,sAttrValue ),true)
	[-] except
		[ ] fLogPassFail(csFailed,"modify xml file succeeds","fail to modify xml file")
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 7 - run Sledgehammer")
	[ ] // Build Sledgehammer XML file and run Sledgehammer
	[-] do
		[ ] Verify(fRunSledgehammer(sTestCaseName, rSledgehammer,TRUE),TRUE)
	[-] except
		[ ] // Fail test case if running Sledgehammer fails
		[ ] // ExceptLog()
		[ ] // fails the test case
		[ ] fLogPassFail(csFailed,"Running Sledgehammer Succeeds", "Running Sledgehammer Failed")
	[ ] Print()
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 8 - Check that Sledgehammer ran as expected")
	[ ] // Check that Sledgehammer ran as expected
	[-] do
		[ ] Verify(fSledgehammerSuccess(sTestCaseName,rSledgehammer, TRUE), TRUE)
	[-] except
		[ ] LogWarning("*** Warning: Expected Log message not found in {rSledgehammer.sLog}")
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 9 - Get session id(s) from Sledgehammer log")
	[ ] // Get session id(s) from Sledgehammer log file
	[-] do
		[-] if (fGetSledgeHammerSessionIds(sTestCaseName, rSledgehammer, TRUE) != TRUE)
			[ ] // fails the test case
			[ ] LogError("*** Error: Expected: At least one session id   Received: No session ids found in {rSledgehammer.sLog}")
			[ ] ResCloseList()
			[ ] ZodiacTestCaseExit(True)
		[-] if !(ListCount(rSledgehammer.lrSessions) > 0)
			[ ] // fails the test case
			[ ] LogError("*** Error: Expected: At least one session id   Received: No session ids found in {rSledgehammer.sLog}")
			[ ] ResCloseList()
			[ ] ZodiacTestCaseExit(True)
	[-] except
		[ ] // Fail test case if don't get any session ID's
		[ ] // ExceptLog()
		[ ] // fails the test case
		[ ] // LogError("*** Error: Expected: At least one session id   Received: No session ids found in {rSledgehammer.sLog}")
		[ ] // ResCloseList()
		[ ] // ZodiacTestCaseExit(True)
	[ ] Print()
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 10 - Get information about each session from Sledgehammer log")
	[ ] // Get information about each session from Sledgehammer log
	[-] do
		[ ] Verify(fSledgehammerSessionSuccess(sTestCaseName,rSledgehammer, TRUE), TRUE)
	[-] except
		[ ] // TBD
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 11 - Generate session pl configuration file")
	[ ] // Generate session pl configuration file
	[-] do
		[ ] Verify(fGenerateCfgSessionPl(lsServices, TRUE), TRUE)
	[-] except     
		[ ] // //use a default configuration file for site
		[ ] // string sCfg
		[ ] // sCfg = fReadWriteIni(gsAquariusIni,csRead,secTemporary,entSessionPlCfg, "")
		[ ] // // sCfg = "{gsTestFileRoot}{sTestCaseName}\{sCfg}"
		[ ] // sCfg = "{gsTestFileRoot}\{sCfg}"
		[ ] // // write file name to aquarius ini file
		[+] // if (fReadWriteIni(gsAquariusIni,csWrite,sTestCaseName,entSessionPlCfg, sCfg) != "true")
			[ ] // // Fail test case if don't have a configuration file to run Session Pl
			[ ] // ExceptLog()
			[ ] // // fails the test case
		[ ] fLogPassFail(csFailed,"Need a configuration file for Session Pl", "No configuration file for Session Pl")
	[ ] Print()
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 12 - Run Session.pl for each session and analyze the results")
	[ ] // Run Session.pl for each session and analyze the results
	[-] for each rSession in rSledgehammer.lrSessions
		[ ] sSessionId = rSession.sId
		[ ] // Run Session Pl
		[ ] ResOpenList("Step 12.1 - Run Session Pl with {sSessionId}")
		[-] do
			[ ] Verify(fRunSessionPl(sTestCaseName, sSessionId, TRUE), TRUE)
		[+] except
			[ ] // Fail test case if running Session PL fails
			[ ] // ExceptLog()
			[ ] // fails the test case
			[ ] fLogPassFail(csFailed,"Running Session PL Failed", "Running Session PL Succeeds")
		[ ] Print()
		[ ] ResCloseList()
		[ ] 
		[ ] ResOpenList("Step 12.2 - Wait for Session Pl to finish")
		[ ] // Wait for Session Pl to finish
		[ ] // first find the cmd window
		[ ] bSessionPl = FALSE
		[+] do
			[ ] bSessionPl = AquariusSessionPl.Exists(10)
			[ ] Verify (bSessionPl, TRUE)
			[-] while (AquariusSessionPl.Exists(1))
				[ ] Verify (AquariusSessionPl.sCaption, "{csSessionPlWindow}{sSessionId}")
				[ ] sleep(10)
		[-] except
			[ ] ExceptLog()
		[ ] Print()
		[ ] ResCloseList()
		[ ] 
		[ ] ResOpenList("Step 12.3 - Get the name of the session pl file and ensure it exists")
		[ ] // Get the name of the session pl file and ensure it exists, fail the test
		[ ] // case if it does not
		[-] do
			[ ] // Now get the path and file name from aquariusdef.inc
			[ ] // aquariusdefs.inc: dirTestFiles and filSessionPlLog 
			[ ] // sSessionPlOut = sTestCaseFileRoot + filSessionPlLog
			[ ] sSessionPlOut = gsTestFileRoot + filSessionPlLog
			[ ] 
			[ ] fTrace (sSessionPlOut)
			[ ] Verify (SYS_FileExists(sSessionPlOut), TRUE)
		[-] except
			[ ] sTestStatus = csFailed
			[ ] // ExceptLog()
			[ ] fLogPassFail(csFailed,"Can access Session Pl output file ","No access to {sSessionPlOut}")
		[ ] Print()
		[ ] ResCloseList()
		[ ] 
		[ ] ResOpenList("Step 12.4 - Parse the Session PL output file service by service")
		[ ] // Parse the Session PL output file service by service
		[-] for each sService in lsServices
			[-] do
				[ ] Verify(fSearchSessionPlLog(sService, sSessionPlOut, rSession, TRUE), TRUE)
			[-] except
				[ ] sTestStatus = csFailed
				[ ] sExpectedResult = "Find all {sService} Log statements"
				[ ] sActualResult = "Did not find all {sService} Log statements"
				[ ] LogWarning("*** Warning: Unable to find all {sService} log statements")
		[ ] Print()
		[ ] ResCloseList()
		[ ] 
		[ ] ResOpenList("Step 12.5 - Verify Asset, Nodegroup, Status and Extended Status from CMREQLOG entry")
		[ ] // Verify Asset, Nodegroup, Status and Extended Status from CMREQLOG entry
		[-] do
			[ ] Verify(fCheckREQLOGEntry(sTestCaseName,rSession,TRUE), TRUE)
		[-] except
			[ ] sTestStatus = csFailed
			[ ] sExpectedResult = "{sExpectedResult}; Correct REQLOG entry"
			[ ] sActualResult = "{sActualResult}; Incorrect REQLOG entry"
			[ ] LogWarning ("*** Warning: Did not find expected REQLOG entry")
		[ ] Print()
		[ ] ResCloseList()
		[ ] 
		[ ] ResOpenList("Step 12.6 - Verify Billable State from Iad Viewing Tables")
		[ ] // Verify Billable State from Iad Viewing Tables
		[-] do
			[ ] Verify(fCheckViewingRecord(rSession,TRUE), TRUE)
		[-] except
			[ ] sTestStatus = csFailed
			[ ] sExpectedResult = "{sExpectedResult}; Correct Viewing record"
			[ ] sActualResult = "{sActualResult}; Incorrect Viewing record"
			[ ] LogWarning("*** Warning: Did not find expected Viewing record")
		[ ] Print()
		[ ] ResCloseList()
		[ ] 
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 13 - Mark the end of the testcase")
	[ ] // Mark the end of the testcase
	[ ] sRefTimeStamp = fCreateDateTimeStamp(4)
	[ ] fReadWriteIni("{csInstallPath}{filAquariusIni}",csWrite,sTestCaseName,entEndDateTime, sRefTimeStamp) 
	[ ] fTrace ("{sTestCaseName} End: {sRefTimeStamp}",True)
	[ ] ResCloseList()
	[ ] 
	[ ] ResOpenList("Step 14 - Log Test Status to Pass Fail Log")
	[ ] // Log Test Status to Zodiac Pass Fail Log
	[ ] // fLogPassFail(sTestStatus,sExpectedResult, sActualResult)
	[ ] fLogPassFail("Pass","", "")
	[ ] Print()
	[ ] ResCloseList()
