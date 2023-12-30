/*------------------------------------------------------------------------------
  Copyright (c) 2005 SeaChange International (China), ZQ Interactive Ltd.
  ------------------------------------------------------------------------------ 
    $Archive: /ZQProjs/TianShan/DataOnDemand/Phase1/DODSTREAMER/filter/itv_parse/Parser/inc/object_debug.h $
    $Author: Admin $
    $Revision: 1 $
    $Date: 10-11-12 16:04 $
  ------------------------------------------------------------------------------ 
*/

#ifndef ITV_OBJECTFETCH_DEBUG
#define ITV_OBJECTFETCH_DEBUG


/**************************************** *********************************/
/* string.h */
#define itv_strtol(a,b,c) strtol((char *)a, (char **)b, (int)c)
#define itv_strtod(a,b) strtod((char *)a, (char **)b)

#define itv_abs(a) abs(a)
#define itv_atoi(a) atoi((char *)a)
#define itv_itoa(a,b,c) itoa((int)a, (char *)b, (int)c)
#define itv_atof(a) atof((char *)a)
#define itv_pow(a,b) pow((double)a, (double)b)
#define itv_floor(a) floor((double)a)

#define itv_strlen(a) strlen((char *)a)
#define itv_strcpy(a,b) strcpy((char *)a, (char *)b)
#define itv_strcmp(a,b) strcmp((char *)a, (char *)b)
#define itv_strncpy(a,b,c) strncpy((char *)a, (char *)b, (int)c)
#define itv_strncmp(a,b,c) strncmp((char *)a, (char *)b, (int)c)
#define itv_strchr(a,b) strchr((char *)a, (char)b)
#define itv_strstr(a,b) strstr((char *)a, (char *)b)
#define itv_strcat(a,b) strcat((char *)a, (char *)b)

#define itv_memset(a,b,c) memset((void *)a, (char)b, (unsigned int)c)
#define itv_memcpy(a,b,c) memcpy((char *)a, (char *)b, (unsigned int)c)
#define itv_memcmp(a,b,c) memcmp((char *)a, (char *)b, (unsigned int)c)


/* time.h */
#define itv_localtime localtime
#define itv_mktime mktime
#define itv_time time
#define itv_gmtime gmtime
#define itv_ctime ctime



/* stdio.h*/
#define itv_sscanf sscanf
#define itv_scanf scanf
#define itv_sprintf sprintf
#define itv_snprintf snprintf
#define itv_print 	PAL_Printf
#define itv_printf 	PAL_Printf
//#define itv_print 	sttbx_Print
#define itv_fread fread
#define itv_fseek fseek
#define itv_ftell ftell
#define itv_fopen fopen
#define itv_fgetc fgetc
#define itv_fgets fgets
#define itv_fputs fputs
#define itv_fputc fputc
#define itv_fclose fclose
#define itv_EOF EOF

/* */
#define itv_va_list va_list
#define itv_va_start va_start
#define itv_vsprintf vsprintf
#define itv_va_end va_end

#if 0
void ItvObj_ErrPrintf(const ITV_BYTE * format, ...);
void ItvObj_WarnPrintf(const ITV_BYTE * format, ...);
void ItvObj_InfoPrintf(const ITV_BYTE * format, ...);
#else
#define ItvObj_ErrPrintf printf
#define ItvObj_WarnPrintf printf
#define ItvObj_InfoPrintf printf
#endif


#define itv_getch() getch()
#define itv_getchar() getchar()


#define ITV_ERR_RETURN(itv_err)	if(itv_err != ITV_NO_ERROR){ ItvObj_ErrPrintf("Error return %d, line is %d, file name is %s\n", itv_err, __LINE__, __FILE__); return itv_err;}
#define ITV_GET_RETURN(itv_err)		if(itv_err <= 0){ItvObj_ErrPrintf("Error get return %d, line is %d, file name is %s\n", itv_err, __LINE__, __FILE__ ); return itv_err;}
#define ITV_ERR_RETURN_NO_LOG(itv_err)	if(itv_err != ITV_NO_ERROR){return itv_err;}
#define ITV_GET_RETURN_NO_LOG(itv_err)		if(itv_err <= 0){return itv_err;}
#define ITV_GET_NULL_RETURN(itv_err)		if(itv_err <= 0){return NULL;}
#define ITV_RETURN_ERROR(a,b)		{ItvObj_ErrPrintf(a);return b;}

