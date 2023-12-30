[ ] //******************************************************************************
[ ] //  NAME:           AQUARIUSDEFS.INC
[ ] //
[ ] //  DESCRIPTION:    Constant definitions common to Aquarius Tests.
[ ] //
[ ] //  HISTORY:
[ ] //
[ ] //  Date            Developer         Description
[ ] //  ********		***********		  ******************************************
[ ] //  02/05/03        C. Callahan       Initial design and development.  First
[ ] //                                    testcase Aquarius001 runs.  Strings and
[ ] //                                    datastructures used by the testcase
[ ] //                                    are defined.  See docs in QA Lotus 
[ ] //                                    Notes db for detailed status.
[ ] //******************************************************************************
[ ] //111111102222222220333333333044444444405555555550666666666077777777708888888880
[ ] //******************************************************************************
[ ] // HELPFUL 
[ ] //******************************************************************************
[ ] const string csNL = chr(10)
[ ] 
[ ] //******************************************************************************
[ ] // potential Zodiac globals
[ ] //******************************************************************************
[ ] const string csITVRoot = "C:\ITV"
[ ] const string csITVRootShared = "C$\ITV"
[ ] const string csSRM1Log = "srm.log"
[ ] const string csSRM2Log = "srm2.log"
[ ] const string csDMN1Log = "idmn.log"
[ ] const string csDMN2Log = "idmn2.log"
[ ] const string csCM1Log = "icm.log"
[ ] const string csCM2Log = "icm2.log"
[ ] const string csSSAPI_CM1Log = "issapi_icm0.log"
[ ] const string csSSAPI_CM2Log = "issapi_icm2.log"
[ ] const string csSSAPI_DMN1Log = "issapi_idmn1.log"
[ ] const string csSSAPI_DMN2Log = "issapi_idmn2.log"
[ ] const string csSSAPI_MOD1Log = "issapi_mod1.log"
[ ] const string csSSAPI_MOD2Log = "issapi_mod2.log"
[ ] const string csSSAPI_SSLog = "issapi_ss0.log"
[ ] const string csMOD1Log = "mod.log"
[ ] const string csMOD2Log = "mod2.log"
[ ] const string csSSLog = "iss0.log"
[ ] const string csMCAClientLog = "mcaclient.log"
[ ] const string csMCAServerLog = "mcaserver.log"
[ ] const string csxmlMODIFYRoot = "Zodiac\Tools\aquarius"
[ ] 
[ ] //******************************************************************************
[ ] // FILENAMES (fil)
[ ] //******************************************************************************
[ ] 
[ ] const string dirTestFiles = "AquariusTestFiles"
[ ] const string filSledgeHammerTemplate = "SledgeHammer.xml"
[ ] const string filSledgeHammerWorking 	= "SledgeHammer.xml"
[ ] const string filSledgehammerLog 	= "SledgeHammer.log"
[ ] const string filSessionPlLog 	= "SessionPl.log"
[ ] const string filSessionPlCfg 	= "SessionPlLogs.cfg"
[ ] 
[ ] //******************************************************************************
[ ] // PATH (dir)
[ ] //******************************************************************************
[ ] const string dirAquariusIni = "zodiac\tools\aquarius\"
[ ] 
[ ] //******************************************************************************
[ ] // INI FILE SECTIONS (sec)
[ ] //******************************************************************************
[ ] const string secTestFiles =  "TestFiles"
[ ] const string secTestCase =  "TestCase"
[ ] const string secSledgehammer = "Sledgehammer"
[ ] const string secSessionPl = "SessionPl"
[ ] const string secTemporary = "Temporary"
[ ] const string secAquarius = "Aquarius0001"
[ ] 
[ ] // const string secODBCDsn= "ODBCDsn"
[ ] // const string secCommandCenter = "CommandCenter"
[ ] 
[ ] //******************************************************************************
[ ] // INI FILE ENTRIES (ent)
[ ] //******************************************************************************
[ ] const string entLocalRoot =  "AquariusLocalRoot"
[ ] const string entEndDateTime	=	"EndDateTime"
[ ] const string entITVServices = "ITVServices"
[ ] const string entSledgehammerXML = "SledgehammerXML"
[ ] const string entSledgehammerTestName = "SledgehammerTestName"
[ ] const string entExecutable = "Executable"
[ ] const string entSessionPlCfg = "SessionPlCfg"
[ ] 
[ ] const string entSledgehammerAssets = "SledgehammerAssets"
[ ] const string entSledgehammerNodegroup= "SledgehammerNodegroup"
[ ] const string entReqlogStatus = "ReqlogStatus"
[ ] const string entReqlogExtendedStatus = "ReqlogExtendedStatus"
[ ] const string entBillable = "Billable"
[ ] 
[ ] // const string entMDSSQLServer = "MDSSQLServer"
[ ] // const string entPSSQLServer = "PSSQLServer"
[ ] // const string entSQLUid = "SQLUid"
[ ] // const string entSQLPassword = "SQLPassword"
[ ] 
[ ] 
[ ] //******************************************************************************
[ ] // TEST CASE DEFAULTS
[ ] //******************************************************************************
[+] const list of string clsDefaultServices = {...}
	[ ] "SRM"
	[ ] "DMN"
	[ ] "CM"
	[ ] "SSAPI"
	[ ] "SS"
	[ ] "MOD"
	[ ] "MCAClient"
	[ ] "MCAServer"
