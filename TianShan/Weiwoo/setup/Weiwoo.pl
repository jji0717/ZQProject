use Getopt::Long;
use Cwd;
use env;
use English;
use Win32::Registry;
use Win32::TieRegistry;


    require "profile.pl";
    &GetOptions(
				'help'           =>	\$ins_help,
				'list'           =>	\$ins_list,
                                'install'        =>     \$install,
                                'upgrade'        =>     \$upgrade,
                                'copy'           =>     \$copy,
                                'uncopy'           =>     \$uncopy,
                                'svcaccount=s'     =>     \$ins_svcaccount,
                                'svcpassword=s'    =>     \$ins_svcpassword,
               );
    usage() if($ins_help);              
    die "The installation must specify Install or Upgrade service by using \"--install\" or \"--upgrade\".For more detail, please using \"--help\"" if (!(($install||$upgrade)&&!$ins_help));
    $ins_servicesname="Weiwoo";
    while (($key, $valuess)= each (%ENV))
    {
    	$EN{$key} = $valuess;
    }
    
    
    $directory=prompt ("Input the directory for install WeiWoo","C:\\TianShan") if ($install);
    if ($upgrade)
    {
    	$key="LMachine/SoftWare/ZQ Interactive/TianShan/Currentversion/Services/Weiwoo_shell";
        $Registry->Delimiter("/");
    	$key = $Registry->{$key};
    	$ImagePath = $key->GetValue(ImagePath);
    	@InstallfilePath = getpath ($ImagePath);
    	$directory=prompt ("Input the directory for install Weiwoo","$InstallfilePath[2]");
    }
    $EXEDIR="$directory\\bin";
    $LOGDIR="$directory\\log";
    $CONFIGDIR="$directory\\etc";
    $DATADIR="$directory\\data";
    $MODULEDIR="$directory\\modules";
    $SVCACCOUNT=$ins_svcaccount if($ins_svcaccount);
    $SVCPASSWORD=$ins_svcpassword if($ins_svcpassword);
    %EN = (
		  TARGETDIR => $directory,
		  EXEDIR    => $EXEDIR,
		  LOGDIR    => $LOGDIR,
		  CONFIGDIR => $CONFIGDIR,
		  DATADIR   => $DATADIR,
		  MODULEDIR   => $MODULEDIR,
	     );
    
    
    my $file="Install.bat";
    open CMDFILE, ">$file";
    printf CMDFILE "setlocal\n";
    printf CMDFILE "set TARGETDIR=$directory\n";
    printf CMDFILE "set EXEDIR=%TARGETDIR%\\bin\n";
    printf CMDFILE "set LOGDIR=%TARGETDIR%\\log\n";
    printf CMDFILE "set CONFIGDIR=%TARGETDIR%\\etc\n";
    printf CMDFILE "set DATADIR=%TARGETDIR%\\data\n";
    printf CMDFILE "set MODULEDIR=%TARGETDIR%\\modules\n";
    getregistry() if($upgrade);
    copyfile() if(!$uncopy);
    instservice($ins_servicesname) if($install);
    promptregistry();
    print CMDFILE "endlocal\n";
    close CMDFILE;
    
    my $choise=getchoise();
	  my $number;
	  for($number=1;$number<100;$number++)
	  {
	    if(($choise =~ /^Y/i) || ($choise =~ /^N/i))
	    {
		$number=100;
	    }
	    else
	    {
		$choise=getchoise();
	    }
	  }
	  if($choise =~ /^Y/i)
	  {	
	    system ("$file");
	    setregistry();
	  }
	  unlink "$file";



sub copyfile()
{
	printf CMDFILE "if not exist $directory md $directory\n";
        printf CMDFILE "if not exist $EXEDIR md $EXEDIR\n";
        printf CMDFILE "if not exist $CONFIGDIR md $CONFIGDIR\n";
        printf CMDFILE "if not exist $LOGDIR md $LOGDIR\n";
        printf CMDFILE "if not exist $DATADIR md $DATADIR\n";
        printf CMDFILE "if not exist $MODULEDIR md $MODULEDIR\n";
        printf CMDFILE "xcopy /fycs bin $EXEDIR\n";
        printf CMDFILE "xcopy /fycs etc $CONFIGDIR\n";
        printf CMDFILE "xcopy /fycs modules $MODULEDIR\n";
}

