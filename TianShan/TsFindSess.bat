@rem = '-*- Perl -*-';
@rem = '
@echo off
perl -S %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
goto EndOfPerl
';
# -----------------------------------------------------------------
# File Name:     TsFindSess.bat
# Author:        Interactive ZQ
# Security:      Tech Mgm
# Description:   This program is to collect the related log of a TianShan session
# Modification Log:
# -----------------------------------------------------------
# 2009-10-10  0.1 Hui.Shao  Create
# 2009-10-13  0.2 Mei.Zhang add Chod and FilterStreamSmithSess
# -----------------------------------------------------------------

use Getopt::Long;
use Win32;
use File::Copy;
use Cwd;
use Sys::Hostname;


# -----------------
# global declares
# -----------------
my ($SESS_Weiwoo, $SESS_Rtsp, $SESS_Purchase, $SESS_Stream, $PathTicket, $StreamType, $AppType,
    $BOX_Weiwoo, $BOX_SS, $BOX_APP, $BOX_CRGW, $WKDIR, $LOGDIR, $Time_Start, $Time_End);

my $LOGFILE;

use Env qw(TEMP);

#$WKDIR = $TEMP;
$WKDIR = "c:/TianShan";
$LOGDIR = "c:/TianShan/logs";
$HOSTNAME = hostname;
$ZIP = "c:/TianShan/utils/zip.exe";

require "tsfind_pro.pl";

my @COLLECTED_FILES;
# ------------------
# main procedure 
# ------------------

#read arguments
if ($ARGV[0])
{
	&GetOptions(
		'wkdir=s'    =>	\$opt_wkdir,
		'rtsp=s'     =>	\$opt_rtspSess,
		'sess=s'    => 	\$opt_sess,
                'logsource=s' =>   \$opt_logsource,
	        'h' =>     	\$opt_help,
                'help'   => 	\$opt_help,
	        'onsite' =>   	\$opt_onsite,
	        'archive'	=>	\$opt_archive,
	        'nss=s'		=>	\$opt_nss,
	);
	
	$WKDIR = fixPathToOS($opt_wkdir) 	if $opt_wkdir;
	$SESS_Rtsp = $opt_rtspSess 		if $opt_rtspSess;
	$SESS_Weiwoo = $opt_sess		if $opt_sess;
	$LOGDIR = $opt_logsource		if $opt_logsource;
	$SESS_Stream = $opt_nss 			if $opt_nss;
}
		
usage() if $opt_help;

&log("********** TsFindSess starts **********");

open HOSTFILE, "<C:\\Windows\\system32\\drivers\\etc\\hosts" or die ("fail to open hosts");
@lines = <HOSTFILE>;
close HOSTFILE;

my %hash="";

hosts: foreach $line (@lines)
{
	@hosts="";
	@hosts=split (/ /,$line);
	$hosts[0] = trim($hosts[0]);
	$hosts[1] = trim($hosts[1]);
	$hash{$hosts[1]} = $hosts[0];
}

FilterNSSSess () if SESS_Nss;

exit;

RtspSessToWeiwooSess($SESS_Rtsp) if $SESS_Rtsp and !$SESS_Weiwoo;

FilterWeiwooSess($SESS_Weiwoo);
FilterTianShanRtspSess($SESS_Rtsp);
FilterPathTicket();
FilterAppPurchase();
FilterStreamSess();

PackResults();

&log("\n********** TsFindSess ends **********\n");

exit 0;

# ------------------------------------------------------
# usage screen
# ------------------------------------------------------
sub usage
{
   print <<EOF;
TsFindSess, 2009-2010, ZQ Interactive, SeaChange Intl

Usage: ZQAutoBuild <options>
TsFindSess is a simple Perl program that collect the log of a specific TianShan session

options
		'wkdir=s'    =>	\$opt_wkdir,
		'rtsp=s'     =>	\$opt_rtspSess,
		'sess=s'    => 	\$opt_sess,
                'logsource=s' =>   \$opt_logdir,
	        'h' =>     	\$opt_help,
                'help'   => 	\$opt_help,
	        'onsite' =>   	\$opt_onsite,

    --wkdir=<path>
        to specify the working path of the program, some temp file would be saved
    --rtsp=<RtspSessionId>
        to specify a RTSP Session Id to search
    --sess=<WeiwooSessId>
        to specify a Weiwoo Session Id to search
    --onsite=<onsite>
        site mode
    --logsource=<path>
        to specify the source path of the TianShan log files

    --h or --help
        display this screen

EOF

exit 0;
}


