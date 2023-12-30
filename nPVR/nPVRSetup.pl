use Getopt::Long;
use Win32;
use Cwd;
use env;


my @value;
my @gene;
my $count;

&main();

sub main()
{
    require "profile.pl";
    
    &GetOptions(
				'help'           =>	\$ins_help,
				'list'           =>	\$ins_list,
                                'service=s'      => 	\$ins_servicesname,
               );
    print "The installation must specify a sevices by using --service=<servicename>" if (!$ins_help&&!$ins_list&&!$ins_servicesname);
    if($ins_help)
    {
        usage();
    }
    
    if ($ins_list)
    {
	print "List all project profiles:\n";
	foreach $services (@All_Services)
	{
		print "$services->{name}\n";
	}
	print "\n";
	exit 0;
    }
    while (($key, $value)= each (%ENV))
    {
    	$EN{$key} = $value;
    }
    
# ------------------------------------------------------
# Install a service based its profile
# ------------------------------------------------------    
    if ($ins_servicesname)
    {
	  $ins_servicesname="AssetGear" if ($ins_servicesname =~ /^AssetGear/i);
          $ins_servicesname="EventCollector" if ($ins_servicesname =~ /^EventCollector/i);
          foreach $serv (@All_Services)
	  {
	  	push (@servicename, $serv) if trim($ins_servicesname) eq trim($serv->{name});
	  }
	  if(!@servicename)
	  {
	  	print ("A manual install must specify a valid service name by using --service=<servicename>, please using --list to print all valid service");
	  	exit 0;
	 }
          my $installstate=prompt ("Do you want to install or upgrade the service?{\"I\" for \"Install\",\"U\" for \"Upgrade\"}","I|U");
	  if (!(($installstate eq "U")||($installstate eq "u")||($installstate eq "I")||($installstate eq "i")))
	  {
	  	print "\nNOTE:Please give the right choice \"I\" or \"U\", thanks!\n";
	  	exit;
	  }
	  if (($installstate eq "U")||($installstate eq "u"))
	  {
	  compare();  
          my @asa=compare();
          my $count1=scalar(@asa);
	  my $sss=0;
	  while($sss<$count1)
	  {
	    my @isag;
	    $a=$asa[$sss];
	    system ("regdmp \\registry\\machine\\$$ins_servicesname[$a][0] 1>$a.txt 2>err.txt");
	    open MF,"<$a.txt";
	    my @c=<MF>;
	    close MF;
            unlink "$a.txt";
            unlink "err.txt";
	    my $c=scalar(@c);
	    my $cc=scalar(@$ins_servicesname);
	    my $i=1;
	    while($i<$c)
	    {
		my @a=split(/=/,$c[$i]);
		my $ii=0;
		while($ii<$cc)
		{
		    $a[0]=trim($a[0]);
		    if (!($$ins_servicesname[$ii][3] =~ m/^\%/))
		    {
		    if(($$ins_servicesname[$ii][0] eq $$ins_servicesname[$a][0])&&($$ins_servicesname[$ii][2] eq $a[0]))
		    {
			if($$ins_servicesname[$ii][4] eq "S")
			{
			    $a[1]=trim($a[1]);
			    my $getstate=getmessage($a[1]);
			    if (!($getstate eq 0))
			    {
			    	$a[1] = $getstate;
			    	$i++;
			    	my $im = $i;
			    	while($im<$c)
			    	{
			    		$c[$im] = trim($c[$im]);
			    		my $mes=getmessage($c[$im]);
			    		if (!($mes eq 0))
			    		{
			    			$a[1] = $a[1]." "."$mes";
			    			$i++;
			    			$im++;
			    		}
			    		else
			    		{
			    			$im = $c;
			    		}
			    		
			    	}
			    	$c[$i]=trim($c[$i]);
			    	$a[1] = $a[1]." "."$c[$i]";
			    }
			    $$ins_servicesname[$ii][3]=$a[1];
			}
		        else
			{
			    $a[1]=trim($a[1]);
			    my @new=split(/ /,$a[1]);
			    my $ccc=scalar(@new);
			    $$ins_servicesname[$ii][3]=$new[1];
			    my $iii=1;
			    while($iii<$ccc)
			    {
				$$ins_servicesname[$ii][3]="$$ins_servicesname[$ii][3]"." "."$new[$iii+1]";
				$iii++;
			    }
			}
		    }
		}
		    $ii++;
	}
		$i++;
	    }
	    
	    $sss++;
	  }
	}

	  $ins_servicesname=trim($ins_servicesname);
	  if (@$ins_servicesname eq NULL)
	  {
	    print "The installation must specify a sevices by using --service=<servicename>";
	    exit 0;
	  }
  
# ------------------------------------------------------	  
#generate the direction to install the service
# ------------------------------------------------------
          my $server=prompt ("If the Server version is windows NT 4.0","Y|N");
          my $direction=prompt ("Input the direction of $ins_servicesname:","C:\\NPVR");
	  my $TARGETDIR=$direction;
          my $EXEDIR="$TARGETDIR\\exe";
          my $LOGDIR="$TARGETDIR\\log";
          my $ITVEXEROOT="C:\\ITV\\EXE";
	  my $JBOSSCLIENT="$EXEDIR\\JMSClient";
	  my $CRASHDIR="$TARGETDIR\\crash";
          my $SVCACCOUNT=".\\SeaChange";
          my $SVCPASSWD="SeaChange";
	  my %EN = (
		  TARGETDIR => $TARGETDIR,
		  EXEDIR    => $EXEDIR,
		  LOGDIR    => $LOGDIR,
		  ITVROOT   => "c:\\itv",
		  ITVEXEROOT  => "c:\\itv\\exe",
		  CRASHDIR  => "$TARGETDIR\\crash",
		  JBOSSCLIENT => "$EXEDIR\\JMSCLIENT",
		  );
		  
	  my $file = "$ins_servicesname.bat";
	  open CMDFILE, ">$file";
	  print CMDFILE "setlocal\n";
	  print CMDFILE "set TARGETDIR=$direction\n";
	  print CMDFILE "set EXEDIR=%TARGETDIR%\\exe\n";
          print CMDFILE "set LOGDIR=%TARGETDIR%\\log\n";
          print CMDFILE "set ITVEXEROOT=C:\\ITV\\EXE\n";
	  print CMDFILE "set JBOSSCLIENT=%EXEDIR%\\JMSCLIENT\n";
          print CMDFILE "set SVCACCOUNT=.\\SeaChange\n";
          print CMDFILE "set SVCPASSWD=SeaChange\n";
          print CMDFILE "if not exist $TARGETDIR md $TARGETDIR\n";
          print CMDFILE "if not exist $EXEDIR md $EXEDIR\n";
          print CMDFILE "if not exist $LOGDIR md $LOGDIR\n";
          print CMDFILE "if not exist $JBOSSCLIENT md $JBOSSCLIENT\n";
          print CMDFILE "if not exist $CRASHDIR md $CRASHDIR\n";
          if ($server =~ /^Y/i)
          {
	  if ($ins_servicesname =~ /^EventCollector/i)
	  {
	    printf CMDFILE "xcopy /sf EventCollector $EXEDIR\n";
	  }
	  if ($ins_servicesname =~ /^AssetGear/i)
	  {
	    printf CMDFILE "xcopy /f AssetGear $EXEDIR\n";
	  }
	  print CMDFILE "cd dll\n";
	  foreach $TAO (@TAO_APIS)
	  {
	    print CMDFILE "xcopy /f \"$TAO\" \"$EXEDIR\"\n";
	  }
	  foreach $VOD (@VOD_APIS)
	  {
	    print CMDFILE "xcopy /f \"$VOD\" \"$EXEDIR\"\n";
	  }
	  print CMDFILE "cd ..\n";
	  print CMDFILE "if not exist \"%ITVEXEROOT%\"\\instserv.exe copy \"instserv.exe\" \"%EXEDIR%\" \n";
          print CMDFILE "if not exist \"%ITVEXEROOT%\"\\srvshell.exe copy \"srvshell.exe\" \"%EXEDIR%\" \n";
	}
	else
	{
	if ($ins_servicesname =~ /^EventCollector/i)
	  {
	    printf CMDFILE "xcopy /sfy EventCollector $EXEDIR\n";
	  }
	  if ($ins_servicesname =~ /^AssetGear/i)
	  {
	    printf CMDFILE "xcopy /fy AssetGear $EXEDIR\n";
	  }
	  print CMDFILE "cd dll\n";
	  foreach $TAO (@TAO_APIS)
	  {
	    print CMDFILE "xcopy /fy \"$TAO\" \"$EXEDIR\"\n";
	  }
	  foreach $VOD (@VOD_APIS)
	  {
	    print CMDFILE "xcopy /fy \"$VOD\" \"$EXEDIR\"\n";
	  }
	  print CMDFILE "cd ..\n";
	  print CMDFILE "if not exist \"%ITVEXEROOT%\"\\instserv.exe copy \"instserv.exe\" \"%EXEDIR%\" /Y\n";
          print CMDFILE "if not exist \"%ITVEXEROOT%\"\\srvshell.exe copy \"srvshell.exe\" \"%EXEDIR%\" /Y\n";
	}
	if (($installstate eq "I")||($installstate eq "i"))
	{
		print CMDFILE "if exist \"%ITVEXEROOT%\"\\srvshell.exe instserv $ins_servicesname \"%ITVEXEROOT%\"\\srvshell.exe local %SVCACCOUNT% %SVCPASSWD% auto own\n";
                print CMDFILE "if not exist \"%ITVEXEROOT%\"\\srvshell.exe instserv $ins_servicesname \"%EXEDIR%\"\\srvshell.exe local %SVCACCOUNT% %SVCPASSWD% auto own\n";
	        print CMDFILE "instserv $ins_servicesname displayname=\"SeaChange $ins_servicesname\"\n";
	}

    
          $count =scalar(@$ins_servicesname);
	  my $i = 0;
	  while($i<$count)
          {
	    $$ins_servicesname[$i][3]=trim($$ins_servicesname[$i][3]);
	    if ($$ins_servicesname[$i][3] =~ m/^\%/)
	    {
		my @convert=split(/%/,$$ins_servicesname[$i][3]);
		if($$ins_servicesname[$i][3] =~ m/;/)
		{
		  $$ins_servicesname[$i][3]="$EN{$convert[1]}"."$convert[2]"."$EN{$convert[3]}"."$convert[4]";  
		}
		else
		{
		    $$ins_servicesname[$i][3]="$EN{$convert[1]}"."$convert[2]";
		}
	    }
            my $value = prompt("$$ins_servicesname[$i][1]","$$ins_servicesname[$i][3]");
            $value=$value."\\" if($value =~ m/.*\\$/);
            print CMDFILE "regsetup \"$$ins_servicesname[$i][0]\" \"$$ins_servicesname[$i][2]\" $$ins_servicesname[$i][4] \"$value\"\n";
            $i++;
	  }
	  print CMDFILE "cd ..\n";
	  print CMDFILE "endlocal\n";
	  close CMDFILE;
	  
	  my $choise=choise();
	  my $number;
	  for($number=1;$number<100;$number++)
	  {
	    if(($choise =~ /^Y/i) || ($choise =~ /^N/i))
	    {
		$number=100;
	    }
	    else
	    {
		$choise=choise();
	    }
	  }
	  if($choise =~ /^Y/i)
	  {
	    system ("$file");
	  }
	  unlink "$file";
	  unlink "err.txt";
    }
    
}
sub choise()
{
    print "Do you want to commit the changes {Y|N}? ";
    my $choose = <stdin>;
    $choose = trim($choose);
    return $choose;
}

