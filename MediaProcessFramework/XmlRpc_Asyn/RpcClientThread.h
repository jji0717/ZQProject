#ifndef __RPC_CLIENT_THREAD_H
#define __RPC_CLIENT_THREAD_H

#include "../../common/NativeThread.h"

#ifdef _cplusplus
extern "C" {
#endif
	
	namespace ZQ
	{
		namespace rpc
		{
			class RpcClientThread : public ZQ::common::NativeThread
			{
				int run();

			public:
				DWORD join(timeout_t timeout = INFINITE);
			};
		}
	}

#ifdef _cplusplus
}
#endif

#endif // __RPC_CLIENT_THREAD_H