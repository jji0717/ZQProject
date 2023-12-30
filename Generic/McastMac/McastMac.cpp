#include <winsock2.h>
#include <stdio.h>
#  pragma comment(lib, "wsock32.lib") // VC link wsock32

#include <stdio.h>
#define MAC_FMT   "01005e%02x%02x%02x"
#define MAC_FMT_D "01:00:5e:%02x:%02x:%02x"

void main(int argi, char* argv[])
{
	union {
		BYTE b[4];
		DWORD dw;
	} addr;

	if (argi <2 || (strcmp(argv[1], "-h") ==0))
	{
		printf("Compute fake MAC address for a Multicast IP address\nUsage: McastMac [-d] <McastIP>\n");
		return;
	}

	int i =1;
	bool deli = false;

	if (deli = (strcmp(argv[1], "-d") ==0))
		i++;

	if ((addr.dw = inet_addr(argv[i])) == INADDR_NONE)
	{
		printf("Invalid IP address: %s\n", argv[i]);
		return;
	}

	BYTE hb=addr.b[1] & 0x7f;

	if (deli)
		printf("MAC=" MAC_FMT_D "\n", hb, addr.b[2], addr.b[3]);
	else
		printf("MAC=" MAC_FMT "\n", hb, addr.b[2], addr.b[3]);
}