include  $(top_srcdir)/config/compileflags.mk

_test := JndiClientTest
_files := NestedJmsSession.java NestedJndiClient.java
GENFILES := $(_files:.java=.h) $(_files:.java=.class) JndiClient.jar
EXTRA_FILES := NestedJmsSession_ReconnectThread.h 'NestedJmsSession$$ReconnectThread.class' $(_test) $(_test).log

_classpath := .:jbossall-client.jar
AUTOMAKE_OPTIONS=foreign
lib_LTLIBRARIES=libJndiClient.la
libJndiClient_la_SOURCES=NestedJndiClient.h NestedJmsSession.h JndiClient.cpp
3rds : $(_files:%.java=%.class)
3rdh : $(_files:%.java=%.h)

JndiClient.jar : $(_files:.java=.class)
	$(JAR) -cvf $@ $^ 'NestedJmsSession$$ReconnectThread.class'

%.h : %.class
	@echo "generating $@ ..."; \
	$(JAVAH) -classpath $(_classpath) $*

%.class : %.java
	@echo "generating $@ ..."; \
	$(JAVAC) -classpath $(_classpath) $<

clean-local:
	-rm -rf {NestedJndiClient,NestedJmsSession}.{h,class} $(csbin_dir)/$(lib_LTLIBRARIES:%.la=%)*  $(cssyb_dir)/$(lib_LTLIBRARIES:%.la=%)*
	-rm -rf NestedJmsSession\$ReconnectThread.class NestedJmsSession_ReconnectThread.h
tsflag:="so"
goal:=libJndiClient.so
include $(top_srcdir)/config/endpros.mk

