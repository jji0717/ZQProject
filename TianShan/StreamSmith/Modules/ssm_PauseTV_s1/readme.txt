
模块名称：
ssm_PauseTV_s1.dll

功能说明：
ssm_PauseTV_s1.dll是作为RtspProxy的一个重要组件而开发的，用来处理客户端机顶盒发来的RTSP请求。分析具体的请求，并将其转化为对其他服务器上的具体功能调用。

安装所需文件：
	ssm_PauseTV_s1.dll、ssm_PauseTV_s1.xml、Install.bat。

安装说明
1．	配置RtspProxy的配置文件RtspProxy.xml。打开RtspProxy.xml看到里面的<StreamSmithSite>  ….. </StreamSmithSite>，在该项中间添加两各子项，<Application path="PauseTV" handler="PauseTV_s1"/>（此值必须固定），另外一个是<module file="e:\zqprojs\streamnow\streamsmith\modules\ssm_pausetv_s1\debug\ssm_pausetv_s1.dll"/>，其中的file键的值应该设置为ssm_PauseTV_s1.dll的路径，需自行设置。
2．	运行Install.bat
ssm_PauseTV_s1.dll 默认安装路径为C:\TianShan\bin
ssm_PauseTV_s1.xml默认安装路径为C:\TianShan\etc
3．	如果需要修改安装路径，运行install.bat之前对脚本进行编辑
   set EXEDIR=C:\TianShan   
将C:\TianShan修改为其他路径。
对ssm_PauseTV_s1.dll配置文件ssm_PauseTV_s1.xml的说明
	Ssm_PauseTV_s1.xml目前共有15个配置项，每一个项的值均为字符串，说明如下：
1．	LOGFILE_NAME：配置ssm_PauseTV_s1.dll的log文件的文件名（要求带有路径）。
2．	LOGFILE_SIZE：设置ssm_PauseTV_s1.dll的log文件的大小（单位：字节）。
3．	LOGFILE_LEVEL：日志的级别，目前请设置为7，设置太小，有些低级别的日志将写不进日志。
4．	CHANNELONDEMANDAPP_NAME：设置连接ChannelOnDemand的名称。
5．	CHANNELONDEMANDAPP_IP：设置连接ChannelOnDemand的IP地址。
6．	CHANNELONDEMANDAPP_PORT：设置连接ChannelOnDemand的端口。
7．	STREAMSERVICE_NAME：设置连接StreamSmith的名称。
8．	STREAMSERVICE_IP：设置连接StreamSmith的IP地址。
9．	STREAMSERVICE_PORT：设置连接StreamSmith的端口。
10．	SINKEVENT_PORT：设置本机侦听事件的端口号，像EndOfStream、BeginningOfStream、EndOfItem等事件。
11．	EVENTCHANNELIMPL_IP：设置连接IceStorm的IP地址。
12．	EVENTCHANNELIMPL_PORT：设置连接IceStorm的端口号。
13．	EXIT_CLEAN：该项的值有两个，ON和OFF。如果将其设置为ON，则RtspProxy服务停止的时候，ssm_PauseTV_s1.dll将销毁远程服务器上面已经创建好的Stream和Purchase。若不需要此功能，可将其设置为OFF。
14．	START_RESUME：该项的值有两个，ON和OFF。如果将其设置为ON，则RtspProxy服务停止的时候，ssm_PauseTV_s1.dll不销毁Stream和Purchase，而是将客户的SessionID、Stream和Purchase保存到文件（文件名可通过下一项配置）中，等RtspProxy再次启动的时候，读取该文件，将Session和Stream、Purchase对应起来，这样的话，RtspProxy重新启动过后，客户能够继续对自己的Stream进行操作。
15．	RESUME_FILENAME：一个文件的名称。该文件用来保存第14项中提到的数据。

安装建议：
	将ssm_PauseTV_s1.dll放到RtspProxy的安装目录（例如C:\TianShan\bin\ssm_PauseTV_s1.dll），将ssm_PauseTV_s1.xml放入RtspProxy安装目录下的configuration文件夹中（例如C:\TianShan\etc\ssm_PauseTV_s1.xml），同时配置上面所述的配置项，像RtspProxy的配置文件RtspProxy.xml中的<module file=”C:\TianShan\bin\ssm_PauseTV_s1.dll”/>，还有就是注册表项HKEY_LOCAL_MACHINE\SOFTWARE\SeaChange\TianShan\CurrentVersion\Services\RTSPPROXY下pluginConfigFilePath的值应该设置为C:\TianShan\bin\configuration\ssm_PauseTV_s1.xml。ssm_PauseTV_s1.dll的log文件可通过ssm_PauseTV_s1.xml的LOGFILE_NAME配置，可将其设置为C:\TianShan\log\ssm_PauseTV_s1.log。
