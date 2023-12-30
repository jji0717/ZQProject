use vars qw(	
	%Service %Config $ITVROOT $LOGFILE $VERBOSE
);


#-------------------------------------------------------------------------------
#		Configuartion for your Service replace %IAM as your service name
#-------------------------------------------------------------------------------
#	here u can install several services in a setup script


%Service = 
(
   'name'         => 'ITVPlayback',								#serivce	name
   'lname'        => 'ZQ ITVPlayback ',							#display	name
   'proname'	  => 'ITV',										#product 	name
   'description'  => 'Sheduled TV Playback for ITV system (ITVPlayback) service is an extensive service for ITV system, in order to support scheduled program streaming.',
   'installpath'  => 'D:\\ITVPlayback\\',						#installion path
   'exe'          => 'ITVPlayback.exe',							#image		name 
   'shellimage'   => 'srvshell.exe',
   'machine'      => 'LocalHost',								#installation machine IP
   'account'      => 'LocalSystem',								#login		username
   'password'	  => 'LocalSystem',								#login		password      
   'mgmtport'     => '6630',
   'shellmgmtport'=> '6631',
   'msgfile'	  => 'ItvMessages.dll',
   'shellMsgFile' => 'ShellMsgs.dll'   
);
%Config = 
(
#	 keyname 					type    	value
	#service framework	configuration 
	"instance"			=>	["dword",	"1"							],
	"LogFileName"			=>	["string",	"C:\\ITV\\log\\ITVPlayback.log"				],
	"LogFileSize"			=>	["dword",	"10000000"						],
	"LogFileTimeOut"		=>	["dword",	"7D0"							],
	"LogLevel"			=>	["dword",	"7"							],
	"LogBufferSize"			=>	["dword",	"1000"							],
	"ShutdownWaitTime"		=>	["dword",	"1388"							],
	"KeepAliveIntervals"		=>	["dword",	"7D0"							],
	#user configuration
	"ISSAppUID"			=>	["dword",	"80002"							],
	"ISSInst"			=>	["dword",	"1"							],
	"PMFillLength"			=>	["dword",	"E10"							],
	"PMMaxSubChannel"		=>	["dword",	"64"							],
	"PMMirrorPath"			=>	["string",	"D:\\ITVPlayback\\DB\\"						],
	"RTSPFreq"			=>	["dword",	"1F4"							],
	"RTSPHostIP"			=>	["string",	"" 							],
	"RTSPHostPort"			=>	["dword",	"554"							],
	"RTSPNsec"			=>	["dword",	"2710"							],
	"RTSPURL"			=>	["string",	"rtsp://AS_ip/mediacluster?ApplicationUID.AssetUID"							],
	"SMBindIP"			=>	["dword",	""							],
	"SMBindPort"			=>	["dword",	"4444"							],
	"SMClientID"			=>	["dword",	"1"							],
	"SMServerIP"			=>	["string",	""							]
);
$Name 			=   $Service{name};
$ITVROOT 		=  	($ENV{'ITVROOT'} or "C:\\ITV");
$LOGFILE 		= 	$ITVROOT."\\log\\$Name"."_setup.log";
$VERBOSE		=	7;