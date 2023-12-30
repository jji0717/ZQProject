#!/usr/bin/perl

# getSHASConfig.pl
#
#  Perl script that gathers the SHAS Config Information for the CD, PD, and LD (and LE) entries
#  from the rc_config output.
#
# Note that if the rc_config output changes for any reason, this script will likely fail.
# Use the -t command line option to enable tracing for assistive debugging.
# Trace lines all output with a preceeding "T:".
#
# Usage: getShasConfig.pl [-ht] CL|CD|PD|LD
#	-h                  : help message
#	-t                   : enable runtime tracing
#	CL|CD|PD|LD   : Cluster; Controller; Physical; Logical Disk");
#
#
#  Revision History
#
#  05-06-2010   mjc    Created 
#  05-18-2010   mjc    Fixed spelling error in "Copy To ID" for LD
#  05-19-2010   mjc    Fixed bad interpreter path at the top of the file from #!usr/bin/perl to #!/usr/bin/perl
#  05-20-2010   mjc    Added CL (Cluster) config support
#  05-21-2010   mjc    Removed the ^M from the #/usr/bin/perl string ! ! ! WTF ! ! ! !
#  05-26-2010   mjc    Fixed syntax on tag fields
#  05-28-2010   mjc    Fixed syntax on PD tag fields; 1-based rather than 0-based on Disk Number; 
#									Add bytes calculation within PD for size fields
# 06-15-2010   mjc    Fixed doube entries for Route in the LD. Removed path from LE and left comment
#								   in the LD string table about Path missing.
# 07-14-2010   mjc    SMIS-98 Part 1: Converted values to strings as needed & changed BIOS string.
# 07-30-2010   mjc    SMIS-98 Part 2: Converted LD & LE field values for bytes*512; Changed to 'Number of Bays'
# 08-04-2010   mjc    SMIS-98 Part 3: Added getOwnerString capability for PD / LD objects
# 08-06-2010   mjc    Added getShasConfig PD <enclosure_id> support; Disabled unitialized warnings
#
#

use warnings;
no  warnings 'portable';  		# Support for 64-bit ints required so we turn off portability warnings
no warnings 'uninitialized';		# This is desired for PD <enclosure_id>, but not required
use strict;
use Getopt::Std;
use Pod::Usage;
use vars qw/ %opt /;

require '/usr/local/seamonlx/bin/common_functions.pl';

my $TRACE = 0;  	# Set this to a 1 for detailed tracing to stdout or use -t on command line for script
my $lineNum = 0;    # Line number in the file being processed
my @fileData;         # Data lines from file in array format

# Globals for Local and Partner ID and Node Name per PD - used for the "Owner" field in PD object
my $LNM;
my $LID;
my $PNM;	
my $PID;

# Global flag used for the passed in enclosure ID for PD, when specified
my $enclosureID = "";

#
# DESCRIPTIVE TAG STRING Tables
#
# The mapping of abbreviated tag to descriptive name via key based associative pair hashs
#

# Cluster
my %cl_descriptives = ( 
"CL",			"Cluster ID",	
"PER",		"Personality",
"LNN",		"Local Node Number",
"PNN",		"Partner Node Number",
"LS",			"Link State",	
"PS",			"Path State",	
"NST",		"Node State",	
"LNM",		"Local Name",	
"LID",		"Local CoreID",	
"PNM",		"Partner Name",	
"PID",		"Partner CoreID"
);

# Controller
my %cd_descriptives = ( 
"CD",		"Enclosure Number",	
"CN",		"Enclosure Name",	
"CSN",	"Serial Number",
"BB",		"BIOS Version",
"MX",		"Size",
"KE",		"License parameters",
"NP",		"Number of Bays",
"VI",		"Vendor ID",
"DI",		"Device ID",
"CLID",	"Enclosure ID" 
);

# Physical Drive
my %pd_descriptives = (
"PD",		"Disk Number",		# This value gets converted from 0-based to 1-based
"TY",		"Disk Type",
"ST",		"State",
"NM",		"Name",
"SD",		"SCSI ID",
"RT",		"Route",
"HD",		"Handle",
"ID",		"Identifier",
"SI",		"Size",						# This value gets converted from sectors to bytes
"GS",		"Global Spare",
"FE",		"Features",
"SN",		"Serial Number",
"MO",	"Model Number",
"FW",	"Firmware Version",
"LS",		"Largest Free Space", 		# This value gets converted from sectors to bytes
"TS",		"Total Space",		# This value gets converted from sectors to bytes
"CD",		"Enclosure Number",
"CH",		"Bay Number",
"PT",		"Port Type",
"OWN",	"Owner ID",
"OOWN",	"Original Owner"
);

