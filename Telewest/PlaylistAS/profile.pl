use vars qw(	
	%Service %Config $ITVROOT $LOGFILE $VERBOSE
);


#-------------------------------------------------------------------------------
#		Configuartion for your Service replace %IAM as your service name
#-------------------------------------------------------------------------------
#	here u can install several services in a setup script


%Service = 
(
   'name'         => 'PlaylistAS',								#serivce	name
   'lname'        => 'ZQ PlaylistAS ',							#display	name
   'proname'	  => 'ITV',										#product 	name
   'description'  => 'Music Playlist Application Server (PlaylistAS) service is an extensive service for ITV system, in order to support playlist formatted streaming.',
   'installpath'  => 'D:\\PlaylistAS\\',						#installion path
   'exe'          => 'PlaylistAS.exe',							#image		name 
   'shellimage'   => 'srvshell.exe',
   'machine'      => 'LocalHost',								#installation machine IP
   'account'      => 'LocalSystem',								#login		username
   'password'	  => 'LocalSystem',								#login		password      
   'mgmtport'     => '6940',
   'shellmgmtport'=> '6941',
   'msgfile'	  => 'ItvMessages.dll',
   'shellMsgFile' => 'ShellMsgs.dll'   
);
%Config = 
(
#	 keyname 					type    	value
	#service framework	configuration 
	"instance"			=>	["dword",	"1"							],
	"LogFileName"			=>	["string",	"C:\\ITV\\log\\PlaylistAS.log"				],
	"LogFileSize"			=>	["dword",	"10000000"						],
	"LogFileTimeOut"		=>	["dword",	"7D0"							],
	"LogLevel"			=>	["dword",	"7"							],
	"LogBufferSize"			=>	["dword",	"1000"							],
	"ShutdownWaitTime"		=>	["dword",	"1388"							],
	"KeepAliveIntervals"		=>	["dword",	"7D0"							],
	#user configuration
	"ISSAppUID"			=>	["dword",	"80002"							],
	"ISSInst"			=>	["dword",	"1"							],
	"SoapWSDLFilePath"		=>	["string",	"http://YourWebServer:90/services/PlaylistSoapInterface?wsdl"						],
	"SoapWSMLFilePath"		=>	["string",	$Service{'installpath'}.'PlaylistSoapInterface.wsml' 							],
	"SoapServiceName"		=>	["string",	"PlaylistSoapInterfaceService"							],
	"SoapPort"			=>	["dword",	"PlaylistSoapInterface"							],
	"SoapTargetNamespace"		=>	["string",	"http://YourWebServer:90/services/PlaylistSoapInterface"							]
);
$Name 			=   $Service{name};
$ITVROOT 		=  	($ENV{'ITVROOT'} or "C:\\ITV");
$LOGFILE 		= 	$ITVROOT."\\log\\$Name"."_setup.log";
$VERBOSE		=	7;