use lib '.';
use integer;
use Getopt::Long;
use Win32;
use Cwd;
use env;
use File::Path;
use Win32::TieRegistry;

my $LOGFILE = "ImportHammer.log";
my $RequestPath = "c:\\request";
my $ECHO = 1;
my $interval = 60;
my $ContentName;
my $TestFile;
my $Bitrate = 3750000;
my $LoopCount = 1;
my $count = 1;
my $type = "MPEG2TS";
my $startTime;
my $endTime;
my $endpoint;
my $volume = "\$";
my $help;
my $nPVR;

&main();

sub usage()
{
      print <<EOF;
Usage: 
  ImportHammer --endpoint=<endpoint> --file=<sourcefile> --basename=<base name of content> --channels=<channels>
      [--lead=<leadcount>] [--loops=<loopcount>] [--bitrate=<bitrate>]  [--type=<source type>] [--interval <interval>] [--logfile <logfile>]  [--virtual <virtualsessioncount>] [--starttime=<starttime>] [--duration=<duration>] [--nPVR]
  ImportHammer --help
This script is designed to test the import works.
Options:
       --endpoint=<endpoint>		ContentStore endpoint, such as "tcp -h <ip> -h <port>"
       --file=<sourcefile>		the full source MPEG2TS|H264 file name to upload
       --channels=<channels>		give a record file name which list all channels
       --basename=<contentname>		base name of content.
       --type=<sourceType>		content type, MPEG2TS or H264, default $type    
       --bitrate=<bitrate>		bitrate of the uploading mpeg2 file, default $Bitrate
       --interval=<interval>		interval for ingest loop back,default $interval sec.
       --logfile=<logfilename>		the logfile name, default $LOGFILE      
       --lead=<leadcount>		lead count, default 1
       --virtual=<count>			virtual count, default 1
       --starttime=<startTime>		ingest start time, default is localtime, normal UTC/local format, or offset of localtime.
       --duration=<endTime>		ingest duration, specify for END time, default is 600 seconds after startTime
       --loop=<loopcount>		the loop count, default 1
       --nPVR				nPVR mode
       --help				display this screen
EOF

exit 0;
}

sub main()
{
	&GetOptions(
				'help'           =>	\$help,
                                'file=s'      => 	\$TestFile,
                                'channels=s'	=>	\$ChannelFile,
                                'lead=s'	=>	\$leadcount,
                                'basename=s'        =>     \$ContentName,
                                'loop=s'        =>     \$LoopCount,
                                'endpoint=s'		=>	\$endpoint,
                                'type=s'           =>     \$type,
                                'bitrate=s'           =>     \$Bitrate,
                                'interval=s'    =>     \$interval,
                                'logfile=s'     =>     \$LOGFILE,
                                'virtual=s'    =>     \$count,
                                'starttime=s'             =>	\$startTime,
                                'duration=s'	   =>	  \$endTime,
                                'nPVR'		=>	\$nPVR,
               );
        usage() if($help);
        die ("please give the enough parameter, for more detail please use \"--help\"") if(!$endpoint)||(!$TestFile&&!$ChannelFile)||(!$ContentName)||($help);
        unlink "$LOGFILE";
        unlink "test.txt";
	
	my %chs;
	%chs = leadchannel() if($ChannelFile);
	$leadcount = 1 if($leadcount);
		
	#Log("************** Import Starts ****************");
	open VOLUME, ">volume.txt";
	printf VOLUME "list volume";
	close VOLUME;
	system ("ContentClient.exe -e \"$endpoint\" -f volume.txt >volume");
	open List_Volume, "< volume" or die("failed to read volume");
	my @volumes;
	my $line;
	ver_h_next_line: while ($line = <List_Volume>)
	{
		push (@volumes, trim($line)) if((!($line =~ /^connected/i))&&(trim($line)));
		next ver_h_next_line;
	}
	close List_Volume;
	my $volume_count = scalar (@volumes);
	my $inte = $count;
	my $res;
	my $counts;
	if ($volume_count)
	{
		$counts = $leadcount*$count if($leadcount);
		$inte = $counts / $volume_count;
		$res = $counts % $volume_count;
	}
	else
	{
		die ("no volume for ingest");
	}
	my $inte_n = $inte+1;
	my $inte_o = $inte;
	$startTime = "0";
	$endTime = "600" if(!$endTime);
	my $check = 0;
	for (my $l=0; $l<$LoopCount; $l++)
	{
		unlink "test.txt";
		open TF, ">>test.txt";
		$lead = 0;
		$c=0;
		$res_t=$res;
		$lead_ver = 0;
		$count_ver = 0;
		for (my $cc = 0; $cc < $volume_count; $cc++)
		{
			$post = 0;
			$inte = $inte_n if($res_t>0);
			$inte = $inte_o if!($res_t>0);
			printf TF "open volume $volumes[$cc]\n" if($lead_ver<$leadcount);
			$c=0;
			$lead = $lead_ver;
			$c_count=$count_ver;
			while(($lead<$leadcount)&&($c<$inte))
			{
				$basename = "$ContentName"."C"."$lead";
				$TestFile = $chs{$basename} if(%chs);
				while($c_count<$count)
				{
					$CName = "$basename"."L"."$l"."_"."$c_count";
					printf TF "open $CName true\n";
					printf TF "set startTime=+$startTime\n";
					printf TF "set endTime=+$endTime\n";
					printf TF "set sourceType=$type\n";
					if($nPVR)
					{
						printf TF "mset nPVRCopy=1\n";
						printf TF "mset ProviderId=SeaChange\n";
						printf TF "mset ProviderAssetId=HB1\n";
						printf TF "mset SubscriberId=$volumes[$cc]\n";
					}
					printf TF "set bitrate=$Bitrate\n";
					printf TF "provision $TestFile\n";
					$c++;
					if($c == $inte)
					{
						if($c_count==$count-1)
						{
							$lead_ver = $lead+1;
							$count_ver = 0;
						}
						else
						{
							$lead_ver = $lead;
							$count_ver = $c_count+1;
						}
						$c_count=$count;
					}
					$c_count++;
				}
				$c_count = 0;
				$lead++;
			}
			$res_t-- if($res_t>0);
		}
		printf TF "close\n";
		close TF;
		Log("************** Import Starts ****************");
		system ("ContentClient.exe -e \"$endpoint\" -f test.txt >>$LOGFILE");
		Log("***one round import in progress, sleep $interval seconds and have another round***") if($l<$LoopCount-1);
		sleep $interval;
		Log("************** Import Complete ****************") if($l == $LoopCount-1);
	}
		
}

