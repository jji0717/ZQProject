#include "admin.h"
#include "getopt.h"

extern "C"
{
#include <time.h>
#include <stdio.h>
}

WeiwooAdminConsole gAdmin;

int main(int argc, char* argv[])
{
	int ch;
	while((ch = getopt(argc, argv, "he:s:b:")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
			gAdmin.usage();
			exit(0);

		case 'e':
			{
				Args args;
				args.push_back(optarg);
				gAdmin.connect(args);
			}
			break;

		case 's':
			{
				Args args;
				args.push_back(optarg);
				gAdmin.subscribe(args);
			}
			break;

		default:
			printf("Error: unknown option %c specified\n", ch);
			exit(1);
		}
	}

	gAdmin.parse(stdin, 1);
	return 0;
}

