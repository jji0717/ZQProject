1. CovAxiomToNGOD.pl
   It's a simple Perl program that help to convert Axiom Asset To NGOD, generate more script
   like asset.csv,renfilenames.bat,rollbackfilenames.bat,oldcontent.txt and newcontent.txt.
   
   asset.csv: List axiom asset metadata,include: ProviderId,ProviderAssetId,FileSize,BitRate,ActualDuration and SupportFileSize.
   	Usage: for import LAM database
   renfilenames.bat: scp rename "AEUID" "PAID_PID"
   	Usage: rename file names on MediaCluster, include main file and all trickfiles.
   rollbackfilenames.bat: scp rename PAID_PID AEUID,
   	Usage: roll back file names.
   oldcontent.txt: List all axiom AEUID,
   	Uaage: calculate AEUID MD5
   newcontent.txt: List all content name in NGOD, it's PAID_PID.
   	Usage: calculate NGOD file MD5, add new contentname to ClusterContentStore database.
   
   runing precondition:
   1) Running on a MediaCluster Node
   2) Active perl must be installed
   
   Steps:
   1) Unzip CovAxiomToNGOD.zip
   2) Open a command line, and run:
   perl CovAxiomToNGOD.pl --IDSServer=<idsserver> --trickfile=<trickfiles>
   
   Usage:
   ConvAxiomToNGOD is a simple Perl program that help to Convert Axiom Asset To NGOD database
   options
   --IDSServer=<IPAddr>
       specify the IDS Server IP Address
   --trickfile=<trickfile>
    	specify trickfiles extension for rename, split with comma, like \"--trickfile=ff,fr,vvx\", default value is \"ff,fr,vvx\"
   --PID=<defaultPID>
    	specify the default PID for element when it is empty, default value is \"seachange.com\"
   --PAIDPrefix=<PAIDPrefix>
	specify the prefix used to generate PAID for element when it is empty, default value is \"SEAC\"
   --logfile=<logfile>
	specify logfile name for record the msg, default value is \"CovAxiomToNGOD.log\"
   --help
        display this screen
        
2. Calmd5.pl
   Calmd5 is a simple perl program that can calculate file MD5, support file include all filename list which want to calculate MD5.
   It will generate a .csv file list filename and MD5
   
   Steps
   Open a command line, and run:
   perl Calmd5.pl --filename=<filename>
   
   eg: if want to calculate oldcontent.txt, run as:
   perl Calmd5.pl --filename=oldcontent.txt
   
   Usage:
   Usage:
    calmd5 <options>
    calmd5 is a simple Perl program that can calculate file md5
    options
    --filename=<filename>
        specify the filename which record all file name want to calculate md5.
    --outputcsv=<out_csv>
    	specify output .csv file name, default value is \"MD5Data.csv\"
    --logfile=<logfile>
	specify logfile name for record the msg, default value is \"calmd5.log\"
    --help
        display this screen
        
3. ContentClient.exe
   Open Content utils, before running the tools, confirm VC80 SP1 runtime distribution have been installed.
   
   Usage:
   ContentClient <endpoint> <file>
   <endpoint> ContentStore endpoint eg. 127.0.0.1:10400
   <file>     name of the file with content name in each line