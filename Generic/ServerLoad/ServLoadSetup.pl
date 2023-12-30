#use strict 'vars';
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
				'help'          =>	\$ins_help,
                                'install'       =>	\$install,
                                'upgrade'       =>	\$upgrade,
                                'uninstall'     =>      \$uninstall,
                                'copy'          =>      \$copy,
                                'uncopy'        =>      \$uncopy,
                                'installroot=s' =>      \$ins_installroot,
                                'svcaccount=s'  =>      \$ins_svcaccount,
                                'svcpassword=s' =>      \$ins_svcpassword,
                                'auto'	        =>      \$ins_auto,
                                'manual'	=>	\$ins_manual,
                                'x64'		=>	\$ins_x64,
                                'x86'		=>	\$ins_x86,
               );

    die "The installation must specify specify Install or Upgrade service by using \"--install\" or \"--upgrade\".For more detail, please using \"--help\"" if (!($install||$upgrade)&&!$ins_help&&!$uninstall);
    usage() if($ins_help);
    
    $ins_servicesname="ServerLoad";
    $servicename="ServerLoad";
    $servicesname_shell="$ins_servicesname"."_Shell";
    
    require "profile.pl";
    
    while (($key, $valuess)= each (%ENV))
    {
    	$EN{$key} = $valuess;
    }
    $ins_property="auto" if(!$ins_manual);
    $ins_property="manual" if($ins_manual);
    $INSTALLROOT=fixPathToOS($ins_installroot) if ($ins_installroot);
    die("You should not give the two different choice, thanks") if($ins_auto&&$ins_manual);
    
    $ins_x86=1 if((!$ins_x64)&&(!$ins_x86));
	$ins_bit = "32bit" if($ins_x86);
	$ins_bit = "64bit" if($ins_x64);
    		
	$OSver=OSver();
	die ("64bit server can not installed on 32bit machine\n") if(($OSver eq "x86")&&($ins_x64));
	if(($OSver eq "x64")&&($ins_x86))
	{
		print "OS version it 64bit, please install x64 serverload\n";
		exit 1;
	}
    
    if($install)
     {
	        $subkey_name="LMachine/SoftWare/ZQ Interactive/SNMPOID/CurrentVersion/Services";
	  		$Registry->Delimiter("/");
    	    $subkey_name = $Registry->{$subkey_name};
    	    if($subkey_name)
    	    {
				@subKeyNames= $subkey_name->SubKeyNames;
				$keyNums = scalar(@subKeyNames);
	  	
				foreach $subkey_names (@subKeyNames)
				{
					$keynamess="LMachine/SoftWare/ZQ Interactive/SNMPOID/CurrentVersion/Services/"."$subkey_names";
					$Registry->Delimiter("/");
					$keynamess = $Registry->{$keynamess};
					$value_id = $keynamess->GetValue(ServiceOID);
					$value_id = hex ($value_id);
					push (@value_id, $value_id);
				}
	  			@value_id = sort {$a <=> $b} @value_id;
	  			$SVCOID = $value_id[$keyNums-1]+1;
	        }
	        $SVCOID=1 if (!$SVCOID);
	        
	}
    
# ------------------------------------------------------
# Install a service based its profile
# ------------------------------------------------------    