[ ] 
[ ] //******************************************************************************
[ ] // XML for modify
[ ] //******************************************************************************
[ ] 
[-] const list of string clsXMLTagSeries = {...}
	[ ] "simulation/clientgroup"
	[ ] "simulation/clientgroup/iptarget"
	[ ] "simulation/clientgroup/iptarget"
	[ ] "simulation/clientgroup/iptarget"
	[ ] "simulation/test/assetlist"
	[ ] "simulation/test/assetlist/asset"
	[ ] "simulation/test/srmlist/srm"
	[ ] "simulation/test/cmlist/srm"
	[ ] "simulation/test/sctplist/srm"
	[ ] "simulation/test/clientattribute"
[ ] 
[-] const list of string clsXMLAttrName = {...}
	[ ] "nodegroup"
	[ ] "mac"
	[ ] "ip"
	[ ] "port"
	[ ] "appuid"
	[ ] "id"
	[ ] "host"
	[ ] "host"
	[ ] "host"
	[ ] "value"
[ ] 
[ ] //******************************************************************************
[ ] // Sledgehammer Info
[ ] //******************************************************************************
[+] type etSledgehammerState is enum
	[ ] undefined
	[ ] initialized
	[ ] starting
	[ ] running
	[ ] complete
[ ] 
[-] type etSessionState is enum
	[ ] created
	[ ] setup
	[ ] terminated
[ ] 
[-] type rtSessionInfo is record
	[ ] string sId
	[ ] etSessionState eSledgehammerState
	[ ] string sAssetId
	[ ] string sPurchaseTime
	[ ] boolean bStatus //Keep track of state of data structure as it's passed around
[ ] 
[-] type rtSledgehammer is record
	[ ] etSledgehammerState eState
	[ ] string sXMLFile
	[ ] string sLog
	[ ] list of rtSessionInfo lrSessions
[ ] 
[ ] const string csSledgehammerStart = "* * *   H A M M E R  (R E) S T A R T   * * *"
[ ] const string csSledgehammerSettop = "ActInit(): created set-top"
[ ] //test "DSM-CC LSC/TCP" completed. 1 of 1 iterations...
[ ] const string csSledgehammerStopPre = "test {Chr(34)}"
[ ] const string csSledgehammerStopPost = "{Chr(34)} completed"
[ ] 
[ ] //******************************************************************************
[ ] // Sledgehammer Strings
[ ] //******************************************************************************
[+] type rtLogMessage is record
	[ ] string sPreSessionId
	[ ] string sPostSessionId
	[ ] boolean bWildCard
	[ ] string sParseFunction
