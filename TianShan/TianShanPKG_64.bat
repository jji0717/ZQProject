setlocal
set PATH=%PATH%;%ZQPROJSPATH%\build\utils
if .%PACKCMD%.==.. set PACKCMD=%ZQPROJSPATH%\build\utils\zip.exe 
if .%VersionCheck%.==.. set VersionCheck=%ZQPROJSPATH%\build\utils\VerCheck.exe

rd /s/q buildtemp
mkdir buildtemp
cd buildtemp

rem =========== Preparing subdir bin ===========
mkdir bin64
cd bin64

copy %ICE_ROOT%\bin\x64\ssleay32.dll
copy %ICE_ROOT%\bin\x64\libeay32.dll

rem copy ICESDK
copy %ICE_ROOT%\ICE.zip
copy %ICE_ROOT%\bin\x64\bzip2.dll
copy %ICE_ROOT%\bin\x64\freeze3?.dll
copy %ICE_ROOT%\bin\x64\ice3?.dll
copy %ICE_ROOT%\bin\x64\icebox3?.dll
copy %ICE_ROOT%\bin\x64\icegrid3?.dll
copy %ICE_ROOT%\bin\x64\icessl3?.dll
copy %ICE_ROOT%\bin\x64\icestorm3?.dll
copy %ICE_ROOT%\bin\x64\icestormservice3?.dll
copy %ICE_ROOT%\bin\x64\iceutil3?.dll
copy %ICE_ROOT%\bin\x64\icexml3?.dll
copy %ICE_ROOT%\bin\x64\libdb4?.dll
copy %ICE_ROOT%\bin\x64\libexpat.dll
copy %ICE_ROOT%\bin\x64\glacier23?.dll

copy %ICE_ROOT%\bin\x64\libexpatw.dll
copy %ICE_ROOT%\bin\x64\slice3?.dll
copy %ICE_ROOT%\bin\x64\stlport_vc646.dll

rem copy MQTT
copy %MQTTPATH%\bin\x64\paho-mqtt3c.dll

rem copy ZQ shell
copy %ZQPROJSPATH%\TianShan\bin64\ZQShell.exe
copy %ZQPROJSPATH%\TianShan\bin64\ZQShell.pdb
copy %ZQPROJSPATH%\TianShan\bin64\ZQShellMsgs.dll
copy %ZQPROJSPATH%\TianShan\bin64\ZQShellMsgs.pdb
copy %ZQPROJSPATH%\TianShan\bin64\ZQCfgPkg.dll
copy %ZQPROJSPATH%\TianShan\bin64\ZQCfgPkg.pdb
copy %ZQPROJSPATH%\TianShan\bin64\ZQSnmpManPkg.dll
copy %ZQPROJSPATH%\TianShan\bin64\ZQSnmpManPkg.pdb
copy %ZQPROJSPATH%\TianShan\bin64\ZQSNMPAgent.dll
copy %ZQPROJSPATH%\TianShan\bin64\ZQSNMPAgent.pdb
copy %ZQPROJSPATH%\TianShan\bin64\ZQSnmp.dll
copy %ZQPROJSPATH%\TianShan\bin64\ZQSnmp.pdb

mkdir java
cd java

copy %ZQPROJSPATH%\generic\JndiClient\JndiClient.jar
copy %ZQPROJSPATH%\generic\JndiClient\jbossall-client.jar
copy %ZQPROJSPATH%\generic\JndiClient\activemq-all-5.5.0.jar
copy %ZQPROJSPATH%\generic\JndiClient\slf4j-nop-1.5.11.jar

rem mkdir JMSClient
rem cd JMSClient
rem xcopy %ZQPROJSPATH%\generic\JMSCppLib\JMSCpp\Java\. /s /y
rem cd ..
cd ..

copy %ZQPROJSPATH%\generic\JMSCppLib\JMSCpp\lib\jmsc.dll

rem copy for TianShanICE
copy %ZQPROJSPATH%\TianShan\bin64\TianShanICE.dll
copy %ZQPROJSPATH%\TianShan\bin64\TianShanICE.pdb

