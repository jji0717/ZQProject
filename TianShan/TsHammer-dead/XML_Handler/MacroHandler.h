#ifndef __MACROHANDLER__
#define __MACROHANDLER__

#include "ZQ_common_conf.h"

#include <string>
#include <vector>

#define INCREEAMENT "INC("	//increase the int variable with 1
#define TAILORINCREMENT "TailorInc("

typedef enum
{
	TAILOR_HEX,
	TAILOR_DECIMAL
}TailorType;

class MacroHandler
{
public:
	MacroHandler();
	~MacroHandler();

	/// @function : INC(1) -> INC(2)
	bool updateMacro(::std::string &value);

	/// @function : INC(1) -> 1
	bool fixupMacro(::std::string &str);

	void setTailorProp(std::string strTailorType, int tailorRange);
private:
	std::string getTailorValue(const std::string strTailor);

private:
	::std::vector<::std::string> _macroVec;

	TailorType _tailorType;
	int _tailorRange;
	DWORD _tailorMaxValue;
	char* _tailorValue;
};

#endif __MACROHANDLER__