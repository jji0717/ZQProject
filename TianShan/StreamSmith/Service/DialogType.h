#ifndef __DIALOGTYPE_H__
#define __DIALOGTYPE_H__

#include "common_define.h"

namespace ZQ{
	namespace StreamSmith{
		class DialogType : public ZQ::DataPostHouse::SharedObject
		{
		public:
			DialogType(int nDialogType);
			~DialogType(void);
		public:
			int getType() const;
		private:
			int _nDialogType;

		};
		typedef ZQ::DataPostHouse::ObjectHandle<DialogType> DialogTypePtr;
	}
}

#endif