rem copy for StreamSmith
copy %ZQPROJSPATH%\TianShan\bin64\streamsmith.exe
copy %ZQPROJSPATH%\TianShan\bin64\streamsmith.pdb
copy %ZQPROJSPATH%\TianShan\bin64\StreamSmithAdmin.exe
copy %ZQPROJSPATH%\TianShan\bin64\StreamSmithAdmin.pdb
copy %ZQPROJSPATH%\TianShan\bin64\StreamClient.exe
copy %ZQPROJSPATH%\TianShan\bin64\StreamClient.pdb
copy %ZQPROJSPATH%\TianShan\bin64\ZQCommonstlp.dll
copy %ZQPROJSPATH%\TianShan\bin64\ZQCommonstlp.pdb

rem copy for Sentry
copy %ZQPROJSPATH%\TianShan\bin64\TSClient.dll
copy %ZQPROJSPATH%\TianShan\bin64\TSClient.pdb
copy %ZQPROJSPATH%\TianShan\bin64\WebLayout.dll
copy %ZQPROJSPATH%\TianShan\bin64\WebLayout.pdb
copy %ZQPROJSPATH%\TianShan\bin64\LogPage.exe
copy %ZQPROJSPATH%\TianShan\bin64\LogPage.pdb
copy %ZQPROJSPATH%\TianShan\bin64\SentrySvc.exe
copy %ZQPROJSPATH%\TianShan\bin64\SentrySvc.pdb
copy %ZQPROJSPATH%\TianShan\bin64\AdminCtrl_web.dll
copy %ZQPROJSPATH%\TianShan\bin64\AdminCtrl_web.pdb
copy %ZQPROJSPATH%\TianShan\bin64\CodMan_Web.dll
copy %ZQPROJSPATH%\TianShan\bin64\CodMan_Web.pdb
copy %ZQPROJSPATH%\TianShan\bin64\ngod2view.dll
copy %ZQPROJSPATH%\TianShan\bin64\ngod2view.pdb
copy %ZQPROJSPATH%\TianShan\bin64\CPCMan_web.dll
copy %ZQPROJSPATH%\TianShan\bin64\CPCMan_web.pdb
copy %ZQPROJSPATH%\TianShan\bin64\CPEMan_web.dll
copy %ZQPROJSPATH%\TianShan\bin64\CPEMan_web.pdb
copy %ZQPROJSPATH%\TianShan\bin64\SysLogger.dll
copy %ZQPROJSPATH%\TianShan\bin64\SysLogger.pdb
copy %ZQPROJSPATH%\TianShan\bin64\MDB_web.dll
copy %ZQPROJSPATH%\TianShan\bin64\MDB_web.pdb
copy %ZQPROJSPATH%\TianShan\bin64\Storage_web.dll
copy %ZQPROJSPATH%\TianShan\bin64\Storage_web.pdb
copy %ZQPROJSPATH%\TianShan\bin64\APM_Web.dll
copy %ZQPROJSPATH%\TianShan\bin64\APM_Web.pdb
copy %ZQPROJSPATH%\TianShan\bin64\NGODSNMPExt.dll
copy %ZQPROJSPATH%\TianShan\bin64\NGODSNMPExt.pdb
copy %ZQPROJSPATH%\TianShan\bin64\NGODHist.dll
copy %ZQPROJSPATH%\TianShan\bin64\NGODHist.pdb
copy %ZQPROJSPATH%\TianShan\bin64\ClibMan_web.dll
copy %ZQPROJSPATH%\TianShan\bin64\ClibMan_web.pdb


copy %SNMP_PLUS_ROOT%\bin\x64\snmp_pp.dll
copy %ZQPROJSPATH%\TianShan\bin64\snmpplug.dll
copy %ZQPROJSPATH%\TianShan\bin64\snmpplug.pdb

