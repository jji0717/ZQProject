#!/usr/bin/perl

## lsi_info.pl
#
#  This program collects and displays the lsi devices information.
#  
#
#  Revision History
#
#  04-21-2010       
#  - Created
#

use warnings;
use strict;
use Getopt::Std;
use Pod::Usage;
use vars qw/ %opt /;

# Declare the subroutines
sub trim($);
		 
#constants definition
		 
my $LSIUTIL_FILE = "/usr/local/seamonlx/utils/lsiutil.x86_64";
my $LSIUTIL = "$LSIUTIL_FILE -i";

sub usage()
{
    pod2usage( "\nThis program collects and displays the lsi devices information.
The output is in the following format:

PCIAddress    Phynum      SasAddress      Port    LinkStatus  LinkRate  MinRate  MaxRate  ErrorCounters
----------    ------     ------------    ------   ----------  --------  -------  -------  -------------
pci_addr   | Phy_number | sas address| port number |up or down| rate   |  rate  | rate |number|number|number|number
......


The error counters are in this order:
invalid dword count | loss of dword sync count | phy reset problem count | running disparity error count

Usage: $0
	   $0  -h
	-h       : help message");
    exit;
}

# Get the command line options
my $opt_string = "h";
getopts($opt_string, \%opt) or usage();
usage() if $opt{h};
usage() if ($#ARGV >= 0);

if ( ! -e $LSIUTIL_FILE )
{
	print $LSIUTIL_FILE, " is not installed on the system. Please install it and try again.\n";
	exit 1;
}

my @ports;
my $number_of_ports;

# run lsiutil and save ports to an array.
{
	local ($/);
	open (PORTINFO, "$LSIUTIL |" );
	my @array = split(/(=)+/, <PORTINFO>);
	close PORTINFO;

	my $header = shift (@array);
	if( defined( $header ) ){
		( $number_of_ports ) = $header =~ /(\d+)\s+MPT\sPort\s+found/s;
	}
	@ports = grep { $_ =~ /Seg\/Bus\/Dev\/Fun/ } @array;
	
}

#parse the setting of each port
if( (! defined($number_of_ports)) || ($number_of_ports == 0 )){
	print "0 MPT Ports found. Exiting...\n";
	exit 0;
}

foreach (@ports)
{
	my $info = $_;

	# get the id
	my ($logicId) = $info =~ /\/proc\/mpt.*LSI\s+Logic\s+([0-9|A-Z]+)\s+/s;


	# get the pci address
	my $seg_dec;  #decimal value
	my $bus_dec;  #decimal value
	my $dev_dec;  #decimal value
	my $fun_dec;  #decimal value

	if( defined ($info) ){
		($seg_dec, $bus_dec, $dev_dec, $fun_dec, ) = $info =~ 
			/Seg.*Board\sName.*Board\sAssembly.*Board\sTracer\s*(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+.*/s;
		
		
		my $seg_hex = sprintf("%04x", $seg_dec);
		my $bus_hex = sprintf("%02x", $bus_dec);
		my $dev_hex = sprintf("%02x", $dev_dec);
		my $fun_hex = sprintf("%x", $fun_dec);
		my $pci_addr = $seg_hex.":".$bus_hex.":".$dev_hex.".".$fun_hex;
		
		
		# get the firmware version
		my ($firmware) = $info =~ 
			/Current\s+active\s+firmware\s+version\s+is\s+[0-9|a-z]+\s\(([\d+|\.]+)\)/s;
		print "\n", $pci_addr, " Firmware Version :", $firmware, "\n\n";
		
		# get the link rate
		my ($links) = $info =~
			/$logicId\'s\s+links\s+are\s+(([\d|\.]+\sG\,\s)+[\d|\.]+\sG)/s;
		
		my @link_rate = split(",", $links);
		
		# get the phy numbers
		my ($phys) = $info =~
			/Phy\s+Parameters\s+for\s+Phynum:\s+((\d+\s+)+\d+)/s;
		my @phynum = split (" ", $phys);
		
		# get the phy min status
		my ($phystatus) = $info =~ /Link\sEnabled:\s+((\w+\s+)+\w+)/s;
		my @status = split(" ", $phystatus);
		
		# get the link min rate
		my ($min_rate) = $info =~ /Link\sMin\sRate:\s+(([\d\.]+\s+)+[\d\.]+)/s;
		my @minrate = split(" ", $min_rate);
		
		# get the link min rate
		my ($max_rate) = $info =~ /Link\sMax\sRate:\s+(([\d\.]+\s+)+[\d\.]+)/s;
		my @maxrate = split(" ", $max_rate);
		
		print "PCIAddress    Phynum      SasAddress      Port    LinkStatus  LinkRate  MinRate  MaxRate  ErrorCounters\n";
		print "----------    ------     ------------    ------   ----------  --------  -------  -------  -------------\n";		
		for ( my $i = 0; $i <= $#phynum; $i ++ )
		{
			# link status
			my $link_status;
			if ( lc($status[$i]) eq "yes" ){
				$link_status = "up";
			}
			else{
				$link_status = "down";
			}
			
			my $phydir = "/sys/bus/pci/devices/$pci_addr/host*/phy-0:$phynum[$i]";
			
			# port sas_address
			my $sas_addr = `cat $phydir/sas_phy:phy*/sas_address`;
			chomp($sas_addr);
			
			# port number
			my $sasport = `ls $phydir/port | grep sas_port`;
			chomp($sasport);
			my ($port) = $sasport =~ /sas_port:(.*)/s;
			
			# error counters
			my $invalid_dword_count = `cat $phydir/sas_phy:phy*/invalid_dword_count`;
			my $loss_of_dword_sync_count = `cat $phydir/sas_phy:phy*/loss_of_dword_sync_count`;
			my $phy_reset_problem_count = `cat $phydir/sas_phy:phy*/phy_reset_problem_count`;
			my $running_disparity_error_count = `cat $phydir/sas_phy:phy*/running_disparity_error_count`;
			chomp($invalid_dword_count);
			chomp($loss_of_dword_sync_count);
			chomp($phy_reset_problem_count);
			chomp($running_disparity_error_count);
			
			$link_rate[$i] =~ s/\s+//g;
			my ($unit) = $link_rate[$i] =~ /.*(\w)/s;
			
			print $pci_addr, " | Phy",$phynum[$i]," | ", $sas_addr,"| ", $port, " |   ", $link_status, "   |  ", $link_rate[$i], "   |  ", trim($minrate[$i]), $unit, "  |  ", trim($maxrate[$i]), $unit, " |  ", $invalid_dword_count, " | ", $loss_of_dword_sync_count, " | ", $phy_reset_problem_count, " | ", $running_disparity_error_count,"\n";
			
			print "\n";
		}
	}
}

# Subroutine to trim off leading and trailing whitespaces.
sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}
