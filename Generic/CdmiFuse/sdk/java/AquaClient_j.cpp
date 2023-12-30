// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: JndiClient.java, hui.shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/CdmiFuse/sdk/java/AquaClient_j.cpp $
// 
// 14    11/19/14 8:05p Hui.shao
// userDomain
// 
// 13    2/20/14 10:02a Hui.shao
// moved core impls into AquaClient.cpp
// 
// 12    1/09/14 4:00p Hongquan.zhang
// 
// 11    12/27/13 12:40p Hongquan.zhang
// 
// 10    12/23/13 3:20p Hui.shao
// 
// 9     12/18/13 5:56p Hui.shao
// 
// 8     12/17/13 11:06a Hui.shao
// 
// 7     11/28/13 4:58p Hui.shao
// mapped all cdmi/noncdmi methods
// 
// 6     11/28/13 3:26p Hui.shao
// 
// 5     11/28/13 11:52a Hui.shao
// read object
// 
// 4     11/27/13 7:50p Hui.shao
// directbuffer
// 
// 3     11/27/13 4:19p Hui.shao
// about the nested obj
// 
// 2     11/27/13 1:56p Hui.shao
// 
// 1     11/20/13 7:46p Hui.shao
// created
// 
// 1     11/19/13 3:15p Hui.shao
// AquaClient draft
// ===========================================================================

#include "../../CdmiFuseOps.h"
#define _CUSTOM_TYPE_DEFINED_
#include "../AquaClientImpl.h"
#include "../NestedClient.h"

#include "FileLog.h"
#include <json/json.h>

// -----------------------------
// JNI utilities
// -----------------------------
#define MAX_MSG_LENGTH   (4096)

static jclass loadClass(JNIEnv* jenv, const char *clsname)
{
	if (NULL == jenv || clsname==NULL || *clsname ==0x00)
		return NULL;

	::std::string str = clsname;
	for (size_t i =0; i < str.size(); i++)
		str[i] = (str[i] == '.') ? '/' : str[i];

	jclass localRefCls = jenv->FindClass(str.c_str());

	if (localRefCls == NULL)
		return NULL; 

	// Create a global reference
	jclass globalRefCls = (jclass)jenv->NewGlobalRef((jclass)localRefCls);
	jenv->DeleteLocalRef(localRefCls);

	return globalRefCls;
}

static jstring newJstring(JNIEnv* jenv, const ::std::string& str)
{
	if (NULL == jenv)
		return NULL;

	jclass strClass  = jenv->FindClass("Ljava/lang/String;");
	jmethodID ctorID = jenv->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
	jbyteArray bytes = jenv->NewByteArray(str.length());
	jenv->SetByteArrayRegion(bytes, 0, str.length(), (const jbyte*)str.c_str());
	jstring encoding = jenv->NewStringUTF("utf-8");
	return (jstring) jenv->NewObject(strClass, ctorID, bytes, encoding);
}

static std::string readJstring(JNIEnv* jenv, jstring jstr)
{
	if (NULL == jenv)
		return "";

	int   length        = jenv->GetStringLength(jstr);  
	const jchar*  jcstr = jenv->GetStringChars(jstr, 0);

	if(length <= 0 || !jcstr)
		return "";

	wchar_t* wbuf = 0;
	char buf[MAX_MSG_LENGTH];
#ifdef ZQ_OS_MSWIN
	wbuf = (wchar_t*)jcstr;
#else
	wbuf = (wchar_t*)malloc((length+1)*sizeof(wchar_t));
	memset(wbuf, '\0', (length+1)*sizeof(wchar_t));
	for(int i = 0; i < length; ++i) {
		wbuf[i] = (wchar_t)jcstr[i];
	}
#endif
	size_t size = wcstombs(buf, wbuf, sizeof(buf)-1);

#ifndef ZQ_OS_MSWIN
	free(wbuf);
#endif

	if (size < 0) 
		return "";  

	jenv->ReleaseStringChars(jstr, jcstr);

	buf[size] = 0;  
	return buf;
}

static jobjectArray newJstringArray(JNIEnv* jenv, const ::std::vector< ::std::string >& strs)
{
	if (NULL == jenv)
		return NULL;

	jarray cls = (jarray)jenv->FindClass("java/lang/String");
	jarray ary = (jarray)jenv->NewObjectArray(strs.size(), (jclass)cls, 0);
	if (NULL == cls || NULL == ary)
		return NULL;

	for (size_t i =0; i < strs.size(); i++)
	{
		jstring str = newJstring(jenv, strs[i]);
		jenv->SetObjectArrayElement((jobjectArray) ary, i, str);
		jenv->DeleteLocalRef(str);
	}

	return (jobjectArray) ary;
}

static int readJstringArray(JNIEnv* jenv, ::std::vector< ::std::string >& strs, jobjectArray jstrs)
{
	strs.clear();

	if (NULL == jenv)
		return 0;

	jsize len = jenv->GetArrayLength(jstrs);
	for (jsize i =0; i < len; i++)
	{
		jstring jstr = (jstring)jenv->GetObjectArrayElement(jstrs, i);
		std::string str = readJstring(jenv, jstr);
		strs.push_back(str);
	}

	return (int) strs.size();
}

