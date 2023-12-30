
#ifndef _ZQ_LISTINFO_H_
#define _ZQ_LISTINFO_H_
	
#include "xmlrpc.h"
#include "systeminfo.h"
#include "MPFVersion.h"
#include "listinfo_def.h"
	
#define MAX_INFORMATION_STR_LEN 256

#define NON_STRUCTURE "NONE"

namespace ZQ
{
namespace MPF
{
	using ZQ::MPF::SysInfo::SystemInfo;

	///ListInfoMethod\n
	///this is a Rpcmethod that list mpf node information
	class DLL_PORT ListInfoMethod : public ZQ::rpc::RpcServerMethod
	{
	private:
		SystemInfo	m_si;
		char		m_strInterface[20];
		int			m_nodetype;
		
		void execute(ZQ::rpc::RpcValue& params, ZQ::rpc::RpcValue& result);
	
		void getSystemInfo(ZQ::rpc::RpcValue& result);
		
		void getNodeType();
	public:
		///constructor
		///@param server - rpc server instance to bind
		ListInfoMethod(ZQ::rpc::RpcServer& server, const char* strInterface, int nodeType);
	
		///destructor
		virtual ~ListInfoMethod();
		
		///set interface string
		///@param strInterface - interface string(ip string)
		void setInterface(const char* strInterface);
		
	protected:
		/// provide a chance for user derived class to do some verification work before
		/// handle the request.  This function will be called at first when information request comes
		///@param[in]	in				-the original input of the request
		///@param[out]	out				-the output if necessary
		///@return						-True if verify passed, False else
		///@remarks						-If this function returns True, execute() will continue to do work.
		/// Otherwise, the execute() exit with the return value got from this function.
		virtual bool OnVerify(ZQ::rpc::RpcValue& in, ZQ::rpc::RpcValue& out)
		{
			return true;
		}
		
		/// provide an entry for user derived class to do some extra request handle.
		/// This function will be called when information request comes
		///@param[in]	strInfoType		-info request type
		///@param[in]	params			-the input parameters
		///@param[out]	result			-the output result
		virtual void OnGetInfo(const char* strInfoType, ZQ::rpc::RpcValue& params, ZQ::rpc::RpcValue& result) = 0;

	};
}
}
	
#endif//_ZQ_LISTINFO_H_
