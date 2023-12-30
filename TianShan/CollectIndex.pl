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
				'help'           =>	\$col_help,
                                'trickfile=s'	=>	\$trickfile,
                                'logfile=s'	=>	\$col_logfile,
                                'filename=s'		=>	\$col_filename,
               );
              
	
	usage() if($col_help);
	$trickfile = "ff,fr,vvx" if(!$trickfile);
	my @trickfile;
	my %AssetSize;
	$LOGFILE;
	$filename = $col_filename if($col_filename);
	$filename = "index.txt" if(!$col_filename);
	$LOGFILE = fixPathToOS($col_logfile) if $col_logfile;
	$LOGFILE = fixPathToOS("CollectIndex.log") if (!$col_logfile);
	unlink "$LOGFILE";
	unlink "$filename";
	&log("********** Start to Get Asset list from MediaCluster **********");
	printf "Start to Get Asset list from MediaCluster\n" if($LOGFILE);
	
	my $cmd = "scp dir >directory.txt";
	$result = system ("$cmd");
	if($result)
	{
		&log("\nFail to get Asset Element list from MediaCluster");
		die("fail to get asset name\n");
	}
	open FILES, "< directory.txt" or Die("failed to read directory.txt");
	my $line;
	my @total_files;
	&log("Check Asset information from list");
	printf "Check Asset information from list\n" if ($LOGFILE);
	ver_h_next_line: while ($line = <FILES>)
	{
		$_=trim($line);
		
		@full_info = split (/ /,$_);
		$full_file = $full_info[scalar(@full_info)-1];
		@files = split (/\\/,$full_file);
		$file = $files[scalar(@files)-1];
		if(!$file||(/^[\s]*Total[\s]+Files:[\s]+.*?/)||(/^[\s]*Total[\s]+Bytes:[\s]+.*?/)||(/^[\s]*Free[\s]+Space:[\s]+.*?/)||(/^[\s]*Free[\s]+Space:[\s]+.*?/))
		{
			next ver_h_next_line;
		}
		elsif((trim($file))&&!(trim($file) =~ /^files/i)&&!(trim($file) =~ /^GB\)/i)&&!(trim($file) =~ /^TB\)/i))
		{
			if(!($file =~ (/[.]/)))
			{
				$AssetEUID = trim($file) ;
				&log("Start to collect data for $AssetEUID");
				$cmd = "echo======== trickindex $AssetEUID========>>$filename";
				system ($cmd);
				$cmd = "trickindex $AssetEUID>>$filename";
				$result = system ($cmd);
				&log("Succeed to collect data for $AssetEUID") if(!$result);
			}
			next ver_h_next_line;
		}
	}
	close FILES;
	&log("Complete collect Asset information");
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

sub usage()
{
    print <<EOF;
Usage:
    CollectIndex <options>
    CollectIndex is a simple Perl program that help to collect index
options
    --filename=<filename>
    	output filename for record asset index data, default value is \"index.txt\"
    --logfile=<logfile>
	specify logfile name for record the msg, default value is \"CollectIndex.log\"
    --help
        display this screen

EOF

exit 0;
}

sub fixPathToOS
{
	my @out = @_;
	for (@out)
	{
		s/\//\\/g;
	   }
	return wantarray ? @out : $out[0];
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