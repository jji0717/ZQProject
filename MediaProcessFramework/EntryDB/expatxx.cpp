// ============================================================================================
// Copyright (c) 1997, 1998 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved. Unpublished rights reserved under the copyright laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the confidential
// technology of ZQ Interactive, Inc. Possession, use, duplication or dissemination of the
// software and media is authorized only pursuant to a valid written license from ZQ Interactive,
// Inc.
// This was copied from enterprise domain object sys, edos's copyright is belong to Hui Shao
//
// This software is furnished under a  license  and  may  be used and copied only in accordance
// with the terms of  such license and with the inclusion of the above copyright notice.  This
// software or any other copies thereof may not be provided or otherwise made available to any
// other person.  No title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and should not be
// construed as a commitment by ZQ Interactive, Inc.
// --------------------------------------------------------------------------------------------
// Author: Hui Shao
// Desc  : modified expatxx
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/expatxx.cpp 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/expatxx.cpp $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 3     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 2     4/12/05 5:39p Hui.shao
// ============================================================================================

#include "Expatxx.h"
#include <assert.h>

#ifdef WIN32 
#  if defined(_MT) || defined(MT)
#     pragma comment(lib, "libexpatMT.lib")
#  else
#     pragma comment(lib, "libexpat.lib")
#  endif
#endif // WIN32

ENTRYDB_NAMESPACE_BEGIN

// class ExpatBase
ExpatBase::ExpatBase(bool createParser)
	      :depth(0)
{
	if (createParser) {
		// subclasses may call this ctor after parser created!
		mParser = XML_ParserCreate(0);
		::XML_SetUserData(mParser, this);
		::XML_SetElementHandler(mParser, startElementCallback, endElementCallback);
		::XML_SetCharacterDataHandler(mParser, charDataCallback);
		::XML_SetProcessingInstructionHandler(mParser, processingInstructionCallback);
		::XML_SetDefaultHandler(mParser, defaultHandlerCallback);
		::XML_SetUnparsedEntityDeclHandler(mParser, unParsedEntityDeclCallback);
		::XML_SetNotationDeclHandler(mParser, notationDeclCallback);
		::XML_SetNotStandaloneHandler(mParser, notStandaloneHandlerCallback);
		::XML_SetNamespaceDeclHandler(mParser, startNamespaceCallback, endNamespaceCallback);
	}
}


ExpatBase::~ExpatBase()
{
	if (mParser)  // allows subclasses to avoid finishing parsing
	  ::XML_ParserFree(mParser);
}

ExpatBase::operator XML_Parser() const
{
	return mParser;
}

int ExpatBase::XML_Parse(const char *s, int len, int isFinal)
{
	return ::XML_Parse(mParser, s, len, isFinal);
}

XML_Error ExpatBase::XML_GetErrorCode()
{
	return ::XML_GetErrorCode(mParser);
}

int ExpatBase::XML_GetCurrentLineNumber()
{
	return ::XML_GetCurrentLineNumber(mParser);
}

const XML_Char* ExpatBase::XML_GetErrorString()
{
	return ::XML_ErrorString(XML_GetErrorCode());
}

void ExpatBase::startElementCallback(void *userData, const XML_Char* name, const XML_Char** atts)
{
	((ExpatBase*)userData)->depth++;
	((ExpatBase*)userData)->startElement(name, atts);
}

void ExpatBase::endElementCallback(void *userData, const XML_Char* name)
{
	((ExpatBase*)userData)->depth--;
	((ExpatBase*)userData)->endElement(name);

	if (((ExpatBase*)userData)->depth ==0)
		((ExpatBase*)userData)->logicalClose();
}

void ExpatBase::startNamespaceCallback(void *userData, const XML_Char* prefix, const XML_Char* uri)
{
	((ExpatBase*)userData)->startNamespace(prefix, uri);
}

void ExpatBase::endNamespaceCallback(void *userData, const XML_Char* prefix)
{
	((ExpatBase*)userData)->endNamespace(prefix);
}

void ExpatBase::charDataCallback(void *userData, const XML_Char* s, int len)
{
	((ExpatBase*)userData)->charData(s, len);
}

void ExpatBase:: processingInstructionCallback(void *userData, const XML_Char* target, const XML_Char* data)
{
	((ExpatBase*)userData)->processingInstruction(target, data);
}

void ExpatBase::defaultHandlerCallback(void* userData, const XML_Char* s, int len)
{
	((ExpatBase*)userData)->defaultHandler(s, len);
}

int ExpatBase::notStandaloneHandlerCallback(void* userData)
{
	return ((ExpatBase*)userData)->notStandaloneHandler();
}

void ExpatBase::unParsedEntityDeclCallback(void* userData, const XML_Char* entityName, const XML_Char* base, const XML_Char* systemId, const XML_Char* publicId, const XML_Char* notationName)
{
	((ExpatBase*)userData)->unparsedEntityDecl(entityName, base, systemId, publicId, notationName);
}

void ExpatBase::notationDeclCallback(void *userData, const XML_Char* notationName, const XML_Char* base, const XML_Char* systemId, const XML_Char* publicId)
{
	((ExpatBase*)userData)->notationDecl(notationName, base, systemId, publicId);
}

