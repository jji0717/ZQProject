@rem = '-*- Perl -*-';
@rem = '
@echo off
perl -S %0 %*
goto EndOfPerl
';

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
				'help'		=>	\$ins_help,
				'list'		=>	\$ins_list,
                                'service=s'	=> 	\$ins_servicesname,
                                'install'	=>	\$install,
                                'upgrade'	=>	\$upgrade,
                                'copy'		=>	\$copy,
                                'uncopy'	=>	\$uncopy,
                                'installroot=s'	=>	\$ins_installroot,
                                'svcaccount=s'	=>	\$ins_svcaccount,
                                'svcpassword=s'	=>	\$ins_svcpassword,
                                'auto'		=>	\$ins_auto,
                                'manual'	=>	\$ins_manual,
                                'x64'		=>	\$ins_x64,
                                'x86'		=>	\$ins_x86,
                                'clusterid=s'	=>	\$ins_clusterid,
                                'instance=s'	=>	\$ins_id,	
                                'holdctf'	=>	\$ins_holdctf,	
               );

    die "The installation must specify a sevices by using --service=<servicename> and specify Install or Upgrade service by using \"--install\" or \"--upgrade\".For more detail, please using \"--help\"" if (!(($install||$upgrade)&&$ins_servicesname)&&!$ins_help&&!$ins_list);
    usage() if($ins_help);
    
    $ins_property="manual" if(!$ins_auto);
    $ins_property="auto" if($ins_auto);
    die("You should not give two conflicting choice, for more detail please use \"--help\"") if($ins_auto&&$ins_manual);
    
    if (!(($ins_servicesname =~ /^MOD/i)||($ins_servicesname =~ /^MediaClusterCS/i)||($ins_servicesname =~ /^NSS/i)||($ins_servicesname =~ /^TSPump/i)||($ins_servicesname =~ /^GBVSS/i)||($ins_servicesname =~ /^GBCS/i)))
    {
    	require "profile.pl";
    	
    	foreach $serv (@All_Services)
	{
		$ins_servicesname=$serv->{name} if (lc($ins_servicesname) eq lc($serv->{name}));
	 	$servicename=$serv if trim($ins_servicesname) eq trim($serv->{name});
	  	$servicesname_shell="$ins_servicesname"."_shell";
	}
    }
    elsif($install)
    {
    	$ins_servicesname = "MediaClusterCS" if($ins_servicesname =~ /^MediaClusterCS/i);
	$ins_servicesname = "MODSvc" if($ins_servicesname =~ /^MOD/i);
	$ins_servicesname = "NSS" if($ins_servicesname =~ /^NSS/i);
	$ins_servicesname = "TSPump" if($ins_servicesname =~ /^TSPump/i);
	$ins_servicesname = "GBVSS" if($ins_servicesname =~ /^GBVSS/i);
	$ins_servicesname = "GBCS" if($ins_servicesname =~ /^GBCS/i);
	$servicename = $ins_servicesname;	
	$servicesname_shell="$ins_servicesname"."_shell";
	$Exist_ins = getinsid($ins_servicesname);
	
	if($Exist_ins > 9)
	{
		print "Instance can not exceeded 9, quit install\n";
		exit;
	}
	
	if($Exist_ins)
	{
	       	$ins_servicesname = "$ins_servicesname"."$Exist_ins" if($ins_servicesname=~ /^MediaClusterCS/i);
	       	$ins_servicesname = "MOD"."$Exist_ins" if ($ins_servicesname=~ /^MODSvc/i);
	       	$ins_servicesname = "NSS"."$Exist_ins" if ($ins_servicesname=~ /^NSS/i);
	       	$ins_servicesname = "TSPump"."$Exist_ins" if ($ins_servicesname=~ /^TSPump/i);
	       	$ins_servicesname = "GBVSS"."$Exist_ins" if ($ins_servicesname=~ /^GBVSS/i);
	       	$ins_servicesname = "GBCS"."$Exist_ins" if ($ins_servicesname=~ /^GBCS/i);
	}
	$Exist_ins = 1 if(!$Exist_ins);
	$servicesname_shell = "$ins_servicesname"."_shell";
	require "profile.pl";
    }
    else
    {
    	$ins_servicesname = "MediaClusterCS" if(lc($ins_servicesname) eq lc(MediaClusterCS));
	$ins_servicesname = "MODSvc" if(lc($ins_servicesname) eq lc(MODSvc));
	$ins_servicesname = "NSS" if(lc($ins_servicesname) eq lc(NSS));
	$ins_servicesname = "TSPump" if(lc($ins_servicesname) eq lc(TSPump));
	$ins_servicesname = "GBVSS" if(lc($ins_servicesname) eq lc(GBVSS));
	$ins_servicesname = "GBCS" if(lc($ins_servicesname) eq lc(GBCS));
	$servicename = "MediaClusterCS" if($ins_servicesname =~ /^MediaClusterCS/i);
	$servicename = "MODSvc" if($ins_servicesname =~ /^MOD/i);
	$servicename = "NSS" if($ins_servicesname =~ /^NSS/i);
	$servicename = "TSPump" if($ins_servicesname =~ /^TSPump/i);
	$servicename = "GBVSS" if($ins_servicesname =~ /^GBVSS/i);
	$servicename = "GBCS" if($ins_servicesname =~ /^GBCS/i);
	$servicesname_shell="$ins_servicesname"."_shell";
	require "profile.pl";
    }
    
    if((!$servicename)&&(!$ins_list))
    {
	print ("A manual install must specify a valid service name by using --service=<servicename>, please using --list to print all valid service");
	exit 0;
    }
    
    
