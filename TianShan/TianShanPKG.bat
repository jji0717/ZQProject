setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe 
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rem rd /s/q buildtemp
if not exist buildtemp mkdir buildtemp
cd buildtemp

rem =========== Preparing subdir bin ===========
mkdir doc
mkdir bin
cd bin

copy %ICE_ROOT%\bin\ssleay32.dll
copy %ICE_ROOT%\bin\libeay32.dll

rem copy ICESDK
copy %ICE_ROOT%\ICE.zip
copy %ICE_ROOT%\bin\bzip2.dll
copy %ICE_ROOT%\bin\freeze3?.dll
copy %ICE_ROOT%\bin\ice3?.dll
copy %ICE_ROOT%\bin\icebox3?.dll
copy %ICE_ROOT%\bin\icegrid3?.dll
copy %ICE_ROOT%\bin\icessl3?.dll
copy %ICE_ROOT%\bin\icestorm3?.dll
copy %ICE_ROOT%\bin\icestormservice3?.dll
copy %ICE_ROOT%\bin\iceutil3?.dll
copy %ICE_ROOT%\bin\icexml3?.dll
copy %ICE_ROOT%\bin\libdb4?.dll
copy %ICE_ROOT%\bin\libexpat.dll

copy %ICE_ROOT%\bin\libexpatw.dll
copy %ICE_ROOT%\bin\slice3?.dll
copy %ICE_ROOT%\bin\stlport_vc646.dll
copy %ICE_ROOT%\bin\glacier23?.dll

rem copy MQTT
copy %MQTTPATH%\bin\x86\paho-mqtt3c.dll

rem copy ITVSDK
rem copy %ITVSDKPATH%\EXE\reporter.dll
rem copy %ITVSDKPATH%\EXE\MCASTSVC.DLL
rem copy %ITVSDKPATH%\EXE\MANPKGU.DLL
rem copy %ITVSDKPATH%\EXE\queue.dll
rem copy %ITVSDKPATH%\EXE\clog.dll
rem copy %ITVSDKPATH%\EXE\CFGPKGU.DLL

rem copy ZQ shell
copy %ZQPROJSPATH%\TianShan\bin\ZQShell.exe
copy %ZQPROJSPATH%\TianShan\bin\ZQShell.pdb
copy %ZQPROJSPATH%\TianShan\bin\ZQShellMsgs.dll
copy %ZQPROJSPATH%\TianShan\bin\ZQShellMsgs.pdb
copy %ZQPROJSPATH%\TianShan\bin\ZQCfgPkg.dll
copy %ZQPROJSPATH%\TianShan\bin\ZQCfgPkg.pdb
copy %ZQPROJSPATH%\TianShan\bin\ZQSnmpManPkg.dll
copy %ZQPROJSPATH%\TianShan\bin\ZQSnmpManPkg.pdb
copy %ZQPROJSPATH%\TianShan\bin\ZQSNMPAgent.dll
copy %ZQPROJSPATH%\TianShan\bin\ZQSNMPAgent.pdb
copy %ZQPROJSPATH%\TianShan\bin\ZQSnmp.dll
copy %ZQPROJSPATH%\TianShan\bin\ZQSnmp.pdb

mkdir java
cd java

copy %ZQPROJSPATH%\generic\JndiClient\JndiClient.jar
copy %ZQPROJSPATH%\generic\JndiClient\jbossall-client.jar
copy %ZQPROJSPATH%\generic\JndiClient\activemq-all-5.5.0.jar
copy %ZQPROJSPATH%\generic\JndiClient\slf4j-nop-1.5.11.jar

rem mkdir JMSClient
rem cd JMSClient
rem xcopy %ZQPROJSPATH%\generic\JMSCppLib\JMSCpp\Java\. /s /y
cd ..
rem cd ..

rem copy for TianShanICE
copy %ZQPROJSPATH%\TianShan\bin\TianShanICE.dll
copy %ZQPROJSPATH%\TianShan\bin\TianShanICE.pdb

rem copy for chodsvc
copy %ZQPROJSPATH%\TianShan\bin\ChODSvc.exe
copy %ZQPROJSPATH%\TianShan\bin\ChODSvc.pdb
copy %ZQPROJSPATH%\generic\JMSCppLib\JMSCpp\lib\jmsc.dll

rem copy for StreamSmith
copy %ZQPROJSPATH%\TianShan\bin\streamsmith.exe
copy %ZQPROJSPATH%\TianShan\bin\streamsmith.pdb
copy %ZQPROJSPATH%\TianShan\bin\StreamSmithAdmin.exe
copy %ZQPROJSPATH%\TianShan\bin\StreamSmithAdmin.pdb
copy %ZQPROJSPATH%\TianShan\bin\StreamClient.exe
copy %ZQPROJSPATH%\TianShan\bin\StreamClient.pdb
copy %ZQPROJSPATH%\TianShan\bin\ZQCommonstlp.dll
copy %ZQPROJSPATH%\TianShan\bin\ZQCommonstlp.pdb