# get original installroot if upgrade the service	  
	  if(($upgrade&&!$ins_installroot)||($uninstall))
	  {
	  	$key_name="LMachine/System/CurrentControlSet/Services/$ins_servicesname";
                $Registry->Delimiter("/");
    	        $key_name = $Registry->{$key_name};
    	        die("The system haven't found original version, please install first") if(!$key_name);
	  	
	  	$key="LMachine/SoftWare/ZQ Interactive/TianShan/Currentversion/Services/$servicesname_shell";
                $Registry->Delimiter("/");
    	        $key = $Registry->{$key};
    	        $ImagePath = $key->GetValue(ImagePath);
    	        @InstallfilePath = getpath ($ImagePath);
    	        $INSTALLROOT=$InstallfilePath[2];
	  } 
	  @serviceregis=@{${$servicename}{Registry}};
	  GetRegistry(${$servicename}{Registry}) if($upgrade);	  
	  if($ins_svcaccount)
	  {
	  	$SVCACCOUNT=$ins_svcaccount;
	  	$SVCACCOUNT=".\\"."$SVCACCOUNT" if ((!($SVCACCOUNT =~ /^LocalSystem/i))&&(!($SVCACCOUNT =~ m/.*\\/)));
	  }
	  else
	  {
	  	$SVCACCOUNT="LocalSystem";
	  }
	  if ($ins_svcpassword)
	  {
	  	$SVCPASSWORD=$ins_svcpassword;
	  }
	  else
	  {
	  	$SVCPASSWORD="";
	  }
	  $EXEDIR="$INSTALLROOT\\Bin";
          $LOGDIR="$INSTALLROOT\\Logs";
          $CONFIGDIR="$INSTALLROOT\\etc";
          $DOCDIR="$INSTALLROOT\\doc";
          $DATADIR="$INSTALLROOT\\Data";
          $SAFESTOREPATH="$INSTALLROOT\\SafeStore";
	  $CRASHDUMPPATH="$INSTALLROOT\\CrashDump";
	  %EN = (
		  TARGETDIR => $INSTALLROOT,
		  EXEDIR    => $EXEDIR,
		  LOGDIR    => $LOGDIR,
		  CONFIGDIR   => $CONFIGDIR,
		  DOCDIR   => $DOCDIR,
		  );		  
	  my $file = "$ins_servicesname.bat";
	  open CMDFILE, ">$file";
	  printf CMDFILE "setlocal\n";
	  printf CMDFILE "set TARGETDIR=$EN{TARGETDIR}\n";
	  printf CMDFILE "set EXEDIR=$EN{EXEDIR}\n";
          printf CMDFILE "set LOGDIR=$EN{LOGDIR}\n";
          printf CMDFILE "set CONFIGDIR=$EN{CONFIGDIR}\n";
          printf CMDFILE "set DOCDIR=$EN{DOCDIR}\n";
	  printf CMDFILE "set SVCACCOUNT=$SVCACCOUNT\n";
          printf CMDFILE "set SVCPASSWORD=$SVCPASSWORD\n";
          if($uninstall)
	  {
	  	printf "Before uninstall service, make sure the service is stopped and close all related folder.If the service is still running, please input \"N\", stop service and run the script again.\n";
	  	sleep (1);
	  	$key="LMachine/SoftWare/ZQ Interactive/TianShan/Currentversion/Services/$ins_servicesname";
                $Registry->Delimiter("/");
    	        $key = $Registry->{$key};
    	        $LogPath = $key->GetValue(logDir);
    	        $ConfigPath = $key->GetValue(configDir);
	  	foreach $file_list (@Filelist)
		{
			printf CMDFILE "if exist $EXEDIR\\$file_list del $EXEDIR\\$file_list /Q\n";
		}
		printf CMDFILE "if exist $ConfigPath\\ServerLoad_Sample.xml del $ConfigPath\\ServerLoad_Sample.xml /Q\n";
		printf CMDFILE "if exist $ConfigPath\\ServerLoad.xml del $ConfigPath\\ServerLoad.xml /Q\n";
		printf CMDFILE "if exist $LogPath\\ServerLoad*.log del $LogPath\\ServerLoad*.log /Q\n";
		printf CMDFILE "del $DOCDIR\\*.* /Q\n";
		printf CMDFILE "rmdir $EXEDIR /Q 1>a 2>b\n";
		printf CMDFILE "rmdir $ConfigPath /Q 1>a 2>b\n";
		printf CMDFILE "rmdir $LogPath /Q 1>a 2>b\n";
		printf CMDFILE "rmdir $DOCDIR /Q 1>a 2>b\n";
		printf CMDFILE "rmdir $INSTALLROOT /Q 1>a 2>b\n";
		if($ins_x64)
		{
			printf CMDFILE "cd bin64\n";
		}
		else
		{
			printf CMDFILE "cd bin\n";
		}
		printf CMDFILE "ZQShell.exe -U $ins_servicesname\n";
		printf CMDFILE "cd ..\n";
	  }
	  copyfile() if (!$uncopy&&!$uninstall);
	  instservice($ins_servicesname) if($install);
	  PromptRegistry(${$servicename}{Registry}) if(!$uninstall);
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
	        SetRegistry(${$servicename}{Registry}) if(!$uninstall);
	        DeleteRegistry() if($uninstall);
	    
	  }
	  unlink "$file";
	  unlink "a" if($uninstall);
	  unlink "b" if($uninstall);
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
    ServLoadSetup <options>
    ServLoadSetup is a simple Perl program that can install ServerLoad based on the project profile