#ifdef ITV_LEAK_DEBUG_MEMORY

void* ItvPAL_AllocMemory(ITV_UDWORD MemorySize, ITV_UDWORD MemoryProperty);
ITV_DWORD ItvPAL_FreeMemory(void* MemoryBlock);

#define ItvPAL_CreateMutex(a)					PAL_CreateMutex(a)
#define ItvPAL_CloseMutex(a)						PAL_CloseMutex(a)
#define ItvPAL_GetMutex(a,b,c)					PAL_GetMutex(a,b,c)
#define ItvPAL_FreeMutex(a)						PAL_FreeMutex(a)
#define ItvPAL_SetMute()							PAL_SetMute()
#define ItvPAL_CancelMute()						PAL_CancelMute()

#define ItvPAL_CreateSemaphore(a)				PAL_CreateSemaphore(a)
#define ItvPAL_CloseSemaphore(a)				PAL_CloseSemaphore(a)
#define ItvPAL_SignalSemaphore(a)				PAL_SignalSemaphore(a)
#define ItvPAL_WaitSemaphore(a,b,c)				PAL_WaitSemaphore(a,b,c)

#define ItvPAL_CreateProcess(a,b,c,d,e)			PAL_CreateProcess(a,b,c,d,e)
#define ItvPAL_ProcessSleep(a)					PAL_ProcessSleep(a)

#define ItvPAL_GetAudioMode()		 			PAL_GetAudioMode()
#define ItvPAL_SetAudioMode(a)					PAL_SetAudioMode(a)
#define ItvPAL_GetAudioCount(a)					PAL_GetAudioCount(a)
#define ItvPAL_GetAudioLanguageName(a,b,c)		PAL_GetAudioLanguageName(a,b,c)
#define ItvPAL_SetAudioLanguage(a)				PAL_SetAudioLanguage(a)
#define ItvPAL_GetAudioLanguage(a,b,c)			PAL_GetAudioLanguage(a)

#define ItvPAL_GetSystemInfo(a)		 			PAL_GetSystemInfo(a)
#define ItvPAL_GetSystemInfoStr(a,b,c)			PAL_GetSystemInfoStr(a,b,c)
#define ItvPAL_GetITVLibraryVersion(a,b)			PAL_GetITVLibraryVersion(a,b)
#define ItvPAL_SetTime(a)						PAL_SetTime(a)
#define ItvPAL_SetTimeEx(a)						PAL_SetTimeEx(a)

#define ItvPAL_ConnectPrivateData(a,b,c)			PAL_ConnectPrivateData(a,b,c)
#define ItvPAL_DisconnectPrivateData(a,b)			PAL_DisconnectPrivateData(a,b)

#define ItvPAL_Tune(a,b,c)	 					PAL_Tune(a,b,c)
#define ItvPAL_GetTSID()							PAL_GetTSID()
#define ItvPAL_SelectMpeg(a,b)					PAL_SelectMpeg(a,b)
#define ItvPAL_PlayMpeg(a,b,c,d,e)				PAL_PlayMpeg(a,b,c,d,e)
#define ItvPAL_StopMpeg()						PAL_StopMpeg()
#define ItvPAL_PlayMpegStill(a,b)					PAL_PlayMpegStill(a,b)
#define ItvPAL_StopMpegStill()					PAL_StopMpegStill()
#define ItvPAL_SwitchVideoMpegStill(a)			PAL_SwitchVideoMpegStill(a)

