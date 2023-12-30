#include "../Tunnel.h"

//McastFwd theServer;
ZQ::common::Log* pProglog = &ZQ::common::NullLogger;
ZQ::common::InetHostAddress gLocalAddrs;

int main(int argc, char* argv[])
{
	ZQ::common::FtHostAddress bind;
	bind += "192.168.0.138";
	bind += "192.168.80.8";

	TunnelConnection::_localid ="35dcab15-99bd-421f-bbbc-aa37a50785b6";

	TunnelConnection* conn = gTunnelManager.connect(bind, bind, 1970, 30000);
	if (NULL ==conn)
	{
		gTunnelManager.close();
		return 0;
	}

	ZQ::common::InetMcastAddress group("225.25.25.25");

	::Sleep(2000);
	ZQ::common::InetHostAddress source(bind.getHostAddress());

	bool ret =true;
	for (int i=0; ret && i<10; i++)
        ret = conn->forwardData((BYTE*)"Hello", strlen("Hello"), source, 1970, group, 1000, 30000);

	::Sleep(20000);
	conn->close();
	delete conn;

	exit(0);
}