# Logical Drives 
my %ld_descriptives = (
"LD",		"Logical Drive Number",
"RT",		"Route",
"HD",		"Handle",
"ID",		"Identifier",
"NM",		"Name",
"OWN",	"Owner Core ID",
"SI",		"Device Size",
"1ST",	"First Count",
"2ND",	"Second Count",
"CA",		"Cache Attribute",
"ST",		"Logical Device State",
"TY",		"Logical Device Type",
"GE",		"Generation Number",
"TPST",	"Task Protect Start",
"TPSI",	"Task Protect Size",
"TPC",	"Task %",
"TTY",	"Task Type",
"TST", 	"Task Status",
"DS",		"Distributed Spare",
"PRE",	"Preferred Element",
"TP",		"Task Priority",
"AT",		"Attributes",
"RC",		"Reference Count",
"OSN",	"OS Name",
"CF",		"Copy from ID",
"CT",		"Copy to ID",
"DP",		"Array Name",
"SP",		"Dedicated Spare"
# PA (Path) is missing?  Maybe need to fix this?  06-15-2010
);

# Logical Elements 
my %le_descriptives = (
"LE",		"Logical Element Number",
"RT",		"Route",
"HD",		"Handle",
"ID",		"Identifier",
"SO",		"Space Offset",
"SS",		"Space Size",
"DO",		"Data Offset",
"DS",		"Data Size",
"ER",		"Errors"
);


