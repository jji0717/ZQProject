use Getopt::Long;
use Win32;
use Cwd;

&main();
sub main()
{
    require "profile.pl";
    &GetOptions(
				'help'           =>	\$ins_help,
                                'install'        =>     \$install,
                                'upgrade'        =>     \$upgrade,
                                'mode=s'         =>     \$ins_mode,
               );
    if($ins_help)
    {
        usage();
    }
    if ((!($install||$upgrade)||!$ins_mode)&&!$ins_help)
    {
    	print "Please specify Install or Upgrade MODPlugin by using \"--install\" or \"--upgrade\" and WFES mode by using \"--mode=single\" or \"--mode=multiple\"";
    	exit 0;
    }
    if(!(($ins_mode eq "single") || ($ins_mode eq "multiple")))
    {
		printf "Please speicify WFES mode by using --mode=\"single\" or \"multiple\"";
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
    $ins_servicesname="SrvLoad";
    
if($installstate && $ins_mode)    
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
		$a[0]=trim($a[0]);
		if (($a[0] eq "AppSitesConfigurationFile") && ($ins_mode eq "single"))
		{
			system ("reg delete HKLM\\Software\\SeaChange\\ITV\\CurrentVersion\\services\\SrvLoad /v AppSitesConfigurationFile /f>temp.txt");
		    	unlink "temp.txt";
		}
		while($ii<$cc)
		{
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

    $directory=prompt ("Input the directory for install SrvLoad","C:\\SrvLoad");
    $EXEDIR="$directory\\exe";
    $LOGDIR="$directory\\log";
    $SVCACCOUNT=".\\SeaChange";
    $SVCPASSWD="SeaChange";
    %EN = (
		  TARGETDIR => $directory,
		  EXEDIR    => $EXEDIR,
		  LOGDIR    => $LOGDIR,
		  ITVROOT   => "c:\\itv",
		  ITVEXEROOT  => "c:\\itv\\exe",
	     );
    mkdir "$directory";
    mkdir "$EXEDIR";
    mkdir "$LOGDIR";
    
    my $file="Install.bat";
    open CMDFILE, ">$file";
    printf CMDFILE "setlocal\n";
    printf CMDFILE "set ITVEXEROOT=C:\\ITV\\EXE\n";
    printf CMDFILE "if not exist $directory md $directory\n";
    printf CMDFILE "if not exist $EXEDIR md $EXEDIR\n";
    printf CMDFILE "if not exist $LOGDIR md $LOGDIR\n";
    printf CMDFILE "xcopy /fycs bin $EXEDIR\n";
    if ($ins_mode =="multiple")
    {
    	printf CMDFILE "if not exist $EXEDIR\\SrvLoadAppsitesCfg.xml xcopy /fycs config $EXEDIR\n";
    }
    printf CMDFILE "if not exist \"%ITVEXEROOT%\"\\instserv.exe copy \"instserv.exe\" \"$EXEDIR\" /Y\n";
    printf CMDFILE "if not exist \"%ITVEXEROOT%\"\\instserv.exe copy \"srvshell.exe\" \"$EXEDIR\" /Y\n";
    if ($installstate eq "I")
    {
    printf CMDFILE "if exist \"%ITVEXEROOT%\"\\srvshell.exe instserv SrvLoad \"%ITVEXEROOT%\"\\srvshell.exe local $SVCACCOUNT $SVCPASSWD auto own\n";
    printf CMDFILE "if not exist \"%ITVEXEROOT%\"\\srvshell.exe instserv SrvLoad \"$EXEDIR\"\\srvshell.exe local $SVCACCOUNT $SVCPASSWD auto own\n";
    printf CMDFILE "instserv SrvLoad displayname=\"SrvLoad\"\n";
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
	if ($$ins_servicesname[$i][2] eq "AppSitesConfigurationFile")
	{
		if ($ins_mode eq "multiple")
		{
			$value = prompt("$$ins_servicesname[$i][1]","$$ins_servicesname[$i][3]");
        	printf CMDFILE "regsetup \"$$ins_servicesname[$i][0]\" \"$$ins_servicesname[$i][2]\" $$ins_servicesname[$i][4] \"$value\"\n";
        }
}
        else
        {
        $value = prompt("$$ins_servicesname[$i][1]","$$ins_servicesname[$i][3]");
        printf CMDFILE "regsetup \"$$ins_servicesname[$i][0]\" \"$$ins_servicesname[$i][2]\" $$ins_servicesname[$i][4] \"$value\"\n";
        }
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

sub usage()
{
    print <<EOF;
Usage:
    SrvLoad <--install | --upgrade>
    SrvLoad <--mode=\"single or multiple\">
    SrvLoad --help
options
    --install
        install the service
    --upgrade
        upgrade the service
    --mode=<single or multiple>
        specify WFES mode \"single\" or \"multiple\"
    --help
        display this screen

EOF

exit 0;
}


sub choise()
{
    print "Do you want to install the service and commit the changes {Y|N}? ";
    my $choose = <stdin>;
    $choose = trim($choose);
    return $choose;
}

sub prompt

{

            my $msg = shift;

            my $default = shift;

            

            print "$msg [$default]:";

 


            my $ch = <STDIN>;

            $ch = trim($ch);

            

            $ch = $default if !$ch;

            

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
	       return ($char);
	}
	else
	{
		return 0;
	}
}