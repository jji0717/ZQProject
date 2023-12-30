include  $(top_srcdir)/config/compileflags.mk

AUTOMAKE_OPTIONS=foreign
lib_LIBRARIES=libvrep.a
libvrep_a_SOURCES=vrep.cpp  VrepListener.cpp  VrepMessage.cpp  VrepNotificationMessage.cpp \
		 VrepOpenMessage.cpp  VrepSpeaker.cpp  VrepStates.cpp  VrepTransport.cpp  \
		 VrepUpdateMessage.cpp  VrepUtils.cpp
libvrep_a_LIBADD=
AM_LDFLAGS=
tsflag:="ar"
goal:=libvrep.a
include $(top_srcdir)/config/endpros.mk

