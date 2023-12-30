#include "PathAdmin.h"

PathAdminConsole gPathAdmin;

int main()
{
	gPathAdmin.parse(stdin, 1);
	return 0;
}