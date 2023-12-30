#!/usr/bin/perl

## enclInfo.pl
#
#  Perl script that prints out the disk information.
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
use Switch;

require '/usr/local/seamonlx/bin/common_functions.pl';

#
# Message about this program and how to use it
#
sub usage()
{
    pod2usage( "\nThis program prints the basic information of an enclosure that has the block device name provided by user.
The output is in name-value pair format like this:
SAS Address: value
SCSI Address: value
...


Usage: $0 [-h] [block_device_name]
	-h       : help message");
    exit;
}


# Get the command line options
my $opt_string = "h";
getopts($opt_string, \%opt) or usage();
usage() if $opt{h};
usage() if ($#ARGV != 0);

# Get the block device name
my $bdname = $ARGV[0];

# Validate if it's a disk drive.
my $typefile = "/sys/block/$bdname/device/type";
if ( -r $typefile)
{
	my $type = `cat $typefile`;
	chomp $type;
	if ( $type != 0 )
	{
		print "Device is not an disk drive. Exiting...\n";
		exit 1;
	}
}
else
{
	print "Failed getting device type. Exiting...\n";
	exit 1;
}


# get the sgname
my $sgstr = `ls /sys/block/$bdname/device | grep scsi_generic`;
my $sgname;
if( defined ($sgstr) ){
	chomp $sgstr;
	($sgname) = $sgstr =~ /scsi_generic:(sg\d+)/;
	if( defined( $sgname) ){ print "SG Name : ", $sgname, "\n"; }
}



# get the SCSI Address
my $scsistr = `ls /sys/block/$bdname/device | grep scsi_disk`;
if( defined ($scsistr)){
	chomp $scsistr;
	my ($scsi) = $scsistr =~ /scsi_disk:(\d+\:\d+\:\d+\:\d+)/s;
	if( defined( $scsi) ){ print "SCSI Address : ", $scsi, "\n"; }
}


# get the vendor
my $vendorfile = "/sys/block/$bdname/device/vendor";
if ( -r $vendorfile )
{
	my $vendor = `cat $vendorfile`;
	if( defined ($vendor)) {
		chomp $vendor;
		print "Vendor : ", $vendor, "\n";
	}
}


# get the model number
my $modelfile = "/sys/block/$bdname/device/model";
if ( -r $modelfile )
{
	my $model = `cat $modelfile`;
	if( defined ($model)) {
		chomp $model;
		print "Model : ", $model, "\n";
	}
}

if( defined ($sgname) ){
    # get the logical address
	my $sasaddr = getSasAddr( $sgname );
	if( $sasaddr ){ 
		print "Logical Address : ", $sasaddr, "\n"; 
	}
	else{
		my $scsi_name_str = `sg_inq -p 0x83 /dev/$bdname | grep -A 1 "SCSI name string:" | grep -v "SCSI name string:"`;
		if ( defined($scsi_name_str) )
		{
			my ($lgid) = $scsi_name_str =~ /.*([\d|\w]{16})$/s;
			if( defined($lgid)){ print "Logical Address : ", $lgid, "\n"; }
		}
	}
		


    # get the serial number

	my $serialstr = `/usr/bin/sg_inq /dev/$sgname 2>&1 | grep "Unit serial number" `;
	if( defined ($serialstr) ){
		chomp $serialstr;
		my ($serialnum) = $serialstr =~ /Unit\sserial\snumber:\s+([\d|\w]+)/s;
		if( defined( $serialnum) ){ print "Serial Number : ", $serialnum, "\n"; }

	}
	
    # get revision level ( firmware version ).
	my $fwverstr = `/usr/bin/sg_inq /dev/$sgname 2>&1 | grep "revision level:"`;
	if( defined ($fwverstr) ){
		chomp $fwverstr;
		my ($fwver) = $fwverstr =~ /Product\s+revision\s+level\:\s+(.*)$/s;
		if( defined( $fwver) ){ print "Firmware Version : ", $fwver, "\n"; }
	}

	# get the size
	my $sizestr = `/usr/bin/sg_readcap /dev/$sgname 2>&1 | grep 'Device size'`;
	if( defined ($sizestr) ){
		chomp $sizestr;
		my ($size) = $sizestr =~ /Device\s+size\:\s+(\d+.*)$/s;
		if( defined($size) ) { print "Capacity : ", make_number_nice( $size ), "\n"; }
	}

	# get the drive temperature
# ACE-10376 my $tempstr = `/usr/bin/sg_logs -p 0xd /dev/$sgname 2>&1 | grep 'Current temperature'`;
#	if( $tempstr )
#	{
#		chomp $tempstr;
#		my ( $temp ) = $tempstr =~ /Current\s+temperature\s+=\s+(\d+\s+C)/s;
#		if( defined ($temp) ){ print "Temperature : ", $temp, "\n"; }
#	}
}


# get the state
my $statefile = "/sys/block/$bdname/device/state";
if( -r $statefile )
{
	my $state = `cat $statefile`;
	if( defined($state) ){
		chomp $state;
		print "State : ", $state, "\n";
	}
}

# get the I/O error count
my $ioerrfile = "/sys/block/$bdname/device/ioerr_cnt";
if( -r $ioerrfile )
{
	my $ioerr = `cat $ioerrfile`;
	if( defined($ioerr) ){
		chomp $ioerr;
		print "I/O Error : ", hex( $ioerr ), "\n";
	}
}

# get the blocks written and blocks read
my $blockrw = `/usr/bin/iostat -d /dev/$bdname | grep $bdname | awk '{print \$5, \$6}'`;
if( defined ($blockrw) )
{
	chomp $blockrw;
	my ($blockr, $blockw) = $blockrw =~ /^(\d+)\s+(\d+)$/s;

	if( defined ($blockr) ){ print "Blocks Read : ", $blockr, "\n"; }
	if( defined ($blockw) ){ print "Blocks Written : ", $blockw, "\n"; }
}

# get the cache attributes
my $caline = `/usr/bin/sg_modes -c 0 -p 8 /dev/$bdname | grep  -A 2 Caching | grep '^ 00 '`;
if( defined ($caline) ){
	my ( $cabits )= $caline =~ /\s+00\s+\d{2}\s+\d{2}\s+\d(\d)/s;
	if( defined ($cabits) )
	{ 
		print "Cache Attribute : ";
		switch( $cabits ){
			case "0" {print "R\n"}
			case "4" {print "RW\n"}
			case "5" {print "W\n"}
			case "1" {print "NONE\n"}
			else { print "\n"};
		}		
	}
}
