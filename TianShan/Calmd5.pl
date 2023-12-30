use lib '.';
use Getopt::Long;
use Win32;
use Cwd;
use env;
use File::Path;
use Win32::TieRegistry;

&main();
sub main()
{
	&GetOptions(
				'help'           =>	\$cal_help,
                'filename=s'	=>	\$filename,
                'outputcsv=s'	=>	\$out_csv,
                'logfile=s'	=>	\$LOGFILE,
               );
	if((!$filename)&&(!$cal_help))
	{
		printf "please give the filename for calculate MD5\n";
		exit 0;
	}
	usage() if($cal_help);
	$out_csv = "MD5Data.csv" if(!$out_csv);
	$LOGFILE = "calmd5.log" if(!$LOGFILE);
	unlink "md5.txt";
	unlink "msg.txt";
	unlink "$out_csv";
	unlink "$LOGFILE";
	open FILENAME, "< $filename" or die ("fail to open $filename");
	&log("********Start to calucate md5 for all files record in $filename********");
	open OUTPUT, ">> $out_csv";
	printf OUTPUT "\"filename\",\"md5\"\n";
	filename_next_line: while ($line = <FILENAME>)
	{
		$line = trim($line);
		&log("calculate $line md5");
		my $cmd = "cp -v -s $line -d md5 >md5.txt 2>>msg.txt";
		$result = system ($cmd);
		if(!$result)
		{
			open MD5, "< md5.txt";
			$md5 = <MD5>;
			close MD5;
			&log("success to calculate [$line] md5, the value is $md5");
			printf OUTPUT "\"$line\",\"$md5\"\n";
		}
		else
		{
			&log("fail to open $line, calucate [$line] md5 failed.");
		}
		next filename_next_line;	
	}
	close FILENAME;
	close OUTPUT;
	&log("********Complete to calucate md5 for all files record in $filename********");
	
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

sub usage
{
	print <<EOF;
Usage:
    calmd5 <options>
    calmd5 is a simple Perl program that can calculate file md5
options
    --filename=<filename>
        specify the filename which record all content want to calculate md5. One content list on a separate line
    --outputcsv=<out_csv>
    	specify output .csv file name, default value is \"MD5Data.csv\"
    --logfile=<logfile>
	specify logfile name for record the msg, default value is \"calmd5.log\"
    --help
        display this screen

EOF

exit 0;
}