/******************************************************************************************************************************************\
PAL layer API for ITV library
This header file defined all the porting layer APIs which is need for the ITV library core.
\******************************************************************************************************************************************/

/******************************************************************************************************************************************\
	Version			Person			Action			Date		Comment
	0.1				William Li		Created			2005-4-20
	0.2				Stanley Li		Modified		2005-4-15
	0.3				Stanley Li		Modified		2005-4-22	Add tune/select/play for video streams, Add Mpeg still prototype
	0.3.1			Stanley Li		Minor Modify	2005-4-22	Add ProgramNumber in the PAL_PlayMpeg, add Symbol Rate in the PAL_Tune
	0.3.2			Stanley Li		Minor Modify	2005-4-22	Modify the return format of PAL_GetHostByName
	0.3.3			Stanley Li		Minor Modify	2005-4-27	Supplyment some definition for inband data extraction, Add PIG support.
																Clear Table ID definition, Add more tune/select callback situation
	0.3.4			ge zhiping		Minor Modify	2005-5-09	Modify the type of file handle from	ITV_DWORD to ITV_UDWORD
	0.3.5			Stanley Li		Minor Modify	2005-5-18	Add PAL_Printf and PAL_GetTime
	0.3.6			ge zhiping		Minor Modify	2005-5-09	Modify the third param of createProcess from  ProcessProperty to ProcessPriority
	0.3.7			Stanley Li		Minor Modify	2005-7-01	Adding PAL_GetTSID() for detecting TSID.
	0.3.8			Stanley Li		Minor Modify	2005-8-01	Adding -1 equals to wait forever for some timeout value
	0.3.9			Jerroy Yin		Minor Modify	2005-8-03	change ITV_FS_CRW to ITV_FS_CWO
	0.3.10			Stanley Li		Minor Modify	2005-8-09	Update the description for PAL_GetSystemInfoStr and PAL_GetSystemInfo
	0.3.11			Stanley Li		Minor Modify	2005-8-30	Add PAL_SetTime()
	0.3.12			Jerroy Yin		Minor Modify	2005-9-2	Add PAL_SwitchVideoMpegStill()
	0.3.13			Jerroy Yin		Minor Modify	2005-9-2	Add PAL_SetTimeEx()
	0.3.14			Jerroy Yin		Minor Modify	2005-11-1	Add PAL_SetDNSServer()
	0.3.15			Jerroy Yin		Minor Modify	2005-11-1	Add the TEL VOD Frequency in the function PAL_GetSystemInfoStr()
	0.3.16			Jerroy Yin		Minor Modify	2005-11-4	Add the TEL VOD QAM mode and Symol Rate in the function PAL_GetSystemInfoStr()
	0.3.17			Jerroy Yin		Minor Modify	2005-12-1	Add the functions of the volume
	0.3.17			ge zhiping		Minor Modify	2005-12-6	Add the functions PAL_RTSPPlayMPEG and PAL_RTSPSetRate
	0.3.18			Jerroy Yin		Minor Modify	2006-4-23	Add the functions assert PAL_GetITVLibraryVersion
	0.3.19			Jerroy Yin		Minor Modify	2006-4-23	Add the functions audio mode and language
	0.3.20			Jerroy Yin		Minor Modify	2006-11-14	Add the functions pause mpeg and resume mpeg
	0.3.21			Jerroy Yin		Minor Modify	2007-1-31	Add the functions about channel count and paramter
\******************************************************************************************************************************************/


#ifndef ITV_PAL_INC
#define ITV_PAL_INC

#include "itv_types.h"
#include "time.h"

/******************************************************************************************************************************************\
OS Core API
\******************************************************************************************************************************************/

/******************************************************************************************************************************************\
Process
Because the VOD application is a multi-task application, so it requires the STB must have be a multi-task system.
The box must provide the way to start a new task, stop specified task, let task ideal (sleep), set task's priority, etc.
Depend on different OS, the task can be named process, thread, or task.
\******************************************************************************************************************************************/
#define PID ITV_DWORD
typedef ITV_DWORD (*PPROCESS_START_ROUTINE)(void* DataBlock);

#ifdef ITV_PAL_TRACE
PID _PAL_CreateProcess(char *_FileName,ITV_UDWORD _LineNo, PPROCESS_START_ROUTINE  ProcessFunction, void* DataBlock, ITV_DWORD ProcessPriority, ITV_DWORD InputHandle, ITV_DWORD OutputHandle);
#define PAL_CreateProcess(a1,a2,a3,a4,a5) _PAL_CreateProcess(__FILE__,__LINE__,a1,a2,a3,a4,a5);
#else
PID PAL_CreateProcess(PPROCESS_START_ROUTINE  ProcessFunction, void* DataBlock, ITV_DWORD ProcessPriority, ITV_DWORD InputHandle, ITV_DWORD OutputHandle);
#endif

/*
Parameter:

(ITV_DWORD *)ProcessFunction(void* argBlock): This is the process function. The new process will start at the beginning of this function
void * DataBlock: The pointer points to a data block which will be transfered to ProcessFunction as its argBlock.
ITV_DWORD ProcessPriority: The new process's priority
ITV_DWORD InputHandle: The input handle, the new process gets input from it.
ITV_DWORD OutputHandle: The output handle, the new process output to it.

Return:

> 0: create successfully and the return value is the new process's pid.
<=0: create fail. The return is the error code.

Description:

This function creates a new process. The parameter PorcessFunction defines the start point of new process. The parameter ProcessProperty defines the new process’s property.
This function also set the input/output device. The input/output device can be a pipe handle, a file handle, or a socket handle. If the input/output device is not defined (equal -1), the new process will manage its input/output device by itself. (*)
For the UNIX like OS, such as Linux and VxWorks, it supports process (some OS call process as task) and POSIX thread. The function creates new process directly. On some other OS which do not support process, such as Windows CE, this function creates a thread instead of process and emulates process creation.
This function do not defined any data block size for process, such as stack, memory pool and other private data. The function will allocate those data depend on the OS support.
The PorcessFunction is defined as (ITV_DWORD *)ProcessFunction(void* ArgBlock). CreateProcess will transfer the DataBlock to processFunction by ArgBlock. ProcessFunction should return an ITV_DWORDeger to indicate the result of the new process return.
If it creates process successfully, it returns a processed of new process. Other process related operation will refer to this PID. If create fail, it will return error code.

(*)In VxWorks, the ioTaskStdSet()/ioTaskStdGet() can set/retrieve the stdin/stdout/error file handle, and the TaskSpawn() can transfer up to 10 parameters to the created task. To implement the CreateProcess function call, the following method can be a reference:
1) Create a new task by TaskSpawn function.  The parameter EntryPt can be set to a “bridge routine? such as bridge(arg1,?. The arg1 to arg10 can be set to the ProcessFunction, DataBlock, InputHandle and OutputHandle.
2) After the bridge() is created successfully, it can set the stdin/stdout use ioTaskStdSet() to set its stdin/stdout.
3) Call ProcessFunction with DataBlock.
4) If it is necessary, restore the stdin/stdout.
5) Terminate bridge itself.
*/

