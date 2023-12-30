[ ] //*****************************************************************************
[ ] //  NAME:					  ZodiacNT.inc
[ ] //
[ ] //  DESCRIPTION:		Defines global functions that access NT components:
[ ] //                  Services, Registry, 'Net use' etc.
[ ] 
[ ] //  HISTORY:
[ ] //
[ ] //  Date						Developer					Description
[ ] //  ********				***********				*****************************************
[ ] //  12/18/02        C. Callahan       Created. Added function fGetIPCAccess()
[ ] //  01/09/03        J. Notaro         Added fManageService()
[ ] //	01/15/03				J. Notaro					Reworked fManageService(), added fServiceStatus()
[ ] //  01/22/03				J. Notaro					Added Registry functions
[ ] //
[ ] //*****************************************************************************
[ ] //
[ ] // Module contains these functions:
[ ] //  NAME             STATUS (peer reviewed on not)
[ ] //	fGetIPCAccess()  Not Reviewed
[ ] //
[ ] //*****************************************************************************
[ ] // fGetIPCAccess()
[ ] // Description:
[ ] //   Executes the net use command to get IPC access to the specified Computer/NIC
[ ] // Returns:
[ ] //   boolean: TRUE if caller can assume NETBUI access.  FALSE if an error occurred
[ ] // Arguments:
[ ] //  in string sNIC
[ ] //     specifies the IPaddress or computer name.  If the supplied address is
[ ] //     0.0.0.0, then the net use command is not executed and the return value is
[ ] //     TRUE.  This allows one to have a "don't care value".
[ ] //  in string sUsername
[ ] //     specifies the Username to use to access the computer
[ ] //  in string sPassword 
[ ] //     specifies the Password for the given Username
[ ] //  out list of string lsCmdOutput optional
[ ] //     If caller supplies the argument, the output from the net use command is 
[ ] //     returned. 
[ ] //*****************************************************************************
[+] boolean fGetIPCAccess (in string sNic, in string sUsername, in string sPassword, out list of string lsCmdOutput optional)
	[ ] list of string lsOutput = {""}
	[ ] integer iReturn = 0
	[ ] 
	[ ] // remove inadvertent trailing white space
	[ ] sNic = RTrim(sNic)
	[ ] sUsername = RTrim(sUsername)
	[ ] sPassword = RTrim(sPassword)
	[ ] 
	[ ] // 0.0.0.0 is a special "don't care" value
	[+] if !(sNic == "0.0.0.0")
		[ ] iReturn = SYS_Execute ("net use \\{sNic} /U:{sUsername} {sPassword}", lsOutput) 
		[ ] // Let caller print results if they want them
		[ ] // Print ("net use {sNic} returned: {iReturn}, {lsOutput}")
		[+] if (lsCmdOutput != NULL)
			[ ] lsCmdOutput = lsOutput
	[+] if (iReturn == 0)
		[ ] return TRUE
	[ ] // from testing, the return value is '2' if an error occurs.  Some errors don't mean
	[ ] // IPC access failed.  Trap for these.
	[+] else if  (iReturn == 2)
		[+] if (StrPos("1219", lsOutput[1]))
			[ ] // Error: "System error 1219 has occurred, The credentials supplied conflict with an existing set of credentials"
			[ ] // Still have IPC access
			[ ] return TRUE
		[ ] // add additional errors, that mean caller still has access.
	[ ] return FALSE
