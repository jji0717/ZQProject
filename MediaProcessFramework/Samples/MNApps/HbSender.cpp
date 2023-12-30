
#include "../../WorkNode/Daemon.h"

int main(void)
{
	Daemon dm("MPF://192.168.1.101:13000");
	dm.addMgmNode("MPF://192.168.1.100:12000/");
	if (!dm.start())
	{
		printf("can not start daemon thread\n");
	}
	
	system("pause");
	return 0;
}