rem copy for RtspProxy
copy %ZQPROJSPATH%\TianShan\bin64\RtspProxy.exe
copy %ZQPROJSPATH%\TianShan\bin64\RtspProxy.pdb

rem copy for MediaClusterCS
copy %ZQPROJSPATH%\TianShan\bin64\MediaClusterCS.exe
copy %ZQPROJSPATH%\TianShan\bin64\MediaClusterCS.pdb

rem copy for CDNCS
copy %ZQPROJSPATH%\TianShan\bin64\CDNCS.exe
copy %ZQPROJSPATH%\TianShan\bin64\CDNCS.pdb

rem copy for CDNSS
rem copy %ZQPROJSPATH%\TianShan\bin64\CDNSS.exe
rem copy %ZQPROJSPATH%\TianShan\bin64\CDNSS.pdb

rem trick dll
copy %VSTRMKITPATH%\TrickFilesIpl.dll
copy %VSTRMKITPATH%\TrickFilesIplA6.dll
copy %VSTRMKITPATH%\TrickFilesLibraryUser.dll
copy %VSTRMKITPATH%\TrickFilesMsgs.dll
copy %VSTRMKITPATH%\BitStrmLibraryUser.dll
copy %VSTRMKITPATH%\MpegLibraryUser.dll
copy %VSTRMKITPATH%\cpuinf32.dll

rem copy for WeiwooService
copy %ZQPROJSPATH%\TianShan\bin64\WeiwooService.exe
copy %ZQPROJSPATH%\TianShan\bin64\WeiwooService.pdb
copy %ZQPROJSPATH%\TianShan\bin64\PathAdmin.exe
copy %ZQPROJSPATH%\TianShan\bin64\PathAdmin.pdb

rem copy for MODAppService
copy %ZQPROJSPATH%\TianShan\bin64\MODAppSvc.exe
copy %ZQPROJSPATH%\TianShan\bin64\MODAppSvc.pdb

copy %ZQPROJSPATH%\TianShan\bin64\GBMODAppSvc.exe
copy %ZQPROJSPATH%\TianShan\bin64\GBMODAppSvc.pdb

rem copy for BroadcastChannel
copy %ZQPROJSPATH%\TianShan\bin64\BcastChannel.exe
copy %ZQPROJSPATH%\TianShan\bin64\BcastChannel.pdb
copy %ZQPROJSPATH%\TianShan\bin64\BcastChMan_web.dll
copy %ZQPROJSPATH%\TianShan\bin64\BcastChMan_web.pdb

rem copy for TSPump
copy %ZQPROJSPATH%\TianShan\bin64\TSPump.exe
copy %ZQPROJSPATH%\TianShan\bin64\TSPump.pdb


rem copy for SiteAdminSvc
copy %ZQPROJSPATH%\TianShan\bin64\SiteAdminSvc.exe
copy %ZQPROJSPATH%\TianShan\bin64\SiteAdminSvc.pdb
copy %ZQPROJSPATH%\TianShan\bin64\EventSender.dll
copy %ZQPROJSPATH%\TianShan\bin64\EventSender.pdb

rem copy for MsgSender
copy %ZQPROJSPATH%\TianShan\bin64\MsgSender.dll
copy %ZQPROJSPATH%\TianShan\bin64\MsgSender.pdb


rem copy for CPE
copy %ZQPROJSPATH%\TianShan\bin64\CPESvc.exe
copy %ZQPROJSPATH%\TianShan\bin64\CPESvc.pdb
rem copy %RTFLIBSDKPATH%\exe\x64\RTFLib.dll
rem copy %RTFLIBSDKPATH%\exe\x64\RTFLib.pdb
rem copy %RTFLIBSDKPATH%\exe\x64\CommonTrickFiles.dll
rem copy %RTFLIBSDKPATH%\exe\x64\CommonTrickFiles.pdb

