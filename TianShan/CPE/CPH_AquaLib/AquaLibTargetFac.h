#ifndef RTIRAW_TARGETFAC_H
#define RTIRAW_TARGETFAC_H


#include "TargetFactoryI.h"
#include "AquaFileSetTarget.h"


#pragma once
namespace ZQTianShan 
{
	namespace ContentProvision
	{
		class FileIoFactory;
		class AquaLibTargetFac : public TargetFactoryI
		{
		public:
			AquaLibTargetFac(FileIoFactory*);
			virtual BaseTarget* create(const char* szName);
		protected:
			FileIoFactory*			_pFileIoFac;
		};

	}}//namespace

#endif