const MIBExpose gTblMibExpose[] = {
{"ssm_NGOD/ssm_NGOD2/RTSPSession/cacheSize",           20},
{"ssm_NGOD/ssm_NGOD2/Database/path",                   24},
{"ssm_NGOD/ssm_NGOD2/Announce/useGlobalCSeq",          30},
{"ssm_NGOD/ssm_NGOD2/Response/setupFailureWithSessId", 102},
{"ssm_NGOD/ssm_NGOD2/Response/streamCtrlProt",         103},
{"ssm_NGOD/ssm_NGOD2/LAM/TestMode/enabled",            110},
{"ssm_NGOD/ssm_NGOD2/playlistControl/enableEOT",       111},
{"RtspProxy/RequestProcess/threads",                   135},
{"RtspProxy/RequestProcess/maxPendingRequest",         137},
{"RtspProxy/EventPublisher/timeout",                   146},
{"RtspProxy/SocketServer/rtspPort",                    154},
{"RtspProxy/SocketServer/maxConnections",              156},
{"RtspProxy/SocketServer/threads",                     157},
{"RtspProxy/SocketServer/idleScanInterval",            162},
{"RtspProxy/SocketServer/maxSessions",                 163},
{"RtspProxy/SocketServer/IncomingMessage/maxLen",      171},
{"#Statistics# ANNOUNCE-per10min",                     300},

{NULL, 0},
};