ITV_DWORD PAL_KillProcess(PID pid);
/*
Parameter:
PID pid:The processID of the process want to be killed.

Return:
0:   kill successfully.
Other:   fail, return the error code.

Description:

This function will kill process specified by processID.
The caller can tell this function how to kill the process. There are several kinds of kill methods. One is just send a kill signal to process. The process receives this signal and do some clear operation. Then the process exit by itself.
Another way is let the OS kill the process without any condition. This maybe makes some data lost if the process killed without save it context and private data.
For those OS which support thread instead of process, the OS maybe can not kill the thread at OS level. This maybe reduces the system’s reliability.
The caller should make sure the killed process ID before call this function. If process be killed by mistake, the result maybe is unexpected.
It do not ensure that the process actual be killed (stopped). In some case, the OS just send a signal to the process and let the process stop by itself. The application makes sure if the process stopped by itself.
*/

#define ITV_PROCESS_IDLE		0
#define ITV_PROCESS_LOW			1
#define ITV_PROCESS_NORMAL		2
#define ITV_PROCESS_HIGH		3
#define ITV_PROCESS_CRITICAL	4

ITV_DWORD PAL_SetProcessPriority(PID pid, ITV_DWORD Priority);
/*
Parameter:
PID pid: The processID of the process.
ITV_DWORD Priority: The priority the process set to.

Return:
0:   set successfully.
Other:  fail. The return is the error code.

Description

This function set the priority of process identified by process ID. The process has following priority number:
0: idle
1: low priority
2: normal priority
3: high priority
4: time critical
Normally, the OS will set the priority automatic. Modify the priority by mistake maybe cause unexpected result.
*/

ITV_DWORD PAL_ProcessSleep(ITV_DWORD SleepTime);
/*
Parameter:
ITV_DWORD SleepTime
How long current process sleep. The time unit is ms.

Return:
0: successfully.
Other: fail. The return is the error code.

Description:
This function makes current process stop running for SleepTime and bring the control to other process.
If the SleepTime is 0, it only gives the control to other process without stop running.
*/

/******************************************************************************************************************************************\
Memory
The box must provide the APIs which can be used for memory management:
The box can allocate specified bytes of memory block for application, can free the memory block allocated from system,
and it should be better to specified the memory’s attribute to protect application data , such as read/write, shared/exclusive, etc.
Functions under this category is used manage the dynamic memory allocation
\******************************************************************************************************************************************/
#define ITV_MEM_GLOBAL      0x00000000
#define ITV_MEM_PRIVATE		0x00000001
#define ITV_MEM_CLEARED		0x00000002

#ifdef ITV_PAL_TRACE
void* _PAL_AllocMemory(char *_FileName,ITV_UDWORD _LineNo,ITV_UDWORD MemorySize, ITV_UDWORD MemoryProperty);
#define PAL_AllocMemory(x,y) _PAL_AllocMemory(__FILE__,__LINE__,x,y)
#else
void* PAL_AllocMemory(ITV_UDWORD MemorySize, ITV_UDWORD MemoryProperty);
#endif
/*
Parameter:
ITV_UDWORD MemorySize: The size the memory allocated. The unit is byte.
ITV_UDWORD MemoryPriority: The priority of the allocated memory.

Return:
The pointer point to the memory block. If allocate fail, it will return null.

Description:
This function allocates a block of memory from OS.
The memory size is specified by caller. For some OS, the allocate size of memory must aligned by word, double word or byte.
So the final memory size depends on OS and maybe not the same as requirement.
The caller also specify the property of memory block.
It can define the access mode of this memory block by MemoryPriority.
For example, Global means the process and it sub-process can access this memory block. Private  means only current process can access this memory block.
The MemoryPriority value can be the combination of following value.

0x00000001: Private, only used by current process
0x00000002: cleared, the memory will be cleared to 0 after allocation.

The value 0x00000000 define the allocated memory is global memory and not cleared.
If allocate memory successfully, it returns a pointer point to the memory block, or return null indicate that allocate memory fail.
*/

ITV_DWORD PAL_FreeMemory(void* MemoryBlock);
/*
Parameter:
 void* MemoryBlock: The begin point of the memory block want to be free.

Return:
0: successfully.
Other: fail.

Description:
This function only free the memory which allocated by AllocMemory. After the memory is free successfully, any operation refer to this memory block cause unexpected result.
*/


#ifdef ITV_PAL_TRACE
ITV_DWORD _PAL_CreateSem(char *_FileName,ITV_UDWORD _LineNo,ITV_DWORD *SemHandle);
#define PAL_CreateSemaphore(x) _PAL_CreateSem(__FILE__,__LINE__,x)
#else
ITV_DWORD PAL_CreateSemaphore(ITV_DWORD *SemHandle);
#endif
/*
Parameter:
ITV_DWORD *SemHandle: pointer to where the semaphore handle will be saved if creates successfully.

Return:
0: successfully.
Other: fail.

Description:
This function creates a semaphore which is used to synchronize different process.
It will return a semaphore handle if create semaphore successfully.
*/

#ifdef ITV_PAL_TRACE 
ITV_DWORD  _PAL_CloseSem(char *_FileName,ITV_UDWORD _LineNo,ITV_DWORD SemHandle);
#define PAL_CloseSemaphore(x) _PAL_CloseSem(__FILE__,__LINE__,x)
#else
ITV_DWORD  PAL_CloseSemaphore(ITV_DWORD SemHandle);
#endif
/*
Parameter:
ITV_DWORD SemHandle:
The semaphore wants to be destroyed.

Return:
0: successfully.
Other: fail.

Description:
This function call will destroy the semaphore created by CreateSemaphore. Any operation refer to this semaphore will cause unexpected result.
*/

#define  ITV_SEM_BLOCKED_MODE     0x00000000
#define  ITV_SEM_UNBLOCKED_MODE   0x00000001

#ifdef ITV_PAL_TRACE
ITV_DWORD _PAL_WaitSemaphore(char *_FileName, ITV_UDWORD _LineNo, ITV_DWORD SemHandle, ITV_DWORD GetFlag, ITV_DWORD GetTimeOut);
#define PAL_WaitSemaphore(a1,a2,a3) _PAL_WaitSemaphore(__FILE__,__LINE__,a1,a2,a3)
#else
ITV_DWORD PAL_WaitSemaphore(ITV_DWORD SemHandle, ITV_DWORD GetFlag, ITV_DWORD GetTimeOut);
#endif
/*
Parameter:
ITV_DWORD SemHandle: The semaphore waiting for
ITV_DWORD WaitFlag: The waiting flag
ITV_DWORD WaitTimeOut: The waiting timeout, the unit is ms. -1 for wait forever

Return:
0: successfully.
Other: fail.

Description:
This function wait the semaphore signaled. The behavior of this function depends on the WaitFlag, if the WaitFlag is 0x00000000, it works in blocked mode. In this mode, if the semaphore has been signaled, the function call return at once, or current process will be blocked until the semaphore signaled or timeout. If the WaitFlag is 0x00000001, it works in unblocked mode.  In this mode, the function always returns at once and the return code will indicate the result.
*/

#ifdef ITV_PAL_TRACE
ITV_DWORD  _PAL_SignalSemaphore(char *_FileName, ITV_UDWORD _LineNo, ITV_DWORD SemHandle);
#define PAL_SignalSemaphore(x) _PAL_SignalSemaphore(__FILE__,__LINE__,x)
#else
ITV_DWORD  PAL_SignalSemaphore(ITV_DWORD SemHandle);
#endif
/*
Parameter:
ITV_DWORD SemHandle: The semaphore wants to be signaled.

Return:
0: successfully.
Other: fail.

Description:
This function signals the semaphore and makes the processes which are waiting this semaphore to continue running.
*/