static int fillStringBuffer(JNIEnv* jenv, jobject jstringbuf, const std::string& str)
{
	if (NULL == jstringbuf || str.length() <=0)
		return 0;

	jstring jstr = newJstring(jenv, str);

	jclass clsStringBuffer = jenv->FindClass("Ljava/lang/StringBuffer;");
	jmethodID ctorID = jenv->GetMethodID(clsStringBuffer, "append", "(Ljava/lang/String;)Ljava/lang/StringBuffer;");
	jenv->CallObjectMethod(jstringbuf, ctorID, jstr);

	jenv->DeleteLocalRef(jstr);

	return str.length();
}

static jbyte* getJBuffer(JNIEnv* jenv, jobject jbytebuf, jlong& len, jlong& cap)
{
	len = cap =0;
	if (NULL == jbytebuf)
		return NULL;

	jbyte* jbuf = (jbyte*) jenv->GetDirectBufferAddress(jbytebuf);
	cap = jenv->GetDirectBufferCapacity(jbytebuf);

	try {
		jclass jcls = jenv->FindClass("Ljava/nio/Buffer;");
		jmethodID mId = jenv->GetMethodID(jcls, "limit", "()I");
		if (NULL != mId)
			len = jenv->CallIntMethod(jbytebuf, mId);

		mId = jenv->GetMethodID(jcls, "flip", "()Ljava/nio/Buffer;");
		if (NULL != mId)
			jenv->CallObjectMethod(jbytebuf, mId);
	}
	catch(...) {}

	return jbuf;
}

static void limitJBuffer(JNIEnv* jenv, jobject jbytebuf, uint32 len)
{
	if (NULL == jbytebuf)
		return;

	try {
		jclass jcls = jenv->FindClass("Ljava/nio/Buffer;");
		jmethodID mId = jenv->GetMethodID(jcls, "limit", "(I)Ljava/nio/Buffer;");
		if (NULL != mId)
			jenv->CallObjectMethod(jbytebuf, mId, (jint)len);

		mId = jenv->GetMethodID(jcls, "flip", "()Ljava/nio/Buffer;");
		if (NULL != mId)
			jenv->CallObjectMethod(jbytebuf, mId);
	}
	catch(...) {}

	/*
	try {
	mId = jenv->GetMethodID(jcls, "limit", "(I)Ljava/nio/Buffer");
	if (NULL != mId)
	jenv->CallObjectMethod(jbytebuf, mId, (jint)len);
	}
	catch(...) {}
	*/
}
// -----------------------------
// JNI Entries
// -----------------------------

/*
* Class:     NestedClient
* Method:    _nlog
* Signature: (ILjava/lang/String;)V
*/
JNIEXPORT void JNICALL Java_com_xormedia_aqua_sdk_NestedClient__1nlog
(JNIEnv *, jclass, jint, jstring)
{
}


static jfieldID fIdNativeClient = NULL;
static jfieldID fIdNativeId = NULL;

#define jlog       (*XOR_Media::AquaClient::pLogger)
#define INVALID_SDK (NULL == XOR_Media::AquaClient::pLogger || NULL == XOR_Media::AquaClient::pThrdPool)

using namespace XOR_Media::AquaClient;
/*
* Class:     NestedClient
* Method:    _init
* Signature: (Ljava/lang/String;)Z
*/
JNIEXPORT jboolean JNICALL Java_com_xormedia_aqua_sdk_NestedClient__1initSDK
(JNIEnv *jenv, jclass jcls, jstring jjsonConfig)
{
	fIdNativeClient = jenv->GetFieldID(jcls, "_nativeClient", "Ljava/lang/Object;");
	fIdNativeId     = jenv->GetFieldID(jcls, "_nativeId",     "Ljava/lang/String;");

	try
	{	
		std::string strConfig = readJstring(jenv, jjsonConfig);
		return AquaClient::initSDK(strConfig) ? JNI_TRUE : JNI_FALSE;
	}
	catch(std::exception&)
	{
	}
	catch (...)
	{
	}

	return JNI_FALSE;
}

/*
* Class:     NestedClient
* Method:    _uninit
* Signature: ()V
*/
JNIEXPORT void JNICALL Java_com_xormedia_aqua_sdk_NestedClient__1uninitSDK(JNIEnv *, jclass)
{
	AquaClient::uninitSDK();
}

#define JCLIENTFMT(_X) CLOGFMT(NestedClient, "client[%p] " _X), pClient
/*
* Class:     NestedClient
* Method:    _create
* Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;
*/
JNIEXPORT jobject JNICALL Java_com_xormedia_aqua_sdk_NestedClient__1create
(JNIEnv * jenv, jobject jobj, jstring jRootURL, jstring jUserDomain, jstring jHomeContainer, jstring jProps)
{
	if (INVALID_SDK)
		return NULL;

	std::string rootUrl       = readJstring(jenv, jRootURL);
	std::string userDomain    = readJstring(jenv, jUserDomain);
	std::string homeContainer = readJstring(jenv, jHomeContainer);
	std::string strProps      = readJstring(jenv, jProps);

	AquaClient* pClient = AquaClient::newClient(rootUrl, userDomain, homeContainer, strProps);

	char idBuf[sizeof(char*)*2+8]="";
	snprintf(idBuf, sizeof(idBuf)-2, "%p", pClient);
	jstring jId = newJstring(jenv, idBuf);
	jenv->SetObjectField(jobj, fIdNativeId, jId);

	return (jobject) pClient;
}