rem copy for Sentry
copy %ZQPROJSPATH%\TianShan\bin\TSClient.dll
copy %ZQPROJSPATH%\TianShan\bin\TSClient.pdb
copy %ZQPROJSPATH%\TianShan\bin\WebLayout.dll
copy %ZQPROJSPATH%\TianShan\bin\WebLayout.pdb
copy %ZQPROJSPATH%\TianShan\bin\LogPage.exe
copy %ZQPROJSPATH%\TianShan\bin\LogPage.pdb
copy %ZQPROJSPATH%\TianShan\bin\SentrySvc.exe
copy %ZQPROJSPATH%\TianShan\bin\SentrySvc.pdb
copy %ZQPROJSPATH%\TianShan\bin\snmpplug.dll
copy %ZQPROJSPATH%\TianShan\bin\snmpplug.pdb
copy %ZQPROJSPATH%\TianShan\bin\AdminCtrl_web.dll
copy %ZQPROJSPATH%\TianShan\bin\AdminCtrl_web.pdb
copy %ZQPROJSPATH%\TianShan\bin\CodMan_Web.dll
copy %ZQPROJSPATH%\TianShan\bin\CodMan_Web.pdb
copy %ZQPROJSPATH%\TianShan\bin\ngod2view.dll
copy %ZQPROJSPATH%\TianShan\bin\ngod2view.pdb
copy %ZQPROJSPATH%\TianShan\bin\CPCMan_web.dll
copy %ZQPROJSPATH%\TianShan\bin\CPCMan_web.pdb
copy %ZQPROJSPATH%\TianShan\bin\CPEMan_web.dll
copy %ZQPROJSPATH%\TianShan\bin\CPEMan_web.pdb
copy %ZQPROJSPATH%\TianShan\bin\SysLogger.dll
copy %ZQPROJSPATH%\TianShan\bin\SysLogger.pdb
copy %ZQPROJSPATH%\TianShan\bin\MDB_web.dll
copy %ZQPROJSPATH%\TianShan\bin\MDB_web.pdb
copy %ZQPROJSPATH%\TianShan\bin\Storage_web.dll
copy %ZQPROJSPATH%\TianShan\bin\Storage_web.pdb
copy %ZQPROJSPATH%\TianShan\bin\APM_Web.dll
copy %ZQPROJSPATH%\TianShan\bin\APM_Web.pdb
copy %ZQPROJSPATH%\TianShan\bin\NGODSNMPExt.dll
copy %ZQPROJSPATH%\TianShan\bin\NGODSNMPExt.pdb
copy %ZQPROJSPATH%\TianShan\bin\NGODHist.dll
copy %ZQPROJSPATH%\TianShan\bin\NGODHist.pdb
copy %ZQPROJSPATH%\TianShan\bin\ClibMan_web.dll
copy %ZQPROJSPATH%\TianShan\bin\ClibMan_web.pdb


copy %SNMP_PLUS_ROOT%\bin\snmp_pp.dll

rem copy for RtspProxy
copy %ZQPROJSPATH%\TianShan\bin\RtspProxy.exe
copy %ZQPROJSPATH%\TianShan\bin\RtspProxy.pdb

rem copy plugin ssm_pausetv_s1
rem copy %ZQPROJSPATH%\TianShan\StreamSmith\Modules\ssm_pausetv_s1\Release\ssm_pausetv_s1.dll

rem copy for MediaClusterCS
copy %ZQPROJSPATH%\TianShan\bin\MediaClusterCS.exe
copy %ZQPROJSPATH%\TianShan\bin\MediaClusterCS.pdb

rem copy for CDNCS
copy %ZQPROJSPATH%\TianShan\bin\CDNCS.exe
copy %ZQPROJSPATH%\TianShan\bin\CDNCS.pdb

rem copy for CDNSS
rem copy %ZQPROJSPATH%\TianShan\bin\CDNSS.exe
rem copy %ZQPROJSPATH%\TianShan\bin\CDNSS.pdb


rem copy for TianShanRDA
rem copy %ZQPROJSPATH%\TianShan\bin\RDA.exe
rem copy %ZQPROJSPATH%\TianShan\bin\RDA.pdb
rem copy %ZQPROJSPATH%\TianShan\bin\isaLink.dll