# List avaiable service can be installed   
    if ($ins_list)
    {
	print "List all service can be installed:\n";
	foreach $services (@All_Services)
	{
		print "$services->{name}\n";
	}
	exit 0;
    }
    
    while (($key, $valuess)= each (%ENV))
    {
    	$EN{$key} = $valuess;
    }
    
    $state = "install" if($install);
    $state = "upgrade" if($upgrade);
    $ins_x86=1 if((!$ins_x64)&&(!$ins_x86));
    $ins_bit = "32bit" if($ins_x86);
    $ins_bit = "64bit" if($ins_x64);
    
#Confirm Operation systerm version
    if((!$ins_list)&&($servicename))
    {
    	$OSver=OSver();
    	die ("64bit server can not installed on 32bit machine\n") if(($OSver eq "x86")&&($ins_x64));
    	if(($OSver eq "x64")&&($ins_x86))
    	{
    		my $confirm = prompt("the Operation System is $OSver, do you still want to $state $ins_bit TianShan on this server, if yes input \"Y\", otherwise, press \"Enter\"","N");
    		exit 0 if($confirm =~ /^N/i);
    		printf "start to install 32bit service on this server\n";
    		$ins_32On64=1;
    		$key_title="LMachine/SoftWare/Wow6432Node/";
    	}
    	else
    	{
    		$key_title="LMachine/SoftWare/";
    	}
    }
    
# Windows snmp installed or not    
    $key_snmp="LMachine/System/CurrentControlSet/Services/SNMP";
    $Registry->Delimiter("/");
    $key_snmp = $Registry->{$key_snmp};
    
    if (($ins_servicesname =~ /^Sentry/i)&&(!$key_snmp))
    {
    	$Not_ins = prompt("SNMP Service have not been installed yet,suggest install SNMP first. Otherwise will add SNMP registry directly.Quit install, press \"Enter\", otherwise input \"I\"","Q");
    	exit 0 if ($Not_ins =~ /^q/i);
    	printf "\nStart to $state sentry\n\n" if($Not_ins =~ /^i/i);
    	sleep 2 if($Not_ins =~ /^i/i);
    }
    	  