#############################################################################
#
# usage() - Message about this script and how to use it
#
##############################################################################
sub usage()
{
	pod2usage( "\nThis program fgets the SHAS Configuration information.
The output of the elements is in the following format:
    tag:data
    ...

Usage: $0 [-ht] CL|CD|PD|LD
	-h                  : help message
	-t                  : enable runtime tracing
	CL|CD|PD|LD   : Cluster; Controller; Physical; Logical Disk information
	
	");
	exit;
}

#############################################################################
#
# CLconvert2string()
# Will convert a tag string's value from a digit to a string
#
# Arguments: The tag field name that may need converting
#					The value of the tag
# Returns: The formatted string
#
##############################################################################
sub CLconvert2string {

    my $tag = shift;
	my $value = shift;
	my $returnString;
	my $found = 0;
	
	$value =~ s/\s+$//;              # Removes all the newline + whitespace
	
	# Search for each of the strings we need to convert
	#print "convert2string: tag=|$tag|   value=|$value| \n";

	# LINK STATE
	# PATH STATE
	if( ($tag eq "Link State") || ($tag eq "Path State") ) {
		if( $value eq "1" ) {
			$found = 1;
			$returnString = "Up";
		} else { 
			if ( $value eq "2" ) {
				$found = 1;
				$returnString = "Down";
			} 
		}
		
		if( !$found ) {
				$found = 1;
				$returnString = "Unknown";
		}
		
	}
	
	# NODE STATE
	if( !$found && ($tag eq "Node State") ) {
		if( $value eq "0" ) {
			$found = 1;
			$returnString = "Initializing";
		} else { 
			if ( $value eq "1" ) {
				$found = 1;
				$returnString = "Singleton";
			} else { 
				if ( $value eq "2" ) {
					$found = 1;
					$returnString = "Singleton, waiting for heal";
				} else { 
					if ( $value eq "3" ) {
						$found = 1;
						$returnString = "Transitioning";
					}	else { 
							if ( $value eq "4" ) {
								$found = 1;
								$returnString = "Mirrored";
							}				
						} 
				}
			}
		}
		
		if( !$found ) {
				$found = 1;
				$returnString = "Unknown";
		}	
	}


	#  PERSONALITY
	if( $tag eq "Personality" ) {
		if( $value eq "0" ) {
			$found = 1;
			$returnString = "Standalone";
		} else { 
			if ( $value eq "1" ) {
				$found = 1;
				$returnString = "Mirrored";
			} else { 
				if ( $value eq "2" ) {
					$found = 1;
					$returnString = "Clustered";
				} 
			}
		}
		
		if( !$found ) {
				$found = 1;
				$returnString = "Unknown";
		}
		
	}	
	
    # We set it to the default string contained in value (leave it unchanged)
	if( $found != 1 ) {
		$returnString = $value;
	}

	# Set the returned value
    $returnString;
}

#############################################################################
#
# LXconvert2string()
# Will convert some LD / LE  tag values to byte value based on multiplying by 512
#
# Arguments: The tag field name that may need converting
#					The value of the tag
# Returns: The formatted string
#
##############################################################################
sub LXconvert2string {

    my $tag = shift;
	my $value = shift;
	my $returnString;
	my $found = 0;
	
	$value =~ s/\s+$//;              # Removes all the newline + whitespace
	
	$returnString = $value;			# Set the value as default - overwritten if needed below
	
	# Search for each of the strings we need to convert
	print "LXconvert2string: tag=|$tag|   value=|$value| \n" if $TRACE;

	# List of strings we want to convert from HEX to Decimal and multiply by 512
	# DEVICE SIZE, DATA OFFSET, DATA SIZE, SPACE OFFSET, SPACE SIZE
	#
	if( ($tag eq "Device Size") || ($tag eq "Data Offset") || ($tag eq "Data Size") || ($tag eq "Space Offset") || ($tag eq "Space Size") ) {
		print "\tFound! 512x   value is |$value|\n" if $TRACE;
		
		my $bytesVal = hex( $value );
		print "T: Converting hex $value to decimal = $bytesVal " if $TRACE;
		$bytesVal *= 512;
		print " and multiply by 512 = $bytesVal\n" if $TRACE;
		$value = make_number_nice( $bytesVal );		# Assign it back to the field converted		
		$found = 1;
		$returnString = $value;
	}
		
	print "LXconvert2string: returnString=|$returnString| \n" if $TRACE;
		
	# Set the returned value
    $returnString;
}

#############################################################################
#
# getCL_LIDandnPID  - Get the Local ID and Partner ID from the CL object
#
#
#
#############################################################################
sub getCL_LIDandPID()
{	
	my $found;
	
# Locate CLUSTER at the beginning of the string
	if( /^CL:/ )
	{
		printf ("T:getCL_LIDandPID on line $lineNum\n" ) if $TRACE;

		s/\n//sg;            # Removes all the newline symbols
		
		my $descsub = 0;	# Description subscript
		my $n = 0;

		# Split the line into chunks based on the colon
		my @tags = split /(\w+): /; 
		my $returnValue;
		foreach my $tagval ( @tags )
		{
			# This is the subscript for the full description associated with the key
			$descsub++;
			
			next if !($n++ % 2);  	# If true, we skip the next tag because we only care about the TAG fields, not their values				
			
			# Not sure why the first line here is always en empty line, but we check and ignore just in case
			if( $tagval ne "" ) { 
				
				# For each key, if we match, we print the output
				foreach my $attr (keys %cl_descriptives) {
					#printf ("T:comparing attr=|$attr| to tagval=|$tagval|\n" ) if $TRACE;
				
					if( $attr eq $tagval ) {
					
						$tags[ $descsub ] =~ s/\^//g;               # Removes the carets '^' from the string
						$tags[ $descsub ] =~ s/\s+$//;              # Removes all the newline symbols & trailing whitespace
						
						# "LNM",		"Local Name",	
						# "LID",		"Local CoreID",	
						# "PNM",		"Partner Name",	
						# "PID",		"Partner CoreID"
						if( $attr eq "LNM" ) {
								$LNM = $tags[ $descsub ];
						}
						if( $attr eq "LID" ) {
								$LID = $tags[ $descsub ];
						}
						if( $attr eq "PNM" ) {
								$PNM = $tags[ $descsub ];
						}
						if( $attr eq "PID" ) {
								$PID = $tags[ $descsub ];
						}
		
					}
				}
			}
		}
		#print "\n";
		$found = 1;			# This is our return value
	}
	else { 
		$found = 0; 
	}
	
	if( $TRACE && $found == 1 ) {
		printf ("== getCL_LIDandPID DATA COLLECTED:\n" ) if $TRACE;
		printf ("\tLNM = |$LNM|\n" ) if $TRACE;
		printf ("\tLID = |$LID|\n" ) if $TRACE;
		printf ("\tPNM = |$PNM|\n" ) if $TRACE;
		printf ("\tPID = |$PID|\n\n" ) if $TRACE;
	}
	

} #end subroutine GetCL_LIDnPID()

#############################################################################
#
# getOwnerString()
# Looks up the owner string based on the OWNer ID field value and returns it
#
# Arguments: OWNer ID hexadecimal value
#					
# Returns: The logical name of the owner (either the local or partner node name)
#
##############################################################################
sub getOwnerString {

    my $id = shift;
	my $returnName;
	
	$id =~ s/\s+$//;              # Removes all the newline + whitespace
	
	# Search for each of the strings we need to convert
	print "getOwnerString: id=|$id|\n" if $TRACE;

	if( $id eq $LID ) {
		$returnName = $LNM;
	} else {
		if( $id eq $PID ) {
			$returnName = $PNM;	
		} else {
			$returnName = "Unknown";
		}
	}

	print "getOwnerString: returnName=|$returnName| \n" if $TRACE;
		
	# Set the returned name value
    $returnName;
}


#############################################################################
#
# readCluster() -  - Reads the CL entry from the rc_config output
#
# 	The mapping:
#
#				Cluster ID:				123 (CL:)
#				Link State:				Up (LS:)
#				Path State:			Up (PS:)
#				Node State:			Mirrored (NST:)
#				Local Name:			UML-6EC6 (LNM:)
#				Local CoreID:			0x085412BE568D27B4 (LID:)
#				Partner Name:		UML-8CE6 (PNM:)
#				Partner CoreID:		0x437C2339FBF7DBD8 (PID:)
#
# 		Additional Fields
#				PER: 	Personality
#				LNN:	Local Node Number
#				PNN:	Partner Node Number
#
#				Node State:
#					(0) Initializing
#					(1) Singleton
#					(2) Singleton, waiting for heal
#					(3) Transitioning
#					(4) Mirrored
#				
#				Personality:
#					(0) Standalone
#					(1) Mirrored
#					(2) Clustered (NOT SUPPORTED)
#				
#				Link State:    
#					(0) Down
#					(1) Up
#					(all others) Unknown
#				
#				Path State:
#					(0) Down
#					(1) Up#
#
#
#############################################################################
sub readCluster()
{	
	my $found;
	
# Locate CLUSTER at the beginning of the string
	if( /^CL:/ )
	{
		printf ("T:CLUSTER on line $lineNum\n" ) if $TRACE;

		s/\n//sg;            # Removes all the newline symbols
		
		my $descsub = 0;	# Description subscript
		my $n = 0;

		# Split the line into chunks based on the colon
		my @tags = split /(\w+): /; 
		my $returnValue;
		foreach my $tagval ( @tags )
		{
			# This is the subscript for the full description associated with the key
			$descsub++;
			
			next if !($n++ % 2);  	# If true, we skip the next tag because we only care about the TAG fields, not their values				
			
			# Not sure why the first line here is always en empty line, but we check and ignore just in case
			if( $tagval ne "" ) { 
				
				# For each key, if we match, we print the output
				foreach my $attr (keys %cl_descriptives) {
					if( $attr eq $tagval ) {
						$tags[ $descsub ] =~ s/\^//g;               # Removes the carets '^' from the string
						
						# Need to check if the field is one we need to convert before just putting the raw value in 
						$returnValue = CLconvert2string( $cl_descriptives{$attr}, $tags[ $descsub ]  );
						
						print "$cl_descriptives{$attr}=$returnValue\n";
						last;   # break out of this foreach
					}
				}
			}
		}
		#print "\n";
		$found = 1;			# This is our return value
	}
	else { 
		$found = 0; 
	}

} #end subroutine readCluster()

#############################################################################
#
# readControllers() -  - Reads the CD entries from the rc_config output
#
#############################################################################
sub readControllers()
{	

# Locate CONTROLLER at the beginning of the string
	if( /^CD:/ )
	{
		printf ("T:CONTROLLER on line $lineNum\n" ) if $TRACE;
		s/\n//sg;            # Removes all the newline symbols
		
		my $descsub = 0;	# Description subscript
		my $n = 0;

		# Split the line into chunks based on the colon
		my @tags = split /(\w+): /; 
		foreach my $tagval ( @tags )
		{
			# This is the subscript for the full description associated with the key
			$descsub++;
			
			next if !($n++ % 2);  	# If true, we skip the next tag because we only care about the TAG fields, noth their values				
			
			# Not sure why the first line here is always en empty line, but we check and ignore just in case
			if( $tagval ne "" ) { 
				
				# For each key, if we match, we print the output
				foreach my $attr (keys %cd_descriptives) {
					if( $attr eq $tagval ) {
						$tags[ $descsub ] =~ s/\^//g;               # Removes the carets '^' from the string
						print "$cd_descriptives{$attr}=$tags[ $descsub ]\n";
						last;   # break out of this foreach
					}
				}
			}
		}
		#print "\n";
	}

} #end subroutine readControllers()

#############################################################################
#
# readPhysDisks() - Reads the PD entries from the rc_config output
#
# A typical PD line from the rc_config output looks like this:
# PD: 1   TY: Disk    ST: Online    ID: 0x106A50D60612927D  NM: //./Core1/Route1/Device24 SD: ^0:0:24:0^
#        CD: 1       CH: 11        RT: 1     HD: 24        FE: 0x00000002            GS: N
#        SI: 0x8583B00             LS: 0x0                 TS: 0x0
#        MO: ^HP      DF072BB6BC                      ^    SN: ^3LQ2G5BR000098259V27^  FW: ^HPDA    ^      PT: 0x00000000  OWN: 0x5A916794E6E7C84E OOWN: 0x5A916794E6E7C84E
#
# Note that this PD line, when the TYpe is of "Disk", spans multiple lines.
# There is code to handle this case below where it continues to process lines until the beginning of a line is non blank. 
#
#############################################################################
sub readPhysDisks()
{

# Locate PHYSICAL DISK
	if( /^PD:/ )
	{
		printf ("T:PHYSICAL DISK on line $lineNum\n" ) if $TRACE;
		s/\n//sg;            # Removes all the newline symbols

		my $TypeDisk = 0;

		do {

			my $descsub = 0;
			my $n = 0;

			# Split the line into chunks based on the colon
			my @tags = split /(\w+): /;
			
			# special case we don't need to report  PD with TY Core, break out
			# but we must read next line of file
			if($tags[3] eq "Core") {
				last READNEXTPD;
			}
			
			foreach my $tagval ( @tags )
			{
				# This is the subscript for the full description associated with the key
				$descsub++;
				
				next if !($n++ % 2);  	# If true, we skip the next tag because we only care about the TAG fields, noth their values				
				
				# Not sure why the first line here is always en empty line, but we check and ignore just in case
				if( $tagval ne "" ) { 
					
					# For each key, if we match, we print the output
					foreach my $attr (keys %pd_descriptives) {
					
						# Do we match the key and the string tag read?
						if( $attr eq $tagval ) {
							
							$tags[ $descsub ] =~ s/\^//g;               # Removes the carets '^' from the string

							#
							# SPECIAL CASE HANDLING FOR FIELDS 
							#
							#	"PD",		"Disk Number"
							#
							
							# Special cases where the value needs to be multiplied by 512 for bytes
							#	"SI",		"Size"
							# "LS",		"Largest Space"
							# "TS",		"Total Space"
							#
							if( $tagval eq "SI" || $tagval eq "LS" || $tagval eq "TS"  ) {
								$tags[ $descsub ] =~ s/\s+$//;              # Removes all the newline + whitespace
							   my $bytesVal = hex( $tags[ $descsub ] );
								print "T: Converting hex $tags[ $descsub ] to decimal = $bytesVal " if $TRACE;
								$bytesVal *= 512;
								print " and multiply by 512 = $bytesVal\n" if $TRACE;
								$tags[ $descsub ] = make_number_nice( $bytesVal );		# Assign it back to the field converted
							}

							# Output the line to standard out
							printf("%s=%s\n", $pd_descriptives{$attr}, $tags[ $descsub ]);
							
							# Check for the OWNer field and convert it to the matching name to add to the output
							if( $tagval eq "OWN" ) {
								my $ownerString = getOwnerString( $tags[ $descsub ] );
								printf("Owner=%s\n", $ownerString );
							}
							
							# Special case where Type is Disk. We have a few more lines worth of data to chew on
							# Check and set flags if we are here. First remove trailing whitespace
							$tags[ $descsub ] =~ s/\s+$//;              # Removes all the newline symbols							

							print "T:Checking tagval |$tagval| and tags |$tags[ $descsub ]|\n" if $TRACE;

							if( $tagval eq "TY" && $tags[ $descsub ] eq "Disk" ) {
								print "T: Setting TypeDisk = 1\n" if $TRACE;
								$TypeDisk = 1;
							}
							
							last if $TypeDisk == 0;   # break out of this foreach only when done
						}
					} #end foreach $attr
				} #end if
			} # End foreach
			
			READNEXTPD:
			if( $TypeDisk == 1 )
			{
				print "T: TypeDisk==1, Reading a line from the file\n" if $TRACE;
				# Read more lines from the file
				$_ = $fileData[ $lineNum++ ];
				print "T:READ: $_\n" if $TRACE;
				s/\n//sg;            # Removes all the newline symbols
				
				if(  ! /^ /  )  # If beginning of the line is blank, we still have PD lines
				{
					print "T: Not blank line, done\n" if $TRACE;
					$TypeDisk = 0;   # else we are done
					$lineNum--;
				}
			}
			
		} while( $TypeDisk == 1 );
		
		#print "\n";

	} # endif

} #end subroutine readPhysDisks()


#############################################################################
#
# readLogicalDisks()  - Reads the LD entries from the rc_config output
#
#############################################################################
sub readLogicalDisks()
{

	my $convertedValue;
	
    # Locate LOGICAL DRIVE
	if( /^LD:/ ) {
	
		my $i;
	
		printf ("T:LOGICAL DRIVE on line $lineNum\n" ) if $TRACE;
		
		for( $i=0; $i <= 3; $i++ )	{  # The LD entries have 4 lines worth of data before the LE's start
	
			s/\n//sg;            # Removes all the newline symbols

			my $descsub = 0;

			# Array fun
			my @tags = split /(\w+): /;						# Split on some chars followed by a colon
			my $n = 0;
			
			foreach my $tagval ( @tags )	{
				# This is the subscript for the full description associated with the key
				$descsub++;
				
				# For efficiency's sake, we skip every other array entry
				next if !($n++ % 2);  	# If true, we skip the next tag because we only care about the TAG fields, not their values
		
				# Not sure why the first line here is always en empty line, but we check and ignore just in case
				if( $tagval ne "" ) { 
					
					# For each key, if we match, we print the output
					foreach my $attr (keys %ld_descriptives) {
						
						#print "T: Comparing |$attr| to |$tagval|\n" if $TRACE;					
						
						# Do we match the key and the string tag read?
						if( $attr eq $tagval ) {
							
							$tags[ $descsub ] =~ s/\^//g;               # Removes the carets '^' from the string

							# Need to check if the field is one we need to convert before just putting the raw value in 
							$convertedValue = LXconvert2string( $ld_descriptives{$attr}, $tags[ $descsub ]  );							

							# Output the line to standard out
							printf("%s=%s\n", $ld_descriptives{$attr}, $convertedValue );
							
							# Check for the OWNer field and convert it to the matching name to add to the output
							if( $tagval eq "OWN" ) {
								my $ownerString = getOwnerString( $convertedValue );
								printf("Owner=%s\n", $ownerString );
							}							
							
							last;   # break out of this foreach only when done
						}
						
					} #end foreach $attr
			
				} #end if tagval
				
	
		} # end foreach

		# Read more lines from the file via the array
		$_ = $fileData[ $lineNum++ ];
		print "T: NEXT LINE $i\n" if $TRACE;	

	} # end if LD



		
	if( /LE:/ ) {			# Handle LE and the PA portion of the LD (which falls after the LE)
	
		print "T: Working LE blocks\n" if $TRACE;
		my $leDone = 0;
		my $lesub;	
		
		while( ! $leDone ) {
			
			$lesub = 0;
			
			s/\n//sg;            # Removes all the newline symbols					
			my @letags = split /(\w+): /;								
			
			if(  ! /^ /  )  { # If beginning of the line is blank, we still have LE lines
				print "T: Not blank line, done with LE\n" if $TRACE;
				$leDone = 1;
				$lineNum--;
			}
			else
			{
				print "T: Blank line, have LE\n" if $TRACE;
				my $n = 0;
				
				foreach my $letagval ( @letags ) 	{
					
					$lesub++;		# Bump description subscript

					# For efficiency's sake, we skip every other array entry
						next if !($n++ % 2);  	# If true, we skip the next tag because we only care about the TAG fields, not their values
						
						foreach my $leattr (keys %le_descriptives) {			
							print "T: Comparing |$leattr| to |$letagval|\n" if $TRACE;

							if( $leattr eq $letagval )  {
							
								print "T: Found |$leattr|\n" if $TRACE;
								# Output the line to standard out
								
								# Need to check if the field is one we need to convert before just putting the raw value in 
								$convertedValue = LXconvert2string( $le_descriptives{$leattr}, $letags[ $lesub ]  );							

								# Output the line to standard out
								printf("%s=%s\n", $le_descriptives{$leattr}, $convertedValue );
								
								last;
								
							} #end if
											
						} # end foreach leattr
										
				} # end foreach letagval
			} # end else

			if( ! $leDone )
			{
				# Read more lines from the file via the array
				$_ = $fileData[ $lineNum++ ];
				print "T: READ: $_\n" if $TRACE;
				s/\n//sg;            # Removes all the newline symbols
			}

			} # End foreach letagval
		} # End else

		#print "\n";
	} # end

} #end subroutine readLogicalDisks()


#############################################################################
#           MAIN
#############################################################################

# Get the command line options
my $opt_string = "ht";
getopts($opt_string, \%opt) or usage();
usage() if $opt{h};
$TRACE=1 if $opt{t};

print "Argument count = $#ARGV\n" if $TRACE;
print join(' ', @ARGV) . "\n" if $TRACE;

# Get the type name we want to retrieve
my $typeToGet = $ARGV[0];

# Handle the different case here for PD <enclosure_id>
# Check for the enclosure ID as an additional argument
if( $typeToGet eq "PD" ) {
	printf ("PD TYPE: Argv[1] is |%s|\n", $ARGV[1]) if $TRACE;
	# Additional argument check for enclosure ID value
	if( $ARGV[1] ne "" ) {
		$enclosureID = $ARGV[1];
		printf ("Set ENCLOSURE ID to |$enclosureID|\n" ) if $TRACE;
	}
}

# Shell command to get the rc_config data, redirecting stdout and stderr
my $RC_CMD = "/usr/local/shas/bin/rc_config /os_name > /tmp/rcc_out.tmp 2>&1";

# Run the command
system( $RC_CMD );

my $data_file;

# We search for only PD data with the matching CD (Enclosure ID) specified
if( $enclosureID ne "" ) {
	printf ("=> MODIFIED PD ENCLOSURE ID SYSTEM CALL. enclosureID=|%s|\n", $enclosureID) if $TRACE;
	$RC_CMD = "cat /tmp/rcc_out.tmp | grep \"CL:\" 2>&1 > /tmp/rcc_out2.tmp";
	# Run the command 
	system( $RC_CMD );

	$RC_CMD = "cat /tmp/rcc_out.tmp | grep \"PD:\" -A4 | grep \"CD: $enclosureID\" -A2 -B1 2>&1 >> /tmp/rcc_out2.tmp";
	# Run the command 
	system( $RC_CMD );

	# Now we are ready for PD parsing later . . .
	
	# Slurp in the whole file at once
	$data_file="/tmp/rcc_out2.tmp";
	open(DAT, $data_file) || die("Could not open data file TWO!");
	@fileData = <DAT>;
	close(DAT);
} else {

	# Slurp in the whole file at once
	my $data_file="/tmp/rcc_out.tmp";
	open(DAT, $data_file) || die("Could not open data file!");
	@fileData = <DAT>;
	close(DAT);
}

# We have the total number of lines
print "T: FILE CONTAINS $#fileData LINES OF DATA\n" if $TRACE;

# Return code value from subroutines, used for CL object data
my $retCode;

# Loop through all the lines in the file.
# Note that some subroutines DO MODIFY the $lineNum variable
for( my $i=0; $i < $#fileData; $i = $lineNum )
{
	$lineNum++;	            # Count what line we are at
	

	# print "T:MAIN LINE #$lineNum\n" if $TRACE;
	
	$_ = $fileData[ $i ];		# We set the global scalar perlvar to the current line to start with
	
	if( $typeToGet eq "CL" )
	{
		$retCode = readCluster();
		print "T: readCluster returned $retCode\n" if $TRACE;	

		# if retCode is 1, we can optimally skip all other processing below and just exit
		if( $retCode == 1 ) { exit 0; }
	}

	if( $typeToGet eq "CD" )
	{
	  readControllers();
	}

	if( $typeToGet eq "PD" )
	{
		getCL_LIDandPID();   # Establish NODE ID's and Names for use
		readPhysDisks();
	}

	if( $typeToGet eq "LD" )
	{
		getCL_LIDandPID();   # Establish NODE ID's and Names for use
		readLogicalDisks();
	}

} 

#print "\n" ;

# End of file