rem trick dll
copy %VSTRMKITPATH%\TrickFilesIpl.dll
copy %VSTRMKITPATH%\TrickFilesIplA6.dll
copy %VSTRMKITPATH%\TrickFilesLibraryUser.dll
copy %VSTRMKITPATH%\TrickFilesMsgs.dll
copy %VSTRMKITPATH%\BitStrmLibraryUser.dll
copy %VSTRMKITPATH%\MpegLibraryUser.dll
copy %VSTRMKITPATH%\cpuinf32.dll

rem copy for Weiwoo
copy %ZQPROJSPATH%\TianShan\bin\WeiwooService.exe
copy %ZQPROJSPATH%\TianShan\bin\WeiwooService.pdb
copy %ZQPROJSPATH%\TianShan\bin\PathAdmin.exe
copy %ZQPROJSPATH%\TianShan\bin\PathAdmin.pdb


rem copy for MovieOnDemand
copy %ZQPROJSPATH%\TianShan\bin\MODAppSvc.exe
copy %ZQPROJSPATH%\TianShan\bin\MODAppSvc.pdb

copy %ZQPROJSPATH%\TianShan\bin\GBMODAppSvc.exe
copy %ZQPROJSPATH%\TianShan\bin\GBMODAppSvc.pdb

rem copy for BroadcastChannel
copy %ZQPROJSPATH%\TianShan\bin\BcastChannel.exe
copy %ZQPROJSPATH%\TianShan\bin\BcastChannel.pdb
copy %ZQPROJSPATH%\TianShan\bin\BcastChMan_web.dll
copy %ZQPROJSPATH%\TianShan\bin\BcastChMan_web.pdb

rem copy for TSPump
copy %ZQPROJSPATH%\TianShan\bin\TSPump.exe
copy %ZQPROJSPATH%\TianShan\bin\TSPump.pdb

rem copy for SiteAdminSvc
copy %ZQPROJSPATH%\TianShan\bin\SiteAdminSvc.exe
copy %ZQPROJSPATH%\TianShan\bin\SiteAdminSvc.pdb
copy %ZQPROJSPATH%\TianShan\bin\EventSender.dll
copy %ZQPROJSPATH%\TianShan\bin\EventSender.pdb

copy %ZQPROJSPATH%\TianShan\bin\MsgSender.dll
copy %ZQPROJSPATH%\TianShan\bin\MsgSender.pdb

rem copy for CPE
copy %ZQPROJSPATH%\TianShan\bin\CPESvc.exe
copy %ZQPROJSPATH%\TianShan\bin\CPESvc.pdb
rem copy %RTFLIBSDKPATH%\exe\Win32\RTFLib.dll
rem copy %RTFLIBSDKPATH%\exe\Win32\RTFLib.pdb
rem copy %RTFLIBSDKPATH%\exe\Win32\CommonTrickFiles.dll
rem copy %RTFLIBSDKPATH%\exe\Win32\CommonTrickFiles.pdb

rem copy CPE Utils
copy %ZQPROJSPATH%\TianShan\bin\CPEClient.exe
copy %ZQPROJSPATH%\TianShan\bin\CPEClient.pdb
copy %ZQPROJSPATH%\TianShan\bin\RTFGen.exe
copy %ZQPROJSPATH%\TianShan\bin\RTFGen.pdb
copy %ZQPROJSPATH%\TianShan\bin\RTIGen.exe
copy %ZQPROJSPATH%\TianShan\bin\RTIGen.pdb
copy %ZQPROJSPATH%\TianShan\bin\TrickGen.exe
copy %ZQPROJSPATH%\TianShan\bin\TrickGen.pdb
copy %ZQPROJSPATH%\TianShan\bin\RTICap.exe
copy %ZQPROJSPATH%\TianShan\bin\RTICap.pdb

rem copy %VSTRMKITPATH%\PacedIndex.dll
rem copy %VSTRMKITPATH%\PacedVV2.dll
rem copy %VSTRMKITPATH%\PacedVVX.dll

rem copy %VSTRMKITPATH%\i386\PacedIndex.dll /y
rem copy %VSTRMKITPATH%\i386\PacedVV2.dll   /y
rem copy %VSTRMKITPATH%\i386\PacedVVX.dll   /y

rem copy for EventGateway
copy %ZQPROJSPATH%\TianShan\bin\EventGateway.exe
copy %ZQPROJSPATH%\TianShan\bin\EventGateway.pdb

rem copy for EventChannel
copy %ZQPROJSPATH%\TianShan\bin\EventChannel.exe
copy %ZQPROJSPATH%\TianShan\bin\EventChannel.pdb

rem copy for NSS
copy %ZQPROJSPATH%\TianShan\bin\NSS.exe
copy %ZQPROJSPATH%\TianShan\bin\NSS.pdb