# get original installroot if upgrade the service
	  if ($upgrade) 
	  {
	  	$key_name="LMachine/System/CurrentControlSet/Services/$ins_servicesname";
                $Registry->Delimiter("/");
    	        $key_name = $Registry->{$key_name};
    	        die("The system haven't found original version, please install first") if(!$key_name);
    	        
    	        if(!$ins_installroot)
    	        {
    	        	$key="$key_title"."ZQ Interactive/TianShan/Currentversion/Services/$servicesname_shell";
                	$Registry->Delimiter("/");
    	        	$key = $Registry->{$key};
    	        	if((!$key)&&(($ins_servicesname =~ /^DOD/i)||($ins_servicesname =~ /^Data/i)))
    	        	{
    	        		$key="$key_title"."ZQ Interactive/DataOnDemand/Currentversion/Services/$servicesname_shell"; 	        		
    	        		$Registry->Delimiter("/");
    	        		$key = $Registry->{$key};
    	        	}
    	        	$ImagePath = $key->GetValue(ImagePath);
    	        	@InstallfilePath = getpath ($ImagePath);
    	        	$INSTALLROOT=$InstallfilePath[2];
    	        }
	  }
	  
	  @serviceregis=@{${$servicename}{Registry}};
	  GetRegistry(${$servicename}{Registry}) if($upgrade);
	  $INSTALLROOT=fixPathToOS($ins_installroot) if $ins_installroot;
	  if($ins_svcaccount)
	  {
	  	$SVCACCOUNT=$ins_svcaccount;
	  	$SVCACCOUNT=".\\"."$SVCACCOUNT" if ((!($SVCACCOUNT =~ /^LocalSystem/i))&&(!($SVCACCOUNT =~ m/.*\\/)));
	  }
	  $SVCPASSWORD=$ins_svcpassword if $ins_svcpassword;
	  if ((($ins_servicesname=~ /^MODSvc/i)||($ins_servicesname=~ /^MediaClusterCS/i)||($ins_servicesname=~ /^NSS/i)||($ins_servicesname=~ /^TSPump/i)||($ins_servicesname =~ /^GBVSS/i)||($ins_servicesname =~ /^GBCS/i))&&($Exist_ins==1))
	  {
	  	$ServiceOID = $servicename->{ServiceOID};
	  }
	  else
	  {
	  	$ServiceOID = $servicename->{ServiceOID}+$Exist_ins*10;
	  }
	  $EXEDIR="$INSTALLROOT\\bin";
          $LOGDIR="$INSTALLROOT\\logs";
          $LOGDIR="$INSTALLROOT\\logs2" if ($ins_servicesname eq "RtspProxy2");
          $CONFIGDIR="$INSTALLROOT\\etc";
          $CONFIGDIR="$INSTALLROOT\\etc2" if ($ins_servicesname eq "RtspProxy2");
          $DATADIR="$INSTALLROOT\\data";
          $DATADIR="$INSTALLROOT\\data2" if ($ins_servicesname eq "RtspProxy2");
          $DOCDIR="$INSTALLROOT\\doc";
          $SDKPATH="$INSTALLROOT\\SDK";
          $MODULEDIR="$INSTALLROOT\\modules";
	  $CRASHDUMPPATH="$LOGDIR\\crashdump";
	  $UTILSDIR="$INSTALLROOT\\utils";
	  $WEBCTRLDIR="$INSTALLROOT\\webctrl";
	  %EN = (
		  TARGETDIR => $INSTALLROOT,
		  EXEDIR    => $EXEDIR,
		  LOGDIR    => $LOGDIR,
		  DATADIR    => $DATADIR,
		  DOCDIR    => $DOCDIR,
		  MODULEDIR    => $MODULEDIR,
		  CONFIGDIR   => $CONFIGDIR,
		  SDKPATH   => $SDKPATH,
		  CRASHDUMPPATH => $CRASHDUMPPATH,
		  UTILSDIR    => $UTILSDIR,
		  WEBCTRLDIR    => $WEBCTRLDIR,
		  );
	  @configfiles=@{${$servicename}{configfile}};
	  my $file = "$ins_servicesname.bat";
	  open CMDFILE, ">$file";
	  printf CMDFILE "setlocal\n";
	  printf CMDFILE "set TARGETDIR=$EN{TARGETDIR}\n";
	  printf CMDFILE "set EXEDIR=$EN{EXEDIR}\n";
          printf CMDFILE "set LOGDIR=$EN{LOGDIR}\n";
          printf CMDFILE "set DATADIR=$EN{DATADIR}\n";
          printf CMDFILE "set DOCDIR=$EN{DOCDIR}\n";
          printf CMDFILE "set MODULEDIR=$EN{MODULEDIR}\n";
	  printf CMDFILE "set CONFIGDIR=$EN{CONFIGDIR}\n";
	  printf CMDFILE "set SDKPATH=$EN{SDKPATH}\n";
	  printf CMDFILE "set CRASHDUMPPATH=$EN{CRASHDUMPPATH}\n";
	  printf CMDFILE "set UTILSDIR=$EN{UTILSDIR}\n";
	  printf CMDFILE "set WEBCTRLDIR=$EN{WEBCTRLDIR}\n";
          printf CMDFILE "set SVCACCOUNT=$SVCACCOUNT\n";
          printf CMDFILE "set SVCPASSWORD=$SVCPASSWORD\n";
          printf CMDFILE "if exist $EXEDIR\\jvm.dll del /Q $EXEDIR\\jvm.dll\n";
	  printf CMDFILE "if not exist $CONFIGDIR md $CONFIGDIR\n";
	  foreach $conf_filename (@configfiles)
          {
        	$conf_filename = "$conf_filename"."*";
        	printf CMDFILE "xcopy /fycsr etc\\$conf_filename $CONFIGDIR\n";
          }
	  printf CMDFILE "xcopy /fycsr etc\\TianShan* $CONFIGDIR\n";
	  copyfile() if (!$uncopy);
	  instservice($ins_servicesname) if($install);
	  PromptRegistry(${$servicename}{Registry});
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
	  	system ("net stop snmp 2>error") if(($key_snmp)&&(!$uncopy));
	  	#system ("ren $EXEDIR bin_bk") if(!$uncopy);
	        system ("$file");
	        SetRegistry(${$servicename}{Registry});
	        if((!$upgrade)&&(($ins_servicesname =~ /^MOD/i)||($ins_servicesname =~ /^MediaClusterCS/i)||($ins_servicesname =~ /^NSS/i)||($ins_servicesname =~ /^TSPump/i)||($ins_servicesname =~ /^GBVSS/i)||($ins_servicesname =~ /^GBCS/i)))
	        {
    			$key="$key_title"."ZQ Interactive/TianShan/Currentversion/Services/$servicesname_shell";
    			$Registry->Delimiter("/");
    			$key = $Registry->{$key};    			
    			$okay = $key->SetValue(Argument, $Exist_ins, REG_SZ);
    			if($Exist_ins>0)
    			{
    				if($Exist_ins>1)
    				{
    					$instanceID = $Exist_ins;
    				}
    				else
    				{
    					$instanceID = $Exist_ins-1;
  				}
    				$instanceID = pack("L", $instanceID);
    				print "instanceID=$instanceID\n";
    				$okay = $key->SetValue(InstanceId, $instanceID, REG_DWORD);
    			}
	        }
	        confsnmp() if($ins_servicesname =~ /^Sentry/i);
	        