sub usage()
{
    print <<EOF;
Usage:
    IsaSetup [--list | --service=<servicename>]
    IsaSetup --help
options
    --service=<sname>
        specify the services to install
    --list
        list all the service
    --help
        display this screen

EOF

exit 0;
}



sub prompt

{

            my $msg = shift;

            my $default = shift;

            

            print "$msg [$default]";

 


            my $ch = <STDIN>;

            $ch = trim($ch);

            

            $ch = $default if ($ch eq "");

            

            return $ch;

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

sub compare()
{
  my @array;
  my $count=scalar(@$ins_servicesname);
  my $a=0;
  while($a<$count)
  {
    if($a==0)
    {
        
        push (@array, $a);
        

    }
    my $s=$a-1;
    while($s>=0)
    {
        if($$ins_servicesname[$a][0] eq $$ins_servicesname[$s][0])
        {
            last;
        }
        else
        {
            $s--;
            if ($s<0)
            {
             push (@array,$a);
            }
        }
    }
    $a++;
  }
return @array;
}

# ------------------------------------------------------

# get a string message for registry

# ------------------------------------------------------

sub getmessage()
{
	my $char = shift;
	$char = trim ($char);
	my @string = split (/ /,$char);
	my $countstr = scalar (@string);
	if($string[$countstr-1] eq "\\")
	{
	        $char="";
	        my $ch=0;
		while ($ch<$countstr-1)
		{
		        $char = "$char"."$string[$ch]";
			$ch++;
	       }
	       if ($char eq "")
	       {
	       	return 0;
	       }
	       return ($char);
	}
	else
	{
		return 0;
	}
}