rem copy for C2SS
copy %ZQPROJSPATH%\TianShan\bin\C2SS.exe
copy %ZQPROJSPATH%\TianShan\bin\C2SS.pdb
copy %ZQPROJSPATH%\TianShan\bin\libasync.dll
copy %ZQPROJSPATH%\TianShan\bin\libasync.pdb

rem copy for EdgeRMService
copy %ZQPROJSPATH%\TianShan\bin\EdgeRMService.exe
copy %ZQPROJSPATH%\TianShan\bin\EdgeRMService.pdb
copy %ZQPROJSPATH%\TianShan\bin\EdgeRMClient.exe
copy %ZQPROJSPATH%\TianShan\bin\EdgeRMClient.pdb
copy %ZQPROJSPATH%\TianShan\bin\ERMIServer.exe
copy %ZQPROJSPATH%\TianShan\bin\ERMIServer.pdb
copy %ZQPROJSPATH%\TianShan\bin\ErmMan_web.dll
copy %ZQPROJSPATH%\TianShan\bin\ErmMan_web.pdb

rem copy for DsmccCRG
copy %ZQPROJSPATH%\TianShan\bin\DsmccCRG.exe
copy %ZQPROJSPATH%\TianShan\bin\DsmccCRG.pdb
copy %ZQPROJSPATH%\TianShan\bin\DsmccCRGClient.exe
copy %ZQPROJSPATH%\TianShan\bin\DsmccCRGClient.pdb

rem copy for ContentClient
copy %ZQPROJSPATH%\TianShan\bin\ContentClient.exe
copy %ZQPROJSPATH%\TianShan\bin\ContentClient.pdb

rem copy for HttpCRG
copy %ZQPROJSPATH%\TianShan\bin\HttpCRG.exe
copy %ZQPROJSPATH%\TianShan\bin\HttpCRG.pdb
copy %ZQPROJSPATH%\TianShan\bin\c2loc_web.dll
copy %ZQPROJSPATH%\TianShan\bin\c2loc_web.pdb

rem copy for DummySS
copy %ZQPROJSPATH%\TianShan\bin\DummySS.exe
copy %ZQPROJSPATH%\TianShan\bin\DummySS.pdb

rem copy for DummySvc
copy %ZQPROJSPATH%\TianShan\bin\DummySvc.exe
copy %ZQPROJSPATH%\TianShan\bin\DummySvc.pdb

rem copy for DataOnDemand

rem copy for TSDump
copy %ZQPROJSPATH%\TianShan\bin\TSDump.exe
copy %ZQPROJSPATH%\TianShan\bin\TSDump.pdb
copy %ZQPROJSPATH%\TianShan\DataOnDemand\Phase2\TsDump\ReadMe.txt

rem copy for DODApp
copy %ZQPROJSPATH%\TianShan\bin\DODApplication.exe
copy %ZQPROJSPATH%\TianShan\bin\DODApplication.pdb
copy %ZQPROJSPATH%\TianShan\bin\JMSDISPATCH.dll
copy %ZQPROJSPATH%\TianShan\bin\JMSDispatch.pdb

rem copy for DODContentStore
copy %ZQPROJSPATH%\TianShan\bin\DODCS.exe
copy %ZQPROJSPATH%\TianShan\bin\DODCS.pdb

rem copy for DataStream
copy %ZQPROJSPATH%\TianShan\bin\DataSS.exe
copy %ZQPROJSPATH%\TianShan\bin\DataSS.pdb

copy %ZQPROJSPATH%\TianShan\bin\DSClient.exe
copy %ZQPROJSPATH%\TianShan\bin\DSClient.pdb

rem copy for ContentLib
copy %ZQPROJSPATH%\TianShan\bin\ContentLib.exe
copy %ZQPROJSPATH%\TianShan\bin\ContentLib.pdb

rem copy for EventRuleEngine
copy %ZQPROJSPATH%\TianShan\bin\EventRuleEngine.exe
copy %ZQPROJSPATH%\TianShan\bin\EventRuleEngine.pdb

rem copy for JndiClient
copy %ZQPROJSPATH%\generic\JndiClient\Release\JndiClient.dll
copy %ZQPROJSPATH%\generic\JndiClient\Release\JndiClient.pdb

copy %ZQPROJSPATH%\TianShan\bin\OpenVBO_web.dll
copy %ZQPROJSPATH%\TianShan\bin\OpenVBO_web.pdb

rem copy for BaseCS
copy %ZQPROJSPATH%\TianShan\bin\BaseCS.exe
copy %ZQPROJSPATH%\TianShan\bin\BaseCS.pdb

rem copy for GBVSS
copy %ZQPROJSPATH%\TianShan\bin\GBVSS.exe
copy %ZQPROJSPATH%\TianShan\bin\GBVSS.pdb

