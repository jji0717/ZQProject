// Command.cpp: implementation of the Command class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Command.h"

#define SETTING_ERRMODE		"errmode"

Commander::Commander()
{
	add("set", new SetCmd(_settingMap), 
		"{set [var_name] [var_value]} show and set option");
	add("quit", new QuitCmd, 
		"{quit} quit the program");

	add("help", new HelpCmd, "{help} show the help information");
	add("exec", new ExecCmd, 
		"{exec <filename>} execute a dsclient script");

	setSettingInt(SETTING_ERRMODE, 0, true);
}

Commander::~Commander()
{

}

bool Commander::add(const std::string& cmdName, CommandHandler* handler, 
					const std::string& desc)
{
	std::pair<HandlerMap::iterator, bool> r;
	HandlerEntry entry;
	entry.cmd = cmdName;
	entry.handler = handler;
	entry.desc = desc;
	r = _handlers.insert(HandlerMap::value_type(cmdName, entry));
	return r.second;
}

Commander::HandlerEntry* Commander::matchHandler(const std::string& cmd)
{
	HandlerMap::iterator it = _handlers.find(cmd);
	if (it != _handlers.end())
		return &it->second;

	HandlerMap::iterator itFound = _handlers.end();
	for (it = _handlers.begin(); it != _handlers.end(); it ++) {
		if (it->first.compare(0, cmd.size(), cmd) == 0) {
			if (itFound == _handlers.end())
				itFound = it;
			else {
				return NULL;
			}
		}
	}

	if (itFound == _handlers.end())
		return NULL;
	else
		return &itFound->second;
}

size_t Commander::parseCmd(const std::string& cmdline, Arguments& args)
{
	std::string::const_iterator s = cmdline.begin(), e = s;
	
	bool space = true;
	bool quota = false;

	while(e != cmdline.end()) {
		if (!quota && (*e == '\x020' || *e == '\t')) {

			if (!space) {
				space = true;
				std::string arg(s, e);
				args.push_back(arg);
			}

		} else if (*e == '\"') {

			if (!quota) {
				
				if (!space) {
					std::string arg(s, e);
					args.push_back(arg);
				}

				s = e;
				s ++;
				quota = true;

			} else {
				std::string arg(s, e);
				args.push_back(arg);
				quota = false;
				space = true;
			}
			
		} else {
			if (space) {
				s = e;

				space = false;
			}
		}
		
		e ++;
	}

	if (quota) {

		return 0;
	}

	if (!space) {
		std::string arg(s, e);
		args.push_back(arg);
	}

	return args.size();
}

int Commander::handle(const std::string& cmdline)
{
	Arguments args;
	int res = parseCmd(cmdline, args);
	if (res == 0)
		return CMD_R_BAD;

	HandlerEntry* entry = matchHandler(args[0]);
	if (entry) {
		assert(entry->handler);
		return entry->handler->handler(*this, args);
	}

	return CMD_R_UNKNOWN;
}

int Commander::print(const char* fmt, ...)
{
	char buf[2025];
	va_list vlist;
	va_start(vlist, fmt);

	int res = vsprintf(buf, fmt, vlist);

	fputs(buf, stdout);

	return res;
}

std::string Commander::inputStr(const std::string& prompt, 
							 const std::string& def)
{
	if (def.size())
		print("%s[%s]:", prompt.c_str(), def.c_str());
	else
		print("%s:", prompt.c_str());

	char buf[512];
	gets(buf);
	if (strlen(buf) <= 0)
		return def;
	
	return buf;
}

int Commander::inputInt(const std::string& prompt, 
						int def)
{
	print("%s[%d]:", prompt.c_str(), def);

	char buf[512];
	gets(buf);
	if (strlen(buf) <= 0)
		return def;
	
	return atoi(buf);
}

bool Commander::setSettingInt(const std::string& name, int value, 
							  bool adding)
{
	char buf[64];

	SettingMap::iterator it = _settingMap.find(name);
	if (it == _settingMap.end()) {
		if (!adding)
			return false;

		std::pair<SettingMap::iterator, bool> r;
			sprintf(buf, "%d", value);
		SettingItem item;
		item.type = SettingItem.INT_TYPE;
		item.value = buf;
		r = _settingMap.insert(SettingMap::value_type(name, item));
		return r.second;
	}

	sprintf(buf, "%d", value);
	it->second.value = buf;

	return true;
}

bool Commander::setSettingString(const std::string& name, 
								 const std::string& value, 
								 bool adding)
{
	SettingMap::iterator it = _settingMap.find(name);
	if (it == _settingMap.end()) {
		if (!adding)
			return false;

		std::pair<SettingMap::iterator, bool> r;
		SettingItem item;
		item.type = SettingItem.STR_TYPE;
		item.value = value;

		r = _settingMap.insert(SettingMap::value_type(name, item));
		return r.second;
	}

	it->second.value = value;

	return true;
}

