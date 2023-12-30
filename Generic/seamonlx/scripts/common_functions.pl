#!/usr/bin/perl
#
# common functions used by multifple perl files are defined
# in this file.
#


# Get the SAS Address
my $SG_SAS_CMD = "/usr/bin/sg_inq -p 0x83";         # command to get sg device sas addresses by sg names

sub getSasAddr
{
    my $sgname = $_[0];
    my $sas = "";

    my $found = 0;
    open (DBINFO, "$SG_SAS_CMD /dev/$sgname 2>&1 |" );
    local ( $/ );
    my @bufs = split ( /Designation descriptor number /, <DBINFO>);
    close DBINFO;

    if ( scalar(@bufs) )
    {
        foreach (@bufs)
        {
            if ( $_ =~ /id_type: NAA,\s+code_set: Binary\s+associated with the target port.+\[0x([0-9a-fA-F]{16})\]/s )
            {
                $sas = $+;
                return $sas;
            }

            elsif ( $_ =~ /id_type: NAA,\s+code_set: Binary\s+associated with the addressed logical unit.+\[0x([0-9a-fA-F]{16})\]/s )
            {
                $sas = $+;
                $found = 1;
            }
            elsif ( (!$found) &&
                    $_ =~ /id_type: NAA,\s+code_set: ASCII\s+associated with the target port.*\s{3,5}(([0-9a-fA-F]{2}\s){8})\s+.*/ms )
            {

                $sas = $1;
                $sas =~ s/\s//g;

            }
        }
    }
    return $sas;
}

#############################################################################
#
# make_number_nice()
# Formats a number string such that it will insert commas into big numbers (1000000 is formatted as "1,000,000"). 
# It will nterposes a comma whenever it sees a group of four consecutive digits, and continues to do so until it no 
# longer matches this pattern.
#
# Arguments: The number to format
# Returns: The formatted number
#
##############################################################################
sub make_number_nice {
    my $num = shift;
    1 while ($num =~ s/(.*\d)(\d\d\d)/$1,$2/g);
    $num;
}

1;