#ifdef ITV_PAL_TRACE
ITV_DWORD _PAL_CreateMutex(char *_FileName,ITV_UDWORD _LineNo,ITV_DWORD *MutexHandle);
#define PAL_CreateMutex(x) _PAL_CreateMutex(__FILE__,__LINE__,x)
#else
ITV_DWORD PAL_CreateMutex(ITV_DWORD *MutexHandle);
#endif
/*
Parameter:
ITV_DWORD *MutexHandle:pointer to where the mutex handle will be saved if creates successfully.

Return:
0: successfully.
Other: fail.

Description:
In order to avoid share resource violation, the best way is use mutex.
Any access to shared resource must after getting mutex, and release mutex after finish accessing those resource.
Only one process can get the mutex at same time.
This function is used to create the mutex. The mutex handle returned if this function call successfully, or return a value less then 0 as error code.
*/

#ifdef ITV_PAL_TRACE 
ITV_DWORD  _PAL_CloseMutex(char *_FileName,ITV_UDWORD _LineNo,ITV_DWORD MutexHandle);
#define PAL_CloseMutex(x) _PAL_CloseMutex(__FILE__,__LINE__,x)
#else
ITV_DWORD  PAL_CloseMutex(ITV_DWORD MutexHandle);
#endif
/*
Parameter:
ITV_DWORD MutexHandle: The mutex want to be destroyed.

Return:
0: successfully.
Other: fail.

Description:
This function destroy the mutex which created by CreateMutex.
If a semaphore is closed, any operation refer to this semaphore will cause unexpected result.
*/

#define  ITV_MUTEX_BLOCKED_MODE     0x00000000
#define  ITV_MUTEX_UNBLOCKED_MODE   0x00000001

#ifdef ITV_PAL_TRACE
ITV_DWORD _PAL_GetMutex(char *_FileName, ITV_UDWORD _LineNo, ITV_DWORD MutexHandle, ITV_DWORD GetFlag, ITV_DWORD GetTimeOut);
#define PAL_GetMutex(x,y,z) _PAL_GetMutex(__FILE__,__LINE__,x,y,z)
#else
ITV_DWORD PAL_GetMutex(ITV_DWORD MutexHandle, ITV_DWORD GetFlag, ITV_DWORD GetTimeOut);
#endif
/*
Parameter:
ITV_DWORD MutexHandle: The mutex want to get
ITV_DWORD GetFlag: The getting flag.
ITV_DWORD GetTimeOut: The getting timeout, the unit is ms. -1 for wait forever.

Return:
0: successfully.
Other: fail.

Description:
This function gets the mutex to get the control of shared resource. The behavior of this function depends on the GetFlag, if the GetFlag is 0x00000000, it works in blocked mode. In this mode, if the mutex is given to other process, the function returns at once, or current process is blocked until the mutex released or timeout. If the WaitFlag is 0x00000001, this function works in unblocked mode. In this mode, the function always returns at once and the return code will indicate the result.
*/

#ifdef ITV_PAL_TRACE
ITV_DWORD  _PAL_FreeMutex(char *_FileName, ITV_UDWORD _LineNo, ITV_DWORD MutexHandle);
#define PAL_FreeMutex(x) _PAL_FreeMutex(__FILE__,__LINE__,x)
#else
ITV_DWORD  PAL_FreeMutex(ITV_DWORD MutexHandle);
#endif
/*
Parameter:
ITV_DWORD MutexHandle: The mutex want to be free.

Return:
0: successfully.
Other: fail.

Description:
The function releases the mutex, and cause the processes blocked by GetMutex to continue running.
*/

/******************************************************************************************************************************************\
Storage
The functions under this category are used to access the data which was stored on different media, such as Flash memory, DiskOnChip, USB storage or other file system.
The box should provide a way to access the these kinds of storage system.
\******************************************************************************************************************************************/

#define ITV_FS_RO 			0x00000001
#define ITV_FS_RW			0x00000002
#define ITV_FS_LOCKEXEC		0x00000004
#define ITV_FS_LOCKWR		0x00000008
#define ITV_FS_LOCKR		0x00000010
#define ITV_FS_ENOEXIST		0x00000020
#define ITV_FS_BIN			0x00000040
#define ITV_FS_TR			0x00000080
#define ITV_FS_CRO			0x00000100
#define ITV_FS_CWO			0x00000200

ITV_UDWORD  PAL_OpenFile(ITV_BYTE *FileName, ITV_UDWORD OpenFlag);
/*
Parameter:
ITV_BYTE* FileName: The full name (include path) of the file want to be opened.
ITV_UDWORD OpenFlag: The opening flag.

Return:
> 0   open or create successfully and return the file handle.
Other:   create fail. The return is the error code.

Description:
This function opens an existing file or create a new file depend on the OpenFlag. The OpenFlag is the combine of following value.
0x00000001: open for read-only
0x00000002: open for write-only
0x00000004: exclusive lock
0x00000008: write lock
0x00000010: read lock
0x00000020: fail if not existing
0x00000040: open as binary
0x00000080: truncated to 0 if existing
0x00000100: create for read-only
0x00000200: create for write-only

It will return a file handle if successfully.

The file name format depends on the OS.
Caller can define the access mode and lock mode when open an existing file or file attribute when it create a new file.
The access mode include read-only, write-only and read-write. It will fail and return error code if read-only file want to be opened in write-only or read-write mode.
The lock mode includes read-lock, write-lock and read-write-lock.
Using read-lock on opening file will cause any other open operation with read access be denied.
Using write-lock on opening file will cause any other open operation with write access be denied.
Using read-write-lock (exclusive lock) on opening file will cause any other open operation with read-write access be denied.
Caller can specified either this call will create a new file when the file is not existing, or just return error code.
If create a new file, caller can define the file attribute on it, such as read-only, write-only and read-write.
The OS should support use the file handle as an input/output device.
*/

ITV_DWORD  PAL_CloseFile(ITV_UDWORD FileHandle);
/*
Parameter:
ITV_DWORD FileHandle: The handle of file wants to be closed.

Return:
0:   close successfully.
Other:   fail, return the error code.

Description:
This function close opened file.
This will cause the file handle be invalid and any operation refer to this file handle fail.
*/

ITV_DWORD  PAL_ReadFile(ITV_UDWORD FileHandle, void *Buf, ITV_DWORD BufLength,ITV_DWORD ReadFlag, ITV_DWORD Timeout);
/*
Parameter:
ITV_DWORD FileHandle: Read data from
void *Buf: Data read to
ITV_DWORD BufLength: Data read length, unit is byte.
ITV_DWORD ReadFlag: Read flag.
ITV_DWORD Timeout: Read timeout if in blocked mode. -1 for wait forever.

Return:
>=0, read successfully and return the actually read length, unit is byte.
< 0, fail. Return error code.

Description:
The function read BufLength of data from the file specified by FileHandle and stored ITV_DWORDo Buf,
The ReadFlag specified the read mode.
The default value, 0x00000000, is means blocked mode. 0x00000001 means unblocked mode.
In blocked mode, the function call only return after it have read BufLength content from file, the file is closed or timeout.
In unblocked mode, the function returns at once although no enough data read from file. The file handle is generated by OpenFile.
The BufLength must less than the size of Buf, or it will cause unexpected result.
If the return value is larger or equal than 0, the return value is the size read successfully.
If return value is less than 0, it indicates the error code.
*/

