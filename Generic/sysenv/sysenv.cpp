// sysenv.cpp : Defines the entry point for the console application.
//
// Writen by Cary
// 2006-03-17

#include "stdafx.h"

void usage()
{
	printf("Usage:\n"
		"\tsysenv [-u][-s][-h] [variable=[string|-]]\n"
			"\t-u\t\tUser environment variable.\n"
			"\t-s\t\tSystem environment variable(default).\n"
			"\t-h\t\tShow help infomation.\n"
			"\tvariable\tEnvironment variable name.\n"
			"\tstring\t\tEnvironment variable value.\n"
			"\t-\t\tRemove environment variable.\n");
}

const char sysEnvVarReg[] = "System\\CurrentControlSet\\Control\\Session Manager\\Environment\\";
const char userEnvVarReg[] = "Environment\\";

void showVars(HKEY key)
{
	char keyName[MAX_PATH];
	BYTE keyValue[0x1000]; // 4k
	int index = 0;
	DWORD nameLen;
	DWORD valLen;
	DWORD type;
	while (true) {
		nameLen = sizeof(keyName);
		valLen = sizeof(keyValue);
		if (RegEnumValue(key, index, keyName, &nameLen, 
			NULL, &type, keyValue, &valLen) != ERROR_SUCCESS) {

			break;
		}
		
		index ++;
		printf("%s=%s\n", keyName, keyValue);
	}
}

bool addVar(HKEY key, char name[], BYTE value[])
{
	return RegSetValueEx(key, name, NULL, REG_EXPAND_SZ, value, 
		strlen((char* )value)) == ERROR_SUCCESS;
}

bool delVar(HKEY key, char name[])
{
	return RegDeleteValue(key, name) == ERROR_SUCCESS;
}

bool parseItem(const char* arg, char name[], char value[])
{
	const char* c = arg;
	char* n = name;
	char* v = value;
	bool val = false;
	while(*c) {
		if (*c == '=') {
			val = true;
		} else {
			if (val) {
				*v = *c;
				v ++;
			}
			else {
				*n = *c;
				n ++;
			}
		}

		c ++;
	}
	*n = 0;
	*v = 0;
	return true;
}

void showSpecVar(HKEY key, const char* name)
{
	BYTE varValue[0x1000]; // 4k	
	DWORD len = sizeof(varValue);
	DWORD type;
	if ((RegQueryValueEx(key, name, NULL, &type, 
		varValue, &len)) == ERROR_SUCCESS) {
		
		printf("%s=%s\n", name, varValue);
	} else {
		printf("Not found variable: %s\n", name);
	}
}

int main(int argc, char* argv[])
{
	LONG r;
	HKEY regRoot = HKEY_LOCAL_MACHINE;
	HKEY regEnv;
	const char* regPath = sysEnvVarReg;
	int n = 1;
	bool showOnly = false;
	if (argc > 1) {
		if (strcmp(argv[1], "-h") == 0) {
			usage();
			return 0;
		}
		
		if (strcmp(argv[1], "-u") == 0) {
			regPath = userEnvVarReg;
			regRoot = HKEY_CURRENT_USER;
			n = 2;
		}

		if (strcmp(argv[1], "-s") == 0) {
			n = 2;
		}
	} else {
		showOnly = true;
	}

	r = RegOpenKey(regRoot, regPath, &regEnv);
	if (r != ERROR_SUCCESS) {
		fprintf(stderr, "Error: %x(can't find path).\n", r);
		return -1;
	}

	if (n == 2 && argc < 3) {
		showOnly = true;
	}

	if (showOnly) {
		showVars(regEnv);
		RegCloseKey(regEnv);
		return 0;
	}

	char varName[MAX_PATH];
	BYTE varValue[0x1000]; // 4k
	bool changed = false;

	for (; n < argc; n ++) {
		if (!parseItem(argv[n], varName, (char *)varValue)) {
			fprintf(stderr, "invalid syntax.\n");
			usage();
			break;
		}

		if (strlen((char* )varValue) <= 0) {
			showSpecVar(regEnv, varName);
		} else if (strcmp((char *)varValue, "-") == 0) {
			if (!delVar(regEnv, varName))
				printf("Not found variable: %s\n", varName);
			else
				changed = true;
		} else {
			if (!addVar(regEnv, varName, varValue)) {
				fprintf(stderr, "unknown error.\n");
				break;
			} else
				changed = true;
		}
	}
	
	RegCloseKey(regEnv);

	if (changed)
		SendMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment");

	return 0;
}
