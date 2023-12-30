#include "DialogType.h"

namespace ZQ{
	namespace StreamSmith{

		DialogType::DialogType(int nDialogType)
			:_nDialogType(nDialogType)
		{
		}

		DialogType::~DialogType(void)
		{
		}

		int DialogType::getType() const
		{
			return _nDialogType;
		}

	}
}