#define ItvPAL_CreateSocket(a,b,c)		 		PAL_CreateSocket(a,b,c)
#define ItvPAL_CloseSocket(a)					PAL_CloseSocket(a)
#define ItvPAL_JoinGroup(a,b)						PAL_JoinGroup(a,b)
#define ItvPAL_LeaveGroup(a)						PAL_LeaveGroup(a)
#define ItvPAL_SetDNSServer(a)					PAL_SetDNSServer(a)
#define ItvPAL_GetHostByName(a)	 				PAL_GetHostByName(a)
#define ItvPAL_ConnectToHost(a,b,c)				PAL_ConnectToHost(a,b,c)
#define ItvPAL_ConnectToHostEx(a,b,c)			PAL_ConnectToHostEx(a,b,c)
#define ItvPAL_DisconnectFromHost(a)				PAL_DisconnectFromHost(a)
#define ItvPAL_Bind(a,b)							PAL_Bind(a,b)
#define ItvPAL_Unbind(a)		 					PAL_Unbind(a)
#define ItvPAL_Listen(a,b)						PAL_Listen(a,b)
#define ItvPAL_SendMsg(a,b,c)					PAL_SendMsg(a,b,c)
#define ItvPAL_ReceiveMessage(a,b,c,d,e)			PAL_ReceiveMessage(a,b,c,d,e)
#define ItvPAL_SendMsgTo(a,b,c,d,e)				PAL_SendMsgTo(a,b,c,d,e)
#define ItvPAL_SendMsgToEx(a,b,c,d,e)		 	PAL_SendMsgToEx(a,b,c,d,e)
#define ItvPAL_ReceiveMessageFrom(a,b,c,d,e,f,g,h) PAL_ReceiveMessageFrom(a,b,c,d,e,f,g,h)
#define ItvPAL_ReceiveMessageFromEx(a,b,c,d,e,f,g,h)	PAL_ReceiveMessageFromEx(a,b,c,d,e,f,g,h)

#define ItvPAL_OpenFile(a,b)						PAL_OpenFile(a,b)
#define ItvPAL_CloseFile(a)						PAL_CloseFile(a)
#define ItvPAL_ReadFile(a,b,c,d,e)					PAL_ReadFile(a,b,c,d,e)
#define ItvPAL_WriteFile(a,b,c)					PAL_WriteFile(a,b,c)
#define ItvPAL_SeekPosition(a,b,c)				PAL_SeekPosition(a,b,c)
#define ItvPAL_RemoveFile(a)						PAL_RemoveFile(a)
#define ItvPAL_GetFileProperty(a,b,c,d,e)			PAL_GetFileProperty(a,b,c,d,e)

#define ItvPAL_RTSPPlayMPEG(a)					PAL_RTSPPlayMPEG(a)
#define ItvPAL_RTSPSetRate(a)					PAL_RTSPSetRate(a)

#else

#define ItvPAL_AllocMemory(a,b)					PAL_AllocMemory(a,b)
#define ItvPAL_FreeMemory(a)					PAL_FreeMemory(a)

#define ItvPAL_CreateMutex(a)					PAL_CreateMutex(a)
#define ItvPAL_CloseMutex(a)						PAL_CloseMutex(a)
#define ItvPAL_GetMutex(a,b,c)					PAL_GetMutex(a,b,c)
#define ItvPAL_FreeMutex(a)						PAL_FreeMutex(a)
#define ItvPAL_SetMute()							PAL_SetMute()
#define ItvPAL_CancelMute()						PAL_CancelMute()

#define ItvPAL_CreateSemaphore(a)				PAL_CreateSemaphore(a)
#define ItvPAL_CloseSemaphore(a)				PAL_CloseSemaphore(a)
#define ItvPAL_SignalSemaphore(a)				PAL_SignalSemaphore(a)
#define ItvPAL_WaitSemaphore(a,b,c)				PAL_WaitSemaphore(a,b,c)

#define ItvPAL_CreateProcess(a,b,c,d,e)			PAL_CreateProcess(a,b,c,d,e)
#define ItvPAL_ProcessSleep(a)					PAL_ProcessSleep(a)

#define ItvPAL_GetAudioMode()		 			PAL_GetAudioMode()
#define ItvPAL_SetAudioMode(a)					PAL_SetAudioMode(a)
#define ItvPAL_GetAudioCount(a)					PAL_GetAudioCount(a)
#define ItvPAL_GetAudioLanguageName(a,b,c)		PAL_GetAudioLanguageName(a,b,c)
#define ItvPAL_SetAudioLanguage(a)				PAL_SetAudioLanguage(a)
#define ItvPAL_GetAudioLanguage(a,b,c)			PAL_GetAudioLanguage(a)

