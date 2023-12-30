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
				'help'           =>	\$cov_help,
                                'IDSServer=s'	=>	\$ids_serverip,
                                'trickfile=s'	=>	\$trickfile,
                                'logfile=s'	=>	\$cov_logfile,
                                'PID=s'		=>	\$cov_pid,
                                'PAIDPrefix=s'	=>	\$cov_paid,
               );
              
	
	die ("You must give IDS Server IP like \"--IDSServer=<IPAddress>\", for more detail please use \"--help\"") if(!$ids_serverip)&&(!$cov_help);
	usage() if($cov_help);
	$trickfile = "ff,fr,vvx" if(!$trickfile);
	my @trickfile;
	my %AssetSize;
	$LOGFILE;
	$LOGFILE = fixPathToOS($cov_logfile) if $cov_logfile;
	$LOGFILE = fixPathToOS("CovAxiomToNGOD.log") if (!$cov_logfile);
	unlink "$LOGFILE";
	unlink "AEUID.txt";
	$PID = $cov_pid if($cov_pid);
	$PAID_PREFIX = $cov_paid if($cov_paid);
	$PID = "seachange.com" if(!$cov_pid);
	$PAID_PREFIX = "SEAC" if(!$cov_paid);
	&log("********** Start to Get Asset list from MediaCluster **********");
	printf "Start to Get Asset list from MediaCluster\n" if($LOGFILE);
	if($trickfile)
	{
		@trickfile = split (/,/,$trickfile);
		$trickfile_count = scalar (@trickfile);	
	}
	
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
				push (@total_files, trim($file));
				$AssetEUID = trim($file) ;
				system ("echo $AssetEUID>>AEUID.txt");
				$ress = $trickfile_count;
				$caulsize = 0;
				$a=0;
				$caulsize = GetFileSize($_);
			}
			elsif($file =~ (/$AssetEUID/))
			{
				$size = GetFileSize($_);
				$ress--;
				$caulsize = $caulsize + $size;
				$AssetSize->{$AssetEUID}=$caulsize if(!$ress);
			}
			next ver_h_next_line;
		}
	}
	close FILES;
	my $AEUID;
	unlink "directory.txt";
	unlink "ae.csv";
	unlink "asset.csv";
	unlink "error.txt";
	unlink "renfilenames.bat";
	unlink "rollbackfilenames.bat";
	unlink "newcontent.txt";
	unlink "oldcontent.txt";
	system ("echo \"AEUID\",\"ProviderId\",\"ProviderAssetId\",\"FileSize\",\"BitRate\",\"ActualDuration\",\"SupportFileSize\",\"MD5Checksum\">>asset.csv");
	open OLDCONTENT, ">>oldcontent.txt";
	my $ver_errormsg;
	&log("Start check asset metadata from IDSServer");
	printf "Start check asset metadata from IDSServer\n" if($LOGFILE);
	my $cmd = "MD4File.exe -i $ids_serverip -f AEUID.txt -l ProviderId,ProviderAssetId,FileSize,BitRate,PlayTime -d $PID -p $PAID_PREFIX >>ae.csv 2>>error.txt";
	system ($cmd);
	&log("complete check asset metadata from IDSServer");
	printf "complete check asset metadata from IDSServer\n" if($LOGFILE);
	open ERRORMSG, "<error.txt";
	errormsg_next_line: while ($line = <ERRORMSG>)
	{
		$_=$line;
		if(/^failed[\s]+to[\s]+bind[\s]+with[\s]+Ids:[\s]+.*?/)
		{
			printf "Failed to connect IDSServer, please confirm IDS service is running\n";
			&log("Failed to connect IDSServer, please confirm IDS service is running");
			close OLDCONTENT;
			unlink "ae.csv";
			unlink "asset.csv";
			unlink "oldcontent.txt";
			exit 0;
		}
		&log("$line");
		$ver_errormsg=1;
		next errormsg_next_line;
	}
	close ERRORMSG;
	&log("Start add Asset information to asset.csv");
	printf "Start add Asset information to asset.csv\n" if($LOGFILE);
	open ASSET_CSV, "ae.csv" or die ("fail to open ae.csv");
	open SCP_CONVERT, ">>renfilenames.bat";
	open SCP_REVERT, ">>rollbackfilenames.bat";
	open NEWCONTENT, ">>newcontent.txt";
	open asset, ">>asset.csv";
	Asset_next_line: while ($line = <ASSET_CSV>)
	{
		$lines = $line;
		my $newname;
		my $oldname;
		@ASSET_Info = split (/\"/,$line);
		$oldname = $ASSET_Info[1];
		$newname = "$ASSET_Info[5]"."_$ASSET_Info[3]" if(($ASSET_Info[3])&&($ASSET_Info[5])&&!($ASSET_Info[3] =~ /^ProviderId/i));
		if($newname)
		{
			
			printf SCP_CONVERT "echo scp rename $oldname $newname >>renfilenames.log\n";
			printf SCP_CONVERT "scp rename $oldname $newname >>renfilenames.log\n";
			printf SCP_REVERT "echo scp rename $newname $oldname >>rollback.log\n";
			printf SCP_REVERT "scp rename $newname $oldname >>rollback.log\n";
			printf NEWCONTENT "$newname\n";
			if(@trickfile)
			{
				foreach $postfix (@trickfile)
				{
					$oldname_postfix = "$oldname".".$postfix";
					$newname_postfix = "$newname".".$postfix";
					printf SCP_CONVERT "echo scp rename $oldname_postfix $newname_postfix >>renfilenames.log\n";
					printf SCP_CONVERT "scp rename $oldname_postfix $newname_postfix >>renfilenames.log\n";
					printf SCP_REVERT "echo scp rename $newname_postfix $oldname_postfix >>rollback.log\n";
					printf SCP_REVERT "scp rename $newname_postfix $oldname_postfix >>rollback.log\n";
				}
			}
			
		}
		printf OLDCONTENT "$oldname\n";
		$lines = trim($lines).",\"$AssetSize->{$oldname}\",\"\"";
		printf asset "$lines\n";
		&log("$oldname SupportFileSize is $AssetSize->{$oldname}");
		next Asset_next_line;
	}
	close asset;
	close SCP_CONVERT;
	close SCP_REVERT;
	close NEWCONTENT;
	close OLDCONTENT;
	close ASSET_CSV;
	unlink "ae.csv";
	&log("Complete add Asset information to asset.csv");
	printf "Complete add Asset information to asset.csv and batch job file" if($LOGFILE);
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
    ConvAxiomToTS <options>
    ConvAxiomToTS is a simple Perl program that help to convert Axiom Asset To NGOD
options
    --IDSServer=<IPAddr>
        specify the IDS Server IP Address
    --trickfile=<trickfile>
    	specify trickfiles extension for rename, split with comma, like \"--trickfile=ff,fr,vvx\", default value is \"ff,fr,vvx\"
    --PID=<defaultPID>
    	specify the default PID for element when it is empty, default value is \"seachange.com\"
    --PAIDPrefix=<PAIDPrefix>
	specify the prefix used to generate PAID for element when it is empty, default value is \"SEAC\"
    --logfile=<logfile>
	specify logfile name for record the msg, default value is \"CovAxiomToNGOD.log\"
    --help
        display this screen

EOF

exit 0;
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

sub GetFileSize($)
{
	$profile = $_;
	@FileSizes = split(/ /,$profile);
	$filesize=0;
	$ver_c = 0;
	$ver_f = scalar(@FileSizes);
	while ($ver_c<$ver_f)
	{
		if (($FileSizes[$ver_c] eq "AM")||($FileSizes[$ver_c] eq "PM"))
		{
			$ver_c= $ver_c+1;
			while($ver_c<$ver_f)
			{
				if($FileSizes[$ver_c])
				{
					$sizes = trim($FileSizes[$ver_c]);
					return ($sizes) if(!($sizes =~ (/,/)));
					@size = split (/,/, $sizes);
					$count = scalar(@size);
					$t=0;
					while($t<$count)
					{
						if($count == 1)
						{
							$filesize = $size[0];
						}
						else
						{
							$files = $size[$t]*(1000 ** ($count-$t-1));
							$filesize = $filesize+$files;
							$t++;
						}
					}
					return ($filesize);
				}
				$ver_c++;
			}
		}
		$ver_c++;
	}	
}