[ ] 
[-] const list of rtLogMessage clrSledgehammerLog = {...}
	[ ] //AC:AC:AC:AC:AC:00/0434133290 sending session setup with AssetUID=0x1bc00a53, AppType=0x1bc00001, HomeID=32, SmartCardID=48, PurchaseID=0, PackageID=0
	[ ] {""," Sending setup request with", FALSE,"fSledgehammerSessionSetup"}
	[ ] //AC:AC:AC:AC:AC:00/0434133290 received OK session setup response with XactionID=1, Freq=573, Prog#=2, ModFmt=QAM256, Response=0x0
	[ ] {""," received OK session setup response",FALSE,""}
	[ ] //AC:AC:AC:AC:AC:00/0434133290 connected LSC interface to 192.168.4.31:8020
	[ ] {""," connected LSC interface",FALSE,""} //fSledgehammerStreamOpConnect"}
	[ ] //AC:AC:AC:AC:AC:00/0434133290 Session terminating normally. Sending session release request. [reason=0x1]
	[ ] {""," Session terminating normally. Sending session release request.",FALSE,""}
	[ ] //AC:AC:AC:AC:AC:00/0434133290 CMDMGR: Client[1]: session release OK
	[ ] {"*"," CMDMGR: Client[*]: session release OK*",TRUE,""} //fSledgehammerSessionReleaseOk"}
[ ] 
[ ] // Error messages when the session fails:\
[ ] // Perhaps look for these when above not successful
[ ] //  MSME:0x00305750:[02/16] - AC:AC:AC:AC:AC:00/1128521818 all UNSessionSetupRequest retry attempts have failed (0 attempts) [XID=1]
[ ] //  MSME:0x00305750:[02] - AC:AC:AC:AC:AC:00/1128521818 CMDMGR: Client[1]: session setup timed out
[ ] 
[ ] // key strings in log messages used to locate information
[ ] // to pull from the message.
[ ] const string csAssetTag = "AssetUID="
[ ] const string csPackageTag = "PackageID="
[ ] 
[ ] //******************************************************************************
[ ] // Session Pl STRINGS
[ ] //******************************************************************************
[ ] const string csSessionPlWindow = "Aquarius Session Pl "
[ ] 
[+] type rSessionLogMarker is record
	[ ] string sStart
	[ ] string sStop
[ ] 
[+] rSessionLogMarker crSRMMarker = {...}
	[ ] "======= [SRM] SEARCHING"
	[ ] "======= [SRM] SEARCH COMPLETE"
[ ] 
[-] const list of rtLogMessage clrSRMLog = {...}
	[ ] {"","Received ClientSessionSetUpRequest",FALSE,""}
	[ ] {"","Sent clientSessionSetupConfirm",FALSE,""}
	[ ] {"","Received ClientReleaseRequest",FALSE,""}
	[ ] {"","Sent ClientReleaseConfirm",FALSE,""}
	[ ] {"","Session Deleted",FALSE,""}
[ ] 
[+] rSessionLogMarker crDMNMarker = {...}
	[ ] "======= [DMN] SEARCHING"
	[ ] "======= [DMN] SEARCH COMPLETE"
[ ] 
[+] const list of rtLogMessage clrDMNLog = {...}
	[ ] {"","ActHandleMessage(): Successfully added messageId",FALSE,""}
	[ ] {"","ActHandleSsCallback(): Received SS callback",FALSE,""}
	[ ] {"","ReqLogDb::AddReqLogEntry(): queued work-item",FALSE,""}
	[ ] {"","ReqLogDb::AddReqLogEntry(): successfully submitted ReqLog entry",FALSE,""}
	[ ] {"","ReqLogDb::UpdateReqLogEntry(): queued work-item",FALSE,""}
	[ ] {"","ReqLogDb::UpdateReqLogEntry(): successfully submitted ReqLog entry update for sessionId",FALSE,""}
[ ] 
[+] rSessionLogMarker crCMMarker = {...}
	[ ] "======= [CM] SEARCHING"
	[ ] "======= [CM] SEARCH COMPLETE"