#If service is nodecs or clustercs, verify class path have been added or not, if not added it	

		# set classpath        
	        #if (($ins_servicesname =~ /^EventGateway/i)||($ins_servicesname =~ /^DODApp/i))
	        #{
	        	$ClassPath = "LMachine/SYSTEM/CurrentControlSet/Control/Session Manager/Environment";
	        	$Registry->Delimiter("/");
    			$ClassPath = $Registry->{$ClassPath};
    			$ClassPath_Ori = $ClassPath->GetValue(classpath);
    			$ClassPath_Real = "$EXEDIR\\"."java\\JMSClient";
    			$ClassPath_Real2 = "$EXEDIR\\"."java\\JMSClient\\jbossall-client.jar";
    			$ClassPath_Real3 = "$EXEDIR\\"."java\\JMSClient\\JndiClient.jar";
    			if ($ClassPath_Ori)
    			{
    				@Classvalues = split (/;/,$ClassPath_Ori);
    				$real1;
    				$real2;
    				$real3;
    				foreach $Classvalue (@Classvalues)
    				{
    					$real1 = 1 if ($Classvalue eq $ClassPath_Real);
    					$real2 = 1 if ($Classvalue eq $ClassPath_Real2);
    					$real3 = 1 if ($Classvalue eq $ClassPath_Real3);
    				}
    				$ClassPath_Ori = "$ClassPath_Ori".";$ClassPath_Real" if(!$real1);
    				$ClassPath_Ori = "$ClassPath_Ori".";$ClassPath_Real2" if(!$real2);
    				$ClassPath_Ori = "$ClassPath_Ori".";$ClassPath_Real3" if(!$real3);
    				$okay=$ClassPath->SetValue(classpath, $ClassPath_Ori, REG_SZ) if(!($real1&&$real2&&$real3));
    			}
    			else
    			{
    				$ClassPath_Ori = "$ClassPath_Real;"."$ClassPath_Real2;"."$ClassPath_Real3";
    				$okay=$ClassPath->SetValue(classpath, $ClassPath_Ori, REG_SZ);
    			}
    			
	        #}
	        modifyOID();
	        system ("net start snmp 2>error") if(($key_snmp)&&(!$uncopy));
	        unlink "error";
	  }
	  unlink "$file";
    
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
    TianShanSetup <options>
    TianShanSetup is a simple Perl program that can install TianShan projects based on the project profile