#define ItvPAL_GetSystemInfo(a)		 			PAL_GetSystemInfo(a)
#define ItvPAL_GetSystemInfoStr(a,b,c)			PAL_GetSystemInfoStr(a,b,c)
#define ItvPAL_GetITVLibraryVersion(a,b)			PAL_GetITVLibraryVersion(a,b)
#define ItvPAL_SetTime(a)						PAL_SetTime(a)
#define ItvPAL_SetTimeEx(a)						PAL_SetTimeEx(a)

#define ItvPAL_ConnectPrivateData(a,b,c)			PAL_ConnectPrivateData(a,b,c)
#define ItvPAL_DisconnectPrivateData(a,b)			PAL_DisconnectPrivateData(a,b)

#define ItvPAL_Tune(a,b,c)	 					PAL_Tune(a,b,c)
#define ItvPAL_GetTSID()							PAL_GetTSID()
#define ItvPAL_SelectMpeg(a,b)					PAL_SelectMpeg(a,b)
#define ItvPAL_PlayMpeg(a,b,c,d,e)				PAL_PlayMpeg(a,b,c,d,e)
#define ItvPAL_StopMpeg()						PAL_StopMpeg()
#define ItvPAL_PlayMpegStill(a,b)					PAL_PlayMpegStill(a,b)
#define ItvPAL_StopMpegStill()					PAL_StopMpegStill()
#define ItvPAL_SwitchVideoMpegStill(a)			PAL_SwitchVideoMpegStill(a)

#define ItvPAL_CreateSocket(a,b,c)		 		PAL_CreateSocket(a,b,c)
#define ItvPAL_CloseSocket(a)					PAL_CloseSocket(a)
#define ItvPAL_JoinGroup(a,b)						PAL_JoinGroup(a,b)
#define ItvPAL_LeaveGroup(a)						PAL_LeaveGroup(a)
#define ItvPAL_SetDNSServer(a)					PAL_SetDNSServer(a)
#define ItvPAL_GetHostByName(a)	 				PAL_GetHostByName(a)
#define ItvPAL_ConnectToHost(a,b,c)				PAL_ConnectToHost(a,b,c)
#define ItvPAL_ConnectToHostEx(a,b,c)			PAL_ConnectToHostEx(a,b,c)
#define ItvPAL_DisconnectFromHost(a)				PAL_DisconnectFromHost(a)
#define ItvPAL_Bind(a,b)							PAL_Bind(a,b)
#define ItvPAL_Unbind(a)		 					PAL_Unbind(a)
#define ItvPAL_Listen(a,b)						PAL_Listen(a,b)
#define ItvPAL_SendMsg(a,b,c)					PAL_SendMsg(a,b,c)
#define ItvPAL_ReceiveMessage(a,b,c,d,e)			PAL_ReceiveMessage(a,b,c,d,e)
#define ItvPAL_SendMsgTo(a,b,c,d,e)				PAL_SendMsgTo(a,b,c,d,e)
#define ItvPAL_SendMsgToEx(a,b,c,d,e)		 	PAL_SendMsgToEx(a,b,c,d,e)
#define ItvPAL_ReceiveMessageFrom(a,b,c,d,e,f,g,h) PAL_ReceiveMessageFrom(a,b,c,d,e,f,g,h)
#define ItvPAL_ReceiveMessageFromEx(a,b,c,d,e,f,g,h)	PAL_ReceiveMessageFromEx(a,b,c,d,e,f,g,h)

#define ItvPAL_OpenFile(a,b)						PAL_OpenFile(a,b)
#define ItvPAL_CloseFile(a)						PAL_CloseFile(a)
#define ItvPAL_ReadFile(a,b,c,d,e)					PAL_ReadFile(a,b,c,d,e)
#define ItvPAL_WriteFile(a,b,c)					PAL_WriteFile(a,b,c)
#define ItvPAL_SeekPosition(a,b,c)				PAL_SeekPosition(a,b,c)
#define ItvPAL_RemoveFile(a)						PAL_RemoveFile(a)
#define ItvPAL_GetFileProperty(a,b,c,d,e)			PAL_GetFileProperty(a,b,c,d,e)