options
    --install
        install the service
    --upgrade
        upgrade the service
    --uninstall
    	uninstall the service
    --installroot=<path>
        specify the location where the service will work, default value is \"C:\\ServerLoad\"
    --svcaccount=<account>
        specify the OS account for install service, default value is \"LocalSystem\"
    --svcpassword=<password>
        specify the password of the indicated account
    --copy
        copy the execution file to target directory
    --uncopy
        do not copy the execution file to target directory, if you specify it then the argument \"--copy\" will invalid
    --manual
        Service Start use \"manual\" property
    --auto
        Service Start automatical when OS Start
    --x64
    	install 64-bit service
    --x86
    	install 32-bit service, default is 32bit
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

            

            $ch = $default if !$ch;

            

            return $ch;

}


# ------------------------------------------------------

# copy file to target directory

# ------------------------------------------------------
sub copyfile

{
	printf CMDFILE "if not exist $INSTALLROOT md $INSTALLROOT\n";
    printf CMDFILE "if not exist $EXEDIR md $EXEDIR\n";
    printf CMDFILE "if not exist $LOGDIR md $LOGDIR\n";
    printf CMDFILE "if not exist $CONFIGDIR md $CONFIGDIR\n";
    printf CMDFILE "if not exist $DOCDIR md $DOCDIR\n";
    if($ins_x64)
    {
    	printf CMDFILE "xcopy /fycsr bin64 $EXEDIR\n";
    }
    else
    {
    	printf CMDFILE "xcopy /fycsr bin $EXEDIR\n";
    }
	#printf CMDFILE "xcopy /fycsr bin $EXEDIR\n";
	printf CMDFILE "xcopy /fycsr etc $CONFIGDIR\n";
	printf CMDFILE "xcopy /fycsr doc $DOCDIR\n";
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

sub instservice

{
	my $profile = shift;
        return if !$profile;
        if($ins_x64)
        {
        	printf CMDFILE "cd bin64\n";
        }
        else
        {
        	printf CMDFILE "cd bin\n";
        }
        printf CMDFILE "ZQShell -i $profile TianShan \"$EXEDIR\"\\ZQShell.exe $SVCOID \"$SVCACCOUNT\" \"$SVCPASSWORD\" \"$EXEDIR\"\\ZQShellMsgs.dll $ins_property\n";
        printf CMDFILE "cd ..\n";
        printf CMDFILE "sc config $profile DisplayName= \"ZQ $profile\"\n";
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

sub GetRegistry($)
{
	my $profile = shift;
        return if !$profile;
        my $Registry_count=scalar(@{$profile});
    	$t=0;
    	my $key;
    	while($t<$Registry_count)
    	{
    		$key="LMachine/"."$$profile[$t][0]";
    		$Registry->Delimiter("/");
    		$key = $Registry->{$key};
    		if($key)
    		{
    			
    			$default_value=$key->GetValue($$profile[$t][2]);
    			$$profile[$t][3] = $default_value if (!($$profile[$t][3] =~ m/^\%/) && $default_value);
    			$t++;
    			
    	        }
    	        else
    	        {
    	        	$t++;
    	        }
        }
}

sub PromptRegistry($)
{
	my $profile = shift;
        return if !$profile;
        my $Registry_count=scalar(@{$profile});
	my $i=0;
	while($i<$Registry_count)
	{
		if ($$profile[$i][3] =~ m/^\%/)
	{
	    my @convert=split(/%/,$$profile[$i][3]);
	    $convert_num=scalar(@convert);
		$conv_temp=1;
		$serviceregis[$i][3]="";
		while($conv_temp<$convert_num)
		{
			$$profile[$i][3]=$$profile[$i][3]."$EN{$convert[$conv_temp]}"."$convert[$conv_temp+1]";
			$conv_temp++;
			$conv_temp++;
		}
	}
	$$profile[$i][3] = prompt("$$profile[$i][1]","$$profile[$i][3]");
	
        $$profile[$i][3]=$$profile[$i][3]."\\" if($$profile[$i][3] =~ m/.*\\$/);

	$i++;
        }
}



sub SetRegistry($)
{
	my $profile = shift;
        return if !$profile;
        my $Registry_count=scalar(@{$profile});
    	$tt=0;
    	my $key;
    	while($tt<$Registry_count)
    	{
    		$ValueName = $$profile[$tt][2];
    		$ValueData = $$profile[$tt][3];
    		$ValueType = $$profile[$tt][4];
    		$ValueData = hex ($ValueData) if($ValueData =~ m/^0x/);
    		$ValueData = pack("L", $ValueData) if($ValueType eq "REG_DWORD");
    		$key="LMachine/"."$$profile[$tt][0]";
    		$Registry->Delimiter("/");
    		$key = $Registry->{$key};
    		if(!$key)
    		{
    			$subKey=$$profile[$tt][0];
    			my $KeyName="LMachine";
    			my $SubKeyName=$$profile[$tt][0];
    			$Registry->Delimiter("/");
    			my($CurrentKey) = $Registry->{$KeyName};
    			my($Result) = $CurrentKey->CreateKey($SubKeyName);
    			die "faile to create key $SubKeyName" if(!$Result);
    			$key=$KeyName."/".$SubKeyName;
    			$key = $Registry->{$key};
    		}
    		$okay = $key->SetValue($ValueName, $ValueData, $ValueType);
    		$tt++;
        }
}

# ------------------------------------------------------

#delete registry key for uninstall

# ------------------------------------------------------

sub DeleteRegistry()
{
        my $Keyname1 = "LMachine/SOFTWARE/ZQ Interactive/TianShan/CurrentVersion/Services";
        my $Keyname2 = "LMachine/SYSTEM/CurrentControlSet/Services/Eventlog/Application";
        my $Keyname3 = "LMachine/SOFTWARE/ZQ Interactive/SNMPOID/CurrentVersion/Services";
        my $SubKeyname1 = "ServerLoad";
        my $SubKeyname2 = "ServerLoad_Shell";
        
        $tips= delete $Registry->{"$Keyname1/" .
                              "$SubKeyname1/"};
        $tips= delete $Registry->{"$Keyname1/" .
                              "$SubKeyname2/"};
	$tips= delete $Registry->{"$Keyname2/" .
                              "$SubKeyname1/"};
        $tips= delete $Registry->{"$Keyname2/" .
                              "$SubKeyname2/"}; 
	$tips= delete $Registry->{"$Keyname3/" .
                              "$SubKeyname1/"};  
}


# ------------------------------------------------------

#get file path and name from registry for upgrade

# ------------------------------------------------------
sub getpath()
{
	my $profile = shift;
	return if !$profile;
	$aat = trim($profile);
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

sub OSver()
{
	my $cmd = "systeminfo >systeminfo.txt";
	system($cmd);
	open OSVER, "<systeminfo.txt" or die ("fail to open systeminfo.txt");
	#unlink "systeminfo.txt";
	my $line;
	ver_next_line: while ($line = <OSVER>)
	{
		$_=$line;
		if((/^[\s]*System[\s]+Type:[\s]+.*?/)||(/^[\s]*System[\s]+type:[\s]+.*?/))
		{
			my @chars = split (/ /,$_);
			my $char_count=scalar(@chars);
			my @char = split (/-/,$chars[$char_count-2]);
			close OSVER if($char[0] =~ /^x64/i);
			unlink "systeminfo.txt";
			return (x64) if($char[0] =~ /^x64/i);
			close OSVER;
			unlink "systeminfo.txt";
			return (x86);
		}
		next ver_next_line;
	}
	printf "Can not get os is 32 or 64bit, please confirm you have install the right TianShan version again\n";
	sleep 3;
	close OSVER;
	unlink "systeminfo.txt";
}