ITV_DWORD  PAL_WriteFile(ITV_UDWORD FileHandle, void *Buf, ITV_DWORD BufLength);
/*
Parameter:
ITV_DWORD FileHandle: File handle the data write to.
void *Buf: The data.
ITV_DWORD BufLength: The size of data area. unit is byte.

Return:
>=0:  successfully. The actually bytes write to, unit is byte.
< 0:  fail. The return value is error code.

Description:
This function writes specified data ITV_DWORDo file.
File handle is generated by OpenFile.
It returns the size written successfully or error code if return code is less 0.
*/

#define ITV_FS_SEEKB	0x00000000
#define ITV_FS_SEEKC	0x00000001
#define ITV_FS_SEEKE	0x00000002

ITV_DWORD PAL_SeekPosition(ITV_UDWORD FileHandle, ITV_DWORD Position, ITV_DWORD SeekMode);
/*
Parameter:
ITV_DWORD FileHandle: File handle
ITV_UDWORD Position: The position value
ITV_DWORD SeekMode: The seek mode.

Return:
>=0:  successfully. The position after seek, unit is byte.
< 0:  fail. The return value is the error code.

Description:
This function change current access point in a file.
It use the file handle generated by OpenFile.
There are three seek mode:
0x00000000: Seek from beginning of file.
0x00000001: Seek from current position of file. By this way, you can get the current position.
0x00000002: Seek from end of file. By this way, you can get the file length.
It will return the position after seek successfully, or return the error code if retune code is less than 0.
*/

ITV_DWORD  PAL_RemoveFile(ITV_BYTE *FileName);
/*
Parameter:
ITV_BYTE *FileName: The full name (include path) of the file want to be removed.

Return:
0:  successfully.
Other:  fail. The return value is the error code.

Description:
This function removes the specified file.
The format of file name depends on the OS storage system.
It will fail if it wants to remove an open file.
*/

ITV_DWORD  PAL_GetFileProperty(ITV_UDWORD FileHandle, ITV_UDWORD *FileSize, ITV_UDWORD *FileCTime,ITV_UDWORD *FileATime, ITV_UDWORD *FileMTime);
/*
Parameter:
ITV_DWORD FileHandle: File handle
unsinged long *FileSize: File size;
ITV_UDWORD *FileCTime: File create time.
ITV_UDWORD *FileATime: File access time.
ITV_UDWORD *FileMTime: File modify time.

Return:
0:  successfully.
Other:  fail. The return is the error code.

Description:
This function returns the file size, create time, last access time and the last modify time.
The three time value are the time since Epoch (0:0:0 UTC, 1/1/1970). And its unit is second.
*/

/******************************************************************************************************************************************\
Network
Network socket is a common method to communication between different hosts. Typically, it is used on an IP network environment.
The box must provide function to create socket, use socket to send/receive messages to/from other host.
In IP environment, it should work with both TCP and UDP.
The functions under this category provide the network service base on socket
\******************************************************************************************************************************************/

#define PAL_AF_INET         2           /*Domain for IP*/
#define PAL_SOCK_STREAM     1           /* stream socket */
#define PAL_SOCK_DGRAM      2           /* data gram socket */
ITV_DWORD  PAL_CreateSocket(ITV_DWORD Domain, ITV_DWORD Type, ITV_DWORD Protocol);

/*
Parameter:
ITV_DWORD Domain: The communication domain.
ITV_DWORD Type: The communication semantics.
ITV_DWORD Protocol: The protocol used.

Return:
> 0   Socket handle
<=0   error code

Description:
This function creates a socket for network communication and initializes its data structure. With this socket, application can communicate with other host to exchange data.
If successfully, it returns a socket handle.
The socket's property can be specified by Domain,Type and Protocol. Refer the file socket.h to see the predefined value of them.
*/

ITV_DWORD  PAL_CloseSocket(ITV_DWORD SocketHandle);
/*
Parameter:
ITV_DWORD SocketHandle: The socket wants to be closed.

Return:
0:   close successfully.
Other:   fail, return the error code.

Description:
This function closes the socket generated by CreateSocket and make this socket handle be invalid. Any operation refer to closed socket will cause unexpected result.
*/

ITV_DWORD  PAL_JoinGroup(ITV_DWORD SocketHandle,ITV_UDWORD MulticastIP);
/*
Parameter:
ITV_DWORD SocketHandle: The socket wants to be closed.
Unsigned long MulticastIP: The multicasting IP address.

Return:
0:   successfully.
Other:   fail, return the error code.

Description:
This function call asks to join a multicast group specified by the multicastIP.
*/

ITV_DWORD  PAL_LeaveGroup(ITV_DWORD SocketHandle);
/*
Parameter:
ITV_DWORD SocketHandle: The socket wants to be closed.

Return:
0:   successfully.
Other:   fail, return the error code.

Description:
This function call asks to leave from a multicast group.
*/


ITV_DWORD PAL_SetDNSServer(ITV_BYTE  *ip);
/*
Parameter:
ITV_BYTE *ip: The DNS Server IP Address String.

Return:
0:   successfully.
Other:   fail, return the error code.

Description:
This function call asks to set DNS Server IP Address.
*/



ITV_DWORD  PAL_GetHostByName(ITV_BYTE  *HostName);
/*
Parameter:
ITV_BYTE *HostName: Host we wants to resolve.

Return:
0:  Failed to resolve the host name
Other:  The ip addr resolved in host order

Description:
This function is used to relove the HostName from DNS. For example it should resovle "www.google.com" to 0x12345678. 
*/

ITV_DWORD  PAL_ConnectToHost(ITV_DWORD SocketHandle, ITV_BYTE *HostName, ITV_DWORD port);
/*
Parameter:
ITV_DWORD SocketHandle: The socket.
ITV_BYTE *HostName: Host to connect.
ITV_DWORD port: The port used to connect.

Return:
0:  connect successfully.
Other:  fail, return the error code.

Description:
This function establishes a connection between local host and remote host.
The remote host is specified by its name or IP address in HostName, and connection port is specified by port.
If the connection is established successfully, the socket can be used to SendMsg and ReceiveMsg.
This function return 0 is successfully, or other value indicates the error code.
*/

ITV_DWORD  PAL_ConnectToHostEx(ITV_DWORD SocketHandle, ITV_DWORD IPAddr, ITV_DWORD port);
/*
Parameter:
ITV_DWORD SocketHandle: The socket.
ITV_DWORD IPAddr: Host to connect.
ITV_DWORD port: The port used to connect.

Return:
0:  connect successfully.
Other:  fail, return the error code.

Description:
This function establishes a connection between local host and remote host.
The remote host is specified by its name or IP address in HostName, and connection port is specified by port.
If the connection is established successfully, the socket can be used to SendMsg and ReceiveMsg.
This function return 0 is successfully, or other value indicates the error code.
*/

ITV_DWORD PAL_DisconnectFromHost(ITV_DWORD SocketHandle);
/*
Parameter:
ITV_DWORD SocketHandle: The socket.

Return:
0:   disconnect successfully.
Other  fail, return the error code.

Description:
This function closes the connection between local host and remote host.
The return value is 0 if it successfully, other value indicate error code.
*/