#define ItvPAL_RTSPPlayMPEG(a)					PAL_RTSPPlayMPEG(a)
#define ItvPAL_RTSPSetRate(a)					PAL_RTSPSetRate(a)
#endif

#define itv_err_print(a, b) if (b) itv_print("%s = 0x%08X   ...\n",a, b); else itv_print("%s OKAY!\n",a); 
#define itv_min(a, b) (((a) < (b)) ? (a) : (b))
#define itv_max(a, b) (((a) > (b)) ? (a) : (b))
#if 0
#define itv_sum2(a, b) (a<<8)|b
#define itv_sum3(a, b, c) (a<<16)|(b<<8)|c
#define itv_sum4(a, b, c, d) ((ITV_DWORD)a<<24)|((ITV_DWORD)b<<16)|((ITV_DWORD)c<<8)|(ITV_DWORD)d
#endif

ITV_WORD itv_sum2(ITV_UBYTE a, ITV_UBYTE b);
ITV_DWORD itv_sum2_int16(ITV_WORD a, ITV_WORD b);
ITV_DWORD itv_sum3(ITV_UBYTE a, ITV_UBYTE b, ITV_UBYTE c);
ITV_DWORD itv_sum4(ITV_UBYTE a, ITV_UBYTE b, ITV_UBYTE c, ITV_UBYTE d);

#define itv_offset24(a) (a>>24)&0x000000FF
#define itv_offset16(a) (a>>16)&0x000000FF
#define itv_offset8(a) (a>>8)&0x000000FF
#define itv_offset0(a) (a&0x000000FF)
#define itv_16sum2(a, b) (a<<16)|b

#define itv_evaluatevaule(a)	(ITV_UWORD)(a&0x0000FFFF)
#define itv_evaluatelength(a, b)	(ITV_UWORD)((((a&0x00030000)>>3)|b)|0x8000)
#define itv_evaluateid(a, b)	(ITV_UWORD)(((a&0x001C0000)>>5)|b)
#define itv_calculatevalue(a, b, c)		((ITV_DWORD)a+(((ITV_DWORD)b&0x6000)<<3)+(((ITV_DWORD)c&0xE000)<<5))
#define itv_calculatelength(a)		(ITV_DWORD)a&0x00001FFF
#define itv_calculateid(a)		(ITV_WORD)a&0x00001FFF
#define itv_rgbto16(a,b,c)		((((ITV_WORD)a)<<7)&0x7C00)|((((ITV_WORD)b)<<2)&0x03E0)|((((ITV_WORD)c)>>3))|0x8000
#ifdef ITV_RGBA4444
#define itv_rgbtohighbyte(r,g)		(r&0xF0)|((g>>4)&0x0F)
#define itv_rgbtolowbyte(g,b)		(b&0xF0)|0x0F
#else
#define itv_rgbtohighbyte(r,g)		((r>>1)&0x7C)|((g>>6)&0x03)|0x80
#define itv_rgbtolowbyte(g,b)		((g<<2)&0xE0)|((b>>3)&0x1F)
#endif
#define itv_swap16(a)	((a<<8)&0xFF00)|((a>>8)&0x00FF)
#define itv_swap32(a)	 ((a << 24) & 0xff000000) |((a <<  8) & 0x00ff0000) |((a >>  8) & 0x0000ff00) |((a >> 24) & 0x000000ff)
#define itv_ARGB1555torgb(a, b)	((ITV_DWORD)(a&0x7C)<<19+(ITV_DWORD)(a&0x03)<<14+(ITV_DWORD)(b&0xE0)<<11+(ITV_DWORD)(b&0x1F)<<3)|0xFF000000
#define itv_RGB888torgb(a, b, c)	((ITV_DWORD)a<<16 +(ITV_DWORD)b<<8 + (ITV_DWORD)c)|0xFF000000

#define itv_CaculateTableId(a)	(ITV_DWORD)((a>>16)&0x000000FF)
#define itv_CaculateTableIdEx(a)	(ITV_DWORD)(a&0x0000FFFF)
#define itv_CaculateToTableId(a, b)	(ITV_DWORD)((a<<16)|b)