rem copy CPE Utils
copy %ZQPROJSPATH%\TianShan\bin64\CPEClient.exe
copy %ZQPROJSPATH%\TianShan\bin64\CPEClient.pdb
copy %ZQPROJSPATH%\TianShan\bin64\RTFGen.exe
copy %ZQPROJSPATH%\TianShan\bin64\RTFGen.pdb
copy %ZQPROJSPATH%\TianShan\bin64\RTIGen.exe
copy %ZQPROJSPATH%\TianShan\bin64\RTIGen.pdb
copy %ZQPROJSPATH%\TianShan\bin64\TrickGen.exe
copy %ZQPROJSPATH%\TianShan\bin64\TrickGen.pdb
copy %ZQPROJSPATH%\TianShan\bin64\RTICap.exe
copy %ZQPROJSPATH%\TianShan\bin64\RTICap.pdb

rem copy %VSTRMKITPATH%\x64\PacedIndex.dll
rem copy %VSTRMKITPATH%\x64\PacedVV2.dll
rem copy %VSTRMKITPATH%\x64\PacedVVX.dll

rem copy for EventChannel
copy %ZQPROJSPATH%\TianShan\bin64\EventChannel.exe
copy %ZQPROJSPATH%\TianShan\bin64\EventChannel.pdb

rem copy for EventGateway
copy %ZQPROJSPATH%\TianShan\bin64\EventGateway.exe
copy %ZQPROJSPATH%\TianShan\bin64\EventGateway.pdb

rem copy for NSS
copy %ZQPROJSPATH%\TianShan\bin64\NSS.exe
copy %ZQPROJSPATH%\TianShan\bin64\NSS.pdb

rem copy for C2SS
copy %ZQPROJSPATH%\TianShan\bin64\C2SS.exe
copy %ZQPROJSPATH%\TianShan\bin64\C2SS.pdb
copy %ZQPROJSPATH%\TianShan\bin64\libasync.dll
copy %ZQPROJSPATH%\TianShan\bin64\libasync.pdb

rem copy for EdgeRMService
copy %ZQPROJSPATH%\TianShan\bin64\EdgeRMService.exe
copy %ZQPROJSPATH%\TianShan\bin64\EdgeRMService.pdb
copy %ZQPROJSPATH%\TianShan\bin64\EdgeRMClient.exe
copy %ZQPROJSPATH%\TianShan\bin64\EdgeRMClient.pdb
copy %ZQPROJSPATH%\TianShan\bin64\ERMIServer.exe
copy %ZQPROJSPATH%\TianShan\bin64\ERMIServer.pdb
copy %ZQPROJSPATH%\TianShan\bin64\ErmMan_web.dll
copy %ZQPROJSPATH%\TianShan\bin64\ErmMan_web.pdb

rem copy for DsmccCRG
copy %ZQPROJSPATH%\TianShan\bin64\DsmccCRG.exe
copy %ZQPROJSPATH%\TianShan\bin64\DsmccCRG.pdb
copy %ZQPROJSPATH%\TianShan\bin64\DsmccCRGClient.exe
copy %ZQPROJSPATH%\TianShan\bin64\DsmccCRGClient.pdb

rem copy for TSHammer
copy %ZQPROJSPATH%\TianShan\bin64\TSHammer.exe
copy %ZQPROJSPATH%\TianShan\bin64\TSHammer.pdb

rem copy for ContentClient
copy %ZQPROJSPATH%\TianShan\bin64\ContentClient.exe
copy %ZQPROJSPATH%\TianShan\bin64\ContentClient.pdb

rem copy for HttpCRG
copy %ZQPROJSPATH%\TianShan\bin64\HttpCRG.exe
copy %ZQPROJSPATH%\TianShan\bin64\HttpCRG.pdb
copy %ZQPROJSPATH%\TianShan\bin64\c2loc_web.dll
copy %ZQPROJSPATH%\TianShan\bin64\c2loc_web.pdb

rem copy for DummySS
copy %ZQPROJSPATH%\TianShan\bin64\DummySS.exe
copy %ZQPROJSPATH%\TianShan\bin64\DummySS.pdb

rem copy for DummySvc
copy %ZQPROJSPATH%\TianShan\bin64\DummySvc.exe
copy %ZQPROJSPATH%\TianShan\bin64\DummySvc.pdb

