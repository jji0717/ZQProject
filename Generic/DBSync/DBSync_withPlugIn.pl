use Getopt::Long;
use Win32;
use Cwd;
use env;

&main();
sub main()
{
    &GetOptions(
				'help'           =>	\$ins_help,
                                'install'        =>     \$install,
                                'upgrade'        =>     \$upgrade,
                                'uninstall'      =>     \$uninstall,
                                'copy'           =>     \$copyfile,
                                'uncopy'         =>     \$uncopyfile,
                                'plugin=s'         =>     \$plugin,
                                'servicename=s'    =>     \$servicename,
               );
    if($ins_help)
    {
        usage();
    }
    print "Please specify Install or Upgrade service by using \"--install\" or \"--upgrade\".For more detail, please using \"--help\"" if (!($install||$upgrade)&&!$ins_help&&!$uninstall);
    
    $ins_servicesname = "DBSync";
    $servicename = $ins_servicesname if(!$servicename);
    $servicename_shell = "$servicename"."_shell";
    
    require "profile.pl";
    
    $status=system ("instserv $servicename status >temp.txt");
    unlink "temp.txt";
    die ("The service $servicename have not been installed, you shouldn't specify upgrade it") if($status && $upgrade);
    die ("The service $servicename have not been installed, you shouldn't uninstall it") if($status && $uninstall);
    
    if ($install)
    {
    	$installstate = "I";
    }
    if ($upgrade)
    {
    	$installstate = "U";
    }

    if ($plugin)
    {
    	my @plugins = split(/,/,$plugin);
    	$csa=scalar(@plugins);
    	$csad=0;
    	while($csad<$csa)
    	{
    		$JMS_DBSyncAddIn="IDSSyncEventPlugin" if(trim($plugins[$csad]) =~ /^IDSSyncEventPlugin/i);
    		$ManualSyncAddIn="ManualSyncPlugin" if(trim($plugins[$csad]) =~ /^ManualSyncPlugin/i);
    		$csad++;
    	}
    	if(($JMS_DBSyncAddIn eq "")&&($ManualSyncAddIn eq ""))
    	{
    		print ("Please give the right plugin name, for more detail,please using \"--help\"");
    		exit 0;
    	}
    }
    
    die ("Please give the right command, for more detail please using \"--help\"") if(($installstate)&&($uninstall));
    
if (($installstate)&&(!$uninstall))
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

    $directory=prompt ("Input the directory for install $servicename","C:\\DBSync") if ($install);
    if ($upgrade)
    {
    	$InstallPathCMD="reg query HKLM\\Software\\SeaChange\\ITV\\Currentversion\\Services\\$servicename_shell /v ImagePath 1>atemp.txt 2>error.txt";
    	@InstallfilePath = getpath ($InstallPathCMD);
    	$directory=prompt ("Input the directory for install DBSync","$InstallfilePath[2]");
    }
    $EXEDIR="$directory\\bin";
    $LOGDIR="$directory\\log";
    $SVCACCOUNT=".\\SeaChange";
    $SVCPASSWD="SeaChange";
    while (($key, $value)= each (%ENV))
    {
    	$EN{$key} = $value;
    }
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
    printf CMDFILE "set EXEDIR=%TARGETDIR%\\bin\n";
    printf CMDFILE "set LOGDIR=%TARGETDIR%\\log\n";
    if(!$uncopyfile)
    {
    printf CMDFILE "if not exist $directory md $directory\n";
    printf CMDFILE "if not exist $EXEDIR md $EXEDIR\n";
    printf CMDFILE "if not exist $LOGDIR md $LOGDIR\n";
    printf CMDFILE "xcopy /fycsr bin $EXEDIR\n";
    if($JMS_DBSyncAddIn eq "IDSSyncEventPlugin")
    {
    	printf CMDFILE "xcopy /fycsr plugin\\IDSSyncEventPlugin.dll $EXEDIR\n";
    	printf CMDFILE "xcopy /fycsr plugin\\IDSSyncEventPlugin.ini $EXEDIR\n";
    	printf CMDFILE "xcopy /fycsr plugin\\jmsc.dll $EXEDIR\n";
    	printf CMDFILE "xcopy /fycsr plugin\\JMSClient\\. $EXEDIR\n";
    }
    if($ManualSyncAddIn eq "ManualSyncPlugin")
    {
    	printf CMDFILE "xcopy /fycsr plugin\\ManualSyncPlugin.dll $EXEDIR\n";
    	printf CMDFILE "xcopy /fycsr plugin\\jmsc.dll $EXEDIR\n";
    	printf CMDFILE "xcopy /fycsr plugin\\JMSClient\\. $EXEDIR\n";
    }
    }
    if ($installstate eq "I")
    {
    printf CMDFILE "instserv $servicename \"$EXEDIR\"\\srvshell.exe local $SVCACCOUNT $SVCPASSWD auto own\n";
    printf CMDFILE "instserv $servicename displayname=\"$servicename\"\n";
    }
    
    $count =scalar(@$ins_servicesname);
    my $i = 0;
    while($i<$count)
    {
	$$ins_servicesname[$i][3]=trim($$ins_servicesname[$i][3]);
	@atmp=split(/\\/,$$ins_servicesname[$i][0]);
        $atemp=scalar(@atmp);
        if(($JMS_DBSyncAddIn eq "") && ($$ins_servicesname[$i][2] eq "IDSSyncEventPluginPath"))
        {
        	$i++ ;
        }
        elsif(($ManualSyncAddIn eq "") && ($$ins_servicesname[$i][2] eq "ManualSyncPluginPath"))
        {
        	$i++ ;
        }
        elsif(($atmp[$atemp-1] eq "IDSSyncEventPlugin") && ($JMS_DBSyncAddIn eq ""))
        {
        	$i++;
        }
        elsif(($atmp[$atemp-1] eq "ManualSyncPlugin") && ($ManualSyncAddIn eq ""))
        {
        	$i++;
        }
        else
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

if($uninstall)
{
	my $uninstall_file="uninstall.bat";
	open MFILE, ">$uninstall_file";
	printf MFILE "setlocal\n";
	$ImagefileCMD = "reg query HKLM\\Software\\SeaChange\\ITV\\Currentversion\\Services\\$servicename_shell /v ImagePath 1>atemp.txt 2>error.txt";
	@Imagefile = getpath ($ImagefileCMD);
	$directorylevels = scalar(@Imagefile);
	foreach $file_list (@Filelist)
	{
		printf MFILE "if exist $Imagefile[0]\\$file_list del $Imagefile[0]\\$file_list /Q\n" if($Imagefile[0]);
	}
	$LogfileCMD = "reg query HKLM\\Software\\SeaChange\\ITV\\Currentversion\\Services\\$servicename /v LogFileName 1>atemp.txt 2>error.txt";
	@Logfile = getpath ($LogfileCMD);
	printf MFILE "if exist $Logfile[0]\\$Logfile[1] del $Logfile[0]\\$Logfile[1] /Q\n" if($Logfile[0]);
	$PluginfileCMD = "reg query HKLM\\Software\\SeaChange\\ITV\\Currentversion\\Services\\$servicename\\IDSSyncEventPlugin /v LogPath 1>atemp.txt 2>error.txt";
	@PluginLogfile = getpath ($PluginfileCMD);
	unlink "atemp.txt" if(!$PluginLogfile[0]);
	unlink "error.txt" if(!$PluginLogfile[0]);
	printf MFILE "if exist $PluginLogfile[0]\\$PluginLogfile[1] del $PluginLogfile[0]\\$PluginLogfile[1] /Q\n" if ($PluginLogfile[0]);
	printf MFILE "rmdir $Imagefile[0]\\java /S /Q 1>a 2>b\n" if($Imagefile[0]);
	printf MFILE "rmdir $Imagefile[0] /Q 1>a 2>b\n" if($Imagefile[0]);
	printf MFILE "rmdir $Logfile[0] /Q 1>a 2>b\n" if($Logfile[0]);
	printf MFILE "rmdir $PluginLogfile[0] /Q 1>a 2>b\n" if ($PluginLogfile[0]);
	for($templevels=$directorylevels-1;$templevels>2;$templevels--)
	{
		printf MFILE "rmdir $Imagefile[$templevels] /Q 1>a 2>b\n";
	}
	printf MFILE "instserv $servicename remove\n";
	
	
	
	compare();  
        my @listcount=compare();
        foreach $listnum (@listcount)
        {
        	printf MFILE "reg delete HKLM\\$$ins_servicesname[$listnum][0] /f 1>a 2>b\n";
        }
	printf MFILE "endlocal\n";
	close MFILE;
	my $choise=uninstallchoise();
        my $number;
    for($number=1;$number<100;$number++)
    {
	if(($choise =~ /^Y/i) || ($choise =~ /^N/i))
	{
            $number=100;
	}
	else
	{
	    $choise=uninstallchoise();
        }
    }
    if($choise =~ /^Y/i)
    {
	system ("$uninstall_file");
    }
    unlink "$uninstall_file";
    unlink "a";
    unlink "b";	
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
    DBSync <options>
    DBSync.pl is a simple Perl program that can install or upgrade DBSync service auto,the registry is base on the profile.
options
    --install
        install the service
    --upgrade
        upgrade the service
    --uninstall
        uninstall the service
    --copy
        copy the execution file to the target directory and modify the registry
    --uncopy
        do not copy the execution file, modify the registry only
    --plugin=<PluginName>,<PluginName>
        specify PlugInName for DBSync. By now, there are two choice \"IDSSyncEventPlugin\" and \"ManualSyncPlugin\"
        if there are more than one plugin files, please split by comma
    --servicename=<servicename>
        specify servicename, default is \"DBSync\"
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

# ------------------------------------------------------

# get registry key from profile

# ------------------------------------------------------

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