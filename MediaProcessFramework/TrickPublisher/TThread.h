// TThread.h: interface for the TThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TTHREAD_H__C7258E84_FD5F_478A_8D60_7258A3FC9366__INCLUDED_)
#define AFX_TTHREAD_H__C7258E84_FD5F_478A_8D60_7258A3FC9366__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class TThread  
{
public:
	HANDLE GetHandle(){return _handle;}
	TThread();
	virtual ~TThread();

	enum Status	
	{
		///the thread is initial
		INITIAL		=1,  
		///the thread is running
		RUNNING,	
		///the thread is disabled
		DISABLED,	
	};
	
	///Starts execution!
	bool start();

	static void Sleep(DWORD msec);

	///retrieve the current thread status
	Status status(void)const{ return _status; }

	///set the status of the current thread
	void setStatus(int sts);

	///query if the thread is still running.
	bool isRunning(void)const{ return _status==RUNNING;}

	///wait until the thread is ended or timeout
	///@param timeoutMillis timeout
	///@return return false only if timeout, other return true means thread have quit.
	bool wait(DWORD timeoutMillis=INFINITE)const;

	///close the job handle.
	void close();

private:
	
	/// the initial steps can be put here after start() is called
	virtual bool init(void)	{ return true; };

	/// should be Overrided! main steps for thread
	virtual int run() =0;

	/// you can do some clean work here, this functions is run in the thread.
	virtual void final() {}

	static DWORD WINAPI _execute(void *pVoid)
	{
		DWORD result=0;
		
		TThread* pThis = (TThread*)pVoid;

		try
		{
			if (pThis ==NULL || !pThis->init())
				return result;
			
			pThis->_status=RUNNING;
			result=pThis->run();

			pThis->_status=DISABLED;
		}
		catch (...) 
		{
			//added by salien
		//	OutputDebugString(L"salien******run exception in _execute\n");
		}
		
		try
		{
			pThis->final();
		}
		catch (...)
		{
			//added by salien
		//	OutputDebugString(L"salien******* final exception in _execute\n");
		}

     // if User call delete this in final() this will cause 
	 // access violation exception
		//pThis->_status=DISABLED;

		return result;
	}

	HANDLE _handle; ///thread handle

	Status _status; ///thread status
};

#endif // !defined(AFX_TTHREAD_H__C7258E84_FD5F_478A_8D60_7258A3FC9366__INCLUDED_)
