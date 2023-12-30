TOPDIR := ..

ifneq ("$(origin ZQProjsPath)", "environment")
$(error 'ZQProjsPath' not defined)
endif

DESTDIR := ./3rdpbin
CP := cp -vfp

include $(TOPDIR)/build/defines.mk


install: destfolder fo_ICE fo_BOOST fo_SNMP fo_MISC

clean:
	rm -vf $(DESTDIR)/*


veryclean: clean
	rm -rf $(DESTDIR)

destfolder:
	mkdir -p $(DESTDIR)


ICE_FILES := Ice IceUtil IceStorm IceBox IceStormService IceGrid Glacier2 Freeze

ifneq ("$(_centos_ver)", "7")
ICE_VERSION := 3.2.2
ICE_VERSION_MAJ := 32
BDB_VERSION := 4.5
else
ICE_VERSION := 3.6.1
ICE_VERSION_MAJ := 36
BDB_VERSION := 5.3
endif

fo_ICE:
	for f in $(ICE_FILES); do \
		$(CP) $(_ice_dir)/lib/{lib$$f.so.$(ICE_VERSION),lib$$f.so.$(ICE_VERSION_MAJ)}  $(DESTDIR)/ ; \
	done; \
	$(CP) $(_ice_dir)/lib/libdb_cxx-$(BDB_VERSION).so  $(DESTDIR)/ 


BOOST_FILES := boost_regex boost_thread-mt boost_system-mt
BOOST_VERSION := 1.56.0
BOOST_VERSION_MAJ := 1
fo_BOOST:
	if [ $(_boost_dir) != "/usr" ]; then \
		for f in $(BOOST_FILES); do \
			$(CP) $(_boost_dir)/lib64/{lib$$f.so.$(BOOST_VERSION),lib$$f.so}  $(DESTDIR)/ ; \
		done \
	fi
	

SNMP_VERSION := 10.0.3
SNMP_VERSION_MAJ := 10
SNMPP_VERSION := 3.2.24
SNMPP_VERSION_MAJ := 3
fo_SNMP:
	if [ $(_net-snmp_dir) != "/usr" ]; then \
		$(CP) $(_net-snmp_dir)/lib/{libsnmp.so.$(SNMP_VERSION),libsnmp.so.$(SNMP_VERSION_MAJ)}  $(DESTDIR)/ ; \
	fi; \
	if [ $(_snmp++_dir) != "/usr" ]; then \
		$(CP) $(_snmp++_dir)/lib/{libsnmp++.so.$(SNMPP_VERSION),libsnmp++.so.$(SNMPP_VERSION_MAJ)}  $(DESTDIR)/ ; \
	fi;

	

NL_VERSION := 1.1
NL_VERSION_MAJ := 1
PCAP_VERSION := 1.3.0
PCAP_VERSION_MAJ := 1
CURL_VERSION := 4.3.0
CURL_VERSION_MAJ := 4
ODBC_VERSION := 2.0.0
ODBC_VERSION_MAJ := 2
UUID_VERSION := 1.2
UUID_VERSION_MAJ := 1
fo_MISC:
	if [ $(_libnl_dir) != "/usr" ]; then \
		$(CP) $(_libnl_dir)/lib/{libnl.so.$(NL_VERSION),libnl.so.$(NL_VERSION_MAJ)}  $(DESTDIR)/ ; \
	fi; \
	if [ $(_pcap_dir) != "/usr" ]; then \
		$(CP) $(_pcap_dir)/lib/{libpcap.so.$(PCAP_VERSION),libpcap.so.$(PCAP_VERSION_MAJ)}  $(DESTDIR)/ ; \
	fi; \
	if [ $(_curl_dir) != "/usr" ]; then \
		$(CP) $(_curl_dir)/lib/{libcurl.so,libcurl.so.$(CURL_VERSION),libcurl.so.$(CURL_VERSION_MAJ)}  $(DESTDIR)/ ; \
	fi; \
	if [ $(_odbc_dir) != "/usr" ]; then \
		$(CP) $(_odbc_dir)/lib/{libodbc.so.$(ODBC_VERSION),libodbc.so.$(ODBC_VERSION_MAJ)}  $(DESTDIR)/ ; \
		$(CP) $(_odbc_dir)/lib/{libodbcinst.so.$(ODBC_VERSION),libodbcinst.so.$(ODBC_VERSION_MAJ)}  $(DESTDIR)/ ; \
	fi; \
	if [ $(_e2fsprogs_dir) != "/usr" ]; then \
		$(CP) $(_e2fsprogs_dir)/lib/{libuuid.so.$(UUID_VERSION),libuuid.so.$(UUID_VERSION_MAJ),libuuid.so}  $(DESTDIR)/ ; \
	fi; \