#define TEST_LEVEL_ERROR	1
#define TEST_LEVEL_WARN	2
#define TEST_LEVEL_SUGGEST	3

#define ITV_DBG_LEVEL(a, b ,msg) \
	{if(!a)\
		{if(b == TEST_LEVEL_ERROR)	{ itv_print("\nError level: \n");( itv_print (msg)); itv_print("\n"); return;} \
		else if(b == TEST_LEVEL_WARN) { itv_print("\nWarn level: \n");( itv_print( msg));itv_print("\n");} \
		else if(b == TEST_LEVEL_SUGGEST) { itv_print("\nSuggest level: \n");( itv_print( msg));itv_print("\n");} \
		}}


#define ITV_DBG_ASSERT_NOT_EQUAL_ERROR(a, b ,errcode) \
	if(a != b)\
		{itv_print("ITV Library Tool find a error, error code number is %d, please read the reference document.\n", errcode);return errcode;}\
	else\
		{itv_print(".");}

#define ITV_DBG_ASSERT_EQUAL_ERROR(a, b ,errcode) \
	if(a == b)\
		{itv_print("ITV Library Tool find a error, error code number is %d, please read the reference document.\n", errcode);return errcode;}\
	else\
		{itv_print(".");}

#define ITV_DBG_ASSERT_LESS_ERROR(a, b ,errcode) \
	if(a < b)\
		{itv_print("ITV Library Tool find a error, error code number is %d, please read the reference document.\n", errcode);return errcode;}\
	else\
		{itv_print(".");}

#define ITV_DBG_ASSERT_MORE_ERROR(a, b ,errcode) \
	if(a > b)\
		{itv_print("ITV Library Tool find a error, error code number is %d, please read the reference document.\n", errcode);return errcode;}\
	else\
		{itv_print(".");}


#define ITV_DEBUG_ERR_TRACE_INFO		4
#define ITV_DEBUG_ERR_WARN_INFO	3
#define ITV_DEBUG_ERR_WARN		2
#define ITV_DEBUG_ERR			1

#define ItvPAL_GetTime				PAL_GetTime
#define ItvPAL_GetStartTime			PAL_GetStartTime
#define ItvPAL_PrintCurrentTime()		;

#ifndef ITV_PRINT_TIME
#ifdef ItvPAL_GetStartTime
#undef ItvPAL_GetStartTime
#define ItvPAL_GetStartTime			PAL_GetTime
#endif


#endif


#ifdef ITV_DEBUG_LEVEL_1
#define ITV_DEBUG_LEVEL		ITV_DEBUG_ERR
#endif

#ifdef ITV_DEBUG_LEVEL_2
#define ITV_DEBUG_LEVEL		ITV_DEBUG_ERR_WARN
#endif

#ifdef ITV_DEBUG_LEVEL_3
#define ITV_DEBUG_LEVEL		ITV_DEBUG_ERR_WARN_INFO
#endif

#ifdef ITV_DEBUG_LEVEL_4
#define ITV_DEBUG_LEVEL		ITV_DEBUG_ERR_TRACE_INFO
#endif

#define ITV_DBG_TRACE(a,b,msg) {if(a||b)	( ItvObj_InfoPrintf msg); }

#define ITV_DBG_INFO(msg) {ItvObj_InfoPrintf msg;}

#define ITV_DBG_WARN(msg) {ItvObj_WarnPrintf msg; }

#define ITV_DBG_ERR(msg) {ItvObj_ErrPrintf msg; }

#define itv_assert(exp) {if(!(exp)){ItvObj_ErrPrintf("\nassert error! file name is %s, line is %d\n", __FILE__, __LINE__);}}

#ifdef ITV_PRINT_TIME
#undef ItvPAL_PrintCurrentTime
#define ItvPAL_PrintCurrentTime()	{ItvObj_ErrPrintf("ItvPAL Print the time is %d, file name is %s, line is %d\n", PAL_GetStartTime(), __FILE__, __LINE__);}
#endif

#endif