static AquaClientImpl* getNativeClientOfJobj(JNIEnv* jenv, jobject jobj)
{
	if (NULL == jenv || NULL == fIdNativeId || NULL == jobj)
		return NULL;

	jstring jId = (jstring) jenv->GetObjectField(jobj, fIdNativeId);
	std::string nativeId = readJstring(jenv, jId);
	AquaClientImpl* pclient = NULL;
	sscanf(nativeId.c_str(), "%p", &pclient);

	return pclient;
}

/*
* Class:     NestedClient
* Method:    _destroy
* Signature: (LNestedClient;)V
*/
JNIEXPORT void JNICALL Java_com_xormedia_aqua_sdk_NestedClient__1destroy(JNIEnv * jenv, jobject jobj)
{
	AquaClient* pClient = getNativeClientOfJobj(jenv, jobj);
	if (NULL == pClient)
		return;

	delete pClient;
}

/*
* Class:     NestedClient
* Method:    _exec_Cdmi
* Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/StringBuffer;)I
*/
JNIEXPORT jint JNICALL Java_com_xormedia_aqua_sdk_NestedClient__1exec_1Cdmi(JNIEnv* jenv, jobject jobj, jstring jcmd, jstring juri, jstring jargs, jobject jresult)
{
	AquaClientImpl* pClient = getNativeClientOfJobj(jenv, jobj);
	if (NULL == pClient)
		return CdmiFuseOps::cdmirc_SDK_BadClient;

	std::string cmd  = readJstring(jenv, jcmd);
	std::string uri  = readJstring(jenv, juri);
	std::string args = readJstring(jenv, jargs);
	std::string strResult;

	Json::Value jsonArgs, jsonResult;
	if (!args.empty() && !Json::Reader().parse(args, jsonArgs))
	{
		jlog(ZQ::common::Log::L_ERROR, JCLIENTFMT("cdmi cmd[%s] uri[%s] bad args: %s"), cmd.c_str(), uri.c_str(), args.c_str());
		return CdmiFuseOps::cdmirc_SDK_BadArgument;
	}

	CdmiFuseOps::CdmiRetCode ret = CdmiFuseOps::cdmirc_SDK_Unsupported;
	ret = pClient->exec_Cdmi(cmd, uri, jsonArgs, jsonResult);

	strResult = Json::FastWriter().write(jsonResult);
	fillStringBuffer(jenv, jresult, strResult);

	return ret;
}

/*
* Class:     NestedClient
* Method:    _exec_nonCdmi
* Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/StringBuffer;[BI)I
*/
JNIEXPORT jint JNICALL Java_com_xormedia_aqua_sdk_NestedClient__1exec_1nonCdmi(JNIEnv* jenv, jobject jobj, jstring jcmd, jstring juri, jstring jargs, jobject jresult, jobject jbuffer)
{
	AquaClientImpl* pClient = getNativeClientOfJobj(jenv, jobj);
	if (NULL == pClient)
		return CdmiFuseOps::cdmirc_SDK_BadClient;

	std::string cmd  = readJstring(jenv, jcmd);
	std::string uri  = readJstring(jenv, juri);
	std::string args = readJstring(jenv, jargs);
	std::string strResult;

	Json::Value jsonArgs, jsonResult;
	if (!args.empty() && !Json::Reader().parse(args, jsonArgs))
	{
		jlog(ZQ::common::Log::L_ERROR, JCLIENTFMT("non-cdmi cmd[%s] uri[%s] bad args: %s"), cmd.c_str(), uri.c_str(), args.c_str());
		return CdmiFuseOps::cdmirc_SDK_BadArgument;
	}

	// about the buffer len
	jlong jlen =0, jcap =0;
	jbyte* jbuf = getJBuffer(jenv, jbuffer, jlen, jcap);
	if (jlen <=0 || NULL == jbuf)
	{
		jbuf = NULL;
		jlen =0;
	}

	uint32 len = (uint32)jlen;

	bool isRecvBuf = false;
	if (0 == cmd.compare("ReadDataObject"))
		isRecvBuf = true;
	
	if (isRecvBuf)
		len = (uint32)jcap;

	CdmiFuseOps::CdmiRetCode ret = CdmiFuseOps::cdmirc_SDK_Unsupported;
	ret = pClient->exec_nonCdmi(cmd, uri, jsonArgs, jsonResult, (char*)jbuf, len);

	if (isRecvBuf)
		limitJBuffer(jenv, jbuffer, len);

	strResult = Json::FastWriter().write(jsonResult);
	fillStringBuffer(jenv, jresult, strResult);
	return ret;
}

