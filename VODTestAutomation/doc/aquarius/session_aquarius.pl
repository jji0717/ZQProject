#!perl -w

use Getopt::Long;
use strict;

my $CFG_FILE    = 'C:\logs.cfg';
my @MODULE_NAME = ('SRM','DMN', 'CM','SS','MOD','MCAClient','MCAServer','Hammer','SSAPI'); #Jeff 11/14/02
my $SRM         = 0;
my $DMN         = 1; #Cathy needs to be in same order as in list above
my $CM          = 2;
my $SS          = 3;
my $MOD         = 4;
my $MCACLI      = 5;
my $MCASER      = 6;
my $HAMMER      = 7;
my $SSAPI       = 8;

my @HMRLOGS     = (
        );
         
my @SRMLOGS     = (
        );

my @DMNLOGS     = (  #Jeff 11/14/02
        );
         
my @CMLOGS      = (
        );

my @SSAPILOGS   = (
        );

my @SSLOGS      = (
        );

my @MODLOGS     = (
        );

my @MCACLILOGS  = (
        );

my @MCASERLOGS  = (
        );

#$OUTPUT   = 'output.txt';
my $INBETWEENS  = 0;
my $DEBUG       = 1;
my $VERBOSE     = 1;

#
# Global Variables
#
my $SS_SID      = '';
my $MCA_SHDL    = '';
my $SESSIONID   = '';
(my $EXEDIR     = $0) =~ s/(.*[\\\/])[^\\\/]+/$1/;
local *OUT;

#
# GetOpts
#
my $opt_c;
my $opt_f;
my $opt_h;
my $opt_help;
my $opt_listlogs;
my $opt_testaccess;
my $opt_sid;
my $opt_verbose;

#
# Main Program
#
sub main
{
  # Get long command line options.
  &GetOptions(
        'c=s'           => \$opt_c,
        'f=s'           => \$opt_f,
        'h'             => \$opt_h,
        'help'          => \$opt_help,
        'listlogs'      => \$opt_listlogs,
        'testaccess'    => \$opt_testaccess,
        'sid=s'         => \$opt_sid,
        'verbose'       => \$opt_verbose,
        );

  $SESSIONID  = $ARGV[0];

  my $file;
  
  if ($opt_help or $opt_h)
  {
    &show_usage();
    exit 0;
  }

  $VERBOSE = $opt_verbose;

  &show_header();

  $opt_c = $CFG_FILE if not $opt_c;
  &ReadConfigFile($opt_c);

  #
  # Go through the target log files and replace any ones that end in `-' with
  # their equivalent file names.
  #
  &MungeLogFiles(\@HMRLOGS);
  &MungeLogFiles(\@SRMLOGS); 
  &MungeLogFiles(\@DMNLOGS);  #Jeff 11/14/02
  &MungeLogFiles(\@CMLOGS);
  &MungeLogFiles(\@SSAPILOGS);
  &MungeLogFiles(\@SSLOGS);
  &MungeLogFiles(\@MODLOGS);
  &MungeLogFiles(\@MCACLILOGS);
  &MungeLogFiles(\@MCASERLOGS);

  if ($opt_listlogs)
  {
    foreach $file (@HMRLOGS,@SRMLOGS,@DMNLOGS,@CMLOGS,@SSAPILOGS,@SSLOGS,@MODLOGS,@MCACLILOGS) #Jeff 11/14/02
    {
      if (not -f $file) { print "Unable to access $file\n"; }
      else { print "Using log $file\n"; }
    }
    exit 0;
  }
  
  if ($opt_testaccess)
  {
    foreach $file (@HMRLOGS,@SRMLOGS,@DMNLOGS,@CMLOGS,@SSAPILOGS,@SSLOGS,@MODLOGS,@MCACLILOGS) #Jeff 11/14/02
    {
      if (not -f $file) { print "Unable to access $file\n"; }
      else { print "$file OK\n"; }
    }
    exit 0;
  }
  
  if ($opt_sid)
  {
    $SS_SID = $opt_sid;
  }

  if (not $SESSIONID and not $opt_sid)
  {
    die "*** ERROR: please specify a session ID\n";
  }
  if ($SESSIONID =~ /-/)
  {
    my ($mac,$num) = split ('-',$SESSIONID);
    $num = 4294967296 - $num;
    &Print("Fixing -ve: $SESSIONID ==> $mac$num\n");
    $SESSIONID = "$mac$num";
  }

  my $outfile = $SESSIONID;
  if ($ARGV[1])
  {
    # A facility was specified, so lets add this to the name of the file.
    $outfile .= "-$ARGV[1]";
  }
  ($outfile = "$outfile.log") =~ s/\//_/g;
  $outfile =~ s/:/\./g;
  $outfile = $opt_f if $opt_f;
  open OUT,">$outfile" or die "*** ERROR: unable to write <$outfile>: $!\n";
  
  foreach $file (@HMRLOGS)
  {
    &SuckLogInfo($HAMMER, $file, $SESSIONID);
  }
  foreach $file (@SRMLOGS)
  {
    &SuckLogInfo($SRM, $file, $SESSIONID);
  }
  foreach $file (@DMNLOGS)  #Jeff 11/14/02
  {
    &SuckLogInfo($DMN, $file, $SESSIONID);
  }
  foreach $file (@CMLOGS)
  {
    &SuckLogInfo($CM, $file, $SESSIONID);
  }
  if ($SS_SID)
  {
    foreach $file (@SSAPILOGS)
    {
      &SuckLogInfo($SSAPI, $file, "Sid=$SS_SID");
    }
    foreach $file (@SSLOGS)
    {
      &SuckLogInfo($SS, $file, "Sid=$SS_SID");
    }
    foreach $file (@MODLOGS)
    {
      &SuckLogInfo($MOD, $file, $SS_SID);
    }
    if ($MCA_SHDL)
    {
      foreach $file (@MCACLILOGS)
      {
        &SuckLogInfo($MCACLI, $file, $MCA_SHDL);
      }

      foreach $file (@MCASERLOGS)
      {
        &SuckLogInfo($MCASER, $file, $MCA_SHDL);
      }
    }
    else
    {
      &Print("*** ERROR: unable to examine MCA logs because an MCA streamhandle was not found!");
    }
  }
  else
  {
    &Print("*** ERROR: unable to examine SS/MOD/MCA logs because an SS Stream ID was not found!");
  }

  close OUT;
}

