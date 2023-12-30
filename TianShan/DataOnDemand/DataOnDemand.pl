use Getopt::Long;
use Win32;
use Cwd;
use env;

&main();
sub main()
{
    require "profile.pl";
    &GetOptions(
				'help'           =>	\$ins_help,
                                'install'        =>     \$install,
                                'upgrade'        =>     \$upgrade,
                              #  'uninstall'      =>     \$uninstall,
                              #  'copy'           =>     \$copyfile,
                              #  'uncopy'         =>     \$uncopyfile,
                                'service=s'      => 	\$ins_servicesname,
                                'list'           =>	\$ins_list,
                                
               );
    print "The installation must specify a sevices by using --service=<servicename> and specify install or upgrade the service by using \"--install\" or \"--upgrade\".For more detail, please using \"--help\"" if (!($install||$upgrade)&&!$ins_help&&!$ins_list&&!$ins_servicesname);
    if($ins_help)
    {
        usage();
    }
    #print "Please specify Install or Upgrade service by using \"--install\" or \"--upgrade\".For more detail, please using \"--help\"" if (!($install||$upgrade)&&!$ins_help&&!$ins_list);
     if ($ins_list)
    {
	print "List all project profiles:\n";
	foreach $services (@All_Services)
	{
		print "$services\n";
	}
	#print "\n";
	exit 0;
    }
    
    if ($install)
    {
    	$installstate = "I";
    }
    if ($upgrade)
    {
    	$installstate = "U";
    }

    while (($key, $value)= each (%ENV))
    {
    	$EN{$key} = $value;
    }
    $ins_servicesname="DataStream" if ($ins_servicesname =~ /^DataStream/i);
    $ins_servicesname="DODApp" if ($ins_servicesname =~ /^DODApp/i);
    
    

if ($installstate)
{
          if ($installstate eq "U")
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

    $directory=prompt ("Input the directory for install $ins_servicesname","C:\\DataOnDemand") if ($install);
    if ($upgrade)
    {
    	$InstallPathCMD="reg query HKLM\\Software\\SeaChange\\DataOnDemand\\Currentversion\\Services\\DODApp_shell /v ImagePath 1>atemp.txt 2>error.txt" if($ins_servicesname eq "DODApp");
    	$InstallPathCMD="reg query HKLM\\SYSTEM\\CurrentControlSet\\Services\\DataStream /v ImagePath 1>atemp.txt 2>error.txt" if($ins_servicesname eq "DataStream");
    	@InstallfilePath = getpath ($InstallPathCMD);
    	$directory=prompt ("Input the directory for install $ins_servicesname","$InstallfilePath[2]");
    }
    $EXEDIR="$directory\\$ins_servicesname";
    $LOGDIR="$directory\\log";
    $DODAppService="$EXEDIR\\DODAppService";
    $DODStreamer="$EXEDIR\\DODStreamer";;
    $SVCACCOUNT="LocalSystem";
    $SVCPASSWD="LocalSystem";
    %EN = (
		  TARGETDIR => $directory,
		  EXEDIR    => $EXEDIR,
		  LOGDIR    => $LOGDIR,
		  ITVROOT   => "c:\\itv",
		  ITVEXEROOT  => "c:\\itv\\exe",
	     );
    
    my $file="Install.bat";
    open CMDFILE, ">$file";
    printf CMDFILE "setlocal\n";
    printf CMDFILE "set ITVEXEROOT=C:\\itv\\exe\n";
    printf CMDFILE "set TARGETDIR=$directory\n";
    printf CMDFILE "set EXEDIR=%TARGETDIR%\\$ins_servicesname\n";
    printf CMDFILE "set LOGDIR=%TARGETDIR%\\log\n";
    printf CMDFILE "if not exist $directory md $directory\n";
    printf CMDFILE "if not exist $EXEDIR md $EXEDIR\n";
    printf CMDFILE "if not exist $LOGDIR md $LOGDIR\n";
    printf CMDFILE "if not exist $DODAppService md $DODAppService\n" if ($ins_servicesname eq "DODApp");
    printf CMDFILE "if not exist $DODStreamer md $DODStreamer\n" if ($ins_servicesname eq "DataStream");
    printf CMDFILE "xcopy /fycs $ins_servicesname\\bin $EXEDIR\n";
    printf CMDFILE "xcopy /fycs $ins_servicesname\\etc $EXEDIR\n" if ($installstate eq "I");
    printf CMDFILE "xcopy /fycs common $EXEDIR\n" if ($ins_servicesname eq "DataStream");
    if ($installstate eq "I")
    {
    printf CMDFILE "instserv $ins_servicesname \"$EXEDIR\"\\srvshell.exe local $SVCACCOUNT \"$SVCPASSWD\" auto own\n";
    printf CMDFILE "instserv $ins_servicesname displayname=\"$ins_servicesname\"\n";
    }
    
    if ($ins_servicesname eq "DataStream")
    {
    	print CMDFILE "regsvr32 /s $EXEDIR\\DataWatcherSource.ax\n";
    	print CMDFILE "regsvr32 /s $EXEDIR\\DataWrapper.ax\n";
    	print CMDFILE "regsvr32 /s $EXEDIR\\ZQBroadcastFilter.ax\n";
    }
    
    $count =scalar(@$ins_servicesname);
    my $i = 0;
    while($i<$count)
    {
	$$ins_servicesname[$i][3]=trim($$ins_servicesname[$i][3]);
	if ($$ins_servicesname[$i][3] =~ m/^\%/)
	{
	    my @convert=split(/%/,$$ins_servicesname[$i][3]);
	    $$ins_servicesname[$i][3]="$EN{$convert[1]}"."$convert[2]";
	}
        my $value = prompt("$$ins_servicesname[$i][1]","$$ins_servicesname[$i][3]") if ($$ins_servicesname[$i][5]);
	$value = $$ins_servicesname[$i][3] if !($$ins_servicesname[$i][5]);
        $value=$value."\\" if($value =~ m/.*\\$/);
        printf CMDFILE "regsetup \"$$ins_servicesname[$i][0]\" \"$$ins_servicesname[$i][2]\" $$ins_servicesname[$i][4] \"$value\"\n";
        $i++;
    }
    printf CMDFILE "endlocal\n";
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
}


exit 0;   
}


