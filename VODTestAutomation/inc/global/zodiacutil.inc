[ ] //*****************************************************************************
[ ] //  NAME:					  ZODIACUTIL.INC
[ ] //
[ ] //  DESCRIPTION:		Includes functions that access an .ini file, reads values
[ ] //                  from it, writes values to it, create Zodiac style time 
[ ] //                  stamps, and functions that don't fit cleanly elsewhere
[ ] //
[ ] //  HISTORY:
[ ] //
[ ] //  Date						Developer					Description
[ ] //  ********				***********				*****************************************
[ ] //  12/18/02        C. Callahan       Created from ZodiacFunc.inc, populated
[ ] //                                    with fCreateDateTimeStamp(), fReadWriteIni(),
[ ] //                                    fDecimalToHex() and fHexToDecimal()
[ ] //
[ ] //  01/02/03				K. Sullivan				Added a third case to fCreateDateTimeStamp.
[ ] //
[ ] //*****************************************************************************
[ ] //
[ ] // Module contains these functions:
[ ] //  NAME                    STATUS (peer reviewed or not)
[ ] //  fCreateDateTimeStamp()  Reviewed
[ ] //  fDecimalToHex()         not Reviewed
[ ] //  fReadWriteIni()         Reviewed
[ ] //  fHexToDecimal()         not Reviewed
[ ] //  fManageService()        not Reviewed
[ ] 
[ ] //*****************************************************************************
[ ] //
[ ] //   FUNCTION		  :	fCreateDateTimeStamp()
[ ] //
[ ] //   ARGUMENTS	  :	integer iFmtType
[ ] //									-	The format to return the date time string in.
[ ] //										1 = Date and time returned as mm-dd-yy@hhmmss.
[ ] //												Use this to create unique backup filenames.
[ ] // 										2 = Date and time returned as yyyymmddhhmmss
[ ] //										3 = Date and time returned as yyyy-mm-dd hh.nn.ss.fff.
[ ] //										4 = Default; Date and time returned as
[ ] //												MM/DD/YY HH:MM:SS AM/PM
[ ] //	               		  
[ ] //   RETURNS	    : Returns a formatted string representation of the 
[ ] //									current date and time.
[ ] //
[ ] //   DESCRIPTION	:	Generates and returns a current date and time stamp 
[ ] //									string in one of three user-specified formats.  
[ ] //
[ ] //*****************************************************************************
[ ] 
[+] public string fCreateDateTimeStamp (int iFmtType)
	[ ] 
	[ ] DATETIME dtCurrDateTime
	[ ] string sDateTime
	[ ] 
	[ ] // Create a current DATETIME string.
	[ ] dtCurrDateTime = GetDateTime()
	[ ] 
	[+] switch (iFmtType)
		[ ] 
		[+] case 1  // A format that can be used to create a unique DOS file name.
			[ ] sDateTime = FormatDateTime(dtCurrDateTime, "mm-dd-yy@hhmmss")
		[ ] 
		[+] case 2 // No symbols
			[ ] sDateTime = FormatDateTime(dtCurrDateTime,"yyyymmddhhmmss")
		[ ] 
		[+] case 3 // A format the same as the format returned from Silk's SYS_GetDirContents function.
			[ ] sDateTime = FormatDateTime(dtCurrDateTime,"yyyy-mm-dd hh.nn.ss.fff")
			[ ] 
		[+] default // default format = MM/DD/YY HH:MM:SS AM/PM
			[ ] sDateTime = FormatDateTime(dtCurrDateTime)
	[ ] 
	[ ] // Return the formatted DT string.
	[ ] return sDateTime
[ ] 
[ ] //*****************************************************************************
[ ] // END: fCreateDateTimeStamp
[ ] //*****************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] // Need Access to the MSVCRT functions to convert decimal to hex and
[ ] //    hex to decimal.
[ ] //    sprintf()
[ ] //    sscanf()
[+] dll "MSVCRT.dll"
	[ ] int sprintf( out string pbuffer, string pformat, int argument)
	[ ] int sscanf(string pbuffer, string pformat, out int argument )
[ ] 
[ ] //******************************************************************************
[ ] // fDecimalToHex()
[ ] // Description:
[ ] //   Converts a Decimal number (supplied as a string) to a Hex number (returned
[ ] //   as a string.  Useful for converting sn ITV Uid from its  SQL stored value
[ ] //   in Decimal to its display value in Hex 
[ ] // Returns:
[ ] //   string: The value of the supplied number in Hex, as a string
[ ] // Arguments:
[ ] //  in string sDecimal
[ ] //     a Decimal number as a string 
[ ] //******************************************************************************
[ ] 
[+] string fDecimalToHex (string sDecimal)
	[ ] int d
	[ ] string sHex
	[ ] string sDecimalF = "%i"
	[ ] string sHexF = "%08x"
	[+] if (sscanf(sDecimal, sDecimalF, d) == 1)
		[+] if (sprintf(sHex, sHexF, d) == 8)
			[ ] Print ("Hex: {sHex} Decimal: {sDecimal}")
			[ ] return sHex
	[ ] return Null
[ ] 
[ ] //******************************************************************************
[ ] // END: fDecimalToHex
[ ] //******************************************************************************
[ ] 
[ ] //******************************************************************************
[ ] // fHexToDecimal()
[ ] // Description:
[ ] //   Converts a Hex number (supplied as a string) to a decimal number (returned
[ ] //   as a string.  Useful for converting an ITV Uid from its display value in
[ ] //   Hex and its SQL stored value in Decimal
[ ] // Returns:
[ ] //   string: The value of the supplied number in Decimal, as a string
[ ] // Arguments:
[ ] //  in string sHex
[ ] //     a Hexidecimal number as a string 
[ ] //------------------------------------------------------------------------------
[ ] 
[+] string fHexToDecimal (string sHex)
	[ ] int h
	[ ] string sDecimal
	[ ] string sHexF = "%x"
	[ ] string sDecimalF = "%010i"
	[+] if (sscanf(sHex, sHexF, h) == 1)
		[+] if (sprintf(sDecimal, sDecimalF, h) == 10)
			[ ] Print ("Hex: {sHex} Decimal: {sDecimal}")
			[ ] return sDecimal
	[ ] return Null
[ ] 
[ ] //*****************************************************************************
[ ] // END: fHexToDecimal
[ ] //*****************************************************************************
[ ] 
[ ] 
[ ] 
[ ] 
[ ] //*****************************************************************************
[ ] // END: ZODIACUTIL.INC
[ ] //*****************************************************************************