[ ] 
[+] const list of rtLogMessage clrCMLog = {...}
	[ ] {"","ActFilterConSetUpIndication",FALSE,""} //Look for Asset=1ed811c7 in this string
	[ ] {"","ActCsetUpRespOK()",FALSE,""} //SetupOK STREAMID Sid=1046 SOP=SC_SOP PrimIp=192.168.104.113 - Port=8002; AltIp=192.168.4.113 - Port=8002 
	[ ] {"","ActAbortStreamSM()",FALSE,""} //ActAbortStreamSM(): CON_SM=[CTD/27] Key=AC:AC:AC:AC:AC:00/0003956469, Status=0x01010000 - Received Release Indication with Reason=1; UserReason=1
	[ ] {"","ActRecordSMEndEvent()",FALSE,""} //: CON_SM=[TM1/4] Key=AC:AC:AC:AC:AC:00/0003956469, Status=0x00000000 - Processing EvtSSMTerminate event
[ ] 
[+] rSessionLogMarker crSSApiMarker = {...}
	[ ] "======= [SSAPI] SEARCHING"
	[ ] "======= [SSAPI] SEARCH COMPLETE"
[ ] 
[+] const list of rtLogMessage clrSSApiLog = {...}
	[ ] {"","Entering IssCreateStream",FALSE,""}
	[ ] {"","Exiting IssDNetReady",FALSE,""}
	[ ] {"","Entering IssTerminateStream",FALSE,""}
	[ ] {"","Processing TerminateStream return Pdu",FALSE,""}
	[ ] {"","Nuking stream, Sid",FALSE,""}
[ ] 
[+] rSessionLogMarker crSSMarker = {...}
	[ ] "======= [SS] SEARCHING"
	[ ] "======= [SS] SEARCH COMPLETE"
[ ] 
[+] const list of rtLogMessage clrSSLog = {...}
	[ ] {"","HLSM::ActionCreate",FALSE,""}
	[ ] {"","StreamMgr::SendCallback (StateChg/ASACCSTRM)",FALSE,""}
	[ ] {"","StreamMgr::SendCallback (StateChg/STRMSTART)",FALSE,""}
	[ ] {"","HLSM::ActionStreamRundown",FALSE,""}
	[ ] {"","HLSM::ActionToTerm TERMINATE_STREAM",FALSE,""}
	[ ] {"","HLSM::ActionHlsmPoof",FALSE,""}
[ ] 
[ ] const string sSSLogPlay ="StreamMgr::SendCallback (StateChg/STRMPLAYING)"
[ ] const string sSSLogRewind = "StreamMgr::SendCallback (StateChg/STRMREWINDING)"
[ ] const string sSSLogFF = "StreamMgr::SendCallback (StateChg/STRMFSTFWDING)"
[ ] 
[+] rSessionLogMarker crMODMarker = {...}
	[ ] "======= [MOD] SEARCHING"
	[ ] "======= [MOD] SEARCH COMPLETE"
[ ] 
[+] const list of rtLogMessage clrMODLog = {...}
	[ ] {"","Entering CModApp::CreateSession",FALSE,""}
	[ ] {"","Exiting CModApp::CreateSession",FALSE,""} // Find Success - Sid=2080.00040001/1 Success, Cnt=5,BillingId=22,PurchaseTime=0,PlayTimeLeft=0,HomeId=32,SmartCardId=48,PurchaseId=0,SigAnalogCopyPurchase=0,PackageId=0
	[ ] {"","Entering ActionAcceptStream",FALSE,""}
	[ ] {"","Exiting ActionAcceptStream",FALSE,""} //Find Success - Sid=2080.00040001/1 Success
	[ ] {"","Entering ActionStartStream",FALSE,"fGetPurchaseTimeFromModLog"} // - Sid=2080.00040001/1,PurchaseTime=1042664833
	[ ] {"","Entering ActionTermRespOk",FALSE,""}
	[ ] {"","Entering CModApp::RemoveSession",FALSE,""}
	[ ] {"","Entering CSession::Uninitialize",FALSE,""}
	[ ] {"","Exiting CSession::Uninitialize",FALSE,""}
[ ] 
[ ] // key strings in log messages used to locate information
[ ] // to pull from the message.
[ ] const string csTag = "PurchaseTime="
[ ] 
[+] rSessionLogMarker crMCAClientMarker = {...}
	[ ] "======= [MCAClient] SEARCHING"
	[ ] "======= [MCAClient] SEARCH COMPLETE"