rem copy for DataOnDemand
rem copy for DODContentStore
copy %ZQPROJSPATH%\TianShan\bin64\DODCS.exe
copy %ZQPROJSPATH%\TianShan\bin64\DODCS.pdb

rem copy for DataStream
copy %ZQPROJSPATH%\TianShan\bin64\DataSS.exe
copy %ZQPROJSPATH%\TianShan\bin64\DataSS.pdb

copy %ZQPROJSPATH%\TianShan\bin64\DSClient.exe
copy %ZQPROJSPATH%\TianShan\bin64\DSClient.pdb

rem copy for ContentLib
copy %ZQPROJSPATH%\TianShan\bin64\ContentLib.exe
copy %ZQPROJSPATH%\TianShan\bin64\ContentLib.pdb

rem copy for EventRuleEngine
copy %ZQPROJSPATH%\TianShan\bin64\EventRuleEngine.exe
copy %ZQPROJSPATH%\TianShan\bin64\EventRuleEngine.pdb

rem copy for JndiClient
copy %ZQPROJSPATH%\generic\JndiClient\x64\Release\JndiClient.dll
copy %ZQPROJSPATH%\generic\JndiClient\x64\Release\JndiClient.pdb

copy %ZQPROJSPATH%\TianShan\bin64\OpenVBO_web.dll
copy %ZQPROJSPATH%\TianShan\bin64\OpenVBO_web.pdb

rem copy for BaseCS
copy %ZQPROJSPATH%\TianShan\bin64\BaseCS.exe
copy %ZQPROJSPATH%\TianShan\bin64\BaseCS.pdb

rem copy for GBVSS
copy %ZQPROJSPATH%\TianShan\bin64\GBVSS.exe
copy %ZQPROJSPATH%\TianShan\bin64\GBVSS.pdb

rem copy for GBVSS
copy %ZQPROJSPATH%\TianShan\bin64\GBCS.exe
copy %ZQPROJSPATH%\TianShan\bin64\GBCS.pdb

cd..

rem =========== Preparing subdir modules ===========
mkdir modules64
cd modules64
copy %ZQPROJSPATH%\TianShan\bin64\ssm_tianshan.dll
copy %ZQPROJSPATH%\TianShan\bin64\ssm_tianshan.pdb

copy %ZQPROJSPATH%\TianShan\bin64\pho_SeaChange.dll
copy %ZQPROJSPATH%\TianShan\bin64\pho_SeaChange.pdb

copy %ZQPROJSPATH%\TianShan\bin64\pho_vss.dll
copy %ZQPROJSPATH%\TianShan\bin64\pho_vss.pdb

rem weiwoo modules for PHO_ERM
copy %ZQPROJSPATH%\TianShan\bin64\pho_ERM.dll
copy %ZQPROJSPATH%\TianShan\bin64\pho_ERM.pdb

copy %ZQPROJSPATH%\TianShan\bin64\pho_HttpSS.dll
copy %ZQPROJSPATH%\TianShan\bin64\pho_HttpSS.pdb

rem weiwoo modules for PHO_bcast
copy %ZQPROJSPATH%\TianShan\bin64\pho_bcast.dll
copy %ZQPROJSPATH%\TianShan\bin64\pho_bcast.pdb

rem A3_message plugin for ContentLib
copy %ZQPROJSPATH%\TianShan\bin64\CRM_A3Message.dll
copy %ZQPROJSPATH%\TianShan\bin64\CRM_A3Message.pdb

rem crm_dsmcc plugin for DsmccCRG
copy %ZQPROJSPATH%\TianShan\bin64\CRM_DSMCC.dll
copy %ZQPROJSPATH%\TianShan\bin64\CRM_DSMCC.pdb

rem copy plugin ssm_tianshan_s1
copy %ZQPROJSPATH%\TianShan\bin64\ssm_tianshan_s1.dll
copy %ZQPROJSPATH%\TianShan\bin64\ssm_tianshan_s1.pdb

