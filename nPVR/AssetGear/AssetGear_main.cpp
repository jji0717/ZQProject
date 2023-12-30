#include "AssetGear_soap.h"

//#define __TEST 
#ifdef __TEST

int main(int argc, char* argv[])
{
	AssetGearService agService(NULL);
	
	agService.start();
	
	::Sleep(1000000);
	
	return 0;
}
#endif // __TEST