ITV_DWORD  PAL_Bind(ITV_DWORD SocketHandle, ITV_DWORD port);
/*
Parameter:
ITV_DWORD SocketHandle: The socket.
ITV_DWORD port: Use this port to connect.

Return:
0:  bind successfully.
Other:   fail, return the error code.

Description:
This function bind socket to local host's port
After bind successfully, the function Listen can be used to waiting for remote host's income connecting.
The return value is 0 if successfully, or other value indicates error code.
*/

ITV_DWORD  PAL_Unbind(ITV_DWORD SocketHandle);
/*
Parameter:
ITV_DWORD SocketHandle: The socket.

Return:
0:   unbind successfully.
Other:   fail, return the error code.

Description:
This function removes the binding from socket.
*/

ITV_DWORD  PAL_Listen(ITV_DWORD SocketHandle, ITV_DWORD msTimeout);
/*
Parameter:
ITV_DWORD SocketHandle: The socket.
ITV_DWORD Timeout: The time out

Return:
> 0   listen successfully and the return value is new socket handle.
<=0   fail, return the error code.

Description:
This function listens on the socket. The current process will be blocked until in coming message is received or timeout.
If it accepts a connection successfully, it will return a new socket which can be used for SendMsg and ReceiveMsg to send and receive messages.
*/

ITV_DWORD PAL_SendMsg(ITV_DWORD SocketHandle, void *Buf, ITV_DWORD BufLength);
/*
Parameter:
ITV_DWORD SocketHandle: Socket
void *Buf: The data
ITV_DWORD BufLength: The length wants to send out. Unit is byte.

Return:
>=0:  successfully. The actually bytes send to, the unit is byte.
< 0:  fail. The return is the error code.

Description:
This function send message to remote host.
The data is specified by it beginning (Buf) and size (BufLength).
The BufLength must not larger than the size of buffer, or it will cause unexpected result.
It will return the number of sent actually if return code is larger or equal 0, or the error code if it fails to send.
*/

#define  PAL_BLOCKED_MODE	  0x00000000
#define  PAL_UNBLOCKED_MODE   0x00000001
#define  PAL_INFINITE         0xFFFFFFFF  // Infinite timeout
ITV_DWORD PAL_ReceiveMessage(ITV_DWORD SocketHandle, void *Buf, ITV_DWORD BufLength, ITV_DWORD RecvFlag, ITV_DWORD  msTimeout);
/*
Parameter:
ITV_DWORD SocketHandle: The socket
void *Buf: Data read to
ITV_DWORD BufLength: Data read length, unit is byte.
ITV_DWORD RecvFlag: Read flag.
ITV_DWORD msTimeout: Read timeout if in blocked mode, in milliSecond. -1 for wait forever

return:
>=0: read successfully and return the actually read length, unit is byte.
< 0: fail. Return error code.

Description:
This function receive BufLength of data from socket specified by SocketHandle and stored ITV_DWORDo Buf,
The RecvFlag specified the receive mode. The default mode, 0x00000000, is blocked mode. 0x00000001 means unblocked mode.
In blocked mode, the function return after it have receive BufLength data from socket, the socket is closed or timeout.
In unblocked mode, the function returns at once although no enough data received from socket.
If return value is larger or equal 0, it indicates the byte received from remote host. If return value is less 0, it indicates error code.
*/

ITV_DWORD  PAL_SendMsgTo(ITV_DWORD SocketHandle, void *Buf, ITV_DWORD BufLength, ITV_BYTE *HostName, ITV_DWORD port);
/*
Parameter:
ITV_DWORD SocketHandle: Socket
void *Buf: The data
ITV_DWORD BufLength: The length wants to send out. Unit is byte.
ITV_BYTE *HostName: The host address the message sends to.
ITV_DWORD port: The host port the message sends to.

Return:
>=0:  successfully. The actually bytes send to, the unit is byte.
< 0:  fail. The return is the error code.

Description:
This function send message to remote host.
The data is specified by it beginning (Buf) and size (BufLength).
The BufLength must not larger than the size of buffer, or it will cause unexpected result.
It will return the number of sent actually if return code is larger or equal 0, or the error code if it fails to send.
*/

ITV_DWORD  PAL_SendMsgToEx(ITV_DWORD SocketHandle, void *Buf, ITV_DWORD BufLength, ITV_DWORD IPAddr, ITV_DWORD port);
/*
Parameter:
ITV_DWORD SocketHandle: Socket
void *Buf: The data
ITV_DWORD BufLength: The length wants to send out. Unit is byte.
ITV_DWORD IPAddr: The ip address the message sends to.
ITV_DWORD port: The host port the message sends to.

Return:
>=0:  successfully. The actually bytes send to, the unit is byte.
< 0:  fail. The return is the error code.

Description:
This function send message to remote host.
The data is specified by it beginning (Buf) and size (BufLength).
The BufLength must not larger than the size of buffer, or it will cause unexpected result.
It will return the number of sent actually if return code is larger or equal 0, or the error code if it fails to send.
*/


#define  ITV_RECEIVE_BLOCKED_MODE     0x00000000
#define  ITV_RECEIVE_UNBLOCKED_MODE   0x00000001
ITV_DWORD  PAL_ReceiveMessageFrom(ITV_DWORD SocketHandle, void *Buf, ITV_DWORD BufLength,ITV_DWORD RecvFlag, ITV_DWORD msTimeout,ITV_BYTE *HostName, ITV_DWORD HostNameLength, ITV_DWORD port);
/*
Parameter:
ITV_DWORD SocketHandle: The socket
void *Buf: Data read to
ITV_DWORD BufLength: Data read length, unit is byte.
ITV_DWORD RecvFlag: Read flag.
ITV_DWORD msTimeout: Read timeout if in blocked mode. -1 for wait forever
ITV_BYTE *HostName: Address of the host which send the message.
Int HostnameLength: Maximize ITV_BYTEacters can be stored in the HostName.
ITV_DWORD port: The port number.

return:
>=0: read successfully and return the actually read length, unit is byte.
< 0: fail. Return error code.

Description:
This function receive BufLength of data from socket specified by SocketHandle and stored ITV_DWORDo Buf,
The RecvFlag specified the receive mode. The default mode, 0x00000000, is blocked mode. 0x00000001 means unblocked mode.
In blocked mode, the function return after it have receive BufLength data from socket, the socket is closed or timeout.
In unblocked mode, the function returns at once although no enough data received from socket.
If return value is larger or equal 0, it indicates the byte received from remote host. If return value is less 0, it indicates error code.
*/

ITV_DWORD  PAL_ReceiveMessageFromEx(ITV_DWORD SocketHandle, void *Buf, ITV_DWORD BufLength, 
		ITV_DWORD RecvFlag, ITV_DWORD msTimeout, ITV_DWORD *pIPAddr, ITV_DWORD IPAddrLength,
		ITV_DWORD port);
