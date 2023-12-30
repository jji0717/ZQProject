#!/usr/bin/perl

## findDiskParent.pl
#
#  Perl script that find the enclosure a given disk attached to.
#
#  Note that id doesn't find the parent device is the parent device
#  is not an enclosure.
#  
#  TODO: this script can be expanded to get the Adapter or other devices
#        the disk attached to. 
#
#  Revision History
#
#  04-22-2010       
#  - Created  jiez
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
    pod2usage( "\nThis program finds the parent enclosure of a disk drive and print
the id of the parent element or 'Disk not attached to an enclosure' if no enclosure
is found as the parent of the disk.

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
my $disksg = $ARGV[0];

# Validate if it's an scsi_disk.
my $type = `cat /sys/class/scsi_generic/$disksg/device/type`;
chomp $type;
if ( $type ne "0" )
{
	print "Device is not an scsi disk. Exiting...\n";
	exit 1;
}



# find the sas address of the disk
my $disksas = getSasAddr($disksg);
if( ! $disksas  ){
	
	# Not a physical drive or not attached to an enclosure.
	# TODO: Add code to process this kind of disks.
	
	print "Disk not attached to an enclosure.\n";
	exit 0;
}
# print $disksg, " : ", $disksas, "\n";


# get the host number of the disk
my $host = `/usr/bin/sg_map -x | grep -w $disksg | awk '{print \$2}'`;
chomp $host;


# get the enclosure under the same host.
my @enclsgs = `sg_map -x | awk '{if (\$2~/$host/ && \$6~/13/) print \$1}'`;

foreach (@enclsgs)
{
	my $sgstr = $_;
	chomp $sgstr;
	my ($enclsg) = $sgstr =~ /\/dev\/(sg.*)\s*$/s;

	# command to get drive bay sas addresses
	my $ENC_CMD = "sg_ses -p 0xa";


	# get the number of bays 
	my $config = `sg_ses -p 0x1 /dev/$enclsg | grep -E -A 2 'Element type: Array device| Element type: Device'`;
	my ($numberofBays) = $config =~ /possible\s+number\s+of\s+elements:\s+(\d+)\s*Description:.*/s;


	#check sg_ses page 0 make sure page 10 is supported
	my $supported = `sg_ses -p 0x0 /dev/$enclsg 2>&1 | grep "\[0xa\]" | wc -l`;
	if ( $supported )
	{

		#Get the sg_ses page 10 output
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
#								print $enclsg, " Bay", $baynum, ": ", $sas, "\n";

								if ( "$sas" eq "$disksas" ){
									print "Parent Enclosure : ", $enclsg, "\n";
									exit;
								}
							}
						}
					}
					
					$bayindex ++;
				}
			}
		}
	}
}

