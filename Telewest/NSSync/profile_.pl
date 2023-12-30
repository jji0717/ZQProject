use vars qw(	
	%Service %Config $ITVROOT $LOGFILE $VERBOSE
);


#-------------------------------------------------------------------------------
#		Configuartion for your Service replace %IAM as your service name
#-------------------------------------------------------------------------------
#	here u can install several services in a setup script


%Service = 
(
   'name'         => 'NSSync',								#serivce	name
   'lname'        => 'NSSync ',							#display	name
   'proname'	  => 'ITV',										#product 	name
   'description'  => 'NSSync is a ZQ service that synchronizes data between LocalAM and Navigation databases.',
   'installpath'  => 'D:\\NSSync\\',						#installion path
   'exe'          => 'NSSync.exe',							#image		name 
   'shellimage'   => 'srvshell.exe',
   'machine'      => 'LocalHost',								#installation machine IP
   'account'      => 'LocalSystem',								#login		username
   'password'	  => 'LocalSystem',								#login		password      
   'mgmtport'     => '6930',
   'shellmgmtport'=> '6931',
   'msgfile'	  => 'ItvMessages.dll',
   'shellMsgFile' => 'ShellMsgs.dll'   
);
%Config = 
(
#	 keyname 					type    	value
	#service framework	configuration 
	"instance"			=>	["dword",	"1"							],
	"LogFileName"			=>	["string",	"C:\\ITV\\log\\NSSync.log"				],
	"LogFileSize"			=>	["dword",	"10000000"						],
	"LogFileTimeOut"		=>	["dword",	"7D0"							],
	"LogLevel"			=>	["dword",	"7"							],
	"LogBufferSize"			=>	["dword",	"1000"							],
	"ShutdownWaitTime"		=>	["dword",	"1388"							],
	"KeepAliveIntervals"		=>	["dword",	"7D0"							],
	#user configuration
	"IZQDBDSN"			=>	["string",	"localam"							],
	"IZQDBName"			=>	["string",	"am"							],
	"IZQDBPassword"			=>	["string",	"lam"							],
	"IZQDBTimeout"			=>	["dword",	"1E"							],
	"IZQDBType"			=>	["string",	"sqlserver"							],
	"IZQDBUserName"			=>	["string",	"lam"							],
	"SuperviseInterval"		=>	["dword",	"3C"							]
);
$Name 			=   $Service{name};
$ITVROOT 		=  	($ENV{'ITVROOT'} or "C:\\ITV");
$LOGFILE 		= 	$ITVROOT."\\log\\$Name"."_setup.log";
$VERBOSE		=	7;