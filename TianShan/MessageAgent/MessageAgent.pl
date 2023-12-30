use Getopt::Long;
use Win32;
use Cwd;

&main();
sub main()
{
    require "profile.pl";
    $ins_servicesname="MessageAgent";
    my $installstate=prompt ("Do you want to install or upgrade the service?{\"I\" for \"Install\",\"U\" for \"Upgrade\"}","I|U");
    if (!(($installstate eq "U")||($installstate eq "u")||($installstate eq "I")||($installstate eq "i")))
	  {
	  	print "\nNOTE:Please give the right choice \"I\" or \"U\", thanks!\n";
	  	exit;
	}
	if (($installstate eq "I")||($installstate eq "i"))
	{
		system("copy config\\config.icebox %ICEROOT%\\bin") == 0 or die("Please verify there is a system environment variable for ICEROOT and the directory %ICEROOT%\\bin already existed, thanks.");
		system("copy config\\config.service %ICEROOT%\\bin") == 0 or die("Please verify there is a system environment variable for ICEROOT and the directory %ICEROOT%\\bin already existed, thanks.");
		system("if not exist %ICEROOT%\\bin\\data md %ICEROOT%\\bin\\data") == 0 or die ("can not create folder %ICEROOT%\\bin\\data");
		system("%ICEROOT%\\bin\\icebox.exe --install TianShanIceStorm --Ice.Config=%ICEROOT%\\bin\\config.icebox");
	}
    
###Read registry in local machine
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
	    chdir "bin";
	    system ("regdmp \\registry\\machine\\$$ins_servicesname[$a][0] 1>$a.txt 2>err.txt");
	    open MF,"<$a.txt";
	    my @c=<MF>;
	    close MF;
            unlink "$a.txt";
            unlink "err.txt";
	    chdir "..";
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

    $directory=prompt ("Input the directory for install MessageAgent","C:\\TianShan");
    $EXEDIR="$directory\\bin";
    $LOGDIR="$directory\\log";
    $CONFIGDIR="$directory\\config";
    $SAFESTOREDIR="$directory\\data";
    $SVCACCOUNT=".\\SeaChange";
    $SVCPASSWD="SeaChange";
    %EN = (
		  TARGETDIR => $directory,
		  EXEDIR    => $EXEDIR,
		  LOGDIR    => $LOGDIR,
		  CONFIGDIR => $CONFIGDIR,
		  SAFESTOREDIR => $SAFESTOREDIR,
		  ITVROOT   => "c:\\itv",
		  ITVEXEROOT  => "c:\\itv\\exe",
	     );
    mkdir "$directory";
    mkdir "$EXEDIR";
    mkdir "$LOGDIR";
    mkdir "$CONFIGDIR";
    mkdir "$SAFESTOREDIR";
    
    my $file="Install.bat";
    open CMDFILE, ">$file";
    printf CMDFILE "setlocal\n";
    printf CMDFILE "set TARGETDIR=$directory\n";
    printf CMDFILE "set EXEDIR=%TARGETDIR%\\bin\n";
    printf CMDFILE "set LOGDIR=%TARGETDIR%\\log\n";
    printf CMDFILE "set CONFIGDIR=%TARGETDIR%\\config\n";
    printf CMDFILE "set SAFESTOREDIR=%TARGETDIR%\\data\n";
    printf CMDFILE "set ITVEXEROOT=C:\\ITV\\EXE\n";
    printf CMDFILE "set SVCACCOUNT=.\\SeaChange\n";
    printf CMDFILE "set SVCPASSWD=SeaChange\n";
    printf CMDFILE "if not exist $directory md $directory\n";
    printf CMDFILE "if not exist $EXEDIR md $EXEDIR\n";
    printf CMDFILE "if not exist $LOGDIR md $LOGDIR\n";
    printf CMDFILE "if not exist $CONFIGDIR md $CONFIGDIR\n";
    printf CMDFILE "if not exist $SAFESTOREDIR md $SAFESTOREDIR\n";
    printf CMDFILE "xcopy /fycs bin $EXEDIR\n";
    printf CMDFILE "xcopy /fycs config\\JmsTopicConfig.xml $CONFIGDIR\n";
    printf CMDFILE "if not exist \"%ITVEXEROOT%\"\\instserv.exe copy \"instserv.exe\" \"$EXEDIR\" /Y\n";
    printf CMDFILE "if not exist \"%ITVEXEROOT%\"\\instserv.exe copy \"srvshell.exe\" \"$EXEDIR\" /Y\n";
    if (($installstate eq "I")||($installstate eq "i"))
    {
    printf CMDFILE "if exist \"%ITVEXEROOT%\"\\srvshell.exe instserv MessageAgent \"%ITVEXEROOT%\"\\srvshell.exe local $SVCACCOUNT $SVCPASSWD auto own\n";
    printf CMDFILE "if not exist \"%ITVEXEROOT%\"\\srvshell.exe instserv MessageAgent \"$EXEDIR\"\\srvshell.exe local $SVCACCOUNT $SVCPASSWD auto own\n";
    printf CMDFILE "instserv MessageAgent displayname=\"MessageAgent\"\n";
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
        my $value = prompt("$$ins_servicesname[$i][1]","$$ins_servicesname[$i][3]");
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