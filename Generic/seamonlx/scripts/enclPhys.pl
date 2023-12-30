#!/usr/bin/perl

## enclPhys.pl
#
#  Perl script that displays the enclosure phy information.
#  
#
#  Revision History
#
#  04-30-2010       
#  - Created  jiez
#

use warnings;
use strict;
use Getopt::Std;
use Pod::Usage;
use vars qw/ %opt /;

#require '/usr/local/seamonlx/bin/common_functions.pl';

#
# Message about this program and how to use it
#
sub usage()
{
    pod2usage( "\nThis program finds the information of Phys of an enclosure.
The information of the phys is listed in the following order:
Port number, Phy number, Attached SAS Address, Link Rate, InvDword, Disparity, SyncLoss, RsetProb, DriveAdd, DrivePull, LinkReset, PhyReset

...

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


# utility used to get phy information
my $DODUTIL = "/usr/local/seachange/bin/dodutil";

# check dodutil is installed
if ( -x $DODUTIL )
{

    # Get the phy info
	my %sas_port = ();

	open (PHYS , "$DODUTIL -t /dev/$enclsg 2>&1 |") or die "Couldn't execute program: $! ";
	
	print "Phy Desc | Phy number | Attached SAS  |Link | InvDword | Disparity | SyncLoss | RsetProb | DriveAdd | DrivePull | LinkRet | PhyRset\n";
	while ( defined ( my $line = <PHYS> ) ){
		chomp($line);

		# uplink Phys
		if( $line =~ /Uplink.*/s ){
			my ($port, $phyIdx, $sas, $linkRate, $invDword, $disparity, $syncloss, $rsetProb, $driveAdd, $drivePull, $linkRset, $phyRset) = $line =~ 
				/Uplink\s+(\d+)\s+(\d+)\s+([\d|\w]+).*(\d+\.\d+Gb)\s{1,17}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)/s;
			
			$sas_port{$sas} = "Uplink ".$port;
			$invDword		= $invDword		?	$invDword	: "0"; 
			$disparity		= $disparity	?	$disparity	: "0";
			$syncloss		= $syncloss		?	$syncloss	: "0";
			$rsetProb		= $rsetProb		?	$rsetProb	: "0";
			$driveAdd		= $driveAdd		?	$driveAdd	: "0";
			$drivePull		= $drivePull	?	$drivePull	: "0";
			$linkRset		= $linkRset		?	$linkRset	: "0";
			$phyRset		= $phyRset		?	$phyRset	: "0";

			print $sas_port{$sas}, "  |  ",  $phyIdx, "    |",  $sas, "| ",  $linkRate, " | ", $invDword, "  | ",  $disparity, "  | ",  $syncloss, "  | ", $rsetProb, "  | ", $driveAdd, "  | ",  $drivePull, "  | ",  $linkRset, "  | ", $phyRset, "\n";
		}

		# uplink phys
		elsif( $line =~ /^\s+\d+\s+/s ){
			my ( $phyIdx, $sas, $linkRate, $invDword, $disparity, $syncloss, $rsetProb, $driveAdd, $drivePull, $linkRset, $phyRset) = $line =~ 
				/\s+(\d+)\s+([\d|\w]+).*(\d+\.\d+Gb)\s{1,17}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)/s;
			
			my $port		= $sas_port{$sas};
			$invDword		= $invDword		?	$invDword	: "0"; 
			$disparity		= $disparity	?	$disparity	: "0";
			$syncloss		= $syncloss		?	$syncloss	: "0";
			$rsetProb		= $rsetProb		?	$rsetProb	: "0";
			$driveAdd		= $driveAdd		?	$driveAdd	: "0";
			$drivePull		= $drivePull	?	$drivePull	: "0";
			$linkRset		= $linkRset		?	$linkRset	: "0";
			$phyRset		= $phyRset		?	$phyRset	: "0";
			
			print $port, "  |  ",  $phyIdx, "    |",  $sas, "| ",  $linkRate, " | ", $invDword, "  | ",  $disparity, "  | ",  $syncloss, "  | ", $rsetProb, "  | ", $driveAdd, "  | ",  $drivePull, "  | ",  $linkRset, "  | ", $phyRset, "\n";
			
		}
		
		# end phys
		elsif( $line =~ /^\s+End\s+\d+/s ){
			my ($port, $phyIdx, $sas, $linkRate, $invDword, $disparity, $syncloss, $rsetProb, $driveAdd, $drivePull, $linkRset, $phyRset) = $line =~ 
				/End\s+(\d+)\s+(\d+)\s+([\d|\w]+).*(\d+\.\d+Gb)\s{1,17}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)\s{1,12}(\d*)/s;

			$invDword		= $invDword		?	$invDword	: "0"; 
			$disparity		= $disparity	?	$disparity	: "0";
			$syncloss		= $syncloss		?	$syncloss	: "0";
			$rsetProb		= $rsetProb		?	$rsetProb	: "0";
			$driveAdd		= $driveAdd		?	$driveAdd	: "0";
			$drivePull		= $drivePull	?	$drivePull	: "0";
			$linkRset		= $linkRset		?	$linkRset	: "0";
			$phyRset		= $phyRset		?	$phyRset	: "0";

			print "Bay ", $port, "  |  ",  $phyIdx, "    |",  $sas, "| ",  $linkRate, " | ", $invDword, "  | ",  $disparity, "  | ",  $syncloss, "  | ", $rsetProb, "  | ", $driveAdd, "  | ",  $drivePull, "  | ",  $linkRset, "  | ", $phyRset, "\n";
		}		
	}
	close PHYS;
}