# ------------------------------------------------------

#get file path and name from registry for uninstall

# ------------------------------------------------------
sub getpath()
{
	my $profile = shift;
	return if !$profile;
	system($profile)==0 or return 0;
	my @array;
	open MYFILE, "<atemp.txt";
	@Ctemp=<MYFILE>;
	close MYFILE;
	unlink "atemp.txt";
	unlink "error.txt";
	$ctmp=scalar(@Ctemp);
	@aatemp = split (/REG_SZ/,$Ctemp[$ctmp-2]);
	
	$aatempcount=scalar(@aatemp);
	$aat = trim($aatemp[$aatempcount-1]);
	@aatmp = split (/\\/,$aat);
	$query=scalar(@aatmp);
	$filePath="$aatmp[0]";
	for($aquery=1; $aquery<$query-1; $aquery++)
	{
		$filePath="$filePath\\$aatmp[$aquery]";
	}
	
	$installpath="$aatmp[0]";
	for($aquery=1; $aquery<$query-2; $aquery++)
	{
		$installpath="$installpath\\$aatmp[$aquery]";
		push (@temparray, $installpath);
	}
	
	push (@array, $filePath);
	push (@array, $aatmp[$query-1]);
	push (@array, $installpath);
	
	foreach $temppath (@temparray)
	{
		push (@array, $temppath);
	}
	
	return @array;
}

sub uninstallchoise()
{
    print "Do you want to uninstall the service {Y|N}?";
    my $choise = <stdin>;
    $choise = trim($choise);
    return $choise;
}


sub choise()
{
    print "Do you want to install the service and commit the changes {Y|N}? ";
    my $choose = <stdin>;
    $choose = trim($choose);
    return $choose;
}


sub usage()
{
    print <<EOF;
Usage:
    DBSync <--service=servicename> <--install | --upgrade>
    DBSync --help
options
    --service=<servicename>
        specify the service name which like to install
    --install
        install the service
    --upgrade
        upgrade the service
    --list
        list all service which can be installed by the script
    --help
        display this screen

EOF

exit 0;
}

sub prompt

{

            my $msg = shift;

            my $default = shift;

            

            print "$msg [$default]:";

 


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