options
    --service=<sname>
        specify the service to install
    --install
        install the service
    --upgrade
        upgrade the service
    --installroot=<path>
        specify the location where the service will work, default value is \"C:\\TianShan\"
    --svcaccount=<account>
        specify the OS account for install service, default value is \"LocalSystem\"
    --svcpassword=<password>
        specify the password of the indicated account
    --copy
        copy the execution file to target directory
    --uncopy
        do not copy the execution file to target directory, if you specify it then the argument \"--copy\" will invalid
    --auto
        Service start automatical after OS starts
    --manual
        Service should start manually after OS starts
    --list
        list all the service can be installed by the script
    --x64
    	install 64-bit service
    --x86
    	install 32-bit service, default is 32bit
    --instance=<instanceid>
    	instance id
    --cluterid=<clusterid>
    	mediaclusterid, add it at service description.
    --holdctf
    	hold commontrickfile original version
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
        printf CMDFILE "if not exist $DATADIR md $DATADIR\n";
        printf CMDFILE "if not exist $DATADIR\\runtime md $DATADIR\\runtime\n";
        printf CMDFILE "if not exist $DOCDIR md $DOCDIR\n";
        printf CMDFILE "if not exist $MODULEDIR md $MODULEDIR\n";
        printf CMDFILE "if not exist $SDKPATH md $SDKPATH\n";
        printf CMDFILE "if not exist $UTILSDIR md $UTILSDIR\n";
        printf CMDFILE "if not exist $WEBCTRLDIR md $WEBCTRLDIR\n";
        printf CMDFILE "if not exist $CRASHDUMPPATH md $CRASHDUMPPATH\n";
        printf CMDFILE "if not exist $LOGDIR\\rtrec md $LOGDIR\\rtrec\n";
        if($ins_x64)
        {
        	printf CMDFILE "xcopy /fycsr bin64 $EXEDIR\n";
		printf CMDFILE "xcopy /fycsr modules64 $MODULEDIR\n";
		printf CMDFILE "xcopy /fycsr utils64 $UTILSDIR\n";
		printf CMDFILE "xcopy /fycsr ctf\\bin64 $EXEDIR\n" if!($ins_holdctf);
		printf CMDFILE "xcopy /fycsr utils64\\ngod2Event_template.mdb $DATADIR\\\n";
		#printf CMDFILE "if not exist $DATADIR\\ngod2Event.mdb ren $DATADIR\\ngod2Event_template.mdb ngod2Event.mdb\n";
        }
        else
        {
        	printf CMDFILE "xcopy /fycsr bin $EXEDIR\n";
		printf CMDFILE "xcopy /fycsr modules $MODULEDIR\n";
		printf CMDFILE "xcopy /fycsr utils $UTILSDIR\n";
		printf CMDFILE "xcopy /fycsr ctf\\bin $EXEDIR\n" if!($ins_holdctf);
		printf CMDFILE "xcopy /fycsr utils\\ngod2Event_template.mdb $DATADIR\\\n";
		#printf CMDFILE "if not exist $DATADIR\\ngod2Event.mdb ren $DATADIR\\ngod2Event_template.mdb ngod2Event.mdb\n";
        }
	printf CMDFILE "if exist doc\\ xcopy /fycsr doc $DOCDIR\n";
	printf CMDFILE "xcopy /fycsr SDK $SDKPATH\n";
	printf CMDFILE "xcopy /fycsr webctrl $WEBCTRLDIR\n";
	printf CMDFILE "xcopy /fycsr TianShanSetup.bat $INSTALLROOT\n";
	printf CMDFILE "xcopy /fycsr profile.pl $INSTALLROOT\n";
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
        printf CMDFILE "cd bin\n" if(!$ins_x64);
        printf CMDFILE "cd bin64\n" if($ins_x64);
        printf CMDFILE "ZQShell -i $profile TianShan \"$EXEDIR\"\\ZQShell.exe $ServiceOID \"$SVCACCOUNT\" \"$SVCPASSWORD\" \"$EXEDIR\"\\ZQShellMsgs.dll $ins_property\n";
        printf CMDFILE "cd ..\n";
        printf CMDFILE "sc config $profile DisplayName= \"ZQ $profile\"\n";
        printf CMDFILE "sc description $profile \"$ins_clusterid\"\n" if(($ins_clusterid) && ($profile =~ /^MediaClusterCS/i ));
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
    		if(($ins_32On64)&&($$profile[$t][0] =~ /^SOFTWARE/i))
    		{
    			@orikey = split (/SOFTWARE\\/,$$profile[$t][0]);
    			$newkey = "SOFTWARE\\Wow6432Node\\"."$orikey[1]";
    		}
    		else
    		{
    			$newkey = $$profile[$t][0];
    		}
    		$key="LMachine/"."$newkey";
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
    		if(($ins_32On64)&&($$profile[$tt][0] =~ /^SOFTWARE/i))
    		{
    			@orikey = split (/SOFTWARE\\/,$$profile[$tt][0]);
    			$newkey = "SOFTWARE\\Wow6432Node\\"."$orikey[1]";
    		}
    		else
    		{
    			$newkey = $$profile[$tt][0];
    		}
    		$key="LMachine/"."$newkey";
    		$Registry->Delimiter("/");
    		$key = $Registry->{$key};
    		if(!$key)
    		{
    			$subKey=$newkey;
    			my $KeyName="LMachine";
    			my $SubKeyName=$newkey;
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


#for support install more than one instance each machine
sub getinsid($)
{
	my $profile = shift;
        return if !$profile;
        return ($ins_id) if $ins_id;
        $profile = "MOD" if($profile =~ /^MODSvc/i);
	my $ZQPath="LMachine/SoftWare/"."ZQ Interactive/TianShan/CurrentVersion/Services";
	$Registry->Delimiter("/");
	$ZQPath = $Registry->{$ZQPath};
	return 0 if(!$ZQPath);
	$ID = 0;
	if($ZQPath)
	{
		@ZQServs = $ZQPath->SubKeyNames;
	        $syspath="LMachine/System/CurrentControlSet/Services";
	        $syspath = $Registry->{$syspath};
	        @SysServs = $syspath->SubKeyNames;
	        foreach $ZQServ (@ZQServs)
	        {
	        	if(($ZQServ =~ /^$profile/i)&&!($ZQServ =~ m/.*_shell\Z/i))
	        	{
	        		foreach $SysServ (@SysServs)
	        		{
	        			if(lc($SysServ) eq lc($ZQServ))
	        			{
	        				push(@installedService, $ZQServ);
	        				@Service_ID = split (/$profile/,$ZQServ);
	        				$ID_Max = $Service_ID[1];
	        				push(@ID_Max, $ID_Max);	
	        			}
	        		}
	        	}
		}
		if(@ID_Max)
	        {
		   	@ID_Max = sort {$a <=> $b} @ID_Max;
	        	$ID_MaxC = scalar(@ID_Max);
	        	$ID_Max = $ID_Max[$ID_MaxC-1]+1;
	        }
		$ID_Max=$ID_Max+1 if($ID_Max == 1);
		return $ID_Max;
	}
}

sub createKey($)
{
	my $profile = shift;
        return if !$profile;
        my $Server = "LMachine";
        my($CurrentKey) = $Registry->{$Server};
	my($Result) = $CurrentKey->CreateKey($profile);
	die "failed to create key $profile" if(!$Result);
}

sub confsnmp()
{
	my $snmp_parameters= "System/CurrentControlSet/Services/SNMP/Parameters";
	my $key_ExtensionAgents = "$snmp_parameters"."/ExtensionAgents";
	my $key_ValidCommunities = "$snmp_parameters"."/ValidCommunities";
	my $key_PermittedManagers = "$snmp_parameters"."/PermittedManagers";

	my $Confirm_ExtensionAgents = LMachine->{$key_ExtensionAgents};
	my $Confirm_ValidCommunities = LMachine->{$key_ValidCommunities};
	my $Confirm_PermittedManagers = LMachine->{$key_PermittedManagers};
	createKey($key_ExtensionAgents) if(!$Confirm_ExtensionAgents);
	createKey($key_ValidCommunities) if(!$Confirm_ValidCommunities);
	createKey($key_PermittedManagers) if(!$Confirm_PermittedManagers);
	
	$Registry->Delimiter("/");
	
	$AgentPath="LMachine/System/CurrentControlSet/Services/SNMP/Parameters/ExtensionAgents";
	$CommuPath="LMachine/System/CurrentControlSet/Services/SNMP/Parameters/ValidCommunities";
	$PermitMgrPath="LMachine/System/CurrentControlSet/Services/SNMP/Parameters/PermittedManagers";
	$Registry->Delimiter("/");
    	$AgentPath = $Registry->{$AgentPath};
    	$CommuPath = $Registry->{$CommuPath};
    	$PermitMgrPath = $Registry->{$PermitMgrPath};
    	
	@AgentValues = $AgentPath->ValueNames;
	$Agent_Count = scalar (@AgentValues);
	
	@AgentValues = sort {$a <=> $b} @AgentValues;
	$Agent_value = $AgentValues[$Agent_Count-1]+1;
	$Agent_data = "SOFTWARE\\ZQ Interactive\\ZQSnmpExtension" if(!$ins_32On64);
	$Agent_data = "SOFTWARE\\Wow6432Node\\ZQ Interactive\\ZQSnmpExtension" if($ins_32On64);
	
	$NGODSnmp_data = "SOFTWARE\\ZQ Interactive\\NgodSnmp" if(!$ins_32On64);
	$NGODSnmp_data = "SOFTWARE\\Wow6432Node\\ZQ Interactive\\NgodSnmp" if($ins_32On64);
	my $exist_Agent;
	my $exist_Ngod;
	
# Confirm SOFTWARE\\ZQ Interactive\\ZQSnmpExtension add or not
	foreach $AgentValue (@AgentValues)
	{
		$age_data = $AgentPath->GetValue($AgentValue);
		$exist_Agent = 1 if(lc($age_data) eq lc($Agent_data));
		$exist_Ngod = 1 if(lc($age_data) eq lc($NGODSnmp_data));
	}
	if(!$exist_Agent)
	{
		$okay1 = $AgentPath->SetValue($Agent_value, $Agent_data, REG_SZ);
		die ("fail to add $Agent_value") if !$okay1;
		$Agent_value=$Agent_value+1;
	}
	if(!$exist_Ngod)
	{
		
		$okay2 = $AgentPath->SetValue($Agent_value, $NGODSnmp_data, REG_SZ);
		die ("fail to add $Agent_value") if !$okay2;
	}
	
	$CommuData = pack("L", 8);
	$okay2 = $CommuPath->SetValue(TianShan, $CommuData, REG_DWORD);
	die ("fail to add Community TianShan") if !$okay2;
	#$okay3 = $PermitMgrPath->SetValue(1, '127.0.0.1', REG_SZ);
	#die ("fail to add PermitMgr") if !$okay3;
	
	my @perkeys = $PermitMgrPath->ValueNames;
	my $perkey;
	foreach $perkey (@perkeys)
	{
	    delete $PermitMgrPath->{$perkey};
	}	
}

# uniform ServiceOID on each server
sub modifyOID()
{
	$subkey_name="$key_title"."ZQ Interactive/SNMPOID/CurrentVersion/Services";
	$Registry->Delimiter("/");
    	$subkey_name = $Registry->{$subkey_name};
    	if($subkey_name)
    	{
    		@subKeyNames= $subkey_name->SubKeyNames;
	  	$keyNums = scalar(@subKeyNames);
	  	
	  	foreach $subkey_names (@subKeyNames)
	  	{
	  		$keynamess="$key_title"."ZQ Interactive/SNMPOID/CurrentVersion/Services/"."$subkey_names";
	  		$Registry->Delimiter("/");
    	                $keynamess = $Registry->{$keynamess};
	  		$Right_OID = $subkey_names -> {ServiceOID};
	  		if($Right_OID)
	  		{
	  			$Right_OID = pack("L", $Right_OID);
	  			$okay = $keynamess->SetValue(ServiceOID, $Right_OID, REG_DWORD);
	  		}
	  	}
	}
}

#confirm OS version is 32-bit or 64-bit
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

endprog:
__END__

:EndOfPerl