/*
Parameter:
ITV_DWORD SocketHandle: The socket
void *Buf: Data read to
ITV_DWORD BufLength: Data read length, unit is byte.
ITV_DWORD RecvFlag: Read flag.
ITV_DWORD Timeout: Read timeout if in blocked mode. -1 for wait forever
ITV_DWORD pIPAddr: The ip address the message received from.
Int IPAddrLength: Maximize ITV_BYTEacters can be stored in the pIPAddr.
ITV_DWORD port: The port number.

return:
>=0: read successfully and return the actually read length, unit is byte.
< 0: fail. Return error code.

Description:
This function receive BufLength of data from socket specified by SocketHandle and stored ITV_DWORDo Buf,
The RecvFlag specified the receive mode. The default mode, 0x00000000, is blocked mode. 0x00000001 means unblocked mode.
In blocked mode, the function return after it have receive BufLength data from socket, the socket is closed or timeout.
In unblocked mode, the function returns at once although no enough data received from socket.
If return value is larger or equal 0, it indicates the byte received from remote host. If return value is less 0, it indicates error code.
*/	
	
		
/******************************************************************************************************************************************\
Tuning
For different kind of box, HFC or IP, there is different path to get the stream.
VOD application requires the box has the capability to receive those stream data, whatever the streaming path.
Typically, in HFC environment, VOD application requires the box can tune to specified frequency with QAM mode;
In IP environment, get data from specified port. If receive a multicast stream, join a multicast group with IGMP first.
ITV Library Core also asks the box provide the way to get in-band private data in MPEG2-TS stream.
\******************************************************************************************************************************************/


#define QAM64		0
#define	QAM128		1
#define	QAM256		2

#define	QAMALL		10
/*
These are the definition of the QAMMode in the HFCTuneDataType
*/

typedef struct
{
	ITV_UDWORD 		Frequency;				// Frequecy in Hz
	ITV_DWORD 		QAMMode;				// QAM Mode of the tuning, 0 for QAM64, 1 for QAM128, 2 for QAM256
	ITV_UDWORD		SymbolRate;				// The annex-A needs a symbolrate to tune in sps
}
HFCTuneDataType;


typedef struct
{
	ITV_UDWORD MCIPAddress;
	ITV_DWORD Port;
}
IPTuneDataType;

typedef union
{
	HFCTuneDataType HFCTuneData;
	IPTuneDataType IPTuneData;
}
TuneDataType;


#define TUNE_HFC			0
#define TUNE_IP				1
/*
these are the tuning types definition
*/

#define TUNE_STATUS_LOCKED		0
#define TUNE_STATUS_UNLOCKED	1
#define TUNE_STATUS_ERROR		2
/* 
these are status code in the tune status callback
*/

typedef void (*PAL_Tune_StatusCallback)(ITV_DWORD Status);
/*
Parameters:
Status:
The result of the tunning, will be one of the value below
TUNE_STATUS_LOCKED: The frequency is locked
TUNE_STATUS_UNLOCKED: The frequency can not be locked
TUNE_STATUS_ERROR: A error occured during tuning
*/

#define SELECT_STATUS_OK			0
#define SELECT_STATUS_ERROR			1
#define SELECT_STATUS_PROGRAMLOST	2

typedef void (*PAL_Select_StatusCallback)(ITV_DWORD Status);
/*
Parameters:
Status:
The result of the select process. will be one of the value below
SELECT_STATUS_OK: Select is successful, the program is find in this MPTS
SELECT_STATUS_ERROR: Select failed
*/

ITV_DWORD PAL_Tune(ITV_DWORD TuneType, TuneDataType* pTuneData, PAL_Tune_StatusCallback pTuneCallback);
/*
Parameters:

TuneType:
TUNE_HFC: tune type is HFC;
TUNE_IP: tune type is IP;

TuneData:
Tune parameter, depend on the TuneType.

pTuneCallback:
The callback to inform the result of the tuning.

Return:
0: tune request is accepted.
-1: tune request is objected

Description:
ITV application calls this function to ask the STB tune to specified stream.
In HFC environment, application will specified the Frequency and QAM mode for tuning;
In IP environment, ask the STB receive the UDP packet at specified multicast IP address and port.
If the multicast IP address is 0.0.0.0, this means receive unicast udp packet.

PAL_Tune is can be used either to extract private data or play the movie or both of them. Means
When user first tune to a frequency and  call PAL_ConnectPrivateData to register to a table/pid 
to extract the private data segment. Then select a video spts to play the video, the original
privated data extraction should not stop as long as the private data is going under. Private
data can only be stopped when the table/pid no long exists or the PAL_DisconnectPrivateData
is called.

If the the frequency is lost during playing/inbanddata receiving, an callback should be called
to notify the frequency is unlocked.
*/

ITV_DWORD PAL_GetTSID(void);
/*
Parameters:

None

Return:
<0:	There is a error occured when user trying to the TSID
>=0: The TSID of current frequency

Description:
This call is used to get the TSID of the locked frequency. The TSID is used for caculating the 
nodegroup user belongs to.
*/

ITV_DWORD PAL_SelectMpeg(ITV_DWORD ProgramNumber, PAL_Select_StatusCallback pSelectCallback);
/*
Parameters:
ProgramNumber: The ProgramNumber needs to be selected out of the MPTS
pSelectCallback: the callback need to be call and reporting the result of the selecting

Return:
0: select request accepted
-1: select request is objected

Description:
This call is used to select out the Program after tuner is tuned to the frequecy successfully. 
This function is a nonblock call, it returns immediatly which indicates if the Select request
is accepted. If requested, a callback is expected to inform the app if the Program Number exists
and selected from the MPTS.

If the program is lost during watching, which indicating the usr should see a black screen,
the callback should occurs to notify the application that the program is lost.
*/

ITV_DWORD PAL_PlayMpeg(ITV_DWORD ProgramNumber, ITV_DWORD x, ITV_DWORD y, ITV_DWORD w, ITV_DWORD h);
/*
Parameters:
Program Number: The program number of the MPTS which user wants to play
x:				The PIG x coordinates
y:				The PIG y coordinates
w:				The PIG width
h:				The PIG height

	
Return:
0: Mpeg is played successfully
-1: play failed

Description:
This call is called to play the mpeg stream which is selected on a specified screen. Multipul PAL_PlayMpeg
call is allowed to resize the current playing video.
*/

ITV_DWORD PAL_PauseMpeg(void);
/*
Parameters:
	None.
	
Return:
0: Mpeg is paused successfully
-1: pause failed

Description:
This call is called to pause the mpeg stream which is played on a specified screen. the screen display the last mpeg still frame.
*/


ITV_DWORD PAL_ResumeMpeg(void);
/*
Parameters:
	None.
	
Return:
0: Mpeg is resumed successfully
-1: resume failed

Description:
This call is called to resume the mpeg stream which is paused on a specified screen.
*/



ITV_DWORD PAL_StopMpeg(void);
/*
Parameters:		None
	
Return:			0: 		Mpeg is stopped
				-1: 	Stop failed
	
Description:
Stop the mpeg. The mpeg should not be seen on the screen after this call.
*/
	
	



ITV_DWORD PAL_PlayMpegStill(ITV_UBYTE* pData, ITV_DWORD lData);
/*
Parameters:

pData:
the pointer to the MpegStill frame

lData:
the length of the pData

Return:
0: mpeg still is displayed successfully
-1: mpeg still play failed 

Discription:
This function is to ask stb to display a mpeg still on the video pane.
*/

ITV_DWORD PAL_StopMpegStill(void);
/*
Parameters:

Return:
0: Successfully stoped
-1: Failed

Discription:
This function is call the clean the Mpegstill pane in case some stb needs to stop the mpeg still playing.
*/

