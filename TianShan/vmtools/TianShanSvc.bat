set CMD=%1
if .. == .%CMD%. set CMD=start

for %%s in (ChOdSvc EventChannel EventGateway MODSvc RtspProxy MediaClusterCS Weiwoo SiteAdminSvc Sentry NSS) do net %CMD% %%s