sub leadchannel()
{
	open channels, "< $ChannelFile";
	my @channel;
	
	channel_h_next_line: while ($line = <channels>)
	{
		push (@channel, trim($line)) if(trim($line));
		next channel_h_next_line;
	}
	
	
	close channels;
	
	my $channel_c=scalar(@channel);
	my %cs = ();
	my $i=0;
	if($channel_c>1)
	{
		my $loops = $leadcount / $channel_c;
		my $loop_res = $leadcount % $channel_c;
		#printf "loops=$loops\n;loop_res=$loop_res";
		my $ii=0;
		my $c=0;
		for (my $cc=0; $cc<$loops; $cc++)
		{
			$i = $channel_c * $cc;
			$c=0;
			while($c<$channel_c)
			{
				$basename = "$ContentName"."C"."$i";
				$cs{$basename} = trim($channel[$c]);
				$c++;
				$i++;
			}
		}
		while($ii<$loop_res)
		{
			$basename = "$ContentName"."C"."$i";
			$cs{$basename} = trim($channel[$ii]);
			$ii++;
			$i++;
		}
	}
	else
	{
		while($i<$leadcount)
		{
			$basename = "$ContentName"."C"."$i";
			$cs{$basename} = trim($channel[0]);
			$i++;
		}
		
	}
	return (%cs);
}

sub Log
{
   my ($log) = @_;

   my (@lines) = split(/\n/,$log);
   my (@lt) = localtime;
   my $time = sprintf "%d\/%02d\/%02d %02d\:%02d\:%02d",
      $lt[5]+1900, $lt[4]+1, $lt[3], $lt[2], $lt[1], $lt[0];

   my $err = open LOG,">>$LOGFILE";
   if (0 == $err)
   {
      $err = open LOG,">$LOGFILE";
   }
    
   if (0 == $err)
   {
      print STDOUT "ERROR: unable to write to \"$LOGFILE\": $!\n";
   }
   else
   {
      my $line;
      foreach $line (@lines)
      {
         if (1 == $ECHO)
         {
            print STDOUT "$line\n";
         }
         print LOG "$time  $line\n";
      }
   }
   close LOG;
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