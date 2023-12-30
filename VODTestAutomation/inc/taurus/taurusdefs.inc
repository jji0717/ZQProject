[ ] //------------------------------------------------------------------------------
[ ] //111111102222222220333333333044444444405555555550666666666077777777708888888880
[ ] // Filename:    aquariusdefs.inc
[ ] // Project:     Aquarius
[ ] // Author:		CAC
[ ] // Description: Defines Aquarius specific constants  
[ ] // Created On:  12/13/2002
[ ] 
[ ] // Significant revisions:
[ ] // mm/dd/yyyy FML Revision Comment
[ ] //------------------------------------------------------------------------------
[ ] 
[ ] ////////////////////////////////////////////////////////////
[ ] // FILE NAMES
[ ] ////////////////////////////////////////////////////////////
[ ] 
[ ] const string csMbsLog = "\itv\log\Mbs.log"
[ ] 
[ ] ///////////////////////////////////////////////////////////
[ ] // SQL QUERIES
[ ] ///////////////////////////////////////////////////////////
[ ] const string csViewingQuery = "select count(*) from Viewing"
[ ] const string csInProgressQuery = "select count(*) from InProgress"
[ ] const string csReportedQuery = "select count(*) from Viewing_Reported where Reported = 1"
[ ] const string csNotReportedQuery = "select count(*) from Viewing_Reported where Reported = 0"
[ ] 
[ ] 
[ ] ///////////////////////////////////////////////////////////
[ ] // LOG FILE STRINGS
[ ] ///////////////////////////////////////////////////////////
[ ] const string csModBillingLogMessage1 = "(MOD) Movies on Demand Service started"
[ ] const string csModBillingLogMessage2 = "(MOD) Established connection to ITV DS"
[ ] const string csModBillingLogMessage3 = "(MOD) Established connection to ITV AD"
[ ] const string csModBillingLogMessage4 = "Entering CModBill::LoadMbsDll"
[ ] 
[ ] const string csMbsDstLogMessage1 = "CConfig::GetInteger: Using database for BillingDLL\DstType value - 1"
[ ] 
[ ] 
[ ] ///////////////////////////////////////////////////////////
[ ] // RECORDS 
[ ] ///////////////////////////////////////////////////////////
[-] type ITVMACHINEIPS is record
	[ ] STRING sCM1
	[ ] STRING sCM2
	[ ] STRING sMDS1
	[ ] STRING sMDS2
	[ ] STRING sAPP1
	[ ] STRING sAPP2
	[ ] STRING sPS1
	[ ] STRING sPS2
[ ] 
[-] type DBLOGONINFO is record
	[ ] STRING sDSN
	[ ] STRING sSRVR
	[ ] STRING sUID
	[ ] STRING sPWD
[ ] 
[ ] ////////////////////////////////////////////////////////////
[ ] // MISCELLANEOUS
[ ] ////////////////////////////////////////////////////////////
[ ] const string csIadDataPurge = "IadData_purge"
[ ] 
[ ] const string csBillingRegistryPath = "HKEY_LOCAL_MACHINE\SOFTWARE\SeaChange\ITV Applications\CurrentVersion\Services\Mod Billing"
