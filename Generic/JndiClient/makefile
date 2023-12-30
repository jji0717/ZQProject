TOPDIR := ../..

include $(TOPDIR)/build/defines.mk

SOLIB := JndiClient

_test := JndiClientTest

OBJS := JndiClient.os

_files := NestedJmsSession.java NestedJndiClient.java
GENFILES := $(_files:.java=.h) $(_files:.java=.class) JndiClient.jar
EXTRA_FILES := NestedJmsSession_ReconnectThread.h 'NestedJmsSession$$ReconnectThread.class' $(_test) $(_test).log

include $(TOPDIR)/build/common.mk

_classpath := .:jbossall-client.jar 

preinstall: JndiClient.jar jbossall-client.jar
	$(Q)install -v -D JndiClient.jar $(_dest)/java/JndiClient.jar
	$(Q)install -v -D jbossall-client.jar $(_dest)/java/jbossall-client.jar

JndiClient.jar: $(_files:.java=.class) 
	$(Q)$(JAR) -cvf $@ $^ 'NestedJmsSession$$ReconnectThread.class' 

%.class: %.java 
	$(Q)echo "generating $@ ..."; \
	$(JAVAC) -classpath $(_classpath) $< 

%.h: %.class
	$(Q)echo "generating $@ ..."; \
	$(JAVAH) -classpath $(_classpath) $*

test: JndiClientTest.cpp
	$(Q)echo "compiling $< ..."
	$(Q)$(CXX) -Wall -g -o JndiClientTest $< -I$(TOPDIR)/Common \
	    -L. -L$(_bindir) -lZQCommon -lJndiClient \
	    -Wl,-rpath=$(_bindir)

# vim: ts=4
