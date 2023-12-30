#!/bin/bash
# Set the Ident bit for Enclosure/Bay to State
# to run:
# ./identDrive.sh /dev/Enclosure_sgName BayNumber|all off|on
# ex:
# ./identDrive.sh /dev/sg25|all 1 off|on

if [[ $# -eq 0 ]]; then
    echo "Usage: identDrive.sh Enclosure_SGname BayNumber|all off|on"
    echo "Example: ./identDrive.sh /dev/sg25 0 on"
    exit 1
fi

Enclosure=${1:-"/dev/sg25"}      # sgName of the Enclosure Processor in format "/dev/Enclosure_sgName"
Bay=${2:-0}                      # 0-11, all
State=${3:-"off"}                # off (0), on (1)

# capture the number of Disk elements (Bays)
Data=$(/usr/bin/sg_ses -p 1 ${Enclosure} | grep -E -A 1 'Element type: Array device| Element type: Device')
NumDisks=$(echo ${Data} | awk '{print $NF}')

JustOneBay="F"
if [[ "$Bay" == "all" || "$Bay" == "ALL" ]]; then
    BayNum=0;
elif [[ $Bay -lt 0 || $Bay -gt 11 && "$Bay" != "all" && "$Bay" != "ALL" ]]; then
   echo "Bay ($Bay) is not valid (values: 0-11 | all)"
   exit 1
elif [[ $BayNum -gt $((${NumDisks}-1)) ]]; then
        echo "Only ($NumDisks) Bays are populated"
        exit 1
else
    BayNum=$Bay
    JustOneBay="T"
fi

if [[ "$State" == "on" || "$State" == "ON" ]]; then
    StateNum=1
elif [[ "$State" == "off" || "$State" == "OFF" ]]; then
    StateNum=0
else
    echo "State ($State) is not valid (values: on | off)"
    exit 1
fi

Data=$(/usr/bin/sg_ses --page=2 --raw ${Enclosure}) # capture the current state of the Ident bits (among other data)

# AWK program to find SelectColumn and IdentColumn and update their data values
echo ${Data} | awk '
BEGIN {
FS=" ";
OFS=" ";
}
{
    NumberOfDisks='${NumDisks}'
    BayNumber='${BayNum}'
    SingleBay="'${JustOneBay}'"
    DesiredIdentState='${StateNum}'

    i=0
    SelectColumnArray[i]=BayNumber * 4              # groups of 4 bytes
    SelectColumnArray[i]=SelectColumnArray[i] + 8   # skip first 8 columns
    SelectColumnArray[i]=SelectColumnArray[i] + 1   # make Columns 1-relative
    IdentColumnArray[i]=SelectColumnArray[i] + 2    # Ident byte is 2 columns beyond the Select Bit byte

    # if we are doing "all" we need to grab all the Select and Ident bit columns
    if ( SingleBay == "F" )
    {
        i++
        while ( i < NumberOfDisks )
        {
            SelectColumnArray[i]=SelectColumnArray[i-1] + 4
            IdentColumnArray[i]=SelectColumnArray[i] + 2
            i++
        }
    }

    for (i=1; i <= NF; i++)
    {
        DataSent=0
        for (j=0; j < NumberOfDisks; j++)
        {
            if ( i == IdentColumnArray[j] && 1 == DesiredIdentState )   # turn ON Ident bit
            {
                printf( "%02x ", or(strtonum($IdentColumnArray[j]),0x02) ) > "/tmp/IdentFile";
                DataSent=1
            }
            else if ( i == IdentColumnArray[j] && 0 == DesiredIdentState )   # turn OFF Ident bit
            {
                printf( "%02x ", and(strtonum($IdentColumnArray[j]),0xFD) ) > "/tmp/IdentFile";
                DataSent=1
            }
            else if ( i == SelectColumnArray[j] )   # turn ON Select bit
            {
                printf( "%02x ", or(strtonum($SelectColumnArray[j]),0x80) ) > "/tmp/IdentFile";
                DataSent=1
            }
            if ( SingleBay == "T" )
            {
                j=NF  # Terminate the for loop
            }
        } # end for loop j

        if ( DataSent == 0 )
        {
            printf( "%s ", $i ) > "/tmp/IdentFile";
        }
    } # end for loop i
}
'

# Update Ident bits in the Enclosure Processor Page 2
/usr/bin/sg_ses --page=2 --control --data=- ${Enclosure} < "/tmp/IdentFile" > /dev/null
if [[ $? -eq 0 ]]; then
    echo "Success: updating Ident bit on ${Enclosure} Bay ${Bay} to ${State}"
else
    echo "Failed to set Ident bit correctly on ${Enclosure} Bay ${Bay} to ${State}"
fi
echo

rm -f /tmp/IdentFile

# NOTE:
# useful ses commands
# to see data by device
# sg_ses --page=2 --inner-hex /dev/sg25
# sg_ses --page=2 --raw /dev/sg25 > t
# sg_ses --page=2 --control --data=- /dev/sg25 < t
