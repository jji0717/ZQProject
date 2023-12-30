#!/usr/bin/perl

## findAdpaterChileElems.pl
#
#  Perl script that find the child elements attached to
#  an adapter and print the elements names and types.
#  
#
#  Revision History
#
#  04-19-2010       
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
    pod2usage( "\nThis program finds the child elements attached to an storage adapter.
The output of the child elements is in the format (each field is described below):
element_type:element_id
element_type:element_id
...

Usage: $0 [-h] [adapter pciaddress]
-h       : help message");
    exit;
}



# Get the command line options
my $opt_string = "h";
getopts($opt_string, \%opt) or usage();
usage() if $opt{h};
usage() if ($#ARGV != 0);

# Get the pci address
my $PCI = $ARGV[0];

# Validate the pci address
my $class_str = `lspci -Dn | grep $PCI | awk '{print \$2}'`;
chomp $class_str;
my $ispci = $class_str =~ /01.*/s;
#print $class_str," ", $ispci, "\n";
if ( ! $ispci )
{
	print "Not a PCI address of storage controller, exiting...\n";
	exit 1;
}


# Get the vendor ID and device ID
my $device_str = `/sbin/lspci -Dn | grep $PCI | awk '{print \$3}'`;
my ($vendorid, $deviceid) = $device_str =~ /([\d|\w]+)\:([\d|\w]+)/s;


# Here is a map of vendor ID, device ID and adapter types
#
# -------------------------------------------------------------------------
#|      Vendor Name       | Vendor ID | Device ID  |     Adapter Type      |
# -------------------------------------------------------------------------|  
#
#|  Intel Corporation     |   8086    |   269e     |  SATA/IDE controller  |           
#
#|  Intel Corporation     |   8086    |   3a20     |  SATA/IDE controller  |
#
#|  Intel Corporation     |   8086    |   3a26     |  SATA/IDE controller  |
#
#|Hewlett-Packard Company |   103c    |   3230     |  HP RAID Controller   |
#
#|                        |           |            |Generic RAID Controller|
#
#|LSI Logic/Symbios Logic |   1000    |   0058     |  SAS/SCSI Adapter     |
#
#|   Atto Technology      |   117c    |   0042     |  SAS/SCSI Adapter     |
# -------------------------------------------------------------------------


# SATA/IDE controllers

if ( $vendorid eq "8086" )
{
#	print $PCI, " ", $vendorid," ", $deviceid, " SATA/IDE controller\n";

	my @block_devs = `tree /sys/bus/pci/devices/$PCI | grep block:`;
	
	if ( scalar(@block_devs) )
	{

		foreach my $line (@block_devs)
		{
			chomp $line;
			my ($devname) = $line =~ /.*block\/(.*)$/s;
			
			# hard disk
			if ( $devname =~ /hd.*/s )
			{
				print "Hard Disk: ",$devname, "\n";
			}
			
			# scsi disk
			elsif ( $devname =~ /sd.*/s )
			{
				# find the serial number of the scsi device
				my $sn_str = `/usr/bin/sg_inq /dev/$devname | grep \"serial number:\"`;
				chomp ($sn_str);
				my ($serial_no) = $sn_str =~ /Unit\s+serial\s+number\:\s+([\d\w]+)$/s;
				if ( defined($serial_no))
				{
					print "SCSI Disk :", $devname, "(", $serial_no, ")\n";
				}
				else
				{
					print "No device found.\n";
				}
			}
		}
	}
	else
	{
		print "No device found.\n"
	}
}


# HP RAID Controller

elsif( ($vendorid eq "103c") && ($deviceid eq "3230") )
{
#	print $PCI, " ", $vendorid," ", $deviceid, " HP RAID Controller\n";
	
	my @block_devs = `tree /sys/bus/pci/devices/$PCI | grep block:`;

	if ( scalar(@block_devs) )
	{
		foreach my $line (@block_devs)
		{
			chomp $line;
			my ($devid) = $line =~ /.*block\/(cciss.*)$/s;
			
			my ($devid1) = $devid =~ /cciss!(.*)/s;
			my $sn_str = `/usr/bin/sg_inq /dev/cciss/$devid1 | grep \"serial number:\"`;
			chomp($sn_str);
			my ($serial_no) = $sn_str =~ /Unit\s+serial\s+number\:\s+([\d\w]+)$/s;
			
			# display serial number if serial number can be found,
			# otherwise, ignore it
			if ( defined($serial_no) )
			{
				print "CCISS_LD :", $devid, "(", $serial_no, ")\n";
			}
			else
			{
				print "No valid device found.\n";
			}
		}
	}
	else
	{
		print "No device found.\n";
	}
}
	
# SAS/SCSI Adapters

elsif( (($vendorid eq "1000")&&($deviceid eq "0058")) ||
 (($vendorid eq "117c")&&($deviceid eq "0042")) )
{
#	print $PCI, " ", $vendorid," ", $deviceid, " SAS/SCSI Adapter\n";
	
	# find the host number
	my $host = `ls /sys/bus/pci/devices/$PCI | grep host`;
	chomp $host;
	
	my ($hostnum) = $host =~ /host(\d+)/s;

	# find enclosures
	my @enclsgnames = `sg_map -x -i | awk '{if (\$6 ~ /13/ && \$2 ~ /$hostnum/) print \$1}'`;
	
	my $numofencls = 0;
	foreach my $line (@enclsgnames)
	{
		chomp $line;
		my ($sg_name) = $line =~ /\/dev\/(sg.*)$/s;
		my $encl_sas = getSasAddr ( $sg_name );
		print "Enclosure :", $sg_name, "(", $encl_sas, ")\n";
		$numofencls ++;
	}
	
	# if no enclosures found, the disks are directly attached to the adapter.
	# todo: this need to be tested 
	my $numofdisks = 0;
	if ( $numofencls == 0 )
	{
		my @disks = `tree /sys/bus/pci/devices/$PCI/host* | grep block:`;
		
		foreach my $disk (@disks)
		{
			chomp $disk;
			my ($sd_name) = $disk =~ /.*block\/(sd\w+)/s;
			my $sn_str = `/usr/bin/sg_inq /dev/$sd_name | grep \"serial number:\"`;
			chomp ($sn_str);
			my ($serial_no ) = $sn_str =~ /Unit\s+serial\s+number\:\s+([\d\w]+)$/s;
			if( defined($serial_no) )
			{
				print "Disk :", $sd_name, "(", $serial_no, ")\n";
				$numofdisks ++;
			}
		}
	}

	if( ( $numofencls == 0 )&&($numofdisks == 0) ){
		print "No device found.\n";
	}
}

# Add other types here