//int ExpatBase::externalEntityRefCallback(XML_Parser parser, const XML_Char* openEntityNames, const XML_Char* base, const XML_Char* systemId, const XML_Char* publicId)
//{
//	((ExpatBase*)parser)->externalEntityRef(openEntityNames, base, systemId, publicId);
//}


void ExpatBase::startElement(const XML_Char*, const XML_Char**)
{
}

void ExpatBase::endElement(const XML_Char*)
{
}

void ExpatBase::startNamespace(const XML_Char* /* prefix */, const XML_Char* /* uri */)
{
}

void ExpatBase::logicalClose()
{
}

void ExpatBase::endNamespace(const XML_Char*)
{
}

void ExpatBase::charData(const XML_Char*, int )
{
}

void ExpatBase::processingInstruction(const XML_Char*, const XML_Char*)
{
}

void ExpatBase::defaultHandler(const XML_Char*, int)
{
}

int ExpatBase::notStandaloneHandler()
{
	return 0;
}

void ExpatBase::unparsedEntityDecl(const XML_Char*, const XML_Char*, const XML_Char*, const XML_Char*, const XML_Char*)
{
}

void ExpatBase::notationDecl(const XML_Char*, const XML_Char*, const XML_Char*, const XML_Char*)
{
}

int ExpatBase::skipWhiteSpace(const char* startFrom)
{
	// use our own XML definition of white space
	// TO DO - confirm this is correct!
	const char* s = startFrom;
	char c = *s;
	while ((c==' ') || (c=='\t') || (c=='\n') || (c=='\r')) {
		s++;
		c = *s;
	}
	const int numSkipped = s - startFrom;
	return numSkipped;
}

bool ExpatBase::emptyCharData(const XML_Char *s, int len)
{
// usually call from top of overriden charData methods
	if (len==0)
		return true;  //*** early exit - empty string, may never occur??
		
// skip newline and empty whitespace
	if (
		((len==1) && ( (s[0]=='\n') || (s[0]=='\r')) ) ||  // just CR or just LF
		((len==2) && (s[0]=='\r') && (s[1]=='\n'))  // DOS-style CRLF
	)
		return true;  //*** early exit - newline
		
	const int lastCharAt = len-1;
	if (s[lastCharAt]==' ') {  // maybe all whitespace
		int i;
		for (i=0; i<lastCharAt; i++) {
			if (s[i]!=' ')
				break;
		}
		if (i==lastCharAt)
			return true;	  //*** early exit - all spaces
	}
	return false;
}

// class ExpatBaseNesting
ExpatBaseNesting::ExpatBaseNesting()
              : mDepth(0), mParent(0)
{
// WARNING
// the assumption that is not obvious here is that if you want to use 
// nested parsers, then your topmost parser must also be an ExpatBaseNesting
// subclass, NOT an ExpatBase subclass, because we need the following special
// callbacks to override those in the ExpatBase ctor
	::XML_SetElementHandler(mParser, nestedStartElementCallback, nestedEndElementCallback);
}

ExpatBaseNesting::ExpatBaseNesting(ExpatBaseNesting* parent)
              :	ExpatBase(false),  // don't create parser - we're taking over from inParent
                mDepth(0), mParent(parent)
{
	mParser = parent->mParser;
	assert(mParser);
	::XML_SetUserData(mParser, this);
}

ExpatBaseNesting::~ExpatBaseNesting()
{
	assert(!mParent);  // if we are a sub-parser, should not delete without calling returnToParent
}

ExpatBaseNesting* ExpatBaseNesting::returnToParent()
{
	ExpatBaseNesting* ret = mParent;
	::XML_SetUserData(mParser, mParent);
	mParent=0;
	mParser=0;  // prevent parser shutdown!!
	delete this;  // MUST BE LAST THING CALLED IN NON-VIRTUAL FUNCTION
	return ret;
}

void ExpatBaseNesting::nestedStartElementCallback(void *userData, const XML_Char* name, const XML_Char** atts)
{
	ExpatBaseNesting* nestedParser = (ExpatBaseNesting*)userData;
	nestedParser->mDepth++;
	((ExpatBase*)userData)->startElement(name, atts);  // probably user override
}

void ExpatBaseNesting::nestedEndElementCallback(void *userData, const XML_Char* name)
{
	ExpatBaseNesting* nestedParser = (ExpatBaseNesting*)userData;
// we don't know until we hit a closing tag 'outside' us that our run is done 	
	if (nestedParser->mDepth==0) {
		ExpatBaseNesting* parentParser = nestedParser->returnToParent();
		nestedEndElementCallback(parentParser, name);   // callbacks for ExpatBaseNesting stay registered, so safe 
		//if we don't invoke their callback, they will not balance their mDepth
	}
	else {
	// end of an element this parser has started
		nestedParser->endElement(name);  // probably user override
		if ((--nestedParser->mDepth) ==0)
			nestedParser->logicalClose();
	}
}

ENTRYDB_NAMESPACE_END
