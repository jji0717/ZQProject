#!/usr/bin/perl

## enclEnvElements.pl
#
#  Perl script that prints out the enclousre elements
#  information. The environmental elements will be displayed are
#
#  devices, cooling fans, power supplies and temperature sensors.
#
#  Revision History
#
#  04-28-2010       
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
    pod2usage( "\nThis program prints the information of an enclosure's elements, including devices, cooling fans, power supplies and temperature sensors by given the enclosure's sgname. if no type is given, all the elements are printed out.
The output is in the following format:
element ID : current_reading ( status )

Usage: $0 [option] <Arguments>
       $0 [-h] 
       $0 [sg_name] [Element type]

Element type:
       disk               :disk devices    
       fan                :cooling fans 
       power              :power supplies
       temperature        :temperature sensors

Options:
	   -h                 :help message

Examples:
       $0 sg25            -- get elements of all the 4 types
       $0 sg25 fan        -- get the cooling fan elements
       $0 sg25 power      -- get the power supply elements
       $0 -h              -- print this usage message
"); 


    exit;
}

# Get the command line options
my $opt_string = "h";
getopts($opt_string, \%opt) or usage();
usage() if $opt{h};
usage() if (($#ARGV > 1) || ($#ARGV < 0));

# Get the sg name
my $enclsg = $ARGV[0];

# Validate if it's an enclosure.
my $type = `cat /sys/class/scsi_generic/$enclsg/device/type 2>&1`;
chomp $type;
if ( $type ne "13"  )
{
	print "First input argument is not an enclosure. Exiting...\n";
	exit 1;
}

# Validate the type
my $elemtype;
if( defined( $ARGV[1] )){
    $elemtype = $ARGV[1];
	if ( ($elemtype ne "disk") && ($elemtype ne "fan") && ($elemtype ne "power") && ($elemtype ne "temperature") )
	{
		print "Invalid argument 2 : Unknown element type. exiting... \n";
	}
}


my $disk_ref = {};
my $fan_ref = {};
my $power_ref = {};
my $temp_ref = {};

my $diskdesc_ref = {};
my $fandesc_ref = {};
my $powerdesc_ref = {};
my $tempdesc_ref = {};


# check sg_ses page 0 make sure page 2 is supported
my $supported = `sg_ses -p 0x0 /dev/$enclsg 2>&1 | grep "\[0x2\]" | wc -l`;
if ( $supported )
{
	# Get the sg_ses page 2 output
	local ( $/ );
	open (ELEMINFO , "sg_ses -p 0x2 /dev/$enclsg 2>&1 |");
	my @chunks = split (/Element type: /, <ELEMINFO>);
	close ELEMINFO;
	

    # Get the sas address of each bay from the page10 output
	
	if( scalar(@chunks) )
	{
		shift( @chunks );
		foreach (@chunks)
		{
			
			my $chunk = $_;
			
			# get the disk devices
			my $disknum = 0;
			if ( $chunk =~ /[D|d]evice,\s+subenclosure\s+id.*/s )
			{
				if( !defined($elemtype) || ($elemtype eq "disk") )
				{
					getElementsStatus( $chunk, $disk_ref );
				}
			}

			# get the fans 
			elsif ( $chunk =~ /Cooling,\s+subenclosure\s+id.*/s )
			{
				if( !defined($elemtype) || ($elemtype eq "fan") )
				{
					getElementsStatus( $chunk, $fan_ref );
				}
			}

			# get the power supplies
			elsif( $chunk =~ /Power\s+supply,\s+subenclosure\s+id.*/s )
			{
				if( !defined($elemtype) || ($elemtype eq "power") )
				{
					getElementsStatus( $chunk, $power_ref );
				}
			}

			# get the temperature sensors
			elsif( $chunk =~ /Temperature\s+sense,\s+subenclosure\s+id.*/s )
			{
				if( !defined($elemtype) || ($elemtype eq "temperature") )
				{
					getElementsStatus( $chunk, $temp_ref );
				}
			}
			
		}
	}


}
else{
	print "sg_ses page 2 is not supported, exiting...\n\n";
	exit 1;
}


# check sg_ses page 0 make sure page 7 is supported. Get device names.
$supported = `sg_ses -p 0x0 /dev/$enclsg 2>&1 | grep "\[0x7\]" | wc -l`;
if ( $supported )
{
	# Get the sg_ses page 7 output
	local ( $/ );
	open (NAMEINFO , "sg_ses -p 0x7 /dev/$enclsg 2>&1 |");
	my @chunks = split (/Element type: /, <NAMEINFO>);
	close NAMEINFO;

	    # Get the sas address of each bay from the page10 output
	
	if( scalar(@chunks) )
	{
		shift( @chunks );
		foreach (@chunks)
		{
			
			my $chunk = $_;
			
			# get the disk devices
			my $disknum = 0;
			if ( $chunk =~ /[D|d]evice,\s+subenclosure\s+id.*/s )
			{
				if( !defined($elemtype) || ($elemtype eq "disk") )
				{
					getElemDesc( $chunk, $diskdesc_ref );
				}
			}

			# get the fans 
			elsif ( $chunk =~ /Cooling,\s+subenclosure\s+id.*/s )
			{
				if( !defined($elemtype) || ($elemtype eq "fan") )
				{
					getElemDesc( $chunk, $fandesc_ref );
				}
			}

			# get the power supplies
			elsif( $chunk =~ /Power\s+supply,\s+subenclosure\s+id.*/s )
			{
				if( !defined($elemtype) || ($elemtype eq "power") )
				{
					getElemDesc( $chunk, $powerdesc_ref );
				}
			}

			# get the temperature sensors
			elsif( $chunk =~ /Temperature\s+sense,\s+subenclosure\s+id.*/s )
			{
				if( !defined($elemtype) || ($elemtype eq "temperature") )
				{
					getElemDesc( $chunk, $tempdesc_ref );
				}
			}
			
		}
	}
}



#output
if ( keys(%$disk_ref) )
{
	foreach my $index (sort keys %$disk_ref)
	{
		print "Disk Element ", $index; 
		if (defined ( $diskdesc_ref->{$index} ))
		{
			print " (", $diskdesc_ref->{$index}, ")";
		}
		print " : ", $disk_ref->{$index}, "\n";
	}
	print "\n";
}


if ( keys(%$fan_ref) )
{
	foreach my $index (sort keys %$fan_ref)
	{
		print "Fan Element ", $index; 
		if (defined ( $fandesc_ref->{$index} ))
		{
			print " (", $fandesc_ref->{$index}, ")";
		}
		print " : ", $fan_ref->{$index}, "\n";
	}
	print "\n";
}

if ( keys(%$power_ref) )
{
	foreach my $index (sort keys %$power_ref)
	{
		print "Power Element ", $index; 
		if (defined ( $powerdesc_ref->{$index} ))
		{
			print " (", $powerdesc_ref->{$index}, ")";
		}
		print " : ", $power_ref->{$index}, "\n";
	}
	print "\n";
}

if ( keys(%$temp_ref) )
{
	foreach my $index (sort keys %$temp_ref)
	{
		print "Temperature Sensor Element ", $index; 
		if (defined ( $tempdesc_ref->{$index} ))
		{
			print " (", $tempdesc_ref->{$index}, ")";
		}
		print " : ", $temp_ref->{$index}, "\n";
	}
	print "\n";
}



sub getElementsStatus
{
	my $chunk = $_[0];
	my $elem_ref = $_[1];

	my @elems = split(/Element /, $chunk);
	shift(@elems);
	
	foreach(@elems)
	{
		my $elem = $_;
		$elem =~ s/\s{2,}/, /g;
		my ($key, $value) = $elem =~/(\d+)\s+status:,(.*)/s;

		if( defined($elem_ref->{$key}) ){
			#todo how to name two sets of same elements?
			$key = $key."+";
		}
		$elem_ref->{$key} = $value;
	}
}


sub getElemDesc
{
	my $chunk = $_[0];
	my $elemdesc_ref = $_[1];

	my @elems = split(/Element /, $chunk);
	shift(@elems);
	
	foreach(@elems)
	{
		my $elem = $_;
	
		my ($key, $value) = $elem =~ /(\d+)\s+descriptor:\s*(.*[\w|\d])/s;
		
		if( defined($elemdesc_ref->{$key}) ){
			#todo how to name two sets of same elements?
			$key = $key."+";
		}
		$elemdesc_ref->{$key} = $value;
	}	
}