[ ] 
[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION    	:		fManageService
[ ] //
[ ] //   REQUIRED
[ ] //   ARGUMENTS    :		string sService
[ ] //											-	The name of the service
[ ] //										string sAction
[ ] //											- The action to perform on the service using instserv
[ ] //										string sMachineIP
[ ] //											- The IP address of the target machine
[ ] // 	 OPTIONAL
[ ] //   ARGUMENTS    : 	string sExtendedStatus
[ ] //											-	On Error this string is set. It's up to the caller to use
[ ] //												this functionality or not. 
[ ] //
[ ] //   RETURNS			: 	boolean -
[ ] //										FALSE	= if the operation failed.
[ ] //										TRUE = if the operation was successful.
[ ] //   DESCRIPTION 	: 	This function allows for the starting, stopping, and
[ ] //										restarting of a service on a target machine.
[ ] //
[ ] //*****************************************************************************
[ ] 
[-] boolean fManageService(in string sService, in string sAction, in string sMachineIP, out string sExtendedStatus optional)
	[ ] 
	[ ] // Variable declarations
	[ ] // ----------------------------------------------------------------
	[ ] // STRING sCommand - Stores the "built" instserv command for 
	[ ] //                   execution by SYS_Execute
	[ ] // STRING sServiceStatus - Status of the service as reported by instserv
	[ ] // STRING sExtended - Extended status returned from fServiceStatus
	[ ] // LIST OF STRING lsInstservOutput - Stores the data returned by instserv
	[ ] // INTEGER iNumTries - Number of times to check service status before failing
	[ ] // INTEGER iCount - Counter Variable
	[ ] //
	[ ] STRING sCommand = ""
	[ ] STRING sServiceStatus = ""
	[ ] STRING sExtended = ""
	[ ] LIST OF STRING lsInstservOutput = {}
	[ ] INTEGER iNumTries = 3
	[ ] INTEGER iCount = 0
	[ ] 
	[ ] // Action request is to stop the service
	[-] if (lower(sAction) == "stop")
		[ ] Print ("Signalling "+chr(34)+sService+chr(34)+" to "+sAction+" on "+sMachineIP)
		[ ] 
		[ ] sCommand ="instserv "+chr(34)+sService+chr(34)+" "+sAction+" \\"+sMachineIP+" y"
		[ ] SYS_Execute(sCommand, lsInstservOutput)
		[ ] 
		[ ] // Make a call to fServiceStatus
		[ ] sServiceStatus = fServiceStatus(sService, sMachineIP, sExtended)
		[ ] 
		[ ] // Service stopped as expected...
		[+] if (sServiceStatus == "Stopped")
			[ ] sExtendedStatus = "OK!"
			[ ] return TRUE
		[ ] 
		[ ] // fServiceStatus has caught an error...let's bubble it up to the caller
		[+] else if (sServiceStatus == "ERROR")
			[ ] sExtendedStatus = sServiceStatus
			[ ] return FALSE
		[ ] 
		[ ] // Service hasn't stopped yet, let's give it chance to stop...
		[ ] // We're going to check 3 times if the service stopped,
		[ ] // sleeping 20 seconds in between each call.
		[+] else
			[ ] 
			[+] while ((sServiceStatus != "Stopped") && (iCount <= iNumTries))
				[ ] sleep (20)
				[ ] 
				[ ] // Make a call to fServiceStatus
				[ ] sServiceStatus = fServiceStatus(sService, sMachineIP, sExtended)
				[ ] 
				[ ] // Increment our counter 
				[ ] iCount++
				[ ] 
				[ ] // The service finally stopped
				[+] if (sServiceStatus == "Stopped")
					[ ] sExtendedStatus = "OK!"
					[ ] return TRUE
				[ ] 
				[ ] // The service didn't stop so we're raising the red flag
				[+] else
					[ ] sExtendedStatus = sServiceStatus
					[ ] return FALSE
	[ ] 
	[ ] // Action request is to start the service
	[+] if (lower(sAction) == "start" || lower(sAction) == "restart")
		[ ] Print ("Signalling "+chr(34)+sService+chr(34)+" to "+sAction+" on "+sMachineIP)
		[ ] 
		[ ] sCommand ="instserv "+chr(34)+sService+chr(34)+" "+sAction+" \\"+sMachineIP+" y"
		[ ] SYS_Execute(sCommand, lsInstservOutput)
		[ ] 
		[ ] // Make a call to fServiceStatus
		[ ] sServiceStatus = fServiceStatus(sService, sMachineIP, sExtended)
		[ ] 
		[ ] // Service stopped as expected...
		[+] if (sServiceStatus == "Running")
			[ ] sExtendedStatus = "OK!"
			[ ] return TRUE
		[ ] 
		[ ] // fServiceStatus has caught an error...let's bubble it up to the caller
		[+] else if (sServiceStatus == "ERROR")
			[ ] sExtendedStatus = sServiceStatus
			[ ] return FALSE
		[ ] 
		[ ] // Service hasn't stopped yet, let's give it chance to start...
		[ ] // We're going to check 3 times if the service is running,
		[ ] // sleeping 20 seconds in between each call.
		[-] else
			[ ] 
			[+] while ((sServiceStatus != "Running") && (iCount <= iNumTries))
				[ ] sleep (20)
				[ ] 
				[ ] // Make a call to fServiceStatus
				[ ] sServiceStatus = fServiceStatus(sService, sMachineIP, sExtended)
				[ ] 
				[ ] // Increment our counter 
				[ ] iCount++
			[ ] 
			[ ] // The service finally stopped
			[-] if (sServiceStatus == "Running")
				[ ] sExtendedStatus = "OK!"
				[ ] return TRUE
			[ ] 
			[ ] // The service didn't stop so we're raising the red flag
			[+] else
				[ ] sExtendedStatus = sServiceStatus
				[ ] return FALSE
	[ ] 
	[ ] // Action request is not recognized
	[+] else
		[ ] sExtendedStatus = chr(34)+sAction+chr(34)+" is not a recognized instserv command"
		[ ] return FALSE
[ ] 
[ ] //*****************************************************************************
[ ] // END: fManageService
[ ] //*****************************************************************************
[ ] 
[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION    	:		fServiceStatus
[ ] //
[ ] //   REQUIRED
[ ] //   ARGUMENTS    :		string sService
[ ] //											-	The name of the service
[ ] //										string sMachineIp
[ ] //											- The IP address of the target machine
[ ] // 	 OPTIONAL
[ ] //   ARGUMENTS    : 	string sExtendedStatus
[ ] //											-	On Error this string is set. It's up to the caller to use
[ ] //										this functionality or not. 
[ ] //
[ ] //   RETURNS			: 	string -
[ ] //											- The status of the service
[ ] //   DESCRIPTION 	: 	This function retrieves the status of a service using the
[ ] //										instserv command.  On error the string "ERROR" is returned
[ ] //										and the optional sExtendedStatus string gets set.
[ ] //
[ ] //*****************************************************************************
[ ] 
[+] string fServiceStatus(in string sService, in string sMachineIP, out string sExtendedStatus optional)
	[ ] 
	[ ] // Variable declarations
	[ ] // ----------------------------------------------------------------
	[ ] // STRING sCommand - The "built" instserv command for 
	[ ] //                   execution by SYS_Execute
	[ ] // STRING sServiceStatus - The return status as reported by instserv
	[ ] // LIST OF STRING lsInstservOutput - The data returned by instserv
	[ ] // INTEGER iColonPosition - The location of the ':' in the instserv output
	[ ] //
	[ ] 
	[ ] STRING sCommand = ""
	[ ] STRING sServiceStatus = ""
	[ ] LIST OF STRING lsInstservOutput = {}
	[ ] INTEGER iColonPosition = 0
	[ ] 
	[ ] Print ("Requesting Status of "+chr(34)+sService+chr(34)+" on "+sMachineIP)
	[ ] 
	[ ] // Build up the insterv command
	[ ] sCommand ="instserv "+chr(34)+sService+chr(34)+" status \\"+sMachineIP+" y"
	[ ] 
	[ ] // Run the instserv command
	[ ] SYS_Execute(sCommand, lsInstservOutput)
	[ ] 
	[+] if (listcount(lsInstservOutput)>=1)
		[ ] // Instserv executable not in path
		[-] if (StrPos("not recognized", lsInstservOutput[1]) > 0)
			[ ] sExtendedStatus = lsInstservOutput[1]
			[ ] return "ERROR"
		[ ] // Specified service does not exist
		[-] if (listcount(lsInstservOutput)>=3) && (StrPos ("1060", lsInstservOutput[1]) > 0)
			[ ] sExtendedStatus = "ERROR - "+sService+" is not an installed service on "+sMachineIP
			[ ] ListPrint(lsInstservOutput)
			[ ] return "ERROR"
		[ ] // No errors found so we can proceed and determine the status of the service
		[-] else
			[ ] // If we don't encounter errors parse the output and return the status
			[ ] iColonPosition = StrPos (":", lsInstservOutput[1])
			[ ] sServiceStatus = SubStr (lsInstservOutput[1], iColonPosition+2)
			[ ] sExtendedStatus = "OK!"
			[ ] return sServiceStatus
	[+] else
		[ ] sExtendedStatus = "ERROR - Unable to retrieve status of service "+sService+" from machine "+sMachineIP
		[ ] return "ERROR"
	[ ] 
	[ ] 
	[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] // END: fServiceStatus
[ ] //*****************************************************************************
[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION    	:		fRegExport
[ ] //
[ ] //   REQUIRED
[ ] //   ARGUMENTS    :		string sServerIP
[ ] //											-	The IP of the server to connect to
[ ] //										string sRegistryKeyPathAndName
[ ] //											- The complete path and key name
[ ] //												ex. SOFTWARE\SeaChange\ITV Applications\CurrentVersion\Services\Mod Billing
[ ] //										string sWhereToPutRegFile
[ ] //											- The name of the file and the complete path to where it should be created
[ ] //												ex. "C:\Registry.reg", "\\192.168.1.1\c$\Registry.reg"
[ ] //											
[ ] // 	 OPTIONAL
[ ] //   ARGUMENTS    : 	string sExtendedStatus
[ ] //											-	On Error this string is set. It's up to the caller to use
[ ] //										    this functionality or not. 
[ ] //
[ ] //   RETURNS			: 	boolean
[ ] //											- True if function succeeded with an "OK!" extendedstatus
[ ] //											- False if function failed with an appropriate error message.
[ ] //   DESCRIPTION 	: 	This function serves as a wrapper for the RemoteRegistry.exe
[ ] //										file that's been generated by perl2exe.  Generates a .reg file backup
[ ] //										of a registry key in a specified registry.
[ ] //
[ ] //*****************************************************************************
[ ] 
[-] boolean fRegExport(in string sPathToRegExe, in string sServerIP, in string sRegistryKeyPathAndName, in string sWhereToPutRegFile, out string sExtendedStatus optional)
	[ ] 
	[ ] 
	[ ] LIST OF STRING lsOutput = {}
	[ ] STRING sCommand = ""
	[ ] 
	[ ] sCommand = "start /b "+sPathToRegExe+"regdmp.exe -m "+"\\"+sServerIP+" "+chr(34)+sRegistryKeyPathAndName+chr(34)+" >"+chr(34)+sWhereToPutRegFile+chr(34)
	[ ] Print(sCommand)
	[ ] SYS_Execute (sCommand, lsOutput)
	[ ] // output with "usage" , something error
	[-] if(ListCount(lsOutput)>4)
		[ ] LogError("****************** registry operation ERROR ********************")
		[ ] print(lsOutput)
		[ ] return FALSE
	[ ] // succeed
	[-] else
		[ ] print (lsOutput)
	[ ] 
	[ ] return TRUE
	[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] // END: fRegExport
[ ] //*****************************************************************************
[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION    	:		fRegImport
[ ] //
[ ] //   REQUIRED
[ ] //   ARGUMENTS    :		string sServerIP
[ ] //											-	The IP of the server to connect to
[ ] //										string sWhereToFindRegFile
[ ] //											- The name of the file and the complete path to where the existing file exists.
[ ] //												ex. "C:\Registry.reg", "\\192.168.1.1\c$\Registry.reg"
[ ] //											
[ ] // 	 OPTIONAL
[ ] //   ARGUMENTS    : 	string sExtendedStatus
[ ] //											-	On Error this string is set. It's up to the caller to use
[ ] //										    this functionality or not. 
[ ] //
[ ] //   RETURNS			: 	boolean
[ ] //											- True if function succeeded with an "OK!" extendedstatus
[ ] //											- False if function failed with an appropriate error message.
[ ] //   DESCRIPTION 	: 	This function serves as a wrapper for the RemoteRegistry.exe
[ ] //										file that's been generated by perl2exe.
[ ] //
[ ] //*****************************************************************************
[ ] 
[-] boolean fRegImport(in string sPathToRegExe, in string sServerIP, in string sWhereToFindRegFile, out string sExtendedStatus optional)
	[ ] 
	[ ] LIST OF STRING lsOutput = {}
	[ ] STRING sCommand = ""
	[ ] 
	[ ] sCommand = "start /b "+sPathToRegExe+"regini.exe -m "+"\\"+sServerIP+" "+chr(34)+sWhereToFindRegFile+chr(34)
	[ ] SYS_Execute (sCommand, lsOutput)
	[-] if(ListCount(lsOutput)>4)
		[ ] LogError("****************** registry operation ERROR ********************")
		[ ] print(lsOutput)
		[ ] return FALSE
	[ ] // succeed
	[-] else
		[ ] print (lsOutput)
	[ ] 
	[ ] return TRUE
	[ ] 
	[ ] 
	[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] // END: fRegImport
[ ] //*****************************************************************************
[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION    	:		fRegDeleteKey
[ ] //
[ ] //   REQUIRED
[ ] //   ARGUMENTS    :		string sServerIP
[ ] //											-	The IP of the server to connect to
[ ] //										string sRegistryKeyPathAndName
[ ] //											- The complete path and key name
[ ] //												ex. SOFTWARE\SeaChange\ITV Applications\CurrentVersion\Services\Mod Billing
[ ] //											
[ ] // 	 OPTIONAL
[ ] //   ARGUMENTS    : 	string sExtendedStatus
[ ] //											-	On Error this string is set. It's up to the caller to use
[ ] //										    this functionality or not. 
[ ] //
[ ] //   RETURNS			: 	boolean
[ ] //											- True if function succeeded with an "OK!" extendedstatus
[ ] //											- False if function failed with an appropriate error message.
[ ] //   DESCRIPTION 	: 	This function serves as a wrapper for the RemoteRegistry.exe
[ ] //										file that's been generated by perl2exe. Deletes a registry key at
[ ] //										the specified path. The delete is recursive, so subkeys are also deleted.
[ ] //
[ ] //*****************************************************************************
[ ] 
[+] boolean fRegDeleteKey(in string sPathToRegExe, in string sServerIP, in string sRegistryKeyPathAndName, out string sExtendedStatus optional)
	[ ] 
	[ ] LIST OF STRING lsOutput = {}
	[ ] STRING sCommand = ""
	[ ] 
	[ ] sCommand = sPathToRegExe+"RemoteRegistry.exe -DK "+chr(34)+sServerIP+chr(34)+" "+chr(34)+sRegistryKeyPathAndName+chr(34)
	[ ] SYS_Execute (sCommand, lsOutput)
	[ ] print (lsOutput)
	[ ] 
	[ ] return TRUE
	[ ] 
	[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] // END: fRegDeleteKey
[ ] //*****************************************************************************
[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION    	:		fRegCreateKey
[ ] //
[ ] //   REQUIRED
[ ] //   ARGUMENTS    :		string sServerIP
[ ] //											-	The IP of the server to connect to
[ ] //										string sRegistryKeyPath
[ ] //											- The complete path and key name
[ ] //												ex. SOFTWARE\SeaChange\ITV Applications\CurrentVersion\Services
[ ] //										string sRegistryKeyName
[ ] //											- The key name to create
[ ] //												ex. Mod Billing
[ ] //											
[ ] // 	 OPTIONAL
[ ] //   ARGUMENTS    : 	string sExtendedStatus
[ ] //											-	On Error this string is set. It's up to the caller to use
[ ] //										    this functionality or not. 
[ ] //
[ ] //   RETURNS			: 	boolean
[ ] //											- True if function succeeded with an "OK!" extendedstatus
[ ] //											- False if function failed with an appropriate error message.
[ ] //   DESCRIPTION 	: 	This function serves as a wrapper for the RemoteRegistry.exe
[ ] //										file that's been generated by perl2exe. Creates a key at a specified
[ ] //										location within the registry.
[ ] //
[ ] //*****************************************************************************
[ ] 
[+] boolean fRegCreateKey(in string sPathToRegExe, in string sServerIP, in string sRegistryKeyPath, in string sRegistryKeyName, out string sExtendedStatus optional)
	[ ] 
	[ ] 
	[ ] LIST OF STRING lsOutput = {}
	[ ] STRING sCommand = ""
	[ ] 
	[ ] sCommand = sPathToRegExe+"RemoteRegistry.exe -CK "+chr(34)+sServerIP+chr(34)+" "+chr(34)+sRegistryKeyPath+chr(34)+" "+chr(34)+sRegistryKeyName+chr(34)
	[ ] SYS_Execute (sCommand, lsOutput)
	[ ] print (lsOutput)
	[ ] 
	[ ] return TRUE
	[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] // END: fRegCreateKey
[ ] //*****************************************************************************
[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION    	:		fRegCreateValue
[ ] //
[ ] //   REQUIRED
[ ] //   ARGUMENTS    :		string sServerIP
[ ] //											-	The IP of the server to connect to
[ ] //										string sRegistryKeyPathAndName
[ ] //											- The complete path and key name
[ ] //												ex. SOFTWARE\SeaChange\ITV Applications\CurrentVersion\Services\Mod Billing
[ ] //										string sValueName
[ ] //											- The value name
[ ] //										string sValueType
[ ] //											- The type of value to be created:
[ ] //												"1" - REG_SZ
[ ] //												"2" - REG_DWORD
[ ] //												"3" - REG_EXPAND_SZ
[ ] //										string sValueContents
[ ] //											- The the contents of the value data.
[ ] //											
[ ] // 	 OPTIONAL
[ ] //   ARGUMENTS    : 	string sExtendedStatus
[ ] //											-	On Error this string is set. It's up to the caller to use
[ ] //										    this functionality or not. 
[ ] //
[ ] //   RETURNS			: 	boolean
[ ] //											- True if function succeeded with an "OK!" extendedstatus
[ ] //											- False if function failed with an appropriate error message.
[ ] //   DESCRIPTION 	: 	This function serves as a wrapper for the RemoteRegistry.exe
[ ] //										file that's been generated by perl2exe.
[ ] //
[ ] //*****************************************************************************
[ ] 
[+] boolean fRegCreateValue(in string sPathToRegExe, in string sServerIP, in string sRegistryKeyPathAndName, in string sValueName, in string sValueType, in string sValueContents, out string sExtendedStatus optional)
	[ ] 
	[ ] 
	[ ] LIST OF STRING lsOutput = {}
	[ ] STRING sCommand = ""
	[ ] 
	[ ] sCommand = sPathToRegExe+"RemoteRegistry.exe -CV "+chr(34)+sServerIP+chr(34)+" "+chr(34)+sRegistryKeyPathAndName+chr(34)+" "+chr(34)+sValueName+chr(34)+" "+chr(34)+sValueType+chr(34)+" "+chr(34)+sValueContents+chr(34)
	[ ] SYS_Execute (sCommand, lsOutput)
	[ ] print (lsOutput)
	[ ] 
	[ ] return TRUE
	[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] // END: fRegCreateValue
[ ] //*****************************************************************************
[ ] 
[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION    	:		fRegDeleteValue
[ ] //
[ ] //   REQUIRED
[ ] //   ARGUMENTS    :		string sServerIP
[ ] //											-	The IP of the server to connect to
[ ] //										string sRegistryKeyPathAndName
[ ] //											- The complete path and key name
[ ] //												ex. SOFTWARE\SeaChange\ITV Applications\CurrentVersion\Services\Mod Billing
[ ] //										string sValueName
[ ] //											- The name of the value to delete within the specified key.
[ ] //											
[ ] // 	 OPTIONAL
[ ] //   ARGUMENTS    : 	string sExtendedStatus
[ ] //											-	On Error this string is set. It's up to the caller to use
[ ] //										    this functionality or not. 
[ ] //
[ ] //   RETURNS			: 	boolean
[ ] //											- True if function succeeded with an "OK!" extendedstatus
[ ] //											- False if function failed with an appropriate error message.
[ ] //   DESCRIPTION 	: 	This function serves as a wrapper for the RemoteRegistry.exe
[ ] //										file that's been generated by perl2exe.  Deletes a specified registry
[ ] //										value.
[ ] //
[ ] //*****************************************************************************
[ ] 
[+] boolean fRegDeleteValue(in string sPathToRegExe, in string sServerIP, in string sRegistryKeyPathAndName, in string sValueName, out string sExtendedStatus optional)
	[ ] 
	[ ] 
	[ ] LIST OF STRING lsOutput = {}
	[ ] STRING sCommand = ""
	[ ] 
	[ ] sCommand = sPathToRegExe+"RemoteRegistry.exe -DV "+chr(34)+sServerIP+chr(34)+" "+chr(34)+sRegistryKeyPathAndName+chr(34)+" "+chr(34)+sValueName+chr(34)
	[ ] SYS_Execute (sCommand, lsOutput)
	[ ] print (lsOutput)
	[ ] 
	[ ] return TRUE
	[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] // END: fRegDeleteValue
[ ] //*****************************************************************************
[ ] 