copy %ZQPROJSPATH%\TianShan\bin64\ssm_ngod2.dll
copy %ZQPROJSPATH%\TianShan\bin64\ssm_ngod2.pdb

rem copy %ZQPROJSPATH%\TianShan\bin64\ssm_ngod.dll
rem copy %ZQPROJSPATH%\TianShan\bin64\ssm_ngod.pdb

rem ssm_richurl
copy %ZQPROJSPATH%\TianShan\bin64\ssm_richurl.dll
copy %ZQPROJSPATH%\TianShan\bin64\ssm_richurl.pdb

rem copy OpenVBO_web
copy %ZQPROJSPATH%\TianShan\bin64\ssm_OpenVBO.dll
copy %ZQPROJSPATH%\TianShan\bin64\ssm_OpenVBO.pdb

copy %ZQPROJSPATH%\TianShan\bin64\OpenVBO_web.dll
copy %ZQPROJSPATH%\TianShan\bin64\OpenVBO_web.pdb

rem copy ssm_LiveChannel
copy %ZQPROJSPATH%\TianShan\bin64\ssm_LiveChannel.dll
copy %ZQPROJSPATH%\TianShan\bin64\ssm_LiveChannel.pdb

rem for CPE service
copy %ZQPROJSPATH%\TianShan\bin64\CPH_RDS.dll
copy %ZQPROJSPATH%\TianShan\bin64\CPH_RDS.pdb
copy %ZQPROJSPATH%\TianShan\bin64\CPH_RTFRDS.dll
copy %ZQPROJSPATH%\TianShan\bin64\CPH_RTFRDS.pdb
copy %ZQPROJSPATH%\TianShan\bin64\CPH_NASCopy.dll
copy %ZQPROJSPATH%\TianShan\bin64\CPH_NASCopy.pdb
copy %ZQPROJSPATH%\TianShan\bin64\CPH_RTI.dll
copy %ZQPROJSPATH%\TianShan\bin64\CPH_RTI.pdb
copy %ZQPROJSPATH%\TianShan\bin64\CPH_RTINAS.dll
copy %ZQPROJSPATH%\TianShan\bin64\CPH_RTINAS.pdb
copy %ZQPROJSPATH%\TianShan\bin64\CPH_nPVR.dll
copy %ZQPROJSPATH%\TianShan\bin64\CPH_nPVR.pdb
copy %ZQPROJSPATH%\TianShan\bin64\CPH_Raw.dll
copy %ZQPROJSPATH%\TianShan\bin64\CPH_Raw.pdb
copy %ZQPROJSPATH%\TianShan\bin64\CPH_CSI.dll
copy %ZQPROJSPATH%\TianShan\bin64\CPH_CSI.pdb

rem copy MODPlugin
copy %ZQPROJSPATH%\TianShan\bin64\MHO_MODPlugIn.dll
copy %ZQPROJSPATH%\TianShan\bin64\MHO_MODPlugIn.pdb
copy %ZQPROJSPATH%\TianShan\bin64\MHO_GBMODPlugIn.dll
copy %ZQPROJSPATH%\TianShan\bin64\MHO_GBMODPlugIn.pdb

rem copy DOD
copy %ZQPROJSPATH%\TianShan\bin64\pho_dod.dll
copy %ZQPROJSPATH%\TianShan\bin64\pho_dod.pdb

rem copy for CRM_A3Server
rem copy %ZQPROJSPATH%\TianShan\bin64\CRM_A3Server.dll
rem copy %ZQPROJSPATH%\TianShan\bin64\CRM_A3Server.pdb

rem copy for CRM_C2Locator
copy %ZQPROJSPATH%\TianShan\bin64\CRM_C2Locator.dll
copy %ZQPROJSPATH%\TianShan\bin64\CRM_C2Locator.pdb

rem copy for CPH_C2Propagation
copy %ZQPROJSPATH%\TianShan\bin64\CPH_C2Propagation.dll
copy %ZQPROJSPATH%\TianShan\bin64\CPH_C2Propagation.pdb

