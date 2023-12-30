	[ ] //*****************************************************************************
	[ ] //  NAME:					  basestate.inc
	[ ] //  DESCRIPTION:		basestate for recovery system to restore
	[ ] //
	[ ] //  HISTORY:
	[ ] //
	[ ] //  Date						Developer					Description
	[ ] //  ********				***********				*****************************************
	[ ] //  8/25/04         rommy             Initial design and development
	[ ] //*****************************************************************************
	[ ] 
	[ ] use "..\..\inc\global\zodiacnt.inc"
	[ ] use "..\..\inc\global\zodiacfiles.inc"
	[ ] use "..\..\inc\global\zodiacdefs.inc"
	[ ] use "..\..\inc\scorpio\scorpio_frame.inc"
	[ ] use "..\..\inc\aquarius\aquariusframe.inc"
	[ ] 
	[-] appstate BaseState()
		[ ] string sExtendedStatus = ""
		[ ] 
		[ ] // Starting necessary services for cleanup
		[ ] gsAM1IP = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter, entAM1IP, "")
		[ ] gsAPP1IP = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter, entAPP1IP, "")
		[ ] gsAPP2IP = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter, entAPP2IP, "") 
		[ ] gsCM1IP = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter, entCM1IP, "") 
		[ ] gsCM2IP = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter, entCM2IP, "") 
		[ ] gsPS1IP = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter, entPS1IP, "") 
		[ ] gsMDS2IP = fReadWriteIni(csInstallPath+filZodiacIni, csRead, secCommandCenter, entMDS2IP, "")
		[-] if (!fManageService(csIAMName, csStart, gsAM1IP, sExtendedStatus))        // IAM
			[ ] LogError("*** Error: Cannot Start "+csIAMName+" on "+gsAM1IP+", Status is {sExtendedStatus}")
			[ ] exit
		[+] if (!fManageService(csMODName, csStart, gsAPP1IP, sExtendedStatus))        // Movies on Demand
			[ ] LogError("*** Error: Cannot Start "+csMODName+" on "+gsAPP1IP+", Status is {sExtendedStatus}")
			[ ] exit
		[+] if (!fManageService(csMOD2Name, csStart, gsAPP2IP, sExtendedStatus))       // Movies on Demand2
			[ ] LogError("*** Error: Cannot Start "+csMOD2Name+" on "+gsAPP2IP+", Status is {sExtendedStatus}")
			[ ] exit
		[-] if (!fManageService(csICMName, csStart, gsCM1IP, sExtendedStatus))         // ICM
			[ ] LogError("*** Error: Cannot Start "+csICMName+" on "+gsCM1IP+", Status is {sExtendedStatus}")
			[ ] exit
		[-] if (!fManageService(csICM2Name, csStart, gsCM2IP, sExtendedStatus))        // ICM2
			[ ] LogError("*** Error: Cannot Start "+csICM2Name+" on "+gsCM2IP+", Status is {sExtendedStatus}")
			[ ] exit
		[-] if (!fManageService(csSCTPName, csStart, gsAPP1IP, sExtendedStatus))       // SCTP
			[ ] LogError("*** Error: Cannot Start "+csSCTPName+" on "+gsAPP1IP+", Status is {sExtendedStatus}")
			[ ] exit
		[-] if (!fManageService(csSCTP2Name, csStart, gsAPP2IP, sExtendedStatus))      // SCTP2
			[ ] LogError("*** Error: Cannot Start "+csSCTP2Name+" on "+gsAPP2IP+", Status is {sExtendedStatus}")
			[ ] exit
		[-] if (!fManageService(csIADName, csStart, gsPS1IP, sExtendedStatus))         // IAD
			[ ] LogError("*** Error: Cannot Start "+csIADName+" on "+gsPS1IP+", Status is {sExtendedStatus}")
			[ ] exit
		[-] if (!fManageService(csSysmonName, csStart, gsMDS2IP, sExtendedStatus))     // Sysmon
			[ ] LogError("*** Error: Cannot Start "+csSysmonName+" on "+gsMDS2IP+", Status is {sExtendedStatus}")
			[ ] exit
		[ ] 
		[ ] //closing all the applications
		[-] if (SeaChangeITVAssetManager.Exists(10))
			[ ] SeaChangeITVAssetManager.SetActive()
			[ ] SeaChangeITVAssetManager.Exit()
		[ ] // SYS_Execute("C:\ITV\EXE\AMAPP.EXE")
		[ ] 
		[-] if (ManUtil.Exists(10))
			[ ] ManUtil.SetActive()
			[ ] ManUtil.Exit()
		[ ] // SYS_Execute("C:\itv\exe\ManUtil.exe")
		[-] if (AquariusSessionPl.Exists(5))
			[ ] AquariusSessionPl.Dismiss()
			[ ] 
		[ ] 
