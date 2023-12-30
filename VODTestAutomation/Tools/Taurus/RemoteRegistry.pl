# Title     :  RemoteRegistry.pl
# Author    :  Jeff Notaro(Jnotaro@schange.com)
# Version   :  Version 1.0

# Version Information
# --------------------------------------------------------------------------
# 1.0 - Initial Release
#
#
#
#
#

#!/perl

#use strict 'vars';

# --------------------------------------------------------------
# Remember to strip out debugging code
# Since we're using lsOutput for silk
# --------------------------------------------------------------


# Includes
# ---------------------------------------------------------------------------
use Win32;
use Win32::Registry;
use Win32::Registry::Find;
use Win32::Registry::IO;
use Win32::API;

# Globals
# ---------------------------------------------------------------------------
#
# ------- Scalar Variables -------
#
# $WindowHandle	 The window handle requested from by Win32::Console module
#
# ------- Arrays -------
#
# @SQL_Scripts   The list of particular SQL scripts that need to be on the CD.
#


$Server;
$BackupLocationAndFile;
$RemoteRegistryObject;
$RegPathAndKey;
$RegistryKey;
$RegPath;
$RegKeyName;
$RegValue;
$ValName;
$ValType;
$ValContents;


# Parse the command line parameters
# Call the right subroutine

if(@ARGV && $ARGV[0] eq '-RE'){
	$Server = $ARGV[1];
	$RegPathAndKey = $ARGV[2];
	$BackupLocationAndFile = $ARGV[3];
	&RegExport;
	
}
if(@ARGV && $ARGV[0] eq '-RI'){
	$Server = $ARGV[1];
	$BackupLocationAndFile = $ARGV[2];
	&RegImport;
}
if(@ARGV && $ARGV[0] eq '-DK'){
	$Server = $ARGV[1];
	$RegPathAndKey = $ARGV[2];
	&DeleteKey;
}
if(@ARGV && $ARGV[0] eq '-CK'){
	$Server = $ARGV[1];
	$RegPath = $ARGV[2];
	$RegKeyName = $ARGV[3];
	&CreateKey;
}
if(@ARGV && $ARGV[0] eq '-DV'){
	$Server = $ARGV[1];
	$RegPathAndKey = $ARGV[2];
	$RegValue = $ARGV[3];
	&DeleteValue;
}
if(@ARGV && $ARGV[0] eq '-CV'){
	$Server = $ARGV[1];
	$RegPathAndKey = $ARGV[2];
	$ValName = $ARGV[3];
	$ValType = $ARGV[4];
	$ValContents = $ARGV[5];
	&CreateValue;
}

#else{
#	print "ERROR\n";
#	print "$ARGV[0] Not a recognized parameter";
#}



# ====================================================================
# sub RegExport
# --------------------------------
sub RegExport{
	# Establish a connection to the remote machine
	$RemoteRegistryObject = Win32::Registry::Connect($Server);
	print "connected to $Server\n";

	$RegistryKey = $RemoteRegistryObject->Open($RegPathAndKey);

	$RegistryKey->Export($BackupLocationAndFile);
	print "Exported a reg file...";
	
	# Closing down the connection
	$RemoteRegistryObject->Close;
	print "Connection Closed";
}

# ====================================================================
# sub RegImport
# --------------------------------
sub RegImport{
	# Establish a connection to the remote machine
	$RemoteRegistryObject = Win32::Registry::Connect($Server);
	print "connected to $Server\n";

	print "Importing a reg file...";
	$RemoteRegistryObject->Import($BackupLocationAndFile);
	
	# Closing down the connection
	$RemoteRegistryObject->Close;
	print "Connection Closed";
}


# ====================================================================
# sub DeleteKey
# --------------------------------
sub DeleteKey{
	# Establish a connection to the remote machine
	$RemoteRegistryObject = Win32::Registry::Connect($Server);
	

	$RegistryKey = $RemoteRegistryObject->Open($RegPathAndKey);
	
	
	# Work around - if we get a false back...retry the delete, then check to see if we get true.
	# if we still get false...return false
	
	# REGSZ billingDLLName mbsdst
	if ($RegistryKey->DeleteKey('', "1")){
		print "TRUE";
	}
	# We have an error...print it out
	else{
		# retry the delete once
		if ($RegistryKey->DeleteKey('', "1")){
			print "TRUE";
		}	
		else{
			print "FALSE";
		}
	}
	# Closing down the connection
	$RemoteRegistryObject->Close;
}



# ====================================================================
# sub CreateKey
# --------------------------------
sub CreateKey{
	# Establish a connection to the remote machine
	$RemoteRegistryObject = Win32::Registry::Connect($Server);
	

	$RegistryKey = $RemoteRegistryObject->Open($RegPath);

	
	# REGSZ billingDLLName mbsdst
	if ($RegistryKey->Create($RegKeyName)){
		print "TRUE";
	}
	# We have an error...print it out
	else{
		print "FALSE";
	}
	# Closing down the connection
	$RemoteRegistryObject->Close;
}




# ====================================================================
# sub DeleteValue
# --------------------------------
sub DeleteValue{
	# Establish a connection to the remote machine
	$RemoteRegistryObject = Win32::Registry::Connect($Server);

	$RegistryKey = $RemoteRegistryObject->Open($RegPathAndKey);

	# REGSZ billingDLLName mbsdst
	if ($RegistryKey->DeleteValue($RegValue) == 1){
		print "TRUE";
	}
	# We have an error...print it out
	else{
		print "FALSE";
	}
	
	# Closing down the connection
	$RemoteRegistryObject->Close;
}

# ====================================================================
# sub CreateValue
# --------------------------------
sub CreateValue{
	# Establish a connection to the remote machine
	$RemoteRegistryObject = Win32::Registry::Connect($Server);
	

	$RegistryKey = $RemoteRegistryObject->Open($RegPathAndKey);
	
	#Switch to case
	if ($ValType == 1){
		# REGSZ billingDLLName mbsdst
		if ($RegistryKey->SetValues($ValName,REG_SZ,$ValContents)){
			print "TRUE";
		}
		# We have an error...print it out
		else{
			print "FALSE";
		}
	}
	elsif ($ValType == 2){
		# REGSZ billingDLLName mbsdst
		if ($RegistryKey->SetValues($ValName,REG_DWORD,$ValContents)){
			print "TRUE";
		}
		# We have an error...print it out
		else{
			print "FALSE";
		}
	}
	elsif ($ValType == 3){
		# REGSZ billingDLLName mbsdst
		if ($RegistryKey->SetValues($ValName,REG_EXPAND_SZ,$ValContents)){
			print "TRUE";
		}
		# We have an error...print it out
		else{
			print "FALSE";
		}
	}
	else{
		print "False";
	}
	# Closing down the connection
	$RemoteRegistryObject->Close;
}