#define SWITCH_SHOW_STILL		0
#define SWITCH_SHOW_VIDEO		1

ITV_DWORD PAL_SwitchVideoMpegStill(ITV_DWORD flag);
/*
Parameters:

flag:
SWITCH_SHOW_VIDEO: show the video layer on mpeg still layer
SWITCH_SHOW_STILL: show the mpeg still layer on video layer

Return:
0: successfully
-1: failed 

Discription:
This function is to switch the mpeg still layer and the video layer.
*/


#define INBANDDATA_START_CMD 		1				//	The inband data is going to arrive
#define INBANDDATA_ARRIVED_CMD 		2				//	The inband data arrived
#define INBANDDATA_STOP_CMD			3				// 	The inband data stoped
/*
These are the CMD definition for first argument of the PPRIVATEDATA_CALLBACK_ROUTINE. 
*/

typedef ITV_DWORD (*PPRIVATEDATA_CALLBACK_ROUTINE)(ITV_DWORD CMD, ITV_DWORD StreamType, ITV_DWORD TableID, ITV_BYTE * pData, ITV_DWORD Length);
/*
Parameters:

CMD:
1: inband data started, please prepare to gather inbanddata
2: inband data segment arrived
3: inband data stops

StreamType:
the StreamType of the segment arrived

TableID:
the TableID of the segment arrived, this Table ID indicates the Table ID and the Table extention ID which is
specified in MPEG2 standard to be filtered, (TableID&0x00FF0000)>>16 is the given TableID, TableID&0x0000FFFF 
is the given table ID extension.

pData:
the data of the segment

Length:
the length of the segment received

Description:
This callback should not be disturbed when a video is being played on the screen. Means when user is watch the video,
the private data still can be extract/filterd and callback through this interface. 
*/

ITV_DWORD  PAL_ConnectPrivateData(ITV_DWORD StreamType, ITV_DWORD TableID, PPRIVATEDATA_CALLBACK_ROUTINE PrivateDataCallBack);
/*
Parameters:

StreamType:Specify the private data stream type

TableID:
the TableID of the segment arrived, this Table ID indicates the Table ID and the Table extention ID which is
specified in MPEG2 standard to be filtered, (TableID&0x00FF0000)>>16 is the given TableID, TableID&0x0000FFFF 
is the given table ID extension.

PrivateDataCallBack:
The call back function which handle the private data or event.


Return:
0: successfully.
1: the specified stream type does not exist in current stream.

Description:
Ask STB gather the private data.
Once the function calls successfully, It will start searching the stream and check if this stream type is existing in the stream.
If this kind of stream is existing in current stream, it will start data gathering and transfer the data to the private data handler which specified in second parameter.
The handler receives two parameters: the CMD and the data. The CMD tell the handler the event, include data start, data coming and data lost, etc. the second is the data object.
The STB need support at least 25 private data at one time.

*/

ITV_DWORD  PAL_DisconnectPrivateData(ITV_DWORD StreamType, ITV_DWORD TableID);

/*
Parameters:

Return:
	>= 0: the channel count.
	else : failure.

Description:
	Get the channel count.
*/

ITV_DWORD PAL_GetChannelTotalCount(void);

/*
Parameters:
	index:  current channel index

Return:
	0: successfully.
	else : failure.

Description:
	Get the current channel index.
*/

ITV_DWORD PAL_GetCurrentChannelIndex(ITV_DWORD *index);


typedef struct
{
	ITV_UDWORD 	Frequency;				// Frequecy in Hz
	ITV_DWORD 		ProgramNumber;
	ITV_DWORD 		QAMMode;				// QAM Mode of the tuning, 0 for QAM64, 1 for QAM128, 2 for QAM256
	ITV_UDWORD		SymbolRate;				// The annex-A needs a symbolrate to tune in sps
}
HFCChannelPara;

typedef union
{
	HFCChannelPara HFCTuneData;
	IPTuneDataType IPTuneData;
}
ChannelDataType;


/*
Parameters:
	index:  channel index, 0 means the first channel.
	TuneType: 
		TUNE_HFC: tune type is HFC;
		TUNE_IP: tune type is IP;
	pChannelData:
		the parameter of the channel.

Return:
	0: successfully.
	else : failure.

Description:
	Get the channel parameter with index.
*/

ITV_DWORD PAL_GetChannelPara(ITV_DWORD index, ITV_DWORD * TuneType, ChannelDataType* pChannelData);




/******************************************************************************************************************************************\
System Manager
These APIs provide are used to check the STB’s capability on audio, video, etc.
\******************************************************************************************************************************************/


#define STB_AC3					1
#define STB_DTS					2
#define STB_MPEG				3
#define STB_SPDIF				4
#define STB_HD					5
#define STB_MAC					6
#define STB_FWV					7
#define STB_FWS					8
#define STB_SCID					9
#define STB_DEVICEID			10
#define STB_VODFREQUENCY		11
#define STB_VODQAMMODE		12
#define STB_VODSYMBOLRATE		13
#define STB_MEMORYSIZE			14
#define STB_CPUSPEED			15
#define STB_NEEDFLUSH			16
#define STB_COLORBIT			17
#define STB_SWAPFLAG			18

ITV_DWORD PAL_GetSystemInfo(ITV_DWORD SystemInfoIndex);
/*
Parameters
SystemInfoIndex: A number to specify what system information want to be retrieved.
Following number is defined now.
1: AC3 Supported.
2: DTS supported.
3: MPEG Audio supported.
4: SPDIF supported.
5: HD supported.
16: Need Flush, if need flush, return 0, else return another value.
17: color bit.
	the offscreen store color bit, return the value, like ARGB1555, return 16, ARGB8888, return 32.
18: the order of data.
	if the order of data use Little endian, then return 0.
	if the order of data use Big Endian, then return 1.

Return
Indicate if specified system capability is support or the support level. If the current index is supported
return 0, else return a negative value to indicate error. If the queried index is not listed in the index  
return 0.

Description
Retrieve system capability.
*/


ITV_DWORD  PAL_GetSystemInfoStr(ITV_DWORD SystemInfoIndex, ITV_BYTE* InfoStr, ITV_DWORD StrLength);
/*
Parameters
SystemInfoIndex:
A number to specify what system information want to be retrieved.
Following number is defined now.
6: Mac Address.
	two-way set box: the mac address of the networkcard, the format is "AA:BB:CC:DD:EE:FF"
	one-way set box: return value -1.
7: Firmware version.
	return the firm ware version in the STB.
8: Hardware version.
	return the hardware version which will register on the server site.
	one-way set box: can not exceed 14 bytes.
	two-way set box: can not exceed 24 bytes.
9: SmartCard ID.
	smart card ID.
	it will be home ID which used on the server site.
10: DeviceID.
	device ID.
	two-way set box: the mac address of the networkcard, the format is "AABBCCDDEEFF"
	one-way set box: return the ID.
11: TELVOD Navigation Frequency.
	two-way set box: return value -1.
	one-way set box: return the TELVOD frequency, the unit is HZ, if the value is 219,000,000Hz, it must return the string "219000000".
12: QAM Mode.
	two-way set box: return value -1.
	one-way set box: return the TELVOD QAM mode, the return string "64" or "128" or "256".
13: SymbolRate.
	two-way set box: return value -1.
	one-way set box: return the TELVOD symbol rate, the unit is SPS, if the value is 6,875,000sps, it must return the string "6875000".
14: Memory Size.
	return the memory size which can give for our application, the unit is byte, if the value is 4,000,000 bytes, it must return the string "4000000".
15: CPU Speed.
	return the CPU Speed, the unit is mips, if the value is 200 mips, it must return the string "200".
	


InfoStr: pointer to where to store the result in string.

StrLength: The length of string.


Return
Return the length of result string if successful, else return a negative value 
to indicate error. If the queried index is not listed in the index  
return 0.

Description
Retrieve system capability. 

*/

