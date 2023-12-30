#!/usr/bin/perl

## findEnclChileElems.pl
#
#  Perl script that find the child elements attached to
#  an enclosure and print the elements' bay number and
#  unique id.
#  
#
#  Revision History
#
#  04-22-2010       
#  - Created  jiez
#
#  05-06-2010 jiez
#  - Added the sgname and block device name of disks.
#
# 06-15-2010 cmw
#  - Added full path names to sg_ commands and to grep.
#  - changed return status string for "Enclosure sgName Children" to lower case like sg_ses delivers it.
# 

use warnings;
use strict;
use Getopt::Std;
use Pod::Usage;
use vars qw/ %opt /;

require '/usr/local/seamonlx/bin/common_functions.pl';

#
# Message about this program and how to use it
#
sub usage()
{
    pod2usage( "\nThis program finds the child elements attached to an enclosure.
The output of the child elements is in the format (each field is described below):
bay_number:element_id
bay_number:element_id
...

where, the element id is the SAS address of the end device attached to that drive 
bay, it is empty if there is no end element attached to the drive bay.

Usage: $0 [-h] [enclosure sg_name]
	-h       : help message");
    exit;
}

# Get the command line options
my $opt_string = "h";
getopts($opt_string, \%opt) or usage();
usage() if $opt{h};
usage() if ($#ARGV != 0);

# Get the sg name
my $enclsg = $ARGV[0];

# Validate if it's an enclosure.
my $type = `cat /sys/class/scsi_generic/$enclsg/device/type`;
chomp $type;
if ( $type ne "13" )
{
	print "Device is not an enclosure. Exiting...\n";
	exit 1;
}


# command to get drive bay sas addresses
my $ENC_CMD = "/usr/bin/sg_ses -p 0xa";


# get the number of bays 
my $config = `/usr/bin/sg_ses -p 0x1 /dev/$enclsg | /bin/grep -E -A 2 'Element type: Array device| Element type: Device'`;
my ($numberofBays) = $config =~ /possible\s+number\s+of\s+elements:\s+(\d+)\s*Description:.*/s;


# build hash table of sas address and sgnames of drives 
my $sas_sg_ref = {};
my @sgnames = `/usr/bin/sg_map -x | awk '{if (\$6 ~ /0/) print \$1}'`;
foreach (@sgnames)
{
	chomp ($_);
	my ($sgname) = $_ =~ /\/dev\/(sg\d+)/s;
	my $sas_address = getSasAddr($sgname);
	if(  $sas_address  ){
		$sas_sg_ref->{$sas_address} = $sgname;
	}
	
}


# the hash table that saves the status with the element index as the key.
my $disk_ref = {};

# check sg_ses page 0 make sure page 2 is supported
my $supported = `/usr/bin/sg_ses -p 0x0 /dev/$enclsg 2>&1 | /bin/grep "\[0x2\]" | wc -l`;
if ( $supported )
{
	# Get the sg_ses page 2 output
	local ( $/ );
	open (ELEMINFO , "/usr/bin/sg_ses -p 0x2 /dev/$enclsg 2>&1 |");
	my @chunks = split (/Element type: /, <ELEMINFO>);
	close ELEMINFO;
	

	if( scalar(@chunks) )
	{
		shift( @chunks );
		foreach (@chunks)
		{
			
			my $chunk = $_;
			
			# get the disk devices
			if ( $chunk =~ /[D|d]evice,\s+subenclosure\s+id.*/s )
			{
				my @elems = split(/Element /, $chunk);
				shift(@elems);
				
				foreach(@elems)
				{
					my $elem = $_;
					$elem =~ s/\s{2,}/, /g;
					my ($key, $value) = $elem =~/(\d+)\s+status:,(.*)/s;
					$disk_ref->{$key} = $value;
				}
			}
		}
	}
}


# check sg_ses page 0 make sure page 10 is supported
$supported = `/usr/bin/sg_ses -p 0x0 /dev/$enclsg 2>&1 | /bin/grep "\[0xa\]" | wc -l`;
if ( $supported )
{

    # Get the sg_ses page 10 output
	local ( $/ );
	open (ENCINFO , "$ENC_CMD /dev/$enclsg 2>&1 |");
	my @chunks = split (/Transport protocol: /, <ENCINFO>);
	close ENCINFO;

	
    # Get the sas address of each bay from the page10 output

	if( scalar(@chunks) )
	{
		
		# The first chunk is the enclosure information, ignore it
		my $parag = shift (@chunks);
		
		# Save each drive bay information
		my $bayindex = 0;
		foreach (@chunks)
		{
			if ( $bayindex < $numberofBays )
			{
				my $elem = $_;
				my ( $baynum ) = $elem =~ /^SAS.*bay\s+number:\s+(\d+).*phy\s+index.*/s;
				if ( ! $baynum )
				{
					$baynum = $bayindex;
				}
				
				
				my @phys = split (/phy index: /, $elem);
				shift(@phys);
				
				my $sas = "";
				foreach (@phys)
				{
					
					if ( $_ =~ /^\d+\s+.*attached\sSAS\saddress.*SAS\saddress:\s0x([0-9a-fA-F]{16})$/ms )
					{
						my $physas = $+;
						if( $physas ne "0000000000000000" )
						{
							$sas = $physas;
						}
					}
				}

				my $bdstr = `ls /sys/class/scsi_generic/$sas_sg_ref->{$sas}/device/ | /bin/grep block:`;
				chomp $bdstr;
				my ($bdname) = $bdstr =~ /block:(\w+)\s*$/s;

				print "Bay", $baynum, " :", $sas, "(", $sas_sg_ref->{$sas}, "|", $bdname, "); ";
				
				
				# get the status bits. page 2 is 1 based.
				my $index = $bayindex + 1;
				my ($prdfail ,$disabled, $swap, $status, $ident, $fault ) = $disk_ref->{$index} =~
					/Predicted\s+failure=(\d)\,\s+Disabled=(\d)\,\s+Swap=(\d)\,\s+status\:\s+(\w+)\,.*Ident=(\d).*Fault\s+sensed=(\d)\,/s;

				print "status:", $status, " Disabled=", $disabled," Fault=", $fault, " Ident=", $ident," Predicted failure=", $prdfail," Swap=",$swap, "\n";
				
				$bayindex ++;
			}
		}
	}
}
