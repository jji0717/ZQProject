@rem = '-*- Perl -*-';
@rem = '
@echo off
perl -S %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
goto EndOfPerl
';


#use strict 'vars';
#use strict;
use File::Find;

require "configurations.pl";

my $buildnum = shift;
die "TianShan build hasn't been specified" if $buildnum eq "";

&main();

sub main()
{
    $TIANSHAN_HOME=fixupPath($TIANSHAN_HOME);
    $BUILDZIP_HOME=fixupPath($BUILDZIP_HOME);
    $TOOLS_HOME=fixupPath($TOOLS_HOME);

    $TIANSHAN_HOME=fixupPath($TIANSHAN_HOME);
    $BUILDZIP_HOME=fixupPath($BUILDZIP_HOME);
    $TOOLS_HOME=fixupPath($TOOLS_HOME);
    
    ExtractBuild($buildnum);
    RenewEtc();  
    chdir($TOOLS_HOME);
    system("call UpdateSvrProf.bat");
}

sub ExtractBuild()
{
    my $build = shift;
    
    system("net use " . $BUILDZIP_HOME . " /u:ab zqshare66");
    my $instkit = fixupPath($BUILDZIP_HOME . "/TianShan.V". $build . ".zip");
    die "can not find installation kit $instkit \n" if not -e $instkit;
    
    chdir($TOOLS_HOME);
    system("call TianShanSvc.bat stop");
    system("net stop SNMP");
    system("net stop Eventlog");
    
    mkdir($TIANSHAN_HOME);
    chdir($TIANSHAN_HOME);

    my $cmd = fixupPath("$TOOLS_HOME/zip") . "  -9 -r logs-" . $TIMESTAMP . " ". fixupPath("logs/*.*");
    print "$cmd\n";
    system("$cmd");	

    system ("rd /s/q bin modules utils logs");
    mkdir("logs");
    my $cmd = fixupPath("$TOOLS_HOME/unzip") . " -o ". $instkit . " -x bin64/*.* -x modules64/*.* -x utils64/*.*";
    print "$cmd\n";
    system("$cmd");	

    system("net start Eventlog");
    system("net start SNMP");
}

sub RenewEtc
{
    chdir($TIANSHAN_HOME);
    my $cmd = fixupPath("$TOOLS_HOME/zip") . "  -9 -r etc-" . $TIMESTAMP . " ". fixupPath("etc/*.*");
    print "$cmd\n";
    system("$cmd");	
    my $dir = fixupPath($TIANSHAN_HOME . "/etc");
    find(\&copySampleXml, "$dir"); #custom subroutine find, parse $dir
}

sub copySampleXml()
{
    return if (not -e or -d);
    my $COPYCMD = "copy /Y";

    $_ = fixupPath($File::Find::name);
    if(/(.*)_sample.xml?/)
    {
    	my $newname = $1 . ".xml";
	my $backupCmd = "copy /Y $_ " . $newname;
	print "$backupCmd\n";
	rename($_, $newname) or system("$backupCmd");
    }
}

sub fixupPath
{
my @out = @_;
   for (@out)
    {
   	s/^\s+//;
      s/\//\\/g;
      s/^\n+//;
   }
   return wantarray ? @out : $out[0];
}


# following gets called recursively for each file in $dir, check $_ to see if you want the file!

endprog:
__END__

:EndOfPerl