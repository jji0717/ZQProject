[ ] //*****************************************************************************
[ ] //  NAME:					  SCORPIOFUNC.INC
[ ] //
[ ] //  DESCRIPTION:		Scorpio-specific libary functions.
[ ] //
[ ] //  HISTORY:
[ ] //
[ ] //  Date						Developer					Description
[ ] //  ********				***********				*****************************************
[ ] //  01/23/03        M. Albanese       Initial design and development
[ ] //
[ ] //*****************************************************************************
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fDBQueryForInt()
[ ] //
[ ] //   ARGUMENTS	  :	string sDBLogOnInfo string sSQLQuery string sDBName optional
[ ] //
[ ] //									-string sDBLogOnInfo is string consisting of the DSN Name, the 
[ ] //									Database Server name, the User Id and the Password of the database 
[ ] //									under test. An example of the string: "DSN=ZFRED1;SRVR=ZFRED1;UID=sa;PWD=".
[ ] //									-string sSQLQuery is the SQL query you want to run against
[ ] //									the database under test, such as "select * from Asset".
[ ] //									-string sDBName is the database name you want to run the query
[ ] //									against, such as IdsData, IcmData, etc.  This is optional.
[ ] //	               		  
[ ] //   RETURNS	    : Return an integer value from the SQL query.
[ ] //
[ ] //   DESCRIPTION	:	Connects to the specified database using the specified log on info
[ ] //									and executes the specified query, and returns the results in a list.
[ ] //
[ ] //
[ ] //	 NOTE					: This is the same code as fDBQuery written by K. Sullivan
[ ] //									except we return an integer rather than 'anytype'.
[ ] //									Needed this capability for grabbing Asset and Ae IDs.
[ ] //
[ ] //*****************************************************************************
[ ] 
[-] INTEGER fDBQueryForInt(string sDBLogOnInfo, string sSQLQuery, string sDBName optional)
	[ ] 
	[ ] HSQL hstmnt
	[ ] HDATABASE hdbc
	[ ] // integer iRetVal by rommy followed
	[ ] integer iRetVal = -1
	[ ] 
	[ ] hdbc = fDBConnect(sDBLogOnInfo)
	[-] if sDBName != NULL
		[ ] hstmnt = DB_ExecuteSql(hdbc,"use {sDBName}")
	[ ] hstmnt = DB_ExecuteSqL (hdbc,sSQLQuery)
	[ ] DB_FetchNext(hstmnt,iRetVal)
	[ ] DB_FinishSQL (hstmnt)
	[ ] DB_Disconnect (hdbc)
	[ ] 
	[ ] return iRetVal
