
#include "../../RequestPoster/TaskRequestPoster.h"


int main(void)
{
	ZQ::MPF::REQPOST::TaskRequestPoster trp("MPF://192.168.1.100:13000/worknode",
		"MPF://192.168.1.100:10000/session?id=TrSxx6Ps");

	ZQ::rpc::RpcValue setrst;
	if (POST_ERR_OK != trp.postSetup("CopyFileWork", setrst))
	{
		printf("can not post setup request\n");
		return -1;
	}

	ZQ::rpc::RpcValue uparam, uresult;
	if (POST_ERR_OK != trp.postUser("haha", uparam, uresult))
		printf("can not post user defined request\n");

	return 0;
}