rem copy for GBCS
copy %ZQPROJSPATH%\TianShan\bin\GBCS.exe
copy %ZQPROJSPATH%\TianShan\bin\GBCS.pdb

cd..

rem =========== Preparing subdir modules ===========
mkdir modules
cd modules
copy %ZQPROJSPATH%\TianShan\bin\ssm_tianshan.dll
copy %ZQPROJSPATH%\TianShan\bin\ssm_tianshan.pdb

copy %ZQPROJSPATH%\TianShan\bin\pho_SeaChange.dll
copy %ZQPROJSPATH%\TianShan\bin\pho_SeaChange.pdb

copy %ZQPROJSPATH%\TianShan\bin\pho_vss.dll
copy %ZQPROJSPATH%\TianShan\bin\pho_vss.pdb

rem weiwoo modules for PHO_ERM
copy %ZQPROJSPATH%\TianShan\bin\pho_ERM.dll
copy %ZQPROJSPATH%\TianShan\bin\pho_ERM.pdb

copy %ZQPROJSPATH%\TianShan\bin\pho_HttpSS.dll
copy %ZQPROJSPATH%\TianShan\bin\pho_HttpSS.pdb

rem weiwoo modules for PHO_bcast
copy %ZQPROJSPATH%\TianShan\bin\pho_bcast.dll
copy %ZQPROJSPATH%\TianShan\bin\pho_bcast.pdb

rem A3_message plugin for ContentLib
copy %ZQPROJSPATH%\TianShan\bin\CRM_A3Message.dll
copy %ZQPROJSPATH%\TianShan\bin\CRM_A3Message.pdb

rem crm_dsmcc plugin for DsmccCRG
copy %ZQPROJSPATH%\TianShan\bin\CRM_DSMCC.dll
copy %ZQPROJSPATH%\TianShan\bin\CRM_DSMCC.pdb

rem copy plugin ssm_tianshan_s1
copy %ZQPROJSPATH%\TianShan\bin\ssm_tianshan_s1.dll
copy %ZQPROJSPATH%\TianShan\bin\ssm_tianshan_s1.pdb


copy %ZQPROJSPATH%\TianShan\bin\ssm_ngod2.dll
copy %ZQPROJSPATH%\TianShan\bin\ssm_ngod2.pdb

rem copy %ZQPROJSPATH%\TianShan\bin\ssm_ngod.dll
rem copy %ZQPROJSPATH%\TianShan\bin\ssm_ngod.pdb


rem ssm_richurl
copy %ZQPROJSPATH%\TianShan\bin\ssm_richurl.dll
copy %ZQPROJSPATH%\TianShan\bin\ssm_richurl.pdb

rem copy OpenVBO_web
copy %ZQPROJSPATH%\TianShan\bin\ssm_OpenVBO.dll
copy %ZQPROJSPATH%\TianShan\bin\ssm_OpenVBO.pdb

copy %ZQPROJSPATH%\TianShan\bin\OpenVBO_web.dll
copy %ZQPROJSPATH%\TianShan\bin\OpenVBO_web.pdb

rem copy ssm_LiveChannel
copy %ZQPROJSPATH%\TianShan\bin\ssm_LiveChannel.dll
copy %ZQPROJSPATH%\TianShan\bin\ssm_LiveChannel.pdb

rem for CPE service
copy %ZQPROJSPATH%\TianShan\bin\CPH_RDS.dll
copy %ZQPROJSPATH%\TianShan\bin\CPH_RDS.pdb
copy %ZQPROJSPATH%\TianShan\bin\CPH_RTFRDS.dll
copy %ZQPROJSPATH%\TianShan\bin\CPH_RTFRDS.pdb
rem copy %ZQPROJSPATH%\TianShan\bin\CPH_NasCopy.dll
rem copy %ZQPROJSPATH%\TianShan\bin\CPH_NasCopy.pdb
copy %ZQPROJSPATH%\TianShan\bin\CPH_RTI.dll
copy %ZQPROJSPATH%\TianShan\bin\CPH_RTI.pdb
copy %ZQPROJSPATH%\TianShan\bin\CPH_RTINAS.dll
copy %ZQPROJSPATH%\TianShan\bin\CPH_RTINAS.pdb
copy %ZQPROJSPATH%\TianShan\bin\CPH_nPVR.dll
copy %ZQPROJSPATH%\TianShan\bin\CPH_nPVR.pdb
copy %ZQPROJSPATH%\TianShan\bin\CPH_Raw.dll
copy %ZQPROJSPATH%\TianShan\bin\CPH_Raw.pdb
copy %ZQPROJSPATH%\TianShan\bin\CPH_CSI.dll
copy %ZQPROJSPATH%\TianShan\bin\CPH_CSI.pdb

