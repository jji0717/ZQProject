#!/usr/bin/perl
#
use Getopt::Long;
use threads;

&main();

sub main()
{
        &GetOptions(
                                'help'          =>      \$help,
				'sleep=s'	=>	\$in_sleep,
                                'interval=s'    =>      \$in_interval,
                                'config=s'      =>      \$in_config,
                                'count=s'       =>      \$in_count,
				'timeout=s'	=>	\$in_timeout,
				'logfile=s'	=>	\$in_log,
				'thread=s'	=>	\$in_thread,
				'contentname=s'	=>	\$in_content,
				'single'	=>	\$in_single,
                    );
	$in_content='000000001' if(!$in_content);
	$in_thread=10 if(!$in_thread);
	$in_timeout=150000 if(!$in_timeout);
	$in_sleep=10 if(!$in_sleep);
        $in_interval=10 if(!$in_interval);
        $in_count=1 if(!$in_count);
        $in_config="./data/configurefile" if(!$in_config);
	usage() if($help);
	$in_log="test.log";
	`del -rf test.log`;	

	`echo \"starting performance test\" >> $in_log`;
	#`date >>$in_log`;
        my $thread=0;
        while($thread < $in_count)
        {
		if($in_count<10000)
		{
			$counts = sprintf "%04d", $thread;
		}
		else
		{
			$counts = sprintf "%05d", $thread;
		}
		$logname = "$thread".".log";
		if($in_single)
		{
			$contentname=$in_content;
			$suffix=0;
		}
		else
		{
			$contentname="$in_content"."$counts";
			$suffix=1;
		}
		#$logname = "$thread".".log";
                $threadname = threads->create('mtclient', "$in_sleep", "$in_config", "$logname", "$in_thread", "$in_timeout", "$contentname", "$suffix" );
                $thread++;
                print "thread=$thread\n";
                push (@thread, $threadname); 
                $in_sleep = $in_interval + $in_sleep;
        }

        foreach $threads (@thread)
        {
        	printf "threads=$threads\n";
                $threads->join;
        }
	`echo \"performance test end\" >>$in_log`;
	#`date >>$in_log`;
}

sub mtclient
{
        my $intervals = @_[0];
        my $configfile = @_[1];
	my $log = @_[2];
	my $threads = @_[3];
	my $timeout = @_[4];
	my $content = @_[5];
	my $suffix_client = @_[6];
        $cmd = "mtclient.exe --ms $intervals --confile $configfile --logfile $log --threads $threads --timeout $timeout --resourse $content --suffix $suffix_client";
        $lt = localtime();
	`echo time=$lt cmd = $cmd >> $in_log`;	
        system ($cmd) or die ("fail to perform $cmd\n");
}


sub usage
{
	print <<EOF;
		
	Usage: udpperf.pl <options>
	udpperf.pl  is a simple perl program than can drive mtclient for dsmcc and lscp test

	options
	--sleep=<s>
		sleep time before the first request send out, in ms
	--interval=<s>
		interval on each session
	--count=<s>
		set up multiple mtclient, the default is \"1\"
	--config=<s>
		config file full path, default is \"./data/configurefile\"
	--timeout=<s>
		timeout in ms, default is 150000
	--thread=<s>
		create multiple thread each mtclient, default is 10
	--contentname=<s>
		base content name
	--single
		single content test
	--help
		help	
EOF

	exit 0;
}
