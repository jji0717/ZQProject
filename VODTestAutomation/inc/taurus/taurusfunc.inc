[ ] //*****************************************************************************
[ ] //  NAME:					  TAURUSFUNC.INC
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
[ ] //  1/2/03          J-Money           Initial design and development
[ ] //*****************************************************************************
[ ] //
[ ] // Module contains these functions:
[ ] //  NAME                    STATUS (peer reviewed or not)
[ ] //  fDBQueryForString()    Reviewed
[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fDBQueryForString()
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
[ ] //*****************************************************************************
[ ] 
[ ] use "..\global\zodiacdatabase.inc"
[+] STRING fDBQueryForString(string sDBLogOnInfo, string sSQLQuery, string sDBName optional)
	[ ] HSQL hstmnt
	[ ] HDATABASE hdbc
	[ ] STRING sReturnString
	[ ] 
	[ ] print ("Starting fDBQuery.")
	[ ] hdbc = fDBConnect(sDBLogOnInfo)
	[-] if sDBName != NULL
		[ ] hstmnt = DB_ExecuteSql(hdbc,"use {sDBName}")
	[ ] hstmnt = DB_ExecuteSqL (hdbc,sSQLQuery)
	[+] // while (DB_FetchNext(hstmnt,lDbList)== TRUE)
		[ ] // // This append statement is what builds the list that the function returns.
		[ ] // // ListAppend (lDbReturnList, lDbList)
		[ ] // sReturnString = lDbList
	[ ] DB_FetchNext(hstmnt, sReturnString)
	[ ] DB_FinishSQL (hstmnt)
	[ ] DB_Disconnect (hdbc)
	[ ] print ("Disconnected from the database.")
	[ ] print ("End fDBQuery.")
	[ ] return sReturnString
[ ] 
[-] STRING fExpand2Unicode(string sSourceStr)
	[ ] INTEGER iFill
	[ ] INTEGER i
	[ ] STRING	sResultStr
	[ ] 
	[ ] iFill=0
	[ ] sResultStr = ""
	[ ] 
	[-] for(i=1;i<=Len(sSourceStr);i++)
		[ ] sResultStr[i*2-1] = sSourceStr[i]
		[ ] sResultStr[i*2] = "$"
		[ ] sResultStr[i*2] = Chr(iFill)
		[ ] 
	[ ] return sResultStr
