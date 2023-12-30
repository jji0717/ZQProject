========================================================================
    DYNAMIC LINK LIBRARY : EGH_MQTT Project Overview
========================================================================

AppWizard has created this EGH_MQTT DLL for you.  

This file contains a summary of what you will find in each of the files that
make up your EGH_MQTT application.


EGH_MQTT.vcproj
    This is the main project file for VC++ projects generated using an Application Wizard. 
    It contains information about the version of Visual C++ that generated the file, and 
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

EGH_MQTT.cpp
    This is the main DLL source file.

	When created, this DLL does not export any symbols. As a result, it  
	will not produce a .lib file when it is built. If you wish this project 
	to be a project dependency of some other project, you will either need to 
	add code to export some symbols from the DLL so that an export library 
	will be produced, or you can set the Ignore Input Library property to Yes 
	on the General propert page of the Linker folder in the project's Property 
	Pages dialog box.

/////////////////////////////////////////////////////////////////////////////
Other notes:

1.	Third library is MQTT1.0.3 at sdk\3rdPartyKits\MQTT1.0.3 
2.	You should set a environment variable [ MQTTPATH = sdk\3rdPartyKits\MQTT1.0.3 (windows), (linux ,you should create a soft link : ln -sf MQTT1.0.3 MQTT)]
3.	You Can make in linux at the project directory to get the .so

About Configure：

关于MQTT的 Routingkey，Exchange？

Exchange   : 对于MQTT plugin的默认是 amq.topic，你可以自己修改,
			 在 RabbitMQ 的配置文件rabbitmq.config 配置为{exchange,         <<"xxxxx">>}
			 参考: 
				http://www.rabbitmq.com/mqtt.html   Plugin Configuration 部分
			 或参考实例
			[
				{rabbit,[{tcp_listeners,[5672]}]},
				{rabbitmq_mqtt, [{default_user,     <<"guest">>},
				{default_pass,     <<"centos">>},
				{allow_anonymous,  true},
				{vhost,            <<"/">>},
				{exchange,         <<"amq_mqtt">>},
				{subscription_ttl, 300000},
				{prefetch,         10},
				%% use DETS (disk-based) store for retained messages
				{retained_message_store, rabbit_mqtt_retained_msg_store_dets},
				%% only used by DETS store
				{retained_message_store_dets_sync_interval, 20000},
				{ssl_listeners,    []},
				{tcp_listeners,    [1883]}]}
			].

Routingkey : 是PlayList
			 参考 EGH_MQTT.xml的 节点 <Channel name="PlayList" id="4"> 

Queue      : 是你自己建立
			 参考： 
				http://www.rabbitmq.com/mqtt.html   How it Works 部分


AppWizard uses "TODO:" comments to indicate parts of the source code you
should add to or customize.

/////////////////////////////////////////////////////////////////////////////