&main();

sub Print
{
  my ($msg) = @_;
  print OUT $msg;
  print $msg;
}

sub SuckLogInfo
{
  my ($module, $file, $str) = @_;
  
  local *IN;
  open IN,$file or
    die "*** ERROR: unable to open log $file: $!\n";

  &Print("\n======= [$MODULE_NAME[$module]] SEARCHING ($file) FOR <$str> =======\n\n");

  if ($INBETWEENS)
  {
    my $firstLine = 0;
    my $firstPos = 0;
    my $lastLine = 0;
    my $line = 0;
    while (<IN>)
    {
      ++$line;
      if (/$str/)
      {
        if (0 == $firstLine)
        {
          $firstLine = $line;
          $DEBUG and &Print("DEBUG: first line found on line $line\n");
          $firstPos = tell IN;
          $DEBUG and &Print("DEBUG: first line found at byte offset $firstPos\n");
        }
        else
        {
          $lastLine = $line;
          #&Print("DEBUG: last line found on line $line\n");
        }
      }
    }
    seek IN,0,0; # seek to the beginning of the file.
    #seek IN,$firstPos,0; # seek to first line position
    #$line = $firstLine;
    $line = 0;
    my $count = $lastLine - $firstLine;
    &Print("Printing $count lines of output...\n");
    getc;
    while (<IN>)
    {
      chomp;
      ++$line;
      if ($line >= $firstLine and $line <= $lastLine)
      {
        &ProcessLine($module, $_);
      }
    }
  }
  else
  {
    while (<IN>)
    {
      if (/$str/)
      {
        &ProcessLine($module, $_);
      }
    }

  &Print("\n======= [$MODULE_NAME[$module]] SEARCH COMPLETE ($file) FOR <$str> =======\n\n");

  }
}