bool Commander::getSettingInt(const std::string& name, int& value)
{
	SettingMap::iterator it = _settingMap.find(name);
	if (it == _settingMap.end())
		return false;

	if (it->second.type != SettingItem::INT_TYPE)
		return false;

	value = atoi(it->second.value.c_str());
	return true;
}

bool Commander::getSettingString(const std::string& name, 
								 std::string& value)
{
	SettingMap::iterator it = _settingMap.find(name);
	if (it == _settingMap.end())
		return false;

	if (it->second.type != SettingItem::STR_TYPE)
		return false;

	value = it->second.value;
	return true;
}

//////////////////////////////////////////////////////////////////////////

SetCmd::SetCmd(SettingMap& settingMap): _settingMap(settingMap)
{

}

int SetCmd::handler(Commander& cmder, const Arguments& args)
{
	if (args.size() == 1) {

		SettingMap::iterator it = _settingMap.begin();
		for (;it != _settingMap.end(); it ++) {
			cmder.print("%s = %s\n", it->first.c_str(), 
				it->second.value.c_str());
		}

		return CMD_R_OK;
	}

	if (args.size() == 2) {
		SettingMap::iterator it = _settingMap.find(args[1]);
		if (it == _settingMap.end()) {
			cmder.print("unknown setting\n");
			return CMD_R_FAILED;
		}

		cmder.print("%s = %s\n", it->first.c_str(), 
			it->second.value.c_str());

		return CMD_R_OK;
	}

	if (args.size() == 3) {

		SettingMap::iterator it = _settingMap.find(args[1]);
		if (it == _settingMap.end()) {
			cmder.print("unknown setting\n");
			return CMD_R_FAILED;
		}

		if (it->second.type == SettingItem::INT_TYPE) {
			int v = atoi(args[2].c_str());
			char buf[64];
			sprintf(buf, "%d", v);
			it->second.value = buf;
		} else {
			it->second.value = args[2];
		}

		return CMD_R_OK;
	}

	return CMD_R_INVLIADARGS;
}

//////////////////////////////////////////////////////////////////////////

int HelpCmd::handler(Commander& cmder, const Arguments& args)

{
	Commander::HandlerMap::iterator it;

	if (args.size() == 1) {
	
		for (it = cmder._handlers.begin(); it != cmder._handlers.end(); 
			it ++) {

			cmder.print("  %-20s%s\n", it->first.c_str(), 
				it->second.desc.c_str());
		}

	} else if (args.size() == 2) {
		Commander::HandlerEntry* entry = cmder.matchHandler(args[1]);
		if (entry == NULL) {
			cmder.print("unknown command\n");
			return CMD_R_FAILED;
		} else {
			cmder.print("  %-20s%s\n", entry->cmd.c_str(), 
				entry->desc.c_str());			
		}
	} else {

		return CMD_R_INVLIADARGS;
	}

	return CMD_R_OK;
}

//////////////////////////////////////////////////////////////////////////

bool ExecCmd::isSpaceLine(const char* cmdline)
{
	const char* c = cmdline;
	while (*c && *c != '\n') {
		if (*c != ' ' && *c != '\t')
			return (*c == '#');
		c ++;
	}

	// ¿ÕÐÐ
	return true;
}

void ExecCmd::printErrMsg(Commander& cmder, const char* cmdline, int err)
{
	switch (err) {
	case CMD_R_FAILED:
		cmder.print("\'%s\', failed\n", cmdline);
		break;
	case CMD_R_BAD:
		cmder.print("\'%s\', invalid command line\n", cmdline);
		break;

	case CMD_R_INVLIADARGS:
		cmder.print("\'%s\', invalid arguments nmuber\n", cmdline);
		break;

	case CMD_R_UNKNOWN:
		cmder.print("\'%s\', unknown command\n", cmdline);
		break;

	default:
		cmder.print("\'%s\', unknown error\n", cmdline);
		break;
	}
}

int ExecCmd::handler(Commander& cmder, const Arguments& args)
{
	if (args.size() < 2) {
		return CMD_R_INVLIADARGS;
	}

	std::ifstream ifstrm(args[1].c_str());
	if (!ifstrm.is_open()) {
		if (!(args.size() > 2 && args[2] == "init"))
			cmder.print("cannot found the file\n");
		return CMD_R_FAILED;
	}

	char cmdline[512];
	int r;
	int errMode;
	while (!ifstrm.eof()) {
		ifstrm.getline(cmdline, sizeof(cmdline));
		if (!isSpaceLine(cmdline)) {
			r = cmder.handle(cmdline);

			if (r == CMD_R_QUIT)
				break;

			cmder.getSettingInt(SETTING_ERRMODE, errMode);

			if (r != CMD_R_OK) {
				if (errMode == 0) {
					printErrMsg(cmder, cmdline, r);
					break;
				}
			}

			if (r != CMD_R_OK && r != CMD_R_FAILED) {
				if (errMode == 1) {
					printErrMsg(cmder, cmdline, r);
					break;
				}
			}
		}
	}

	return CMD_R_OK;
}
