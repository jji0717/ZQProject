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
				'help'           =>	\$ins_help,
				'list'           =>	\$ins_list,
                                'service=s'      => 	\$ins_servicesname,
                                'install'        =>     \$install,
                                'upgrade'        =>     \$upgrade,
                                'copy'           =>     \$copy,
                                'uncopy'           =>     \$uncopy,
                                'installroot=s'    =>     \$ins_installroot,
                                'svcaccount=s'     =>     \$ins_svcaccount,
                                'svcpassword=s'    =>     \$ins_svcpassword,
               );

    die "The installation must specify a sevices by using --service=<servicename> and specify Install or Upgrade service by using \"--install\" or \"--upgrade\".For more detail, please using \"--help\"" if (!(($install||$upgrade)&&$ins_servicesname)&&!$ins_help&&!$ins_list);
    usage() if($ins_help);
    
    $servicesname_shell="$ins_servicesname"."_shell";
    
    require "profile.pl";
    
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

# match valid service name
          $length1=length($ins_servicesname);
	  foreach $serv (@All_Services)
	  {
	  	$length2=length($serv->{name});
	        if($length1 == $length2)
	  	{
	  		$ins_servicesname=$serv->{name} if ($ins_servicesname =~ /^$serv->{name}/i);
	  		$servicename=$serv if trim($ins_servicesname) eq trim($serv->{name});
	  		$servicesname_shell="$ins_servicesname"."_shell";
	        }
	  }
	  if(!$servicename)
	  {
	  	print ("A manual install must specify a valid service name by using --service=<servicename>, please using --list to print all valid service");
	  	exit 0;
	  }
	  
# get original installroot if upgrade the service	  
	  if($upgrade&&!$ins_installroot)
	  {
	  	$key_name="LMachine/System/CurrentControlSet/Services/$ins_servicesname";
                $Registry->Delimiter("/");
    	        $key_name = $Registry->{$key_name};
    	        die("The system haven't found original version, please install first") if(!$key_name);
	  	
	  	$key="LMachine/SoftWare/ZQ Interactive/DataOnDemand/Currentversion/Services/$servicesname_shell";
                $Registry->Delimiter("/");
    	        $key = $Registry->{$key};
    	        $ImagePath = $key->GetValue(ImagePath);
    	        @InstallfilePath = getpath ($ImagePath);
    	        $INSTALLROOT=$InstallfilePath[2];
    	        
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
	 # $SVCOID=${$servicename}{InstallID};
	  $EXEDIR="$INSTALLROOT\\Bin";
          $LOGDIR="$INSTALLROOT\\Logs";
          $CONFIGDIR="$INSTALLROOT\\etc";
          $DATADIR="$INSTALLROOT\\Data";
          $SAFESTOREPATH="$INSTALLROOT\\SafeStore";
	  %EN = (
		  TARGETDIR => $INSTALLROOT,
		  EXEDIR    => $EXEDIR,
		  LOGDIR    => $LOGDIR,
		  DATADIR    => $DATADIR,
		  CONFIGDIR   => $CONFIGDIR,
		  );
	  my $file = "$ins_servicesname.bat";
	  open CMDFILE, ">$file";
	  printf CMDFILE "setlocal\n";
	  printf CMDFILE "set TARGETDIR=$EN{TARGETDIR}\n";
	  printf CMDFILE "set EXEDIR=$EN{EXEDIR}\n";
          printf CMDFILE "set LOGDIR=$EN{LOGDIR}\n";
          printf CMDFILE "set DATADIR=$EN{DATADIR}\n";
          printf CMDFILE "set CONFIGDIR=$EN{CONFIGDIR}\n";
	  printf CMDFILE "set SVCACCOUNT=$SVCACCOUNT\n";
          printf CMDFILE "set SVCPASSWORD=$SVCPASSWORD\n";
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
	        system ("$file");
	        SetRegistry(${$servicename}{Registry});
	    
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
    DataOnDemandSetup <options>
    DataOnDemandSetup is a simple Perl program that can install DataOnDemand projects based on the project profile
options
    --service=<sname>
        specify the service to install
    --install
        install the service
    --upgrade
        upgrade the service
    --installroot=<path>
        specify the location where the service will work, default value is \"C:\\DataOnDemand\"
    --svcaccount=<account>
        specify the OS account for install service, default value is \"LocalSystem\"
    --svcpassword=<password>
        specify the password of the indicated account
    --copy
        copy the execution file to target directory
    --uncopy
        do not copy the execution file to target directory, if you specify it then the argument \"--copy\" will invalid.
    --list
        list all the service can be installed by the script
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
        printf CMDFILE "if not exist $CONFIGDIR md $CONFIGDIR\n";
	printf CMDFILE "xcopy /fycsr bin $EXEDIR\n";
	printf CMDFILE "xcopy /fycsr etc $CONFIGDIR\n";
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
        printf CMDFILE "cd bin\n";
        printf CMDFILE "ZQShell -i $profile DataOnDemand \"$EXEDIR\"\\ZQShell.exe $SVCOID \"$SVCACCOUNT\" \"$SVCPASSWORD\" \"$EXEDIR\"\\ZQShellMsgs.dll auto\n";
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