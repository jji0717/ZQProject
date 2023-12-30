#!/usr/bin/perl

## enclInfo.pl
#
#  Perl script that prints out the enclousre basic information.
#
#  Revision History
#
#  04-27-2010       
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
    pod2usage( "\nThis program prints the basic information of an enclosure that has the sg name provided by user.
The output is in name-value pair format like this:
product ID : value
SAS Address: value
Number of Drive Bays: value
...

Usage: $0 [-h] [sg_name]
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
if ( $type != 13 )
{
	print "Device is not an enclosure. Exiting...\n";
	exit 1;
}

# get the sas address
my $enclsas = getSasAddr ( $enclsg );

# get the SCSI Address
my $scsi = `/usr/bin/sg_map -x | grep -w $enclsg | awk '{if (\$6 ~ /13/) print \$2, \$3, \$4, \$5}'`;
if( defined ($scsi)){
	chomp $scsi;
	$scsi =~ s/\s+/\:/g;
}

# get the vendor and product and revision.
my $line = `/usr/bin/sg_ses -p 0x1 /dev/$enclsg | grep vendor: | grep product:`;
my $vendor;
my $productid;
my $revision;
if( defined( $line) ){
	chomp $line;
	($vendor, $productid, $revision) = $line =~ /vendor\:\s+(.*)\s+product\:\s+(.*)\s+rev\:\s+(.*)$/s;
}

# get the number of drive bays, cooling fans, power supplies, temperature sensors.
my $bayinfo = `/usr/bin/sg_ses -p 0x1 /dev/$enclsg | grep -E -A 2 'Element type: Array device| Element type: Device'`;
my $numberofBays;
if ( defined ($bayinfo) ){
	($numberofBays) = $bayinfo =~ /possible\s+number\s+of\s+elements:\s+(\d+)\s*Description:.*/s;
}

# get the firmware version.
my $dodfile = "/usr/local/seachange/bin/dodutil";
my $version;
my $build;
if( -X $dodfile){
	my $fwlines = `$dodfile -f /dev/$enclsg | grep -A 2 Running`;
	($version, $build) = $fwlines =~ /Version\s+\:\s+(.*)\s+Build\s+\:\s+.*(\(.*\))$/s;
	if( defined ( $version ) && defined ($build)){
		$version =~ s/\s+//g;
		$build =~ s/\s+//g;
	}
}

# Get the status
my $cmd = "sg_ses -p 2 /dev/$enclsg | grep 'status: ' | grep -v Overall | grep -v Unsupported";
open(SOUT, "$cmd |") || die "Failed: $!\n";

my $overall_status = "OK";
while( <SOUT> )
{
	my ( $status ) = $_ =~ /.*status\:\s+(.*)$/s;
	chomp $status;
	if ( $status ne "OK" && $status ne "ok" )
	{
		$overall_status = "Critical";
	}
}

# Get the Unit serial number
my $usn_str = `/usr/bin/sg_inq -p 0x80 /dev/$enclsg | grep  "Unit serial number:"`;
my $usn;
if (defined ($usn_str)){
	($usn ) = $usn_str =~ /Unit\s+serial\s+number:\s+([\d|\w]+)\s+/s;
}



# output
if( defined( $usn ) )
{
	print "Unit Serial Number : ";
	print $usn, "\n";
}

if ( defined( $enclsas ) )
{
	print "SAS Address : ";
	 print $enclsas, "\n";
}

if ( defined( $scsi ) )
{
	print "SCSI Address : ";
	print $scsi, "\n";
}

if ( defined( $vendor ) )
{
	print "Vendor : ";
	print $vendor, "\n";
}

if( defined( $productid ) )
{
	print "Product ID : ";
	print $productid, "\n";
}

if( defined( $revision ) )
{
	print "Revision : ";
	print $revision, "\n";
}

if( defined( $version ) )
{
	print "Firmware Version : ";
	print $version;
}

if( defined( $build ) )
{
	print $build, "\n";
}

print "Overall Status : ";
if( defined( $overall_status) )
{
	print $overall_status, "\n";
}

print "Number of Drive Bays : ";
if( defined( $numberofBays ) )
{
	print $numberofBays, "\n";
}