sub instservice()
{
	my $profile = shift;
        return if !$profile;
        printf CMDFILE "cd bin\n";
        printf CMDFILE "ZQShell -i $profile TianShan \"$EXEDIR\"\\ZQShell.exe 1 $SVCACCOUNT \"$SVCPASSWORD\" \"$EXEDIR\"\\ZQShellMsgs.dll auto\n";
        printf CMDFILE "cd ..\n";
}

sub getregistry()
{
	my $count=scalar(@$ins_servicesname);
    	$t=0;
    	my $key;
    	while($t<$count)
    	{
    		$key="LMachine/"."$$ins_servicesname[$t][0]";
    		$Registry->Delimiter("/");
    		$key = $Registry->{$key};
    		if($key)
    		{
    			
    			$default_value=$key->GetValue($$ins_servicesname[$t][2]);
    			$$ins_servicesname[$t][3] = $default_value if (!($$ins_servicesname[$t][3] =~ m/^\%/));
    			$t++;
    	        }
    	        else
    	        {
    	        	$t++;
    	        }
        }
}

sub promptregistry()
{
	my $tmpcount=scalar(@$ins_servicesname);
	my $i=0;
	while($i<$tmpcount)
	{
		if ($$ins_servicesname[$i][3] =~ m/^\%/)
	{
	    my @convert=split(/%/,$$ins_servicesname[$i][3]);
	    $convert_num=scalar(@convert);
		$conv_temp=1;
		$$ins_servicesname[$i][3]="";
		while($conv_temp<$convert_num)
		{
		   $$ins_servicesname[$i][3]=$$ins_servicesname[$i][3]."$EN{$convert[$conv_temp]}"."$convert[$conv_temp+1]";
		   $conv_temp++;
	           $conv_temp++;
		}
	}
	$$ins_servicesname[$i][3] = prompt("$$ins_servicesname[$i][1]","$$ins_servicesname[$i][3]");
	
        $$ins_servicesname[$i][3]=$$ins_servicesname[$i][3]."\\" if($$ins_servicesname[$i][3] =~ m/.*\\$/);

	$i++;
        }
}

sub setregistry()
{
	my $countt=scalar(@$ins_servicesname);
    	$tt=0;
    	my $key;
    	while($tt<$countt)
    	{
    		$ValueName = $$ins_servicesname[$tt][2];
    		$ValueData = $$ins_servicesname[$tt][3];
    		$ValueType = $$ins_servicesname[$tt][4];
    		$ValueData = hex ($ValueData) if($ValueData =~ m/^0x/);
    		$ValueData = pack("L", $ValueData) if($ValueType eq "REG_DWORD");
    		$key="LMachine/"."$$ins_servicesname[$tt][0]";
    		$Registry->Delimiter("/");
    		$key = $Registry->{$key};
    		if(!$key)
    		{
    			$subKey=$$ins_servicesname[$tt][0];
    			my $KeyName="LMachine";
    			my $SubKeyName=$$ins_servicesname[$tt][0];
    			$Registry->Delimiter("/");
    			my($CurrentKey) = $Registry->{$KeyName};
    			my($Result) = $CurrentKey->CreateKey($SubKeyName);
    			die "faile to create key $SubKeyName" if(!$Result);
    			$key=$KeyName."/".$SubKeyName;
    			$key = $Registry->{$key};
    		}
    		$okay = $key->SetValue( $ValueName, $ValueData, $ValueType );
    		$tt++;
        }
	
	
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

sub getchoise()
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
    Weiwoo <--install | --upgrade> [--copy | --uncopy]
    Weiwoo --help
options
    --install
        install the service
    --upgrade
        upgrade the service
    --copy
        copy the execution file to the target directory and modify the registry
    --uncopy
        do not copy the execution file, modify the registry only        
    --svcaccount=<account>
        specify os account for install the service, default is LocalMachine.
    --svcpassword=<password>
        specify the password for os account, default is \"\".
    --help
        display this screen

EOF

exit 0;
}