/*Version History:
  ------------------------------------------------------------------------------ 
 
  $Log: /ZQProjs/TianShan/DataOnDemand/Phase1/DODSTREAMER/filter/itv_parse/Parser/inc/object_debug.h $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:38 Admin
// Created.
// 
// 1     08-12-08 11:11 Li.huang
// 
// 1     07-03-08 11:46 Cary.xiao
 * 
 * 1     07-03-01 17:50 Jin.liu
 * 
 * 3     07-02-13 15:05 Jerroy.yin
 * 
 * 2     07-01-29 21:51 Jerroy.yin
 * 
 * 1     07-01-08 13:05 Jerroy.yin
 * 
 * 2     07-01-08 12:55 Jerroy.yin
 * 
 * 13    07-01-02 11:37 Jerroy.yin
 * 
 * 12    06-12-27 20:19 Jerroy.yin
 * 
 * 11    12/13/06 4:13p Hmzhang
 * 
 * 10    06-12-12 21:01 Jerroy.yin
 * 
 * 9     06-12-12 20:45 Jerroy.yin
 * 
 * 8     06-10-25 23:09 Jerroy.yin
 * 
 * 7     9/23/06 12:23p Hao.wang
 * 
 * 6     9/21/06 3:58p Jerroy.yin
 * 
 * 5     06-09-20 15:04 Jin.liu
 * 
 * 4     06-09-20 14:04 Jin.liu
 * 
 * 3     06-08-17 15:54 Jerroy.yin
 * 
 * 2     8/01/06 9:25p Hao.wang
 * 
 * 1     06-07-31 11:08 Jerroy.yin
 * 
 * 38    06-07-11 21:28 Jerroy.yin
 * 
 * 37    7/07/06 8:06p Zhenkun.lai
 * 
 * 36    7/07/06 3:07p Hao.wang
 * 
 * 35    06-06-08 15:39 Jerroy.yin
 * 
 * 34    06-06-01 12:25 Hao.wang
 * 
 * 33    06-05-30 12:01 Hao.wang
 * 
 * 32    06-05-26 17:20 Hao.wang
 * 
 * 31    06-05-26 15:56 Hao.wang
 * 
 * 30    06-05-26 12:32 Jerroy.yin
 * 
 * 29    06-05-19 16:25 Hao.wang
 * 
 * 28    06-05-19 15:49 Jerroy.yin
 * 
 * 27    06-05-11 17:00 Jerroy.yin
 * 
 * 26    06-04-12 17:20 Jerroy.yin
 * 
 * 25    06-04-12 17:18 Jerroy.yin
 * 
 * 24    06-04-12 16:32 Jerroy.yin
 * 
 * 23    06-04-12 16:02 Jerroy.yin
 * 
 * 22    06-04-12 16:00 Jerroy.yin
 * 
 * 21    06-04-12 15:53 Jerroy.yin
 * 
 * 20    06-03-15 16:41 Jerroy.yin
 * 
 * 19    06-02-22 12:12 Jerroy.yin
 * 
 * 18    06-02-10 20:52 Jerroy.yin
 * 
 * 17    06-01-23 15:03 Jerroy.yin
 * 
 * 16    05-11-24 16:00 Jerroy.yin
 * 
 * 15    05-11-21 22:55 Jerroy.yin
 * 
 * 14    05-11-16 15:12 Jerroy.yin
 * 
 * 13    05-11-03 21:36 Jerroy.yin
 * 
 * 12    05-10-31 21:01 Jerroy.yin
 * 
 * 11    05-10-17 16:05 Jerroy.yin
 * 
 * 10    05-09-22 21:44 Jerroy.yin
 * 
 * 9     05-09-19 17:06 Jerroy.yin
 * 
 * 8     05-09-02 22:11 Jerroy.yin
 * 
 * 7     05-09-01 19:36 Jerroy.yin
 * 
 * 6     05-08-31 14:43 Jerroy.yin
 * 
 * 5     05-08-17 13:53 Jerroy.yin
 * 
 * 4     05-08-11 21:11 Jerroy.yin
 * 
 * 3     05-08-04 23:38 Jerroy.yin
 * 
 * 2     05-08-02 19:15 Jerroy.yin
*/