rem copy for ssm_GBss
copy %ZQPROJSPATH%\TianShan\bin64\ssm_GBss.dll
copy %ZQPROJSPATH%\TianShan\bin64\ssm_GBss.pdb

rem copy for ssm_gb_s1
copy %ZQPROJSPATH%\TianShan\bin64\ssm_GBstb.dll
copy %ZQPROJSPATH%\TianShan\bin64\ssm_GBstb.pdb

rem copy EGH_SOAP_CME
copy %ZQPROJSPATH%\TianShan\bin64\EGH_SOAP_CME.dll
copy %ZQPROJSPATH%\TianShan\bin64\EGH_SOAP_CME.pdb

rem copy EGH_JMS
copy %ZQPROJSPATH%\TianShan\bin64\EGH_JMS.dll
copy %ZQPROJSPATH%\TianShan\bin64\EGH_JMS.pdb

rem copy EGH_JMS
copy %ZQPROJSPATH%\TianShan\bin64\EGH_MQTT.dll
copy %ZQPROJSPATH%\TianShan\bin64\EGH_MQTT.pdb

rem copy EGH_SnmpTrap
copy %ZQPROJSPATH%\TianShan\bin64\EGH_SnmpTrap.dll
copy %ZQPROJSPATH%\TianShan\bin64\EGH_SnmpTrap.pdb


cd ..

rem =========== Preparing subdir utils ===========
mkdir utils64
cd utils64
rem copy %ZQPROJSPATH%\build\utils\depends_x64.exe depends.exe
rem copy %ZQPROJSPATH%\build\utils\depends_x64.dll depends.dll
copy %ZQPROJSPATH%\generic\VerCheck\VerCheck64.exe
copy %ZQPROJSPATH%\TianShan\bin\dot.exe
copy %ZQPROJSPATH%\TianShan\bin\freetype6.dll
copy %ZQPROJSPATH%\TianShan\bin\jpeg.dll
copy %ZQPROJSPATH%\TianShan\bin\png.dll
copy %ZQPROJSPATH%\TianShan\bin\z.dll
copy %ZQPROJSPATH%\TianShan\bin\zlib1.dll
copy %ZQPROJSPATH%\build\utils\VerCheck.exe
copy %ZQPROJSPATH%\build\utils\zip.exe
copy %ZQPROJSPATH%\TianShan\bin64\ChPub.exe
copy %ZQPROJSPATH%\TianShan\bin\sac.exe
copy %ZQPROJSPATH%\TianShan\bin64\PathAdmin.exe
copy %ICE_ROOT%\bin\x64\ice3?.dll
copy %ICE_ROOT%\bin\x64\iceutil3?.dll
copy %ICE_ROOT%\bin\x64\libdb4?.dll
copy %ICE_ROOT%\bin\x64\stlport_vc646.dll
copy %ICE_ROOT%\bin\x64\libexpat.dll

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

copy %ZQPROJSPATH%\TianShan\TsStat.bat

rem copy zqsnmp Utils
copy %ZQPROJSPATH%\TianShan\x64\release\zqsnmp.exe

cd ..

mkdir ctf
cd ctf
mkdir bin64
cd bin64
copy %RTFLIBSDKPATH%\exe\x64\CommonTrickFiles.dll
copy %RTFLIBSDKPATH%\exe\x64\CommonTrickFiles.pdb
cd ..
cd ..

rem =========== Kit siganitures ===========
%VersionCheck% bin64 >VersionInfo_TianShan64.txt
%VersionCheck% modules64 >>VersionInfo_TianShan64.txt
%VersionCheck% ctf\\bin64 >>VersionInfo_TianShan64.txt

copy VersionInfo_TianShan64.txt bin64\

del /q/f  ..\TianShan_setup.zip

rem %PACKCMD% -r ..\TianShan_setup.zip .

rem pause

cd ..
rem rd /s/q buildtemp

endlocal