rem copy MODPlugin
copy %ZQPROJSPATH%\TianShan\bin\MHO_MODPlugIn.dll
copy %ZQPROJSPATH%\TianShan\bin\MHO_MODPlugIn.pdb
copy %ZQPROJSPATH%\TianShan\bin\MHO_GBMODPlugIn.dll
copy %ZQPROJSPATH%\TianShan\bin\MHO_GBMODPlugIn.pdb

rem copy EGH_SOAP_CME
copy %ZQPROJSPATH%\TianShan\bin\EGH_SOAP_CME.dll
copy %ZQPROJSPATH%\TianShan\bin\EGH_SOAP_CME.pdb

rem copy EGH_JMS
copy %ZQPROJSPATH%\TianShan\bin\EGH_JMS.dll
copy %ZQPROJSPATH%\TianShan\bin\EGH_JMS.pdb

rem copy EGH_MQTT
copy %ZQPROJSPATH%\TianShan\bin\EGH_MQTT.dll
copy %ZQPROJSPATH%\TianShan\bin\EGH_MQTT.pdb

rem copy EGH_SnmpTrap
copy %ZQPROJSPATH%\TianShan\bin\EGH_SnmpTrap.dll
copy %ZQPROJSPATH%\TianShan\bin\EGH_SnmpTrap.pdb

rem copy DOD
copy %ZQPROJSPATH%\TianShan\bin\pho_dod.dll
copy %ZQPROJSPATH%\TianShan\bin\pho_dod.pdb

rem copy for CRM_A3Server
rem copy %ZQPROJSPATH%\TianShan\bin\CRM_A3Server.dll
rem copy %ZQPROJSPATH%\TianShan\bin\CRM_A3Server.pdb

rem copy for CRM_C2Locator
copy %ZQPROJSPATH%\TianShan\bin\CRM_C2Locator.dll
copy %ZQPROJSPATH%\TianShan\bin\CRM_C2Locator.pdb

rem copy for CPH_C2Propagation
copy %ZQPROJSPATH%\TianShan\bin\CPH_C2Propagation.dll
copy %ZQPROJSPATH%\TianShan\bin\CPH_C2Propagation.pdb

rem copy for ssm_GBss
copy %ZQPROJSPATH%\TianShan\bin\ssm_GBss.dll
copy %ZQPROJSPATH%\TianShan\bin\ssm_GBss.pdb

rem copy for ssm_gb_s1
copy %ZQPROJSPATH%\TianShan\bin\ssm_GBstb.dll
copy %ZQPROJSPATH%\TianShan\bin\ssm_GBstb.pdb

cd ..

rem =========== Preparing subdir utils ===========
mkdir utils
cd utils
rem copy %ZQPROJSPATH%\build\utils\DEPENDS.EXE
copy %ZQPROJSPATH%\TianShan\bin\dot.exe
copy %ZQPROJSPATH%\TianShan\bin\freetype6.dll
copy %ZQPROJSPATH%\TianShan\bin\jpeg.dll
copy %ZQPROJSPATH%\TianShan\bin\png.dll
copy %ZQPROJSPATH%\TianShan\bin\z.dll
copy %ZQPROJSPATH%\TianShan\bin\zlib1.dll
copy %ZQPROJSPATH%\build\utils\VerCheck.exe
copy %ZQPROJSPATH%\build\utils\zip.exe
copy %ZQPROJSPATH%\TianShan\bin\ChPub.exe
copy %ZQPROJSPATH%\TianShan\bin\sac.exe
copy %ZQPROJSPATH%\TianShan\bin\PathAdmin.exe
rem copy %ZQPROJSPATH%\TianShan\bin\readDB.exe
rem copy %ZQPROJSPATH%\TianShan\bin\writeDB.exe
copy %ICE_ROOT%\bin\ice3?.dll
copy %ICE_ROOT%\bin\iceutil3?.dll
copy %ICE_ROOT%\bin\libdb4?.dll
copy %ICE_ROOT%\bin\stlport_vc646.dll
copy %ICE_ROOT%\bin\libexpat.dll

copy %ZQPROJSPATH%\build\utils\tail.exe
copy %ZQPROJSPATH%\build\utils\cygiconv-2.dll
copy %ZQPROJSPATH%\build\utils\cygintl-8.dll
copy %ZQPROJSPATH%\build\utils\cygwin1.dll

