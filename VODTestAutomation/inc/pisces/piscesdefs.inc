[ ] //*****************************************************************************
[ ] //  NAME:					  PISCESDEFS.INC
[ ] //
[ ] //  DESCRIPTION:		Definition library for Pisces component of Zodiac project.
[ ] //
[ ] //  HISTORY:
[ ] //
[ ] //  Date						Developer					Description
[ ] //  ********				***********				*****************************************
[ ] //  12/11/02        K. Sullivan       Initial design and development.
[ ] //
[ ] //*****************************************************************************************************
[ ] 
[ ] // Service Names
[ ] 
[+] LIST OF STRING lsITVServices1= {...}
	[ ] "Movies on Demand"
	[ ] "ICM"
	[ ] "SCTP"
	[ ] "MOD Billing"
	[ ] "IAM"
[ ] STRING sITVService1
[ ] 
[+] LIST OF STRING lsITVServices2= {...}
	[ ] "Movies on Demand2"
	[ ] "ICM2"
	[ ] "SCTP2"
[ ] STRING sITVService2
[ ] 
[ ] const string sIISName = "IISAdmin"
[ ] 
[ ] // Records
[ ] 
[+] type ITVMACHINEIPS is record
	[ ] STRING sCM1
	[ ] STRING sCM2
	[ ] STRING sMDS1
	[ ] STRING sMDS2
	[ ] STRING sAPP1
	[ ] STRING sAPP2
	[ ] STRING sPS1
	[ ] STRING sPS2
[ ] 
[+] type ReportData is record
	[ ] WINDOW wReport
	[ ] WINDOW wParam
	[ ] WINDOW wDisplay
	[ ] WINDOW wExport
	[ ] STRING sDateFrom
	[ ] STRING sDateTo
[ ] 
[ ] //Misc
[ ] 
[ ] //Service control commands.
[ ] const string sStop = "stop"
[ ] const string sStart = "start"
[ ] const string sRestart = "restart"
[ ] const string sStatus = "status"
[ ] 
