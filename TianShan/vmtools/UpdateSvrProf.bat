@rem = '-*- Perl -*-';
@rem = '
@echo off
perl -S %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
goto EndOfPerl
';

# use strict;
use File::Find;
use File::Copy;
use Class::Struct;

require "configurations.pl";

struct AdapterInfo => {};

my $TIANSHAN_SS ="";
my $TIANSHAN_PG ="";
my $TIANSHAN_IG ="";
my $HOSTNAME ="";
my @TIANSHAN_NICS;

&main();

sub mySub()
{
    return if (not -e or -d);

    $_ = fixupPath($File::Find::name);
    if(/(.*)_sample.xml?/)
    {
	my $cmd = "copy /Y $_ $1.xml";
	print "$cmd\n";
	system("$cmd");	
    }
}

sub main()
{
    searchForIP();
    updateHosts();
    updateTsDef();
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

sub searchForIP
{
    open FCMD, "hostname | " or return;
    $_= <FCMD>;
    $HOSTNAME =$1 if(/^[\s]*([a-zA-Z0-9\-_\.]*).*?/);
    close FCMD;

    open FCMD, "ipconfig.exe | " or return;
    my $adapter = AdapterInfo->new();
        
    my $line;
    next_line: while ($line = <FCMD>)
    {
        $_= trim($line);
	if(/^[\s]*Ethernet[\s]+adapter[\s]+(.*):.*?/)
	{
            if ($adapter->{ip} ne "" && $adapter->{name} ne $1)
            {
                push @TIANSHAN_NICS, $adapter;
                $adapter = AdapterInfo->new();
            }
            
            $adapter->{name} = $1;
            next next_line;
        }
        
	if(/^[\s]*IP[\s]+Address.*:[\s]+([:\.0-9a-fA-F:]*).*?/)
        {
            $adapter->{ip} = $1 if ($adapter->{ip} eq "");
            next next_line;
        }
    }
    close FCMD;
    
    push @TIANSHAN_NICS, $adapter if ($adapter->{ip} ne "" && $adapter->{name} ne "");
    
    foreach $adapter (@TIANSHAN_NICS)
    {
        printf "NIC[%s]: %s\n", $adapter->{name}, $adapter->{ip};
	$TIANSHAN_SS = $adapter->{ip} if ($adapter->{name} eq "SS");
	$TIANSHAN_PG = $adapter->{ip} if ($adapter->{name} eq "PG");
	$TIANSHAN_IG = $adapter->{ip} if ($adapter->{name} eq "IG");
    }  
}

sub updateHosts()
{
    $TIANSHAN_PG = $TIANSHAN_SS if ($TIANSHAN_PG eq "");
    $TIANSHAN_SS = $TIANSHAN_PG if ($TIANSHAN_SS eq "");
    $TIANSHAN_IG = $TIANSHAN_PG if ($TIANSHAN_IG eq "");
    printf "updating hosts:\n\tTIANSHAN_SS=$TIANSHAN_SS\n\tTIANSHAN_PG=$TIANSHAN_PG\n\tTIANSHAN_IG=$TIANSHAN_IG\n";
    my $fn = fixupPath("c:/WINDOWS/system32/drivers/etc/hosts");
#    $fn = fixupPath("d:/temp/hosts");

    open FHOSTS_I, "< $fn" or return;
    open FHOSTS_O, "> $fn.tmp" or return;
    my $line;

    next_hostsline: while ($line = <FHOSTS_I>)
    {
        $_= trim($line);
        
	if(/^[\s]*(.*)[\s]+(.*)_SS[\s]*?/ && $2 eq $HOSTNAME)
        {
            printf FHOSTS_O "$TIANSHAN_SS \t$HOSTNAME". "_SS\t\t# auto-updated @ $UPDATE_TIME\n";
            next next_hostsline;
        }

	if(/^[\s]*(.*)[\s]+(.*)_PG[\s]*?/ && $2 eq $HOSTNAME)
        {
            printf FHOSTS_O "$TIANSHAN_PG \t$HOSTNAME". "_PG\t\t# auto-updated @ $UPDATE_TIME\n";
            next next_hostsline;
        }

	if(/^[\s]*(.*)[\s]+(.*)_IG[\s]*?/ && $2 eq $HOSTNAME)
        {
            printf FHOSTS_O "$TIANSHAN_PG \t$HOSTNAME". "_IG\t\t# auto-updated @ $UPDATE_TIME\n";
            next next_hostsline;
        }

        printf FHOSTS_O "$_\n";
    }
    
    close FHOSTS_I;
    close FHOSTS_O;
    copy($fn.".tmp", $fn) and unlink($fn.".tmp");
}

sub updateTsDef()
{
    $TIANSHAN_HOME = fixupPath($TIANSHAN_HOME);
    my $fn = fixupPath($TIANSHAN_HOME . "/etc/TianShanDef.xml");
    printf "updating $fn:\n\tTIANSHAN_HOME=$TIANSHAN_HOME\n\tNETID=$HOSTNAME\n";

    open FDEF_I, "< $fn" or return;
    open FDEF_O, "> $fn.tmp" or return;
    my $line;

    next_defline: while ($line = <FDEF_I>)
    {
        $_= trim($line);
        
	if(/^[\s]*<[\s]*property[\s]*name[\s]*=[\s]*\"ServerNetIf\"[\s]+value[\s]*=[\s]*\"(.*)\".*?/)
        {
            printf FDEF_O "<property name=\"ServerNetIf\" value=\"$HOSTNAME". "_SS\" />\t\t<!-- auto-updated @ $UPDATE_TIME-->\n";
            next next_defline;
        }

	if(/^[\s]*<[\s]*property[\s]*name[\s]*=[\s]*\"IngestNetIf\"[\s]+value[\s]*=[\s]*\"(.*)\".*?/)
        {
            printf FDEF_O "<property name=\"IngestNetIf\" value=\"$HOSTNAME". "_IG\" />\t\t<!-- auto-updated @ $UPDATE_TIME-->\n";
            next next_defline;
        }
        
	if(/^[\s]*<[\s]*property[\s]*name[\s]*=[\s]*\"TianShanHomeDir\"[\s]+value[\s]*=[\s]*\"(.*)\".*?/)
        {
            printf FDEF_O "<property name=\"TianShanHomeDir\" value=\"%s\" />\t\t<!-- auto-updated @ $UPDATE_TIME-->\n", fixupPath($TIANSHAN_HOME);
            next next_defline;
        }
	
	if(/^[\s]*<[\s]*property[\s]*name[\s]*=[\s]*\"HostNetID\"[\s]+value[\s]*=[\s]*\"(.*)\".*?/)
        {
            printf FDEF_O "<property name=\"HostNetID\" value=\"$HOSTNAME\" />\t\t<!-- auto-updated @ $UPDATE_TIME-->\n";
            next next_defline;
        }

        printf FDEF_O "$_\n";
    }
    
    close FDEF_I;
    close FDEF_O;
    copy($fn.".tmp", $fn) and unlink($fn.".tmp");
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
	s/^\n+//g;
	s/^\r+//g;
	s/\s+$//;
	s/^\n+$//;
    }
    return wantarray ? @out : $out[0];
}

# following gets called recursively for each file in $dir, check $_ to see if you want the file!

endprog:
__END__

:EndOfPerl