sub ProcessLine
{
  my ($module, $line) = @_;
  chomp ($line);
  &Print("$line\n");
  if ($module == $SRM)
  {
  }
  elsif ($module == $DMN) #Jeff 11/14/02
  {
  }
  elsif ($module == $CM)
  {
    if ($line =~ /Calling IssCreateStream\(Asset=(.*?),SsSpec=(\d+)\/(\d+),Nodegroup=(\d+),Sid=(\d+)/)
    {
      if ($SS_SID)
      {
        &Print("*** WARNING ***: I already found a stream ID ($SS_SID) for $SESSIONID\n");
      }
      $SS_SID = $5;
      $DEBUG and &Print("DEBUG: found SS SID=$SS_SID, Asset=$1, SsSpec=$2-$3\n");
    }
  }
  elsif ($module == $SS)
  {
    if ($line =~ /.* streamhandle[:=](.*?),/i)
    {
      if ($MCA_SHDL)
      {
        &Print("*** WARNING ***: I already found a streamhandle ($MCA_SHDL) for stream ID $SS_SID\n");
      }
      $MCA_SHDL = $1;
      $MCA_SHDL =~ tr/a-f/A-F/;
      $DEBUG and &Print("DEBUG: found MCA streamhandle=$MCA_SHDL\n");
    }
  }
  elsif ($module == $MCACLI)
  {
  }
  elsif ($module == $MOD)
  {
  }
}

sub MungeLogFiles
{
  my ($listRef) = @_;
  my @tmp = ();
  my $initialCount = $#$listRef + 1;
  foreach (@$listRef)
  {
    if (/-$/)
    {
      my $i = 1;
      while (1)
      {
        my $fileName = sprintf '%s%03d.log', $_, $i++;
        if (-f $fileName)
        {
          $VERBOSE and print "file `$fileName' added to list\n";
          push @tmp, $fileName;
        }
        else
        {
          $VERBOSE and print "file `$fileName' not found - quitting\n";
          last;
        }
      }
    }
    elsif (-f $_)
    {
      push @tmp, $_;
    }
    else
    {
      $VERBOSE and print "Removing log file `$_' from consideration because it does not exist\n";
    }
  }

  @$listRef = @tmp;
  my $finalCount = $#tmp + 1;
  $VERBOSE and print "init=$initialCount, final=$finalCount\n";
}

sub ReadConfigFile
{
  my ($file) = @_;
  local *FILE;
  open FILE, $file or
    die "*** ERROR: unable to open config $file: $!\n";

  $DEBUG and print "CFG: Reading configuration from $file...\n";

  my $module = -1;

  while (<FILE>)
  {
    chomp;
    next if /^\s*$/;
    next if /^\s*#/;
    if (/\s*\[HAMMER\]\s*/i) {
      print "CFG: FOUND SECTION [HAMMER]\n";
      $module = $HAMMER; next;
    } elsif (/^\s*\[SRM\]\s*/i) {
      print "CFG: FOUND SECTION [SRM]\n";
      $module = $SRM; next;
    } elsif (/^\s*\[DMN\]\s*/i) {            #Jeff 11/14/02
      print "CFG: FOUND SECTION [DMN]\n";
      $module = $DMN; next;
    } elsif (/^\s*\[CM\]\s*/i) {
      print "CFG: FOUND SECTION [CM]\n";
      $module = $CM     and next;
    } elsif (/^\s*\[SSAPI\]\s*/i) {
      print "CFG: FOUND SECTION [SSAPI]\n";
      $module = $SSAPI  and next;
    } elsif (/^\s*\[SS\]\s*/i) {
      print "CFG: FOUND SECTION [SS]\n";
      $module = $SS; next;
    } elsif (/^\s*\[MOD\]\s*/i) {
      print "CFG: FOUND SECTION [MOD]\n";
      $module = $MOD    and next;
    } elsif (/^\s*\[MCACLIENT\]\s*/i) {
      print "CFG: FOUND SECTION [MCACLIENT]\n";
      $module = $MCACLI and next;
    } elsif (/^\s*\[MCASERVER\]\s*/i) {
      print "CFG: FOUND SECTION [MCASERVER]\n";
      $module = $MCASER and next;
    }
    $DEBUG and print "CFG: ADDING [$MODULE_NAME[$module]]: $_\n";
    push @HMRLOGS,    $_ if $module == $HAMMER;
    push @SRMLOGS,    $_ if $module == $SRM;
    push @DMNLOGS,    $_ if $module == $DMN;
    push @CMLOGS,     $_ if $module == $CM;
    push @SSAPILOGS,  $_ if $module == $SSAPI;
    push @SSLOGS,     $_ if $module == $SS;
    push @MODLOGS,    $_ if $module == $MOD;
    push @MCACLILOGS, $_ if $module == $MCACLI;
    push @MCASERLOGS, $_ if $module == $MCASER;
  }
  close FILE;
}

sub show_header
{
   (my $version = '$Revision: 1.1 $') =~ s/.+(\d+\.\d+).*/$1/g;
   print <<EOF;

Session Dumper - Version $version
   Copyright(C) 2001 by SeaChange International, Inc.

EOF
}

sub show_usage
{
   &show_header();
   (my $progName = $0) =~ s/\//\\/g;
   $progName =~ s/.*\\//g;
   print <<EOF;
This program extracts from each components log file, data associated with the
session specified on the command line.  The session ID must can be either a MAC
address, or a MAC address/session ID number pair (eg:
01:02:03:04:05:06/0000012345).

Usage: $progName [options] <session ID>

Where options are:
   -h,--help          - show this usage information
   
EOF
}

__END__
:END
