// Command.h: interface for the Command class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COMMAND_H__C54F27D9_7719_4B8B_AB4B_762C73BB4CBB__INCLUDED_)
#define AFX_COMMAND_H__C54F27D9_7719_4B8B_AB4B_762C73BB4CBB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define	CMD_R_OK			0
#define	CMD_R_FAILED		-1
#define	CMD_R_BAD			-2
#define	CMD_R_QUIT			-3
#define	CMD_R_INVLIADARGS	-4
#define	CMD_R_UNKNOWN		-5

class Commander;

typedef std::vector<std::string> Arguments;

class CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args)
	{
		return CMD_R_FAILED;
	}
};

struct SettingItem {

	std::string	value;
	enum {
		INT_TYPE, 
		STR_TYPE
	} type;

};

typedef std::map<std::string, SettingItem> SettingMap;

class HelpCmd;
class ExecCmd;

class Commander
{
	friend class HelpCmd;
	friend class ExecCmd;
public:
	Commander();
	virtual ~Commander();

	bool add(const std::string& cmdName, CommandHandler* handler, 
		const std::string& desc = "");
	
	int handle(const std::string& cmdline);

	int print(const char* fmt, ...);
	std::string inputStr(const std::string& prompt, 
		const std::string& def = "");

	int inputInt(const std::string& prompt, int def = 0);

	bool setSettingInt(const std::string& name, int value, 
		bool adding = false);
	bool setSettingString(const std::string& name, const std::string& value, 
		bool adding = false);

	bool getSettingInt(const std::string& name, int& value);
	bool getSettingString(const std::string& name, std::string& value);

protected:
	struct HandlerEntry {
		std::string		cmd;
		CommandHandler* handler;
		std::string		desc;
	};

	size_t parseCmd(const std::string& cmdline, Arguments& args);
	HandlerEntry* matchHandler(const std::string& cmd);

protected:
	
	typedef std::map<std::string, HandlerEntry> HandlerMap;

	HandlerMap	_handlers;

	SettingMap	_settingMap;
};

//////////////////////////////////////////////////////////////////////////

class SetCmd: public CommandHandler {
public:
	SetCmd(SettingMap& settingMap);

	virtual int handler(Commander& cmder, const Arguments& args);

protected:
	
	SettingMap&	_settingMap;
};

//////////////////////////////////////////////////////////////////////////

class QuitCmd: public CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args)
	{
		return CMD_R_QUIT;
	}
};

//////////////////////////////////////////////////////////////////////////

class HelpCmd: public CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args);
};

class ExecCmd: public CommandHandler {
public:
	virtual int handler(Commander& cmder, const Arguments& args);

protected:
	bool isSpaceLine(const char* cmdline);
	void printErrMsg(Commander& cmder, const char* cmdline, int err);
};

#endif // !defined(AFX_COMMAND_H__C54F27D9_7719_4B8B_AB4B_762C73BB4CBB__INCLUDED_)