# ------------------------------------------------------
# build a project based its profile
# paramters: profile
# ------------------------------------------------------
sub RtspSessToWeiwooSess
{
	my $rtspSess = shift;
	&log("looking up RtspSess[$rtspSess]");
	
	my $outputfile = fixPathToOS($WKDIR . "/". $HOSTNAME. "_ssm_" . $rtspSess . ".log");
	unlink $outputfile;
	
	FilterTianShanRtspSess($rtspSess);
	Die("failed to find weiwoo session for RtspSess[$rtspSess]") if !$SESS_Weiwoo;
	&log("found WeiwooSess[$SESS_Weiwoo] by RtspSess[$rtspSess]");
}
	
sub FilterTianShanRtspSess
{
	my $rtspSess = shift;
	return if !$rtspSess;
	
	my $outputfile = fixPathToOS($WKDIR . "/". $HOSTNAME. "_ssm_" . $rtspSess . ".log");
	my @ofilestat = stat($outputfile);
	return if $ofilestat[7] > 100;

	&log("filtering SSM by RtspSess[$rtspSess]");
	
	my $srcLogs;
	
	if($opt_archive)
	{
		$rtspserver=$server{RTSPPROXY};
		$srcLogs = $LOGDIR .'/'.$rtspserver. '/ssm_TianShan_S1*.log';
	}
	else
	{
		$srcLogs = $LOGDIR . '/ssm_TianShan_S1*.log';
	}
	printf "srclogs=$srcLogs\n";
	
	#my @files = grep { -f and -T } glob fixPathToOS($LOGDIR . '/ssm_TianShan_S1*.log');
	my @files = grep { -f and -T } glob fixPathToOS($srcLogs);
	my @foundlines=("");

	loc_nextssmfile: foreach $file (@files)
	{
		open FILE, "<$file" or next loc_nextssmfile;
		my @lines = grep /$rtspSess/i, <FILE>;
		close FILE;
		
		loc_nextssmline: foreach $line (@lines)
		{
			#template to find WeiwooSess: 09-30 12:06:37.046 [    INFO ] [HandleSetup/345     | 00000BE0]  Req(02518B20)Sess(550664192)Seq(1)Mtd(ContentHandler:SETUP) weiwoo session(6KAiLMGRZGEIvpsm9Q3GOZ) created , and proxy is [Session/6KAiLMGRZGEIvpsm9Q3GOZ -t:tcp -h 10.50.12.25 -p 10100 -t 5000]
			if ($line =~ /[^\[]*\[    INFO \].*weiwoo session\((.*)\) created.*\[.*[\s]+-h[\s]*([^\s]*).*/i)
			{
				$SESS_Weiwoo = $1;
				$BOX_Weiwoo = $2;
				next loc_nextssmline;
			}
		}
		
		splice(@foundlines, $#foundlines, 0, @lines);
	}
	
	@foundlines = sort @foundlines;
	open FILE, ">" . $outputfile or Die("can not open $outputfile to write");
	
	my $time_start = substr($foundlines[1], 0, 18);
	my $time_end = substr($foundlines[$#foundlines], 0, 18);
	
	print FILE @foundlines;
	close FILE;
	
	push(@COLLECTED_FILES, $outputfile);
	
	$Time_Start = $time_start if !$Time_start or $Time_start > $time_start;
	$Time_End = $time_end if !$Time_End or $Time_End < $time_end;
	&log("  ->found session lifetime: $Time_Start ~ $Time_End");

	@foundlines=("");
	&log("filtering RtspProxy log by RtspSess[$rtspSess]");
	@files = grep { -f and -T } glob fixPathToOS($LOGDIR . '/RtspProxy.*log');
        my %sockets =();
        my ($teststart, $testend) = (substr($Time_Start, 0, 13)."0.000", substr($Time_End, 0, 13)."9.999");
		
loc_nextrtspfile: foreach $file (@files)
	{
		open FILE, "<$file" or next loc_nextrtspfile;
		
		loc_nextrtspline: while ($line = <FILE>)
		{
			next loc_nextrtspline if ($line lt $teststart or $line gt $testend);
                        push (@foundlines, $line);
			
			#template to find socket: 09-30 13:44:19.640 [    INFO ] Request processed: session[762935299] seq[14] verb[PAUSE] duration=[859/859]msec  startline [RTSP/1.0 200 OK] socket(0000006c 192.168.81.103:4064) 
			if ($line =~ /[^\[]*\[    INFO \].*Request processed: session\[$rtspSess\].* socket\(([^\s]+) ([^\)]+)\).*/i)
			{
                            $sockets{$1} = $2;
                            next loc_nextrtspline;
			}
		}
		close FILE;
	}
        
        my $match_exp = $rtspSess;
        foreach $s (keys %sockets)
        {
            $match_exp = $match_exp . "|" . $s;
        }

	@foundlines = grep /$match_exp/i, @foundlines;
	&log("  ->found session and its sockets: $match_exp");
	@foundlines = sort @foundlines;

	$outputfile = fixPathToOS($WKDIR . "/". $HOSTNAME. "_rtsp_" . $rtspSess . ".log");
	open FILE, ">" . $outputfile or Die("can not open $outputfile to write");
	print FILE @foundlines;
	close FILE;
	push(@COLLECTED_FILES, $outputfile);
}

sub FilterWeiwooSess
{
	my $sess = shift;
	return if !$sess;
	
	my $outputfile = fixPathToOS($WKDIR . "/". $HOSTNAME. "_Weiwoo_" . $sess . ".log");

	&log("filtering Weiwoo log by Sess[$sess]");
	
	my $srcLogs;
	
	if($opt_archive)
	{
		if($BOX_Weiwoo)
		{
			$weiwoo=$server{$BOX_Weiwoo};
		}
		else
		{
			$weiwoo=$server{WEIWOO};
		}
		print "box_weiwoo=$BOX_Weiwoo\n";
		print "weiwoo=$weiwoo\n";
		$srcLogs = $LOGDIR .'/'.$weiwoo. '/Weiwoo*.log';
	}
	else
	{
		$srcLogs = $LOGDIR . '/Weiwoo*.log';
	}
	
	my @files = ("");
	
	
	if ($opt_onsite)
	{
		$BOX_Weiwoo = $hash{$BOX_Weiwoo}if ($hash{$BOX_Weiwoo});
		$srcLogs = "//". "$BOX_Weiwoo" . "/" . "$LOGDIR";
		
		$srcLogs =~ s/:/\$/g;
		$srcLogs =~ s/\//\\/g;
		
		
		#@files = grep { -f and -T } glob fixPathToOS($srcLogs);
		
		
		opendir (DIR, $srcLogs) or die "can't open the directory!";
		my @dir = readdir DIR;
		my $dir;
		foreach $dir (@dir)
		{
			if ($dir =~ /^Weiwoo/i)
			{
				$dir = $srcLogs . '/' . $dir;
				push @files, $dir;
			}
		}
	}
	else
	{
		@files = grep { -f and -T } glob fixPathToOS($srcLogs);
	}
	

	#my @files = grep { -f and -T } glob fixPathToOS($LOGDIR . '/Weiwoo*.log');
	my @found_lines=("");

	loc_nextwwfile: foreach $file (@files)
	{
		open FILE, "<$file" or next loc_nextwwfile;
		my @lines = grep /$sess/i, <FILE>;
		close FILE;
		
		loc_nextwwline: foreach $line (@lines)
		{

			#template to find stream and pathticket: 09-30 13:43:50.546 [    INFO ] SessStateInService sess[660u0PnfisK2FxRYg45Y57:Provisioned(1)] stream instance with ticket[cf7480dc-40e7-4f3e-9b88-d5438d180124] created: NSS/cf7480dc-40e7-4f3e-9b88-d5438d180124 -t:tcp -h 10.50.12.25 -p 10800
			#template for Playlist: 10-01 00:00:42.734 [    INFO ] SessStateInService sess[6LRBvoRwcMdB8Qwm5Vpqjg:Provisioned(1)] stream instance with ticket[374769d8-9c04-4e48-9832-0accd8b39822] created: PlayList/03eb5721-486b-46c3-b08f-8cf437f70658 -t:tcp -h 10.3.0.250 -p 10700 -t 10000
			if ($line =~ /[^\[]*\[    INFO \].*stream instance with ticket\[(.*)\] created: ([^\/]*)\/([^\s]+).*[\s]+-h[\s]+([^\s]*).*/i)
			{
				$PathTicket = $1;
				$StreamType = $2;
				$SESS_Stream = $3;
				$BOX_SS = $4;
				next loc_nextwwline;
			}
			
			#template to find stream and pathticket: 09-30 13:43:50.390 [    INFO ] SessStateProvisioned sess[660u0PnfisK2FxRYg45Y57:NotProvisioned(0)] resolve purchase OK, at [ModPur/0b93ca31-5ced-4112-9a4a-372563eadf60 -t:tcp -h 10.50.12.25 -p 11100 -t 10000]
			if ($line =~ /[^\[]*\[    INFO \].*resolve purchase OK, at \[([^\/]*)\/([^\s]+).*[\s]+-h[\s]+([^\s]*).*/i)
			{
				$AppType = $1;
				$SESS_Purchase = $2;
				$BOX_APP = $3;
				next loc_nextwwline;
			}

			#template to find stream and pathticket: 09-30 13:43:50.437 [    INFO ] SessStateInService Session[660u0PnfisK2FxRYg45Y57] connect purchase proxy [ModPur/0b93ca31-5ced-4112-9a4a-372563eadf60 -t:tcp -h 10.50.12.25 -p 11100 -t 10000] successfully
			#10-01 00:12:52.718 [    INFO ] SessStateInService Session[5tgY5faV8ZBH71CbcHlLbX] connect purchase proxy [ChannelPurchase/41a4d474-b7bd-425d-9b22-4dfce5d91347 -t:tcp -h 10.50.5.173 -p 10900 -t 10000] successfully
			if ($line =~ /[^\[]*\[    INFO \].*connect purchase proxy \[([^\/]*)\/([^\s]+).*[\s]+-h[\s]+([^\s]*).*/i)
			{
				$AppType = $1;
				$SESS_Purchase = $2;
				$BOX_APP = $3;
				next loc_nextwwline;
			}
		}
		
		splice(@found_lines, $#found_lines, 0, @lines);
	}
	
	&log("  ->found Purchase[$AppType/$SESS_Purchase] on server[$BOX_APP], rendered as Stream[$StreamType/$SESS_Stream] on server[$BOX_SS] w/ PathTicket[$PathTicket]");
	@found_lines = sort @found_lines;
	my $time_start = substr($found_lines[1], 0, 18);
	my $time_end = substr($found_lines[$#found_lines], 0, 18);
	
	$Time_Start = $time_start if !$Time_start or $Time_start > $time_start;
	$Time_End = $time_end if !$Time_End or $Time_End < $time_end;

	open FILE, ">" . $outputfile or Die("can not open $outputfile to write");
	print FILE @found_lines;
	close FILE;
	push(@COLLECTED_FILES, $outputfile);
}

sub FilterPathTicket
{
	my $sess = $SESS_Weiwoo;
	return if !$sess;
	
	my $outputfile = fixPathToOS($WKDIR . "/". $HOSTNAME. "_Path_" . $sess . ".log");

	&log("filtering Path log by WeiwooSess[$sess]");
	my $srcLogs;
	
	if($opt_archive)
	{
		$weiwoo=$server{WEIWOO};
		$srcLogs = $LOGDIR .'/'.$weiwoo. '/Path*.log';
	}
	else
	{
		$srcLogs = $LOGDIR . '/Path*.log';
	}
	
	my @files = ("");
	
	
	if ($opt_onsite)
	{
		$BOX_Weiwoo = $hash{$BOX_Weiwoo }if ($hash{$BOX_Weiwoo});
		$srcLogs = "//". "$BOX_Weiwoo" . "/" . "$LOGDIR";
		#$srcLogs = "\\\\". "10.50.5.135" . "\\" . "$LOGDIR";
		
		$srcLogs =~ s/:/\$/g;
		$srcLogs =~ s/\//\\/g;
		
		
		#@files = grep { -f and -T } glob fixPathToOS($srcLogs);
		
		
		opendir (DIR, $srcLogs) or die "can't open the directory!";
		my @dir = readdir DIR;
		my $dir;
		foreach $dir (@dir)
		{
			if ($dir =~ /^Path/i)
			{
				$dir = $srcLogs . '/' . $dir;
				push @files, $dir;
			}
		}
	}
	else
	{
		@files = grep { -f and -T } glob fixPathToOS($srcLogs);
	}

	#my @files = grep { -f and -T } glob fixPathToOS($LOGDIR . '/Path*.log');
	my @found_lines=("");

	loc_nextpathfile: foreach $file (@files)
	{
		open FILE, "<$file" or next loc_nextpathfile;
		my @lines = grep /$sess/i, <FILE>;
		close FILE;
		
		splice(@found_lines, $#found_lines, 0, @lines);
	}
	
	@found_lines = sort @found_lines;
	open FILE, ">" . $outputfile or Die("can not open $outputfile to write");
	
	print FILE @found_lines;
	close FILE;
	push(@COLLECTED_FILES, $outputfile);

	&log("filtering PHO log by WeiwooSess[$sess]");
	$outputfile = fixPathToOS($WKDIR . "/". $HOSTNAME. "_PHO_" . $sess . ".log");
	
	#$srcLogs = $LOGDIR . '/Pho*.log';
	
	if($opt_archive)
	{
		$weiwoo=$server{WEIWOO};
		$srcLogs = $LOGDIR .'/'.$weiwoo. '/Pho*.log';
	}
	else
	{
		$srcLogs = $LOGDIR . '/Pho*.log';
	}
	
	@files = ("");
	
	
	if ($opt_onsite)
	{
		#$srcLogs = "\\\\". "10.50.5.135" . "\\" . "$LOGDIR";
		$BOX_Weiwoo = $hash{$BOX_Weiwoo}if ($hash{$BOX_Weiwoo});
		$srcLogs = "//". "$BOX_Weiwoo" . "/" . "$LOGDIR";
		
		
		$srcLogs =~ s/:/\$/g;
		$srcLogs =~ s/\//\\/g;
		
		
		#@files = grep { -f and -T } glob fixPathToOS($srcLogs);
		
		
		opendir (DIR, $srcLogs) or die "can't open the directory!";
		my @dir = readdir DIR;
		my $dir;
		foreach $dir (@dir)
		{
			if ($dir =~ /^Pho/i)
			{
				$dir = $srcLogs . '/' . $dir;
				push @files, $dir;
			}
		}
	}
	else
	{
		@files = grep { -f and -T } glob fixPathToOS($srcLogs);
	}
	

	#@files = grep { -f and -T } glob fixPathToOS($LOGDIR . '/PHO*.log');
	@found_lines=("");

	loc_nextphofile: foreach $file (@files)
	{
		open FILE, "<$file" or next loc_nextphofile;
		my @lines = grep /$sess/i, <FILE>;
		close FILE;
		
		splice(@found_lines, $#found_lines, 0, @lines);
	}
	
	@found_lines = sort @found_lines;
	open FILE, ">" . $outputfile or Die("can not open $outputfile to write");
	
	print FILE @found_lines;
	close FILE;
	push(@COLLECTED_FILES, $outputfile);
}

sub FilterModPurchase
{
	my $sess = $SESS_Purchase;
	$BOX_APP = $HOSTNAME if !$BOX_APP;
	return if !$sess;
	
	my $outputfile = fixPathToOS($WKDIR . "/". $BOX_APP. "_MOD_" . $sess . ".log");

	&log("filtering MOD by purchase[$sess]");
	
	my $srcLogs;
	
	if($opt_archive)
	{
		$modsvc=$server{$BOX_APP};
		print "mod=$modsvc\n";
		$srcLogs = $LOGDIR .'/'.$modsvc. '/MODSvc*.log';
	}
	else
	{
		$srcLogs = $LOGDIR . '/MODSvc*.log';
	}
	
	my @files = ("");
	
	
	if ($opt_onsite)
	{
		$BOX_APP = $hash{$BOX_APP}if ($hash{$BOX_APP});
		$srcLogs = "//". $BOX_APP. "/" . $LOGDIR;
		$srcLogs =~ s/:/\$/g;
		$srcLogs =~ s/\//\\/g;
		
		opendir (DIR, $srcLogs) or die "can't open the directory!";
		my @dir = readdir DIR;
		my $dir;
		foreach $dir (@dir)
		{
			if ($dir =~ /^Weiwoo/i)
			{
				$dir = $srcLogs . '/' . $dir;
				push @files, $dir;
			}
		}
	}
	else
	{
		@files = grep { -f and -T } glob fixPathToOS($srcLogs);
	}

	#my @files = grep { -f and -T } glob fixPathToOS($LOGDIR . '/MODSvc*.log');
	my @found_lines=("");

	loc_nextmodfile: foreach $file (@files)
	{
		open FILE, "<$file" or next loc_nextmodfile;
		my @lines = grep /$sess/i, <FILE>;
		close FILE;
		
		splice(@found_lines, $#found_lines, 0, @lines);
	}
	
	@found_lines = sort @found_lines;
	open FILE, ">" . $outputfile or Die("can not open $outputfile to write");
	
	print FILE @found_lines;
	close FILE;
	push(@COLLECTED_FILES, $outputfile);

	$outputfile = fixPathToOS($WKDIR . "/". $BOX_APP. "_MODAuth_" . $sess . ".log");
	
	#$srcLogs = $LOGDIR . '/MODPlugin*.log';
	
	if($opt_archive)
	{
		$weiwoo=$server{WEIWOO};
		$srcLogs = $LOGDIR .'/'.$MODSvc. '/MODPlugin*.log';
	}
	else
	{
		$srcLogs = $LOGDIR . '/MODPlugin*.log';
	}
	
	
	if ($opt_onsite)
	{
		$BOX_APP = $hash{$BOX_APP}if ($hash{$BOX_APP});
		$srcLogs = "//". $BOX_APP. "/" . $LOGDIR;
		$srcLogs =~ s/:/\$/g;
		$srcLogs =~ s/\//\\/g;
		
		opendir (DIR, $srcLogs) or die "can't open the directory!";
		my @dir = readdir DIR;
		my $dir;
		foreach $dir (@dir)
		{
			if ($dir =~ /^MODPlugin/i)
			{
				$dir = $srcLogs . '/' . $dir;
				push @files, $dir;
			}
		}
	}
	else
	{
		@files = grep { -f and -T } glob fixPathToOS($srcLogs);
	}
	
	@files = grep { -f and -T } glob fixPathToOS($LOGDIR . '/MODPlug*.log');
	@found_lines=("");

	loc_nextmodfile: foreach $file (@files)
	{
		open FILE, "<$file" or next loc_nextmodfile;
		my @lines = grep /$sess/i, <FILE>;
		close FILE;
		
		splice(@found_lines, $#found_lines, 0, @lines);
	}
	
	@found_lines = sort @found_lines;
	open FILE, ">" . $outputfile or Die("can not open $outputfile to write");
	
	print FILE @found_lines;
	close FILE;
	push(@COLLECTED_FILES, $outputfile);
}

sub FilterChodPurchase
{
	my $sess = $SESS_Purchase;
	$BOX_APP = $HOSTNAME if !$BOX_APP;
	return if !$sess;
	
	my $outputfile = fixPathToOS($WKDIR . "/". $BOX_APP. "_ChOD_" . $sess . ".log");

	&log("filtering COD by purchase[$sess]");

	my @files = grep { -f and -T } glob fixPathToOS($LOGDIR . '/ChOD*.log');
	my @found_lines=("");

	loc_nextmodfile: foreach $file (@files)
	{
		open FILE, "<$file" or next loc_nextmodfile;
		my @lines = grep /$sess/i, <FILE>;
		close FILE;
		
		splice(@found_lines, $#found_lines, 0, @lines);
	}
	
	@found_lines = sort @found_lines;
	open FILE, ">" . $outputfile or Die("can not open $outputfile to write");
	
	print FILE @found_lines;
	close FILE;
	push(@COLLECTED_FILES, $outputfile);
}

sub FilterStreamSess
{
	$_ = $StreamType;
	
	SWITCH:
	{
		#/NSS/ && do {
		/STREAM/ && do {
			FilterNSSSess();
			return;
		};
		/PlayList/ && do {
			FilterStreamSmithSess();
			return;
		};
	}
}

sub FilterAppPurchase
{
	$_ = $AppType;
	
	SWITCH:
	{
		/ModPur/ && do {
			FilterModPurchase();
			return;
		};
		/ChannelPurchase/ && do {
			FilterChodPurchase();
			return;
		};
	}
}

sub FilterNSSSess
{
	my $sess = $SESS_Stream;
	return if !$sess;

	#$BOX_SS = $HOSTNAME if !$BOX_SS;
	
	my $outputfile = fixPathToOS($WKDIR . "/". $BOX_SS. ".NSS_" . $sess . ".log");

	&log("filtering NSS by Sess[$sess]");
	#my $srcLogs = $LOGDIR . '/NSS*.log';
	
	my $srcLogs;
	
	if($opt_archive)
	{
		if($BOX_SS)
		{
			$NSS=$server{$BOX_SS};
		}
		else
		{
			$NSS=$server{NSS};
		}
		print "BOX_SS=$BOX_SS\n";
		print "NSS=$NSS\n";
		$srcLogs = $LOGDIR .'/'.$NSS. '/NSS*.log';
	}
	else
	{
		$srcLogs = $LOGDIR . '/NSS*.log';
	}
	
	
	my @files = ("");
	
	if ($opt_onsite)
	{
		$BOX_APP = $hash{$BOX_SS}if ($hash{$BOX_APP});
		$srcLogs = "//". $BOX_SS. "/" . $LOGDIR;
		$srcLogs =~ s/:/\$/g;
		$srcLogs =~ s/\//\\/g;
		
		opendir (DIR, $srcLogs) or die "can't open the directory!";
		my @dir = readdir DIR;
		my $dir;
		foreach $dir (@dir)
		{
			if ($dir =~ /^NSS/i)
			{
				$dir = $srcLogs . '/' . $dir;
				push @files, $dir;
			}
		}
	}
	else
	{
		@files = grep { -f and -T } glob fixPathToOS($srcLogs);
	}
	
	
	my @found_lines=("");
	my $NGOD_RTSPSess;

	loc_nextnssfile: foreach $file (@files)
	{
		open FILE, "<$file" or next loc_nextnssfile;
		my @lines = grep /$sess/i, <FILE>;
		close FILE;
		
		loc_nextnssline: foreach $line (@lines)
		{
			#template to find NSS RTSP Sess: 09-30 13:43:52.296 [   DEBUG ] [ngod_parse_thread/297     | 00000854]  parse(find session[cf7480dc-40e7-4f3e-9b88-d5438d180124:766801923])
			#template to find NSS RTSP Sess: 03-08 17:24:52.624 [    INFO ] [RTSPClient/1261    | 000009E4]  sendMessage() req[SETUP(123)@00000884] SETUP rtsp://10.80.66.47 RTSP/1.0..CSeq: 123..Content-Type: application/sdp..OnDemandSessionId: eec3dd76-98ef-4390-9a87-7b461f629f51..Require: com.comcast.ngod.r2.decimal_npts..SessionGroup: 80468.02..StreamControlProto: rtsp..Transport:  MP2T/DVBC/UDP;unicast;destination=10.80.72.15;client_port=274;bandwidth=8000000;client=;sop_group=CDNCD;sop_name=CDNCD..User-Agent: NSS/1.15..Volume: library..Content-Length: 161..Date: Thu, 08 Apr 2012 09:24:52 GMT....v=0..o=- eec3dd76-98ef-4390-9a87-7b461f629f51  IN IP4 0.0.0.0..s=..c=IN IP4 0.0.0.0..t=0 0..a=X-playlist-item: DOX 12022710594400001831 0- ..m=video 0 udp MP2T..
			#if ($line =~ /[^\[]*\[.*\].*parse\(find session\[[^:]*:([^\]]+)\].*/i)
			#{
			#	$NGOD_RTSPSess = $1;
			#	next loc_nextnssline;
			#}
			if ($line =~ /[^\[]*\[    INFO \].*SETUP rtsp:\/\/(.*) RTSP\/1.0.*OnDemandSessionId: ([^\s]*)..Require.*/i)
			{
				$BOX_OSTR = $1;
				$SESS_OSTR= $2;
				next loc_nextnssline;
			}

		}
			
		splice(@found_lines, $#found_lines, 0, @lines);
	}
	
	print "BOX_OSTR=$BOX_OSTR\n";
	print "SESS_OSTR=$SESS_OSTR\n";
	
	&log("  ->found NGOD_RTSPSess[$NGOD_RTSPSess]") if $NGOD_RTSPSess;
	@found_lines = sort @found_lines;
	open FILE, ">" . $outputfile or Die("can not open $outputfile to write");
	
	print FILE @found_lines;
	close FILE;
	push(@COLLECTED_FILES, $outputfile);
}

sub FilterStreamSmithSess
{
	my $sess = $SESS_Stream;
	return if !$sess;

	$BOX_SS = $HOSTNAME if !$BOX_SS;
	
	my $outputfile = fixPathToOS($WKDIR . "/". $BOX_SS. ".StreamSmith_" . $sess . ".log");

	&log("filtering StreamSmith by Sess[$sess]");
	my $srcLogs = $LOGDIR . '/StreamSmith*.log';
	my @files = ("");
	
	if ($opt_onsite)
	{
		$BOX_APP = $hash{$BOX_SS}if ($hash{$BOX_APP});
		$srcLogs = "//". $BOX_APP. "/" . $LOGDIR;
		$srcLogs =~ s/:/\$/g;
		$srcLogs =~ s/\//\\/g;
		
		opendir (DIR, $srcLogs) or die "can't open the directory!";
		my @dir = readdir DIR;
		my $dir;
		foreach $dir (@dir)
		{
			if ($dir =~ /^StreamSmith/i)
			{
				$dir = $srcLogs . '/' .$dir;
				push @files, $dir;
			}
		}
	}
	else
	{
		@files = grep { -f and -T } glob fixPathToOS($srcLogs);
	}
	

	my @found_lines=("");

	loc_nextnssfile: foreach $file (@files)
	{
		#$file =  $srcLogs . "/$file";
		open FILE, "<$file" or next loc_nextnssfile;
		#open FILE, "<$file" or die "aa";
		@lines = grep /$sess/i, <FILE>;
		close FILE;
		
		splice(@found_lines, $#found_lines, 0, @lines);
	}
	
	@found_lines = sort @found_lines;
	open FILE, ">" . $outputfile or Die("can not open $outputfile to write");
	
	print FILE @found_lines;
	close FILE;
	
	push(@COLLECTED_FILES, $outputfile);
}

sub PackResults
{
	my $files = join(" ", @COLLECTED_FILES);
	my $zipfile = ("");
	$zipfile = "W" . $SESS_Weiwoo . "_R" . $SESS_Rtsp . ".zip" if($SESS_Rtsp);
	$zipfile = "W" . $SESS_Weiwoo . ".zip" if(!$SESS_Rtsp);
        
	&log("packaging files [$files] into archive $zipfile");
	my $cmd = fixPathToOS($ZIP) . " " . $zipfile . " " . $files;
	unlink $zipfile;
	!system($cmd) or Die("failed to execute cmd: $cmd");
	&log("cleaning up temporary files");
	foreach $file (@COLLECTED_FILES) { unlink $file; }
}

# ------------------------------------------------------
# trim a string
# ------------------------------------------------------
sub trim
{
my @out = @_;
   for (@out)
	{
   	s/^\s+//;
      s/\s+$//;
      s/^\n+$//;
   }
   return wantarray ? @out : $out[0];
}

# ------------------------------------------------------
# fixPathToOS
# ------------------------------------------------------
sub fixPathToOS
{
	my @out = @_;
	for (@out)
	{
		s/\//\\/g;
	   }
	return wantarray ? @out : $out[0];
}

# ------------------------------------------------------
# fixPathToLogic
# ------------------------------------------------------
sub fixPathToLogic
{
	my @out = @_;
	for (@out)
	{
		s/\\/\//g;
	}
	return wantarray ? @out : $out[0];
}

sub Die
{
   &log(@_);
   $BUILD_BATCH{failed} = 1;
   emailReport() if $EMAILNOTIF;
   exit -1;
}

sub log
{
   my ($log) = @_;
   # Only print to the log file if we are initialized.
   my (@lines) = split(/\n/,$log);
   my (@lt) = localtime;
   my $time = sprintf "%d\/%02d\/%02d %02d\:%02d\:%02d",
      $lt[5]+1900, $lt[4]+1, $lt[3], $lt[2], $lt[1], $lt[0];
   my $succ = open LOG, ">> $LOGFILE";
   foreach $line (@lines)
   {
      print LOG "$time  $line\n" if $succ;
      print "$line\n" if !$succ;
   }
   close LOG if $succ;
}

endprog:
__END__

:EndOfPerl