copy %ZQPROJSPATH%\build\utils\GetAllLogFiles.bat
copy %ZQPROJSPATH%\build\utils\GetStreamLog.bat
copy %ZQPROJSPATH%\build\utils\timestamp.exe
copy %ZQPROJSPATH%\TianShan\SiteAdminSvc\service\TxnData_Template.mdb
copy %ZQPROJSPATH%\TianShan\etc\OSTREvent_template.mdb
copy %ZQPROJSPATH%\TianShan\etc\ngod2Event_template.mdb
copy %ZQPROJSPATH%\TianShan\TsFindSess.bat

copy %ZQPROJSPATH%\TianShan\TsStat.bat

rem copy zqsnmp Utils
copy %ZQPROJSPATH%\TianShan\release\zqsnmp.exe

rem copy for TSHammer
copy %ZQPROJSPATH%\TianShan\bin\TSHammer.exe
copy %ZQPROJSPATH%\TianShan\bin\TSHammer.pdb
copy %ZQPROJSPATH%\generic\TsHammer\RtspScript.xml RtspScript_sample.xml

rem copy for tslicapp
copy %ZQPROJSPATH%\TianShan\bin\tslicapp.exe
copy %ZQPROJSPATH%\TianShan\bin\tslicapp.pdb

cd ..

rem =========== Preparing subdir webctrl ===========
mkdir webctrl
xcopy /fycsr %ZQPROJSPATH%\TianShan\webctrl webctrl

rem =========== Preparing subdir etc ===========
mkdir etc
cd etc
copy %ZQPROJSPATH%\TianShan\etc\TianShanDef.xml TianShanDef_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\ChannelOnDemand.xml ChannelOnDemand_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\CODEvent.ini
copy %ZQPROJSPATH%\TianShan\etc\StreamSmith.xml StreamSmith_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\SiteAdminSvc.xml SiteAdminSvc_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\RtspProxy.xml RtspProxy_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\ssm_TianShan_s1.xml ssm_TianShan_s1_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\ssm_richurl.xml ssm_richurl_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\ssm_NGOD2.xml ssm_NGOD2_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\ssm_NGOD2_SOP.xml ssm_NGOD2_SOP_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\ssm_LiveChannel.xml ssm_LiveChannel_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\NSS.xml NSS_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\C2SS.xml C2SS_sample.xml
copy %ZQPROJSPATH%\\generic\TsHammer\RtspScript.xml RtspScript_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\NGODHist.xml NGODHist_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\MediaClusterCS.xml MediaClusterCS_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\Weiwoo.xml Weiwoo_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\MovieOnDemand.xml MovieOnDemand_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\GBMovieOnDemand.xml GBMovieOnDemand_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\MsgSender.xml MsgSender_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\EventSender.xml EventSender_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\EventSenderForSiteAdmin.xml EventSenderForSiteAdmin_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\Sentry.xml Sentry_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\weblayout.xml weblayout_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\weblayout_NGOD.xml weblayout_NGOD_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\snmpsender.xml snmpsender_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\syntax.xml
copy %ZQPROJSPATH%\TianShan\TianShan.MIB
copy %ZQPROJSPATH%\TianShan\etc\CPE.xml	CPE_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\CPH_RDS.xml	 CPH_RDS_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\CPH_RTFRDS.xml	 CPH_RTFRDS_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\CPH_NasCopy.xml	 CPH_NasCopy_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\CPH_RTI.xml	 CPH_RTI_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\CPH_RTINAS.xml	 CPH_RTINAS_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\CPH_nPVR.xml	 CPH_nPVR_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\CPH_Raw.xml	 CPH_Raw_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\CPH_CSI.xml	 CPH_CSI_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\EventGateway.xml	 EventGateway_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\EGH_SOAP_CME.xml	 EGH_SOAP_CME_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\EGH_JMS.xml	 EGH_JMS_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\EGH_MQTT.xml	 EGH_MQTT_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\EventChannel.xml	 EventChannel_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\BroadcastChannel.xml	BroadcastChannel_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\CDNCS.xml	CDNCS_sample.xml
rem copy %ZQPROJSPATH%\TianShan\etc\CDNSS.xml	CDNSS_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\HttpCRG.xml	HttpCRG_sample.xml
rem copy %ZQPROJSPATH%\TianShan\etc\CRM_A3Server.xml	CRM_A3Server_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\CRM_C2Locator.xml	CRM_C2Locator_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\EdgeRMSvc.xml		EdgeRMSvc_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\pho_ERM.xml		pho_ERM_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\EGH_SnmpTrap.xml	EGH_SnmpTrap_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\ContentLib.xml ContentLib_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\EventRuleEngine.xml EventRuleEngine_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\EventRules.xml EventRules_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\CRM_A3Message.xml CRM_A3Message_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\CPH_C2Propagation.xml CPH_C2Propagation_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\ssm_OpenVBO.xml	 ssm_OpenVBO_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\ssm_OpenVBO_Streamers.xml	 ssm_OpenVBO_Streamers_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\COF_SystemEvent.xml	 COF_SystemEvent_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\COF_WinEvent.xml	 COF_WinEvent_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\DEF_SystemEvent.xml	 DEF_SystemEvent.xml
copy %ZQPROJSPATH%\TianShan\etc\DEF_WinEvent.xml	 DEF_WinEvent.xml
copy %ZQPROJSPATH%\TianShan\etc\dummyss.conf	 dummyss_sample.conf

