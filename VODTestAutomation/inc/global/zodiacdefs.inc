[ ] //*****************************************************************************
[ ] //  NAME:					  ZODIACDEFS.INC
[ ] //
[ ] //  DESCRIPTION:		Constant definitions common to all Zodiac components.
[ ] //                  
[ ] //
[ ] //  HISTORY:
[ ] //
[ ] //  Date						Developer					Description
[ ] //  ********				***********				*****************************************
[ ] //  12/11/02        M. Albanese       Initial design and development
[ ] //
[ ] //*****************************************************************************
[ ] 
[ ] //*****************************************************************************
[ ] // NEW ADDED GLOBAL sPS1VALS BY ZQ
[ ] //*****************************************************************************
[ ] 
[ ] // path for the test
[ ] const string csInstallPath		= "c:\autotest\"
[ ] 
[ ] // script log name
[ ] string gsScriptName 
[ ] string gsPassFailLog 
[ ] 
[ ] // service names
[ ] const string csIAMName = "IAM"
[ ] const string csMODName = "Movies on Demand"
[ ] const string csMOD2Name = "Movies on Demand2"
[ ] const string csICMName= "ICM"
[ ] const string csICM2Name = "ICM2"
[ ] const string csSCTPName = "SCTP"
[ ] const string csSCTP2Name = "SCTP2"
[ ] const string csMODBillingName = "MOD Billing"
[ ] const string csIADName = "IAD"
[ ] const string csSysmonName = "Sysmon"
[ ] 
[ ] // service status params
[ ] const string csStop = "stop"
[ ] const string csStart = "start"
[ ] const string csRestart = "restart"
[ ] const string csStatus = "status"
[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] // FILENAMES (fil)
[ ] //*****************************************************************************
[ ] 
[ ] const string filZodiacIni			= "zodiac\ctl\global\zodiac.ini"
[ ] 
[ ] const string filAquariusIni 	= "zodiac\ctl\aquarius\aquariuscfg.ini"
[ ] const string filAquariusXls 	= "zodiac\ctl\aquarius\aquariuscfg.xls"
[ ] const string filAquariusLog 	= "zodiac\log\asPWDquarius\aquariuslog.txt"
[ ] 
[ ] const string filPiscesIni 		= "zodiac\ctl\pisces\piscescfg.ini"
[ ] const string filPiscesXls 		= "zodiac\ctl\pisces\piscescfg.xls"
[ ] const string filPiscesLog 		= "zodiac\log\pisces\pisceslog.txt"
[ ] 
[ ] const string filScorpioIni 		= "zodiac\ctl\scorpio\scorpiocfg.ini"
[ ] const string filScorpioXls 		= "zodiac\ctl\scorpio\scorpiocfg.xls"
[ ] const string filScorpioLog 		= "zodiac\log\scorpio\scorpiolog.txt"
[ ] 
[ ] const string filTaurusIni 		= "zodiac\ctl\taurus\tauruscfg.ini"
[ ] const string filTaurusXls 		= "zodiac\ctl\taurus\tauruscfg.xls"
[ ] const string filTaurusLog 		= "zodiac\log\taurus\tauruslog.txt"
[ ] 
[ ] //*****************************************************************************
[ ] // INI FILE SECTIONS (sec)
[ ] //*****************************************************************************
[ ] 
[ ] const string secGeneric				=	"Generic"
[ ] 
[ ] //*****************************************************************************
[ ] // INI FILE ENTRIES (ent)
[ ] //*****************************************************************************
[ ] 
[ ] const string entStartDateTime	=	"StartDateTime"
[ ] const string entDataDrive 		= "DataDrive"
[ ] // machine ips 
[ ] const string entAM1IP = "AM1IP"
[ ] const string entCM1IP = "CM1IP"
[ ] const string entCM2IP = "CM2IP"
[ ] const string entMDS1IP = "MDS1IP"
[ ] const string entMDS2IP = "MDS2IP"
[ ] const string entAPP1IP = "APP1IP"
[ ] const string entAPP2IP = "APP2IP"
[ ] const string entPS1IP = "PS1IP"
[ ] const string entPS2IP = "PS2IP"
[ ] 
[ ] string gsAM1IP = ""
[ ] string gsCM1IP = ""
[ ] string gsCM2IP = ""
[ ] string gsMDS1IP = ""
[ ] string gsMDS2IP = ""
[ ] string gsAPP1IP = ""
[ ] string gsAPP2IP = ""
[ ] string gsPS1IP = ""
[ ] string gsPS2IP = ""
[ ] 
[ ] // database connection params
[ ] const string entMDSSQLServer = "MDSSQLServer"
[ ] const string entPSSQLServer = "PSSQLServer"
[ ] const string entSQLUid = "SQLUid"
[ ] const string entSQLPassword = "SQLPassword"
[ ] const string secODBCDsn = "ODBCDsn"
[ ] const string entDSN = "MDSDSN"
[ ] const string secCommandCenter = "CommandCenter"
[ ] const string entPassword = "Password"
[ ] 
[ ] string gsMDSSQLServer = ""
[ ] string gsDSN = ""
[ ] string gsUid = ""
[ ] string gsPassword = ""	
[ ] string gsLocalITVRoot = ""
[ ] string gsRemoteITVRoot = ""
[ ] string gsRemoteExternalRoot = ""
[ ] 
[ ] //ITVRoot ent&sec
[ ] const string secITVRoot = "ITVRoot"
[ ] const string entLocalITVRoot = "LocalITVRoot"
[ ] const string entRemoteITVRoot = "RemoteITVRoot"
[ ] const string entRemoteITVExternalRoot = "RemoteITVExternalRoot"
[ ] 
[ ] //VideoServers params
[ ] const string secVideoServers = "VideoServers"
[ ] const string entVideoServers = "VideoServers"
[ ] const string entUploadClusterN0IP = "UploadClusterN0IP"
[ ] const string entUploadClusterN1IP = "UploadClusterN1IP"
[ ] const string entUploadClusterN2IP = "UploadClusterN2IP"
[ ] const string entTargetCluster1N0IP = "TargetCluster1N0IP"
[ ] const string entTargetCluster1N1IP = "TargetCluster1N1IP"
[ ] const string entTargetCluster1N2IP = "TargetCluster1N2IP"
[ ] const string entTargetCluster2N0IP = "TargetCluster2N0IP"
[ ] const string entTargetCluster2N1IP = "TargetCluster2N1IP"
[ ] const string entTargetCluster2N2IP = "TargetCluster2N2IP"
[ ] const string entAccount = "Account"
[ ] const string entv = "Password"
[ ] 
[ ] string gsUploadClusterN0IP = ""
[ ] string gsUploadClusterN1IP = ""
[ ] string gsUploadClusterN2IP = ""
[ ] string gsTargetCluster1N0IP = ""
[ ] string gsTargetCluster1N1IP = ""
[ ] string gsTargetCluster1N2P = ""
[ ] string gsTargetCluster2N0IP = ""
[ ] string gsTargetCluster2N1IP = ""
[ ] string gsTargetCluster2N2IP = ""
[ ] 
[ ] //*****************************************************************************
[ ] // DATABASE STRINGS
[ ] //*****************************************************************************
[ ] 
[ ] const string csDnpConfig			=	"DnpConfig"
[ ] const string csIadData				=	"IadData"
[ ] const string csIamData				=	"IamData"
[ ] const string csIcmData				=	"IcmData"
[ ] const string csIdsData				=	"IdsData"
[ ] const string csScConfig				=	"ScConfig"
[ ] const string csSubscriberData	=	"SubscriberData"
[ ] 
[ ] //*****************************************************************************
[ ] // MISCELLANEOUS STRINGS
[ ] //*****************************************************************************
[ ] 
[ ] const string csPass 					= "Pass"
[ ] const string csFail 					= "Fail"
[ ] const string csPassed 				= "Passed"
[ ] const string csFailed 				= "Failed"
[ ] const string csRead 					= "Read"
[ ] const string csWrite 					= "Write"
[ ] const string csMainLogPath 		= "zodiac\log\"
[ ] 
[ ] //*****************************************************************************
[ ] // END: ZODIACDEFS.INC
[ ] //*****************************************************************************
