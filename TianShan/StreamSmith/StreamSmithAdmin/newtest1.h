


Thread1 ->

��ѯvtrsm session��״̬

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

	seek��һ��ָ����λ��
	1.	�� VstrmFindFirstFileEx ȡ��vstrm�ļ��Ĵ�С
	2.	�� VstrmCreateFile  && VstrmReadFile ... ȡ���ļ���bitrate
	3.	�����ļ�����������ʱ�� the playtime of the vtrsm file playTime
	4.	����load sessionʱ��timeOffset =playTimie-offset
	5.	�� VstrmClassControlSessionEx1(_mgr.classHandle(), 
											0,
											VSTRM_GEN_CONTROL_LOAD,
											&parmsV2, 
											sizeof(parmsV2), 
											cbItemCompleted, 
											(ULONG)this);
		��load���vstrm file,�������潫����IOCTL_LOAD_PARAMS::TimeSkip=timeOffset
	6.	���ϵĲ鿴��Thread1��ȡ����session����Ϣ
	
	