
ģ�����ƣ�
ssm_PauseTV_s1.dll

����˵����
ssm_PauseTV_s1.dll����ΪRtspProxy��һ����Ҫ����������ģ���������ͻ��˻����з�����RTSP���󡣷�����������󣬲�����ת��Ϊ�������������ϵľ��幦�ܵ��á�

��װ�����ļ���
	ssm_PauseTV_s1.dll��ssm_PauseTV_s1.xml��Install.bat��

��װ˵��
1��	����RtspProxy�������ļ�RtspProxy.xml����RtspProxy.xml���������<StreamSmithSite>  ��.. </StreamSmithSite>���ڸ����м�����������<Application path="PauseTV" handler="PauseTV_s1"/>����ֵ����̶���������һ����<module file="e:\zqprojs\streamnow\streamsmith\modules\ssm_pausetv_s1\debug\ssm_pausetv_s1.dll"/>�����е�file����ֵӦ������Ϊssm_PauseTV_s1.dll��·�������������á�
2��	����Install.bat
ssm_PauseTV_s1.dll Ĭ�ϰ�װ·��ΪC:\TianShan\bin
ssm_PauseTV_s1.xmlĬ�ϰ�װ·��ΪC:\TianShan\etc
3��	�����Ҫ�޸İ�װ·��������install.bat֮ǰ�Խű����б༭
   set EXEDIR=C:\TianShan   
��C:\TianShan�޸�Ϊ����·����
��ssm_PauseTV_s1.dll�����ļ�ssm_PauseTV_s1.xml��˵��
	Ssm_PauseTV_s1.xmlĿǰ����15�������ÿһ�����ֵ��Ϊ�ַ�����˵�����£�
1��	LOGFILE_NAME������ssm_PauseTV_s1.dll��log�ļ����ļ�����Ҫ�����·������
2��	LOGFILE_SIZE������ssm_PauseTV_s1.dll��log�ļ��Ĵ�С����λ���ֽڣ���
3��	LOGFILE_LEVEL����־�ļ���Ŀǰ������Ϊ7������̫С����Щ�ͼ������־��д������־��
4��	CHANNELONDEMANDAPP_NAME����������ChannelOnDemand�����ơ�
5��	CHANNELONDEMANDAPP_IP����������ChannelOnDemand��IP��ַ��
6��	CHANNELONDEMANDAPP_PORT����������ChannelOnDemand�Ķ˿ڡ�
7��	STREAMSERVICE_NAME����������StreamSmith�����ơ�
8��	STREAMSERVICE_IP����������StreamSmith��IP��ַ��
9��	STREAMSERVICE_PORT����������StreamSmith�Ķ˿ڡ�
10��	SINKEVENT_PORT�����ñ��������¼��Ķ˿ںţ���EndOfStream��BeginningOfStream��EndOfItem���¼���
11��	EVENTCHANNELIMPL_IP����������IceStorm��IP��ַ��
12��	EVENTCHANNELIMPL_PORT����������IceStorm�Ķ˿ںš�
13��	EXIT_CLEAN�������ֵ��������ON��OFF�������������ΪON����RtspProxy����ֹͣ��ʱ��ssm_PauseTV_s1.dll������Զ�̷����������Ѿ������õ�Stream��Purchase��������Ҫ�˹��ܣ��ɽ�������ΪOFF��
14��	START_RESUME�������ֵ��������ON��OFF�������������ΪON����RtspProxy����ֹͣ��ʱ��ssm_PauseTV_s1.dll������Stream��Purchase�����ǽ��ͻ���SessionID��Stream��Purchase���浽�ļ����ļ�����ͨ����һ�����ã��У���RtspProxy�ٴ�������ʱ�򣬶�ȡ���ļ�����Session��Stream��Purchase��Ӧ�����������Ļ���RtspProxy�����������󣬿ͻ��ܹ��������Լ���Stream���в�����
15��	RESUME_FILENAME��һ���ļ������ơ����ļ����������14�����ᵽ�����ݡ�

��װ���飺
	��ssm_PauseTV_s1.dll�ŵ�RtspProxy�İ�װĿ¼������C:\TianShan\bin\ssm_PauseTV_s1.dll������ssm_PauseTV_s1.xml����RtspProxy��װĿ¼�µ�configuration�ļ����У�����C:\TianShan\etc\ssm_PauseTV_s1.xml����ͬʱ���������������������RtspProxy�������ļ�RtspProxy.xml�е�<module file=��C:\TianShan\bin\ssm_PauseTV_s1.dll��/>�����о���ע�����HKEY_LOCAL_MACHINE\SOFTWARE\SeaChange\TianShan\CurrentVersion\Services\RTSPPROXY��pluginConfigFilePath��ֵӦ������ΪC:\TianShan\bin\configuration\ssm_PauseTV_s1.xml��ssm_PauseTV_s1.dll��log�ļ���ͨ��ssm_PauseTV_s1.xml��LOGFILE_NAME���ã��ɽ�������ΪC:\TianShan\log\ssm_PauseTV_s1.log��
