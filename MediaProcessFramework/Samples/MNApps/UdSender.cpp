

#include "../../WorkNode/WorkFactory.h"

class TestWork: public ZQ::MPF::WorkNode::BaseWork
{
public:
	TestWork(ZQ::MPF::WorkNode::WorkFactory& factory)
		:BaseWork(&factory)
	{}

	bool TestUpdateSession()
	{
		ZQ::rpc::RpcValue attr, exattr, result;
		return updateSession("sds", attr, exattr, result);
	}
};

int main(void)
{
	ZQ::MPF::WorkNode::WorkFactory wf;
	TestWork bw(wf);
		
	system("pause");
	return 0;
}
