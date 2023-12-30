


Thread1 ->

轮询vtrsm session的状态

	if (!DeviceIoControl (vodHandle,
				(DWORD) IOCTL_VOD_DRIVER_QUERY_NEXT_SESSION, 
				_pSessionInfoParam[i],
				VOD_IOCTL_SESS_INFO_BUF_SIZE, 
				_pSessionInfoParam[i],
				VOD_IOCTL_SESS_INFO_BUF_SIZE, 
				&amountRead,
				NULL))
				break;



Thread2 ->

	seek到一个指定的位置
	1.	用 VstrmFindFirstFileEx 取得vstrm文件的大小
	2.	用 VstrmCreateFile  && VstrmReadFile ... 取得文件的bitrate
	3.	计算文件的正常播放时间 the playtime of the vtrsm file playTime
	4.	计算load session时的timeOffset =playTimie-offset
	5.	用 VstrmClassControlSessionEx1(_mgr.classHandle(), 
											0,
											VSTRM_GEN_CONTROL_LOAD,
											&parmsV2, 
											sizeof(parmsV2), 
											cbItemCompleted, 
											(ULONG)this);
		来load这个vstrm file,参数里面将传入IOCTL_LOAD_PARAMS::TimeSkip=timeOffset
	6.	不断的查看从Thread1中取到的session的信息
	
	