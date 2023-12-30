use vars qw(	
	%Service %Config $ITVROOT $LOGFILE $VERBOSE
);


#-------------------------------------------------------------------------------
#		Configuartion for your Service replace %IAM as your service name
#-------------------------------------------------------------------------------
#	here u can install several services in a setup script


%Service = 
(
   'name'         => 'SrvLoad',								#serivce	name
   'lname'        => 'ZQ SrvLoad ',							#display	name
   'proname'	  => 'ITV',										#product 	name
   'description'  => 'This Service retrieve the CM load information from MDS',
   'installpath'  => 'C:\\SrvLoad\\',						#installion path
   'exe'          => 'SrvLoad.exe',							#image		name 
   'shellimage'   => 'srvshell.exe',
   'machine'      => 'LocalHost',								#installation machine IP
   'account'      => 'LocalSystem',								#login		username
   'password'	  => 'LocalSystem',								#login		password      
   'mgmtport'     => '6321',
   'shellmgmtport'=> '6322',
   'msgfile'	  => 'ItvMessages.dll',
   'shellMsgFile' => 'ShellMsgs.dll'   
);
%Config = 
(
#	 keyname 					type    	value
	#service framework	configuration 
	"instance"				=>	["dword",	"1"								],
	"LogFileName"			=>	["string",	"C:\\ITV\\log\\FtpMover.log"	],
	"LogFileSize"			=>	["dword",	"200000000"						],
	"LogFileTimeOut"		=>	["dword",	"2000"							],
	"LogLevel"				=>	["dword",	"7"								],
	"LogBufferSize"			=>	["dword",	"20000"							],
	"ShutdownWaitTime"		=>	["dword",	"3000"							],
	"KeepAliveIntervals"	=>	["dword",	"3000"							],
	#user configuration
	"LAMDBDSN"				=>	["string",	"sqlserver"						],
	"LAMDBNAME"				=>	["string",	"am"							],
	"LOADFILENAME"			=>	["string",	"D:\LOADFILE\LOADFILE.xml"		],
	"READINTERVAL"			=>	["string",	"2000"							]		
);
$Name 			=   $Service{name};
$ITVROOT 		=  	($ENV{'ITVROOT'} or "C:\\ITV");
$LOGFILE 		= 	$ITVROOT."\\log\\$Name"."_setup.log";
$VERBOSE		=	7;