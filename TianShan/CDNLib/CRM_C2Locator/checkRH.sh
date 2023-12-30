# This script is a workaround to check health of remote UML head from SCS server
# usage: checkRH <IP or hostname of UML head>
# return: errorlevel ==0 if remote is healthy, otherwise not

UMLHead=$1
UMLMntPoint=/mnt/bwfs

ping -c 1 -w 1 $UMLHead

if [ 0 != $? ]; then
   echo "failed to ping $UMLHead"
   exit -1
fi

ssh root@$UMLHead "df -t enfs|grep $UMLMntPoint"

if [ 0 != $? ]; then
   echo "couldn't find enfs mounted on $UMLHead"
   exit -2
fi

TestContent="checkRH.by.i$HOSTNAME:`date +%FT%T`"
TmpFile=$UMLMntPoint/c2files/checkRH.by.$HOSTNAME

ssh root@$UMLHead "echo $TestContent > $TmpFile"

if [ 0 != $? ]; then
   echo "$UMLHead couldn't write file $TmpFile"
   exit -3
fi

ReadContent=""
ReadContent=`ssh root@$UMLHead cat $TmpFile`

if [ "$ReadContent" != "$TestContent" ]; then
   echo "$UMLHead failed to match the content of $TmpFile"
   exit -4
fi

ssh root@$UMLHead rm -f $TmpFile
if [ 0 != $? ]; then
   echo "$UMLHead failed to delete tempfile $TmpFile"
   exit -5
fi 

echo "$UMLHead appears healthy"

exit 0


# The way to avoid ssh require a password via interactions.
#
# 1) edit /etc/ssh/sshd_config on UML head by enable line
#      PubkeyAuthentication yes
#    then restart sshd
# 2) on SCS, run
#      ssh-keygen -t rsa
#    to generate rsa key pair. Do NOT specify a password in this step
# 3) Open the id_rsa.pub file generated in step 2),
#    copy its content into .ssh/authorized_keys under the root's homedir of UML head
