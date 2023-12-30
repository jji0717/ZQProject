${GSoapPath}/wsdl2h -o ${ZQProjsPath}/TianShan/EventGateway/Modules/EGH_SOAP/EGH_SOAP_CME/CMEService.h  ${ZQProjsPath}/TianShan/EventGateway/Modules/EGH_SOAP/EGH_SOAP_CME/CMEService.wsdl 

${GSoapPath}/soapcpp2 -I/usr/share/gsoap/import -L -C -x -pCMESOAP -d  ${ZQProjsPath}/TianShan/EventGateway/Modules/EGH_SOAP/EGH_SOAP_CME/   ${ZQProjsPath}/TianShan/EventGateway/Modules/EGH_SOAP/EGH_SOAP_CME/CMEService.h
