ant -f ${ZQProjsPath}/Generic/CdmiFuse/sdk/java/ant.xml
javah -classpath ${ZQProjsPath}/Generic/CdmiFuse/sdk/java:${ZQProjsPath}/Generic/CdmiFuse/sdk/java/bin/AquaClient.jar -o ${ZQProjsPath}/Generic/CdmiFuse/sdk/NestedClient.h  com.xormedia.aqua.sdk.NestedClient