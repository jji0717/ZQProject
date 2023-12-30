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
// Desc  : modified expatxx header
// --------------------------------------------------------------------------------------------
// Revision History: 
// $Header: /ZQProjs/MediaProcessFramework/EntryDB/expatxx.h 1     10-11-12 16:00 Admin $
// $Log: /ZQProjs/MediaProcessFramework/EntryDB/expatxx.h $
// 
// 1     10-11-12 16:00 Admin
// Created.
// 
// 1     10-11-12 15:32 Admin
// Created.
// 
// 4     4/13/05 6:34p Hui.shao
// changed namespace
// 
// 3     4/12/05 5:39p Hui.shao
// ============================================================================================

// this class is created as a static liberary 21Jul02

#ifndef __EXPATXX_H__
#define __EXPATXX_H__

#include "EntryDB.h"
#include "expat.h"

ENTRYDB_NAMESPACE_BEGIN

/*
// Interface IXMLParserCallBack
class IXMLParserCallBack
{
public:
	virtual void startElement(const XML_Char* name, const XML_Char** atts) =0;
	virtual void endElement(const XML_Char*) =0;
	virtual void charData(const XML_Char*, int len) =0;
	virtual void processingInstruction(const XML_Char* target, const XML_Char* data) =0;
	virtual void defaultHandler(const XML_Char*, int len) =0;
	virtual int  notStandaloneHandler() =0;
	virtual void unparsedEntityDecl(const XML_Char* entityName, const XML_Char* base, const XML_Char* systemId, const XML_Char* publicId, const XML_Char* notationName) =0;
	virtual void notationDecl(const XML_Char* notationName, const XML_Char* base, const XML_Char* systemId, const XML_Char* publicId) =0;
	virtual void startNamespace(const XML_Char* prefix, const XML_Char* uri) =0;
	virtual void endNamespace(const XML_Char*) =0;

	virtual void logicalClose() =0;
};
*/

// class ExpatBase
class ExpatBase // : public IXMLParserCallBack
{
public:
	ExpatBase(bool createParser=true);
	virtual ~ExpatBase();

	operator XML_Parser() const;
	
	bool emptyCharData(const XML_Char* s, int len); // utility often used in overridden charData

	// overrideable callbacks, from IXMLParserCallBack
	virtual void startElement(const XML_Char* name, const XML_Char** atts);
	virtual void endElement(const XML_Char*);
	virtual void charData(const XML_Char*, int len);
	virtual void processingInstruction(const XML_Char* target, const XML_Char* data);
	virtual void defaultHandler(const XML_Char*, int len);
	virtual int  notStandaloneHandler();
	virtual void unparsedEntityDecl(const XML_Char* entityName, const XML_Char* base, const XML_Char* systemId, const XML_Char* publicId, const XML_Char* notationName);
	virtual void notationDecl(const XML_Char* notationName, const XML_Char* base, const XML_Char* systemId, const XML_Char* publicId);
	virtual void startNamespace(const XML_Char* prefix, const XML_Char* uri);
	virtual void endNamespace(const XML_Char*);
	
	virtual void logicalClose();

	// XML interfaces
	int        XML_Parse(const char* buffer, int len, int isFinal);
	XML_Error  XML_GetErrorCode();
	int        XML_GetCurrentLineNumber();
	const XML_Char* XML_GetErrorString();
	
protected:
	XML_Parser mParser;

	int depth; // for logical close

// interface functions for callbacks
public:
	static void startElementCallback(void *userData, const XML_Char* name, const XML_Char** atts);
	static void endElementCallback(void *userData, const XML_Char* name);
	static void startNamespaceCallback(void *userData, const XML_Char* prefix, const XML_Char* uri);
	static void endNamespaceCallback(void *userData, const XML_Char* prefix);
	static void charDataCallback(void *userData, const XML_Char* s, int len);
	static void processingInstructionCallback(void *userData, const XML_Char* target, const XML_Char* data);
	static void defaultHandlerCallback(void* userData, const XML_Char* s, int len);
	static int  notStandaloneHandlerCallback(void* userData);	
	static void unParsedEntityDeclCallback(void* userData, const XML_Char* entityName, const XML_Char* base, const XML_Char* systemId, const XML_Char* publicId, const XML_Char* notationName);
	static void notationDeclCallback(void *userData, const XML_Char* notationName, const XML_Char* base, const XML_Char* systemId, const XML_Char* publicId);
	
// utilities
	static int skipWhiteSpace(const char*);	
};


// class ExpatBaseNesting
class ExpatBaseNesting : public ExpatBase
{
// subclass to support a hierarchy of parsers, in a sort of recursion or
// 'nesting' approach, where a top-level parser might create sub-parsers for part of a file
public:
	ExpatBaseNesting();
	ExpatBaseNesting(ExpatBaseNesting* parent);  // NOT a copy ctor!! this is a recursive situation
	virtual ~ExpatBaseNesting();
	
	ExpatBaseNesting* returnToParent();

protected:
	int	mDepth;	
	ExpatBaseNesting* mParent;

// interface functions for callbacks
public:
	static void nestedStartElementCallback(void* userData, const XML_Char* name, const XML_Char** atts);
	static void nestedEndElementCallback(void* userData, const XML_Char* name);
};

ENTRYDB_NAMESPACE_END

#endif //__EXPATXX_H__

