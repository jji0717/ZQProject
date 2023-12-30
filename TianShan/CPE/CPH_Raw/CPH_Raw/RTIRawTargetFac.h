#ifndef RTIRAW_TARGETFAC_H
#define RTIRAW_TARGETFAC_H


#include "TargetFactoryI.h"
#include "RTIRawTarget.h"


#pragma once
namespace ZQTianShan 
{
	namespace ContentProvision
	{
		class FileIoFactory;
		class RTIRawTargetFac : public TargetFactoryI
		{
		public:
			RTIRawTargetFac(FileIoFactory*);
			virtual BaseTarget* create(const char* szName);
		protected:
			FileIoFactory*			_pFileIoFac;
		};

	}}//namespace

#endif