Summary: seamonlx -- Seachange system monitoring deamon
Name: seamonlx
Version: %sversion.%srelease
Release: %sbuildnumber
License: Proprietary
Group: SeaChange
URL: http://www/schange.com
BuildRoot: /var/tmp/%{name}-buildroot

%description
SeaChange Seamon deamon monitors system health and generates alerts.

%prep

%install
mkdir -p ${RPM_BUILD_ROOT}/%targetdir
mkdir -p ${RPM_BUILD_ROOT}/%targetdir/../utils
mkdir -p ${RPM_BUILD_ROOT}/etc/init.d
install %seamonlxDir/seamonlx ${RPM_BUILD_ROOT}/%targetdir
install %seamonlxDir/../scripts/seamonlxd ${RPM_BUILD_ROOT}/etc/init.d
install %seamonlxDir/../scripts/lsi_info.pl ${RPM_BUILD_ROOT}/%targetdir
install %seamonlxDir/../scripts/findAdapterChildelem.pl ${RPM_BUILD_ROOT}/%targetdir
install %seamonlxDir/../scripts/findEnclChildelems.pl ${RPM_BUILD_ROOT}/%targetdir
install %seamonlxDir/../scripts/common_functions.pl ${RPM_BUILD_ROOT}/%targetdir
install %seamonlxDir/../scripts/enclElements.pl ${RPM_BUILD_ROOT}/%targetdir
install %seamonlxDir/../scripts/enclInfo.pl ${RPM_BUILD_ROOT}/%targetdir
install %seamonlxDir/../scripts/enclPhys.pl ${RPM_BUILD_ROOT}/%targetdir
install %seamonlxDir/../scripts/diskInfo.pl ${RPM_BUILD_ROOT}/%targetdir
install %seamonlxDir/../tools/lsiutil.x86_64 ${RPM_BUILD_ROOT}/%targetdir/../utils

%post
cp /etc/syslog.conf /etc/syslog.conf.sav
echo "local6.none     /var/log/messages" >> /etc/syslog.conf
echo "local6.*        /var/log/alert.log" >> /etc/syslog.conf
cp /etc/services /etc/services.sav
echo "seamonlx-xmlrpc	59732/tcp	#SEAC Mon xmlrpc" >> /etc/services
echo "seamonlx-xmlrpc	59732/udp	#SEAC Mon xmlrpc" >> /etc/services
echo "seamonlx-broadcast	59733/tcp	#SEAC Mon broadcast" >> /etc/services
echo "seamonlx-broadcast	59733/udp	#SEAC Mon broadcast" >> /etc/services
cp /etc/logrotate.conf /etc/logrotate.sav
echo "# parameters for alert.log updated by seamonlx setup" >> /etc/logrotate.conf
echo "/var/log/alert.log {"                                 >> /etc/logrotate.conf
echo "    size=20M        "                                 >> /etc/logrotate.conf
echo "    rotate 5        "                                 >> /etc/logrotate.conf
echo "}                   "                                 >> /etc/logrotate.conf
chkconfig --add seamonlxd
/sbin/service syslog restart
   
%postun
cp /etc/syslog.conf /etc/syslog.conf.seamonlx
cp /etc/services /etc/services.seamonlx
(echo "g/local6/d"; echo 'wq') | ex -s /etc/syslog.conf
(echo "g/seamonlx/d";echo 'wq') | ex -s /etc/services
(echo "g/seamonlxalert/d";echo 'wq') | ex -s /etc/logrotate.conf
rm -f /etc/init.d/seamonlxd
chkconfig --del seamonlxd
/sbin/service syslog restart

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
%attr(0755, root, root) /%targetdir/seamonlx
%attr(0755, root, root) /etc/init.d/seamonlxd
%attr(0755, root, root) /%targetdir/lsi_info.pl
%attr(0755, root, root) /%targetdir/findAdapterChildelem.pl
%attr(0755, root, root) /%targetdir/findEnclChildelems.pl
%attr(0755, root, root) /%targetdir/common_functions.pl
%attr(0755, root, root) /%targetdir/enclElements.pl
%attr(0755, root, root) /%targetdir/enclInfo.pl
%attr(0755, root, root) /%targetdir/enclPhys.pl
%attr(0755, root, root) /%targetdir/diskInfo.pl
%attr(0755, root, root) /%targetdir/../utils/lsiutil.x86_64

%doc


%changelog
* Wed Mar 17 2010 root <jie.zhang@schange.com> - 
- Initial build.