[ ] 
[+] const list of rtLogMessage clrMCAClientLog = {...}
	[ ] {"","Stream state: MCA_STATE_ALLOC to MCA_STATE_IDLE",FALSE,""}
	[ ] {"","Stream state: MCA_STATE_IDLE to MCA_STATE_PLAYOUT",FALSE,""}
	[ ] {"","API: MCATerminateStream ( Enter )",FALSE,""}
	[ ] {"","Stream state: MCA_STATE_PLAYOUT to MCA_STATE_STOPPED",FALSE,""}
	[ ] {"","Stream state: MCA_STATE_STOPPED to MCA_STATE_TERMINATE",FALSE,""}
	[ ] {"","Context block deleted key.parentHandle",FALSE,""}
[ ] 
[+] rSessionLogMarker crMCAServerMarker = {...}
	[ ] "======= [MCAServer] SEARCHING"
	[ ] "======= [MCAServer] SEARCH COMPLETE"
[ ] 
[+] const list of rtLogMessage clrMCAServerLog = {...}
	[ ] {"","Init stream created new stream context",FALSE,""}
	[ ] {"","Building Request Stream Event response [(0x3AE0000) MCA_NORMAL - Normal completion] [(0x0) The operation completed successfully.]",FALSE,""}
	[ ] {"","Stream state: MCA_STATE_IDLE to MCA_STATE_PRIMED",FALSE,""}
	[ ] {"","Stream state: MCA_STATE_PRIMED to MCA_STATE_PLAYOUT",FALSE,""}
	[ ] {"","Received Terminate stream message from CtxHandle",FALSE,""}
	[ ] {"","Stream state: MCA_STATE_PLAYOUT to MCA_STATE_STOPPING",FALSE,""}
	[ ] {"","Stream state: MCA_STATE_STOPPING to MCA_STATE_TERMINATE",FALSE,""}
	[ ] {"","Building Terminate Stream response [(0x3AE0000) MCA_NORMAL - Normal completion] [(0x0) The operation completed successfully.]",FALSE,""}
	[ ] {"","Context block deleted key.parentHandle",FALSE,""}
[ ] 
[ ] const string sMCAServerLogPlay = "NPT pos: -5/0 motion: 1/1"
[ ] const string sMCAServerLogRewind = "NPT pos: -5/0 motion: -6/1"
[ ] const string sMCAServerLogFF = "NPT pos: -5/0 motion: 6/1"
[ ] 
[ ] //******************************************************************************
[ ] // DB Queriess
[ ] //******************************************************************************
[ ] const string csGetAssetIdREQLOGPre = "select ASSET from CMREQLOG where SESSION = '"
[ ] const string csGetAssetIdREQLOGPost = "'"
[ ] const string csGetNodegroupREQLOGPre = "select NODEGRP from CMREQLOG where SESSION = '"
[ ] const string csGetNodegroupREQLOGPost = "'"
[ ] const string csGetStatusREQLOGPre = "SELECT NAME from CMREQLOG JOIN CMREQLOG_STATUS_CODES ON CMREQLOG.STATUS = CMREQLOG_STATUS_CODES.STATUS where CMREQLOG.SESSION = '"
[ ] const string csGetStatusREQLOGPost = "'"
[ ] const string csGetExtStatusREQLOGPre = "SELECT NAME from CMREQLOG JOIN CMREQLOG_EXTSTATUS_CODES ON CMREQLOG.EXTENDEDSTATUS = CMREQLOG_EXTSTATUS_CODES.STATUS where CMREQLOG.SESSION = '"
[ ] const string csGetExtStatusREQLOGPost = "'"
[ ] const string csGetBillableIadPre = "SELECT Billable from Viewing_Billable JOIN Viewing ON Viewing_Billable.OBJECT_UID = Viewing.OBJECT_UID where Viewing.NAME = '"
[ ] const string csGetBillableIadPost = "'"
[ ] //******************************************************************************
[ ] // END: AQUARIUSDEFS.INC
[ ] //******************************************************************************
