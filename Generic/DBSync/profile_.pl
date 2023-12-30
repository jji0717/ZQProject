use vars qw(	
	%Service %Config $ITVROOT $LOGFILE $VERBOSE
);


#-------------------------------------------------------------------------------
#		Configuartion for your Service replace %IAM as your service name
#-------------------------------------------------------------------------------
#	here u can install several services in a setup script


%Service = 
(
   'name'         => 'DBSync',									#serivce	name
   'lname'        => 'ZQ DB synchronize Service',				#display	name
   'proname'	  => 'ITV',										#product 	name
   'description'  => 'This Service synchronize the asset information from ITV to LAM',
   'installpath'  => 'c:\\DBSync\\BIN\\',						#installion path
   'exe'          => 'DBSync.exe',								#image		name 
   'shellimage'   => 'srvshell.exe',
   'machine'      => 'LocalHost',								#installation machine IP
   'account'      => 'LocalSystem',								#login		username
   'password'	  => 'LocalSystem',								#login		password      
   'mgmtport'     => '6301',
   'shellmgmtport'=> '6302',
   'msgfile'	  => 'ItvMessages.dll',
   'shellMsgFile' => 'ShellMsgs.dll'   
);
%Config = 
(
#	 keyname 					type    	value
	#service framework	configuration 
	"instance"				=>	["dword",	"1"								],
	"LogFileName"			=>	["string",	"C:\\DBSync\\log\\DBSync.log"	],
	"LogFileSize"			=>	["dword",	"200000000"						],
	"LogFileTimeOut"		=>	["dword",	"2000"							],
	"LogLevel"				=>	["dword",	"7"								],
	"LogBufferSize"			=>	["dword",	"20000"							],
	"ShutdownWaitTime"		=>	["dword",	"3000"							],
	"KeepAliveIntervals"	=>	["dword",	"3000"							],
	#user configuration
	"ITVServerAddress"		=>	["string",	"192.168.12.12"					],
	"ITVUserName"			=>	["string",	"sa"							],
	"IZQDBDSN"				=>	["string",	"sqlserver"						],
	"IZQDBUserName"			=>	["string",	"am"							],
	"IZQDBPassword"			=>	["string",	"am"							],
	"IZQDBType"				=>	["string",	"SQL Server"					],
	"IZQPlayInterval"		=>	["dword",	"0"								],
	"ITVCheckInterval"		=>	["dword",	"1"								]
);
$Name 			=   $Service{name};
$ITVROOT 		=  	($ENV{'ITVROOT'} or "C:\\ITV");
$LOGFILE 		= 	$ITVROOT."\\log\\$Name"."_setup.log";
$VERBOSE		=	7;