copy %ZQPROJSPATH%\TianShan\etc\DsmccCRG.xml	DsmccCRG_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\crm_Dsmcc.xml	crm_Dsmcc_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\DsmccScript.xml DsmccScript_sample.xml

copy %ZQPROJSPATH%\TianShan\etc\COF_LinuxSystemEvent.xml	 COF_LinuxSystemEvent_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\COF_SystemEvent.xml	 COF_SystemEvent_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\COF_WinEvent.xml	 COF_WinEvent_sample.xml

copy %ZQPROJSPATH%\TianShan\etc\DEF_LinuxSystemEvent.xml	DEF_LinuxSystemEvent_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\DEF_SystemEvent.xml	 DEF_SystemEvent_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\DEF_WinEvent.xml	 DEF_WinEvent_sample.xml

copy %ZQPROJSPATH%\TianShan\etc\ssm_GBss.xml	 ssm_GBss_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\ssm_Gbss_Streamers.xml	 ssm_Gbss_Streamers_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\GBVSS.xml	 GBVSS_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\GBCS.xml	 GBCS_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\ssm_GBstb.xml   ssm_GBstb_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\TSPump.xml	TSPump_sample.xml

copy %ZQPROJSPATH%\TianShan\etc\DummySvc.xml	DummySvc_sample.xml
copy %ZQPROJSPATH%\TianShan\etc\Keyfile.xml	Keyfile.xml

rem copy for DOD Config file
copy %ZQPROJSPATH%\TianShan\DataOnDemand\Phase2\etc\DODApp.xml DODApp_Sample.xml
copy %ZQPROJSPATH%\TianShan\DataOnDemand\Phase2\etc\DODContentStore.xml DODContentStore_Sample.xml
copy %ZQPROJSPATH%\TianShan\DataOnDemand\Phase2\etc\DataStream.xml DataStream_Sample.xml

copy %ZQPROJSPATH%\TianShan\DataOnDemand\Phase2\DODApp\Localconfig.xml Localconfig_Sample.xml
copy %ZQPROJSPATH%\TianShan\DataOnDemand\Phase2\DOD_Initial_for_Conf\def\ConfDefine_DOD.xml
copy %ZQPROJSPATH%\TianShan\DataOnDemand\Phase2\DOD_Initial_for_Conf\languages\*.xml

cd ..

rem =========== Preparing subdir sdk ===========
mkdir sdk
cd sdk

copy /y %ZQPROJSPATH%\TianShan\SDK\TianShanSDK.zip

cd..

cd ctf
mkdir bin
cd bin
copy %RTFLIBSDKPATH%\exe\Win32\CommonTrickFiles.dll
copy %RTFLIBSDKPATH%\exe\Win32\CommonTrickFiles.pdb
cd ..
cd ..

copy %ZQPROJSPATH%\TianShan\TianShanSetup.bat
copy %ZQPROJSPATH%\TianShan\TianShanSetup.bat TianShanSetup.pl
copy %ZQPROJSPATH%\TianShan\profile.pl
copy %ZQPROJSPATH%\build\utils\VerCheck.exe

rem =========== Kit siganitures ===========
%VersionCheck% bin >VersionInfo_TianShan32.txt
%VersionCheck% modules >>VersionInfo_TianShan32.txt
%VersionCheck% sdk >>VersionInfo_TianShan32.txt
%VersionCheck% ctf\\bin >>VersionInfo_TianShan32.txt

copy %ZQPROJSPATH%\TianShan\Components4RTF.V1.6.zip
copy %ZQPROJSPATH%\TianShan\Components4RTF.V2.0.zip
copy %ZQPROJSPATH%\TianShan\Components4CTF.V1.0.zip

copy VersionInfo_TianShan32.txt bin\

xcopy /fsi *.pdb ..\TianShanSymbols

del /s *.pdb 

del /q/f  ..\TianShan_setup.zip

%PACKCMD% -r ..\TianShan_setup.zip .

rem pause

cd ..
rd /s/q buildtemp

cd TianShanSymbols

del /q/f ..\TianShanSymbols.zip

%PACKCMD% -r ..\TianShanSymbols.zip .

cd ..
rd /s/q TianShanSymbols

endlocal
