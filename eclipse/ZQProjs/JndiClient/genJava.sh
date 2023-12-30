javac -classpath ${ZQProjsPath}/Generic/JndiClient:${ZQProjsPath}/Generic/JndiClient/jbossall-client.jar  ${ZQProjsPath}/Generic/JndiClient/NestedJmsSession.java ${ZQProjsPath}/Generic/JndiClient/NestedJndiClient.java -Xlint:unchecked
javah -classpath ${ZQProjsPath}/Generic/JndiClient:${ZQProjsPath}/Generic/JndiClient/jbossall-client.jar -d ${ZQProjsPath}/Generic/JndiClient NestedJmsSession NestedJndiClient
jar -cvf ${ZQProjsPath}/Generic/JndiClient/JndiClient.jar ${ZQProjsPath}/Generic/JndiClient/*.class
install -v -D ${ZQProjsPath}/Generic/JndiClient/JndiClient.jar ${ZQProjsPath}/TianShan/bin64/java/JndiClient.jar
install -v -D ${ZQProjsPath}/Generic/JndiClient/jbossall-client.jar ${ZQProjsPath}/TianShan/bin64/java/jbossall-client.jar