ITV_DWORD  PAL_Printf(const char *format, ...);
/*

Parameters:
	format: the formatted string to be printed on the console
	
Return:
	-1 if error, 0 if successful
	
Description:
	This function is used to print out the debug info for debugging
	
*/	

extern ITV_DWORD  PAL_GetITVLibraryVersion(ITV_BYTE *buff, ITV_DWORD length);
/*

Parameters:
	buff: pointer to where to store the version string
	length: the length of the input buffer.
	
Return:
	> 0: return the version string length.
	<=0: failure.
	
Description:
	This function is used to get the ITV Library version number.
	
*/	

	

ITV_DWORD  PAL_GetTime(void);
/*
Return:
	> 0 :  Return the UTC time in seconds successfully.
	-1:  fail. The return is the error code.

Description:
	This API is used to poll the system time, since Epoch (0:0:0 UTC, 1/1/1970).
	And its unit is second.	
*/

ITV_DWORD  PAL_SetTime( ITV_DWORD utcTime );
/*
Return:
	 0 :  Time has been set successfully.
	-1:  fail.

Description:
	This API is used to set/adjust system time. The time should be accepted as
	utc time.
*/

ITV_DWORD  PAL_SetTimeEx( ITV_TIMEDATE sys_time );
/*
Return:
	 0 :  Time has been set successfully.
	-1:  fail.

Description:
	This API is used to set/adjust system time. 
	the time format is declared in <time.h> 
	struct tm {
	  int tm_sec;   // seconds after the minute, 0 to 60 (0 - 60 allows for the occasional leap second)
	  int tm_min;   // minutes after the hour, 0 to 59
	  int tm_hour;  // hours since midnight, 0 to 23 
	  int tm_mday;  // day of the month, 1 to 31
	  int tm_mon;   // months since January, 0 to 11 
	  int tm_year;  // years since 1900 
	  int tm_wday;  // days since Sunday, 0 to 6 
	  int tm_yday;  // days since January 1, 0 to 365 
	  int tm_isdst; // Daylight Savings Time flag 
	};
*/

ITV_DWORD  PAL_GetStartTime(void);
/*
Return:
	 >0 : the start time from the application start, the unit is ms
	<0:  fail.

Description:
	This API is used to get the start left time, the unit is ms.
*/


#define ITV_STB_VOLUME_NORMAL		0x00000000
#define ITV_STB_VOLUME_MUTE		0x00000001

ITV_DWORD PAL_GetVolumeStatus(void);
/*

Parameters:
	void
	
Return:
	ITV_STB_VOLUME_NORMAL: the normal status.
	ITV_STB_VOLUME_MUTE: mute status.
	
Description:
	This function is used to get the volume status.
	
*/	

ITV_DWORD PAL_GetVolumeParameter(ITV_DWORD *vol_min, ITV_DWORD *vol_max);
/*

Parameters:
	vol_min: used to get the min volume number.
	vol_max: used to get the max volume number.
	
Return:
	-1 if error, 0 if success.
	
Description:
	This function is used to get the volume parameter of the STB.
	the default step is 1, it means if user press volume + one time, the volume number will add 1.
	
*/	


ITV_DWORD PAL_GetCurrentVolume(void);
/*

Parameters:
	void
	
Return:
	-1 if error, the real volume number if success.
	
Description:
	This function is used to get the current volume number.
	
*/

ITV_DWORD PAL_SetVolume(ITV_DWORD VolumeNum);
/*

Parameters:
	VolumeNum: the volume number which user want.
	
Return:
	-1 if error, 0 if success.
	
Description:
	This function is used to set the volume number.
	
*/	


ITV_DWORD PAL_SetMute(void);
/*

Parameters:
	void.
	
Return:
	-1 if error, 0 if success.
	
Description:
	This function is used to set the mute.
	
*/	

ITV_DWORD PAL_CancelMute(void);
/*

Parameters:
	void.
	
Return:
	-1 if error, 0 if success.
	
Description:
	This function is used to cancel the mute, and the STB will comeback the current volume number.
	
*/

ITV_DWORD PAL_RTSPPlayMPEG(ITV_BYTE *pURL);
/*
Parameters:
	pURL: Specify the URL of VOD stream

Return:
	-1 if error, 0 if success.

Description:
    This function is only used to demand mive in unicast mode on winCE 4.2 version
*/

ITV_DWORD PAL_RTSPSetRate(ITV_DWORD dwRate);
/*
Parameters:
	pURL: Specify the dwRate of RTSP demand stream

Return:
	-1 if error, 0 if success.

Description:
    This function is only used to set play rate of RTSP demand stream

*/


#define ITV_AUDIO_MODE_STEREO		1
#define ITV_AUDIO_MODE_LEFT		2
#define ITV_AUDIO_MODE_RIGHT		3

ITV_DWORD PAL_GetAudioMode(void);
/*

Parameters:
	void.
	
Return:
	ITV_AUDIO_MODE_STEREO: means stereo audio.
	ITV_AUDIO_MODE_LEFT: means left audio.
	ITV_AUDIO_MODE_RIGHT: means right audio.
	
Description:
	This function is used to get the audio mode.
	
*/	



ITV_DWORD PAL_SetAudioMode(ITV_DWORD type);
/*

Parameters:
	type:
	ITV_AUDIO_MODE_STEREO: means stereo audio.
	ITV_AUDIO_MODE_LEFT: means left audio.
	ITV_AUDIO_MODE_RIGHT: means right audio.
	
Return:
	0: successful.
	else: failure.
	
Description:
	This function is used to set the audio mode.
	
*/	

ITV_DWORD PAL_GetAudioCount(void);
/*

Parameters:
	void.

Return:
	> 0: the audio count.
	< 0: failure.

Description:
	This function is used to get the audio count.

*/	



ITV_DWORD PAL_GetAudioLanguageName(ITV_DWORD index, ITV_BYTE *buff, ITV_DWORD length);
/*

Parameters:
	index: the language name which want, the first index is 0.
	buff: the input buffer.
	length: the length of the buffer.
	
Return:
	>= 0: the return buffer length.
	< 0: failure.
	
Description:
	This function is used to get the audio name.
	
*/	


ITV_DWORD PAL_SetAudioLanguage(ITV_DWORD index);
/*

Parameters:
	index: the index of the audio language, the first index is 0.
	
Return:
	0: success.
	else: failure.

Description:
	This function is used to set the audio language.
	
*/	


ITV_DWORD PAL_GetAudioLanguage(ITV_DWORD *index);
/*

Parameters:
	*index: the index of the audio language, the first index is 0.
	
Return:
	0: success.
	else: failure.

Description:
	This function is used to set the audio language.
	
*/	


#endif

