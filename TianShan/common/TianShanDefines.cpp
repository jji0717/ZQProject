#include "ZQ_common_conf.h"
#include "Exception.h"
#include "Locks.h"
#include "TianShanDefines.h"
#include "TianShanIce.h"
#include "Log.h"
#include "TimeUtil.h"
#include <set>
#include "FileSystemOp.h"

extern "C"
{
#include <stdio.h>

#ifdef ZQ_OS_MSWIN
	#include <io.h>
#else
	#include <sys/stat.h>
#endif
};

using namespace ::TianShanIce::SRM;
using namespace ::TianShanIce;

namespace ZQTianShan {

void printLine(const char* line)
{
	if (line)
#ifdef glog
		glog(::ZQ::common::Log::L_DEBUG, "%s", line);
#else
		printf("%s\n", line);
#endif // glog
}

#define SWITCH_EX(_CASE, _STR)	case _CASE: return #_STR;
#define SWITCH_CASE_STATE(_ST, _STR)	SWITCH_EX(/*::TianShanIce::##*/_ST, _STR)
const char* ObjStateStr(const ::TianShanIce::State state)
{
	switch(state)
	{
		SWITCH_CASE_STATE(stNotProvisioned, NotProvisioned);
		SWITCH_CASE_STATE(stProvisioned,	Provisioned);
		SWITCH_CASE_STATE(stInService,      InService);
		SWITCH_CASE_STATE(stOutOfService,   OutOfService);
	default:
		return "<Unknown>";
	}
}
#undef SWITCH_CASE_STATE

::TianShanIce::State StrToStateId(const char* stateStr)
{
#ifdef ZQ_OS_MSWIN
#  define SWITCH_CASE_STATE(_ST)	else if (0 == stricmp(#_ST, stateStr)) return ::TianShanIce::st##_ST
#else
#  define SWITCH_CASE_STATE(_ST)	else if (0 == strcasecmp(#_ST, stateStr)) return ::TianShanIce::st##_ST
#endif

	if (0);
	SWITCH_CASE_STATE(NotProvisioned);
	SWITCH_CASE_STATE(Provisioned);
	SWITCH_CASE_STATE(InService);
	SWITCH_CASE_STATE(OutOfService);

	return ::TianShanIce::stNotProvisioned;

#undef SWITCH_CASE_STATE
}



#define SWITCH_CASE_RESTYPE(_RT)	SWITCH_EX(/*::TianShanIce::SRM::##*/_RT, _RT)
const char* ResourceTypeStr(const ::TianShanIce::SRM::ResourceType& type)
{
	switch(type) 
	{
	    SWITCH_CASE_RESTYPE(rtURI);
		SWITCH_CASE_RESTYPE(rtStorage);
		SWITCH_CASE_RESTYPE(rtStreamer);
		SWITCH_CASE_RESTYPE(rtServiceGroup);
		SWITCH_CASE_RESTYPE(rtMpegProgram);
		SWITCH_CASE_RESTYPE(rtTsDownstreamBandwidth);
		SWITCH_CASE_RESTYPE(rtIP);
		SWITCH_CASE_RESTYPE(rtEthernetInterface);
		SWITCH_CASE_RESTYPE(rtPhysicalChannel);
		SWITCH_CASE_RESTYPE(rtAtscModulationMode);
		SWITCH_CASE_RESTYPE(rtHeadendId);
		SWITCH_CASE_RESTYPE(rtClientConditionalAccess);
		SWITCH_CASE_RESTYPE(rtServerConditionalAccess);
	default:
		return "Unkown";
	}
}
#undef SWITCH_CASE_RESTYPE

#define SWITCH_CASE_RES_ST(_RS)	SWITCH_EX(/*::TianShanIce::SRM::##*/_RS, _RS)
const char* ResourceStatusStr(const ::TianShanIce::SRM::ResourceStatus status)
{
	switch(status)
	{
		SWITCH_CASE_RES_ST(rsRequested);
		SWITCH_CASE_RES_ST(rsInProgress);
		SWITCH_CASE_RES_ST(rsAlternateAssigned);
		SWITCH_CASE_RES_ST(rsFailed);
		SWITCH_CASE_RES_ST(rsUnprocessed);
		SWITCH_CASE_RES_ST(rsInvalid);
		SWITCH_CASE_RES_ST(rsReleased);
	default:
		return "<Unknown>";
	}
}

const char* ResourceAttrStr(const ::TianShanIce::SRM::ResourceAttribute attr)
{
	switch(attr)
	{
		SWITCH_CASE_RES_ST(raMandatoryNonNegotiable);
		SWITCH_CASE_RES_ST(raMandatoryNegotiable);
		SWITCH_CASE_RES_ST(raNonMandatoryNonNegotiable);
		SWITCH_CASE_RES_ST(raNonMandatoryNegotiable);
	default:
		return "<Unknown>";
	}
}
#undef SWITCH_CASE_RES_ST
#undef SWITCH_EX

void dumpResourceMap(const ::TianShanIce::SRM::ResourceMap& resources, const char* linePreffix, fDumpLine dumpLine, void* pCtx)
{
	if (NULL == dumpLine)
		return;

	char buf[DumpPrefixBufSize];
	sprintf(buf, "%s", (linePreffix?linePreffix:""));
	char *p = buf + strlen(buf);

	sprintf(p, ">> begin of resource map:");
	dumpLine(buf, pCtx);

	for (::TianShanIce::SRM::ResourceMap::const_iterator it = resources.begin(); it != resources.end(); it++)
	{
		switch(it->first)
		{
#define SWITCH_CASE_RES(_R) case ::TianShanIce::SRM::_R: sprintf(p, #_R ": "); break;
			SWITCH_CASE_RES(rtURI);
			SWITCH_CASE_RES(rtStorage);
			SWITCH_CASE_RES(rtStreamer);
			SWITCH_CASE_RES(rtServiceGroup);
			SWITCH_CASE_RES(rtMpegProgram);
			SWITCH_CASE_RES(rtTsDownstreamBandwidth);
			SWITCH_CASE_RES(rtIP);
			SWITCH_CASE_RES(rtEthernetInterface);
			SWITCH_CASE_RES(rtPhysicalChannel);
			SWITCH_CASE_RES(rtHeadendId);
			SWITCH_CASE_RES(rtClientConditionalAccess);
			SWITCH_CASE_RES(rtServerConditionalAccess);
			SWITCH_CASE_RES(rtAtscModulationMode);
			SWITCH_CASE_RES(rtContentProvision)
#undef SWITCH_CASE_RES			
		default:
			sprintf(p, "<UnknownResource>: "); break;
		}
		dumpResource(it->second, buf, dumpLine, pCtx);
	}

	sprintf(p, "<< end of resource map.");
	dumpLine(buf, pCtx);
}

void dumpResource(const ::TianShanIce::SRM::Resource& res, const char* linePreffix, fDumpLine dumpLine, void* pCtx)
{
	if (NULL == dumpLine)
		return;

	char buf[DumpPrefixBufSize] = "", *p =buf;
	if (linePreffix)
	{
		sprintf(p, "%s", linePreffix);
		p += strlen(p);
	}

	sprintf(p, "status(%s); attr(%s) data:", ResourceStatusStr(res.status), ResourceAttrStr(res.attr));
	dumpLine(buf, pCtx);
	*p='\0';
	dumpValueMap(res.resourceData, buf, dumpLine, pCtx);
}

void dumpValueMap(const ::TianShanIce::ValueMap& valmap, const char* linePreffix, fDumpLine dumpLine, void* pCtx)
{
	if (NULL == dumpLine)
		return;

	char buf[DumpPrefixBufSize] = {0}, *p =buf;
	int nBufSize = sizeof(buf) - 1;
	int nSizeLeft = nBufSize;
	if (linePreffix)
	{
		snprintf(p, nSizeLeft, "%s", linePreffix);
		int nLen = strlen(p);
		p += nLen;
		nSizeLeft -= nLen;
	}

	for (::TianShanIce::ValueMap::const_iterator it = valmap.begin(); it != valmap.end(); it ++)
	{
		snprintf(p, nSizeLeft, "%s: ", it->first.c_str());
		dumpVariant(it->second, buf, dumpLine, pCtx);
	}
}

void dumpVariant(const ::TianShanIce::Variant& val, const char* linePreffix, fDumpLine dumpLine, void* pCtx)
{
	if (NULL == dumpLine)
		return;

	char buf[4096] = {0}, *p =buf;
	int nBufSize = sizeof(buf) - 1;
	int nSizeLeft = nBufSize;
	size_t nLen;

	if (linePreffix)
	{
		snprintf(p, nSizeLeft, "%s", linePreffix);
		nLen = strlen(p);
		p += nLen;
		nSizeLeft -= nLen;
	}

	snprintf(p, nSizeLeft, "{(%c) ", (val.bRange ? 'R' : 'E'));
	nLen = strlen(p);
	p += nLen;
	nSizeLeft -= nLen;	

	switch (val.type)
	{
	case ::TianShanIce::vtInts:
		{
			snprintf(p, nSizeLeft, "I: "); 
			nLen = strlen(p);
			p += nLen;
			nSizeLeft -= nLen;	

			for (::TianShanIce::IValues::const_iterator it = val.ints.begin(); it < val.ints.end(); it++)
			{
				snprintf(p, nSizeLeft, " %d; ", *it);
				nLen = strlen(p);
				p += nLen;
				nSizeLeft -= nLen;	
			}
		}
		break;

	case ::TianShanIce::vtLongs:
		{
			snprintf(p, nSizeLeft, "L: ");
			nLen = strlen(p);
			p += nLen;
			nSizeLeft -= nLen;	

			for (::TianShanIce::LValues::const_iterator it = val.lints.begin(); it < val.lints.end(); it++)
			{
				snprintf(p, nSizeLeft, " "FMT64"; ", *it);
				nLen = strlen(p);
				p += nLen;
				nSizeLeft -= nLen;	
			}
		}
		break;

	case ::TianShanIce::vtStrings:
		{
			snprintf(p, nSizeLeft, "S: "); 
			nLen = strlen(p);
			p += nLen;
			nSizeLeft -= nLen;	

			for (::TianShanIce::StrValues::const_iterator it = val.strs.begin(); it < val.strs.end(); it++)
			{
//				snprintf(p, nSizeLeft, " \"%s\"; ", *it); 
				snprintf(p, nSizeLeft, " \"%s\"; ", (*it).c_str());
				nLen = strlen(p);
				p += nLen;
				nSizeLeft -= nLen;	
			}
		}
		break;

	case ::TianShanIce::vtBin:
		{
			snprintf(p, nSizeLeft, "B: ");
			nLen = strlen(p);
			p += nLen;
			nSizeLeft -= nLen;	

			for (::TianShanIce::BValues::const_iterator it = val.bin.begin(); it < val.bin.end(); it++)
			{
				snprintf(p, nSizeLeft, " %02x ", *it); 
				nLen = strlen(p);
				p += nLen;
				nSizeLeft -= nLen;	
			}
		}
		break;
		
	default:
		break;
	}
	snprintf(p, nSizeLeft, "}");

	dumpLine(buf, pCtx);
}

::TianShanIce::Variant& PDField(::TianShanIce::ValueMap& PD, const char* field)
{
	if (NULL == field || *field ==0x00)
		_IceThrow<TianShanIce::InvalidParameter> (EXPFMT(ZQTianShanUtil, 300, "NULL field to access private data"));

	::TianShanIce::ValueMap::iterator it = PD.find(field);
	if (PD.end() == it)
		_IceThrow<TianShanIce::InvalidParameter> (EXPFMT(ZQTianShanUtil, 301, "private data field \"%s\" is not found"), field);
	
	return it->second;
}

typedef ::std::vector<Ice::Byte> ByteSet;
typedef ::std::set<Ice::Long> LongSet;
typedef ::std::set<Ice::Int>  IntSet;
typedef ::std::vector<std::string> StrSet;
typedef ::std::set<Ice::Float> FloatSet;

bool IntersectVariant(const ::TianShanIce::Variant& v1, const ::TianShanIce::Variant& v2, ::TianShanIce::Variant& result, bool& isNarrowed)
{
	// validate if type matches
	if (v1.type != v2.type)
		return false;

	result.type = v1.type;
	isNarrowed = false;

#if defined(ZQ_OS_MSWIN) && defined(_MSC_VER)

#define ITEMDIFF_BY_METHOD(_COMP) item1.## _COMP##(item2)
#define ITEMDIFF_BY_OP() (item1 - item2)
#define TEST_NARROWED(_VAR, _FIELD, _FIELDTYPE, _DIFF) { isNarrowed = false; if (_VAR._FIELD.size() ==1) isNarrowed = true; \
	else if (_VAR.bRange && _VAR._FIELD.size() >=2 ) {_FIELDTYPE##::value_type& item1 = _VAR._FIELD[0]; _FIELDTYPE##::value_type& item2 = _VAR._FIELD[0]; isNarrowed = (0== _DIFF); } }

	// test if one is empty
#define COPY_IF_EMPTY(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF) result.##_FIELD##.clear(); if (_V1.##_FIELD##.size() <=0) { result.##_FIELD = _V2.##_FIELD; result.bRange = _V2.bRange;	TEST_NARROWED(result, _FIELD, _FIELDTYPE, _DIFF); return true; } \
	else if (_V2.##_FIELD##.size() <=0) {	result.##_FIELD = _V1.##_FIELD;	result.bRange = _V1.bRange;	 TEST_NARROWED(result, _FIELD, _FIELDTYPE, _DIFF); return true; }

#define INIT_VARS(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF)	_FIELDTYPE val1 = _V1.##_FIELD; _FIELDTYPE val2 = v2.##_FIELD; \
	if (v1.bRange && val1.size() <2 || v2.bRange && val2.size() <2) \
	{ TEST_NARROWED(result, _FIELD, _FIELDTYPE, _DIFF); return true; } // no intersection at all, return result as empty
	
#define PROCESS_BOTH_RANGE(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF) 	::std::sort(val1.begin(), val1.end()); ::std::sort(val2.begin(), val2.end()); \
	if (v1.bRange && v2.bRange) \
	{	result.bRange = true; \
	{_FIELDTYPE##::value_type& item1 = val1[0]; _FIELDTYPE##::value_type& item2 = val2[0]; result.##_FIELD##.push_back(_DIFF >0 ? item1 : item2); } \
	{_FIELDTYPE##::value_type& item1 = val1[1]; _FIELDTYPE##::value_type& item2 = val2[1]; result.##_FIELD##.push_back(_DIFF <0 ? item1 : item2); } \
	_FIELDTYPE##::value_type& item1 = result.##_FIELD[0]; _FIELDTYPE##::value_type& item2 = result.##_FIELD[1]; \
	if (_DIFF >0)	result.##_FIELD##.clear(); \
	TEST_NARROWED(result, _FIELD, _FIELDTYPE, _DIFF); return true; }
	
#define PROCESS_SINGE_RANGE(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF) if (_V1.bRange || _V2.bRange) \
	{ if (v2.bRange) std::swap<_FIELDTYPE >(val1, val2); _FIELDTYPE##::iterator it; \
	{ _FIELDTYPE##::value_type& item2 = val1[0]; for (it = val2.begin(); it < val2.end(); it++) { _FIELDTYPE##::value_type& item1 = *it; if (_DIFF >=0) break; } } \
	{ _FIELDTYPE##::value_type& item2 = val1[1]; for (; it < val2.end(); it++) { _FIELDTYPE##::value_type& item1 = *it; if (_DIFF <=0) result.##_FIELD##.push_back(item1); } } \
	TEST_NARROWED(result, _FIELD, _FIELDTYPE, _DIFF); return true; }
	
#define PROCESS_BOTH_ENUM(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF) \
	for (_FIELDTYPE##::iterator it1 = val1.begin(), it2 = val2.begin();	it1 < val1.end() && it2 < val2.end(); ) \
	{ _FIELDTYPE##::value_type& item1 = *it1; _FIELDTYPE##::value_type& item2 = *it2; int comp = _DIFF; if (comp <0) {	it1++;	continue; } if (comp >0) { it2++; continue;	} result.##_FIELD##.push_back(*it1++); it2++; } \
	TEST_NARROWED(result, _FIELD, _FIELDTYPE, _DIFF);

#define PROCESS_BOTH_ENUM2(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF) \
	for (_FIELDTYPE##::iterator it1 = val1.begin(), it2 = val2.begin();	it1 < val1.end() && it2 < val2.end(); ) \
	{ _FIELDTYPE##::value_type& item1 = *it1; _FIELDTYPE##::value_type& item2 = *it2; _FIELDTYPE##::value_type comp = _DIFF; if (comp <0) {	it1++;	continue; } if (comp >0) { it2++; continue;	} result.##_FIELD##.push_back(*it1++); it2++; } \
	TEST_NARROWED(result, _FIELD, _FIELDTYPE, _DIFF);

#else//non-ZQ_OS_MSWIN
/**************************** is same as ZQ_OS_MSWIN except delete "##" ***********************************/
#define ITEMDIFF_BY_METHOD(_COMP) item1. _COMP(item2)
#define ITEMDIFF_BY_OP() (item1 - item2)
#define TEST_NARROWED(_VAR, _FIELD, _FIELDTYPE, _DIFF) { isNarrowed = false; if (_VAR._FIELD.size() ==1) isNarrowed = true; \
	else if (_VAR.bRange && _VAR._FIELD.size() >=2 ) {_FIELDTYPE::value_type& item1 = _VAR._FIELD[0]; _FIELDTYPE::value_type& item2 = _VAR._FIELD[0]; isNarrowed = (0== _DIFF); } }

	// test if one is empty
#define COPY_IF_EMPTY(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF) result._FIELD.clear(); if (_V1._FIELD.size() <=0) { result._FIELD = _V2._FIELD; result.bRange = _V2.bRange;	TEST_NARROWED(result, _FIELD, _FIELDTYPE, _DIFF); return true; } \
	else if (_V2._FIELD.size() <=0) {	result._FIELD = _V1._FIELD;	result.bRange = _V1.bRange;	 TEST_NARROWED(result, _FIELD, _FIELDTYPE, _DIFF); return true; }

#define INIT_VARS(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF)	_FIELDTYPE val1 = _V1._FIELD; _FIELDTYPE val2 = v2._FIELD; \
	if (v1.bRange && val1.size() <2 || v2.bRange && val2.size() <2) \
	{ TEST_NARROWED(result, _FIELD, _FIELDTYPE, _DIFF); return true; } // no intersection at all, return result as empty
	
#define PROCESS_BOTH_RANGE(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF) 	::std::sort(val1.begin(), val1.end()); ::std::sort(val2.begin(), val2.end()); \
	if (v1.bRange && v2.bRange) \
	{	result.bRange = true; \
	{_FIELDTYPE::value_type& item1 = val1[0]; _FIELDTYPE::value_type& item2 = val2[0]; result._FIELD.push_back(_DIFF >0 ? item1 : item2); } \
	{_FIELDTYPE::value_type& item1 = val1[1]; _FIELDTYPE::value_type& item2 = val2[1]; result._FIELD.push_back(_DIFF <0 ? item1 : item2); } \
	_FIELDTYPE::value_type& item1 = result._FIELD[0]; _FIELDTYPE::value_type& item2 = result._FIELD[1]; \
	if (_DIFF >0)	result._FIELD.clear(); \
	TEST_NARROWED(result, _FIELD, _FIELDTYPE, _DIFF); return true; }
	
#define PROCESS_SINGE_RANGE(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF) if (_V1.bRange || _V2.bRange) \
	{ if (v2.bRange) std::swap<_FIELDTYPE >(val1, val2); _FIELDTYPE::iterator it; \
	{ _FIELDTYPE::value_type& item2 = val1[0]; for (it = val2.begin(); it < val2.end(); it++) { _FIELDTYPE::value_type& item1 = *it; if (_DIFF >=0) break; } } \
	{ _FIELDTYPE::value_type& item2 = val1[1]; for (; it < val2.end(); it++) { _FIELDTYPE::value_type& item1 = *it; if (_DIFF <=0) result._FIELD.push_back(item1); } } \
	TEST_NARROWED(result, _FIELD, _FIELDTYPE, _DIFF); return true; }
	
#define PROCESS_BOTH_ENUM(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF) \
	for (_FIELDTYPE::iterator it1 = val1.begin(), it2 = val2.begin();	it1 < val1.end() && it2 < val2.end(); ) \
	{ _FIELDTYPE::value_type& item1 = *it1; _FIELDTYPE::value_type& item2 = *it2; int comp = _DIFF; if (comp <0) {	it1++;	continue; } if (comp >0) { it2++; continue;	} result._FIELD.push_back(*it1++); it2++; } \
	TEST_NARROWED(result, _FIELD, _FIELDTYPE, _DIFF);

#define PROCESS_BOTH_ENUM2(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF) \
	for (_FIELDTYPE::iterator it1 = val1.begin(), it2 = val2.begin();	it1 < val1.end() && it2 < val2.end(); ) \
	{ _FIELDTYPE::value_type& item1 = *it1; _FIELDTYPE::value_type& item2 = *it2; _FIELDTYPE::value_type comp = _DIFF; if (comp <0) {	it1++;	continue; } if (comp >0) { it2++; continue;	} result._FIELD.push_back(*it1++); it2++; } \
	TEST_NARROWED(result, _FIELD, _FIELDTYPE, _DIFF);

#endif

	
#define PROCESS_INTERSECTION(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF) \
	{ COPY_IF_EMPTY(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF); \
	  INIT_VARS(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF); \
	PROCESS_BOTH_RANGE(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF); \
	result.bRange = false; \
	PROCESS_SINGE_RANGE(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF); \
	PROCESS_BOTH_ENUM(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF); }

#define PROCESS_INTERSECTION2(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF) \
	{ COPY_IF_EMPTY(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF); \
	INIT_VARS(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF); \
	PROCESS_BOTH_RANGE(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF); \
	result.bRange = false; \
	PROCESS_SINGE_RANGE(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF); \
	PROCESS_BOTH_ENUM2(_V1, _V2, _FIELD, _FIELDTYPE, _DIFF); }

	switch(result.type)
	{
	case ::TianShanIce::vtBin:
		PROCESS_INTERSECTION(v1, v2, bin, TianShanIce::BValues, ITEMDIFF_BY_OP());
		break;

	case ::TianShanIce::vtInts:
		PROCESS_INTERSECTION(v1, v2, ints, TianShanIce::IValues, ITEMDIFF_BY_OP());
		break;

	case ::TianShanIce::vtLongs:
		PROCESS_INTERSECTION2(v1, v2, lints, TianShanIce::LValues, ITEMDIFF_BY_OP());
		break;

	case ::TianShanIce::vtStrings:
		PROCESS_INTERSECTION(v1, v2, strs, TianShanIce::StrValues, ITEMDIFF_BY_METHOD(compare));
		break;

	case ::TianShanIce::vtFloats:
		PROCESS_INTERSECTION2(v1, v2, floats, TianShanIce::FValues, ITEMDIFF_BY_OP());
		break;

	default:
		return false;
	}

#undef PROCESS_INTERSECTION
#undef PROCESS_INTERSECTION2
#undef COPY_IF_EMPTY
#undef INIT_VARS
#undef PROCESS_BOTH_RANGE
#undef PROCESS_SINGE_RANGE
#undef PROCESS_BOTH_ENUM
#undef PROCESS_BOTH_ENUM2

	return true;
}

bool InterRestrictResource(const ::TianShanIce::SRM::Resource& res1, const ::TianShanIce::SRM::Resource& res2, ::TianShanIce::SRM::Resource& result)
{
	result.resourceData.clear();
	::TianShanIce::StrValues keysNeedToIntersection;

	::TianShanIce::ValueMap::const_iterator itRes1Data = res1.resourceData.begin();
	::TianShanIce::ValueMap::const_iterator itRes2Data = res2.resourceData.begin();

	while (itRes1Data != res1.resourceData.end() && itRes2Data != res2.resourceData.end())
	{
		int comp = itRes1Data->first.compare(itRes2Data->first);
		if (comp <0)
		{
			// only exists in res1, simply add into the result
			result.resourceData.insert(*itRes1Data++);
			continue;
		}

		if (comp ==0)
		{
			// exists in both, remember in keysNeedToIntersection
			keysNeedToIntersection.push_back(itRes1Data->first);
			itRes1Data++; itRes2Data++;
			continue;
		}

		// only exists in res2, simply add into the result
		result.resourceData.insert(*itRes2Data++);
	}

	while (itRes1Data != res1.resourceData.end())
	{
		// only exists in res1, simply add into the result
		result.resourceData.insert(*itRes1Data++);
	}

	while (itRes2Data != res2.resourceData.end())
	{
		// only exists in res1, simply add into the result
		result.resourceData.insert(*itRes2Data++);
	}

	bool bMandatory = !((::TianShanIce::SRM::raNonMandatoryNonNegotiable == res1.attr) && (::TianShanIce::SRM::raNonMandatoryNegotiable == res1.attr)
					&& (::TianShanIce::SRM::raNonMandatoryNegotiable == res2.attr) && (::TianShanIce::SRM::raNonMandatoryNegotiable == res2.attr));

	bool bNegotiable = !((::TianShanIce::SRM::raMandatoryNonNegotiable == res1.attr) || (::TianShanIce::SRM::raNonMandatoryNonNegotiable == res1.attr)
					|| (::TianShanIce::SRM::raMandatoryNonNegotiable == res2.attr) || (::TianShanIce::SRM::raNonMandatoryNonNegotiable == res2.attr));

	// now start merge those exists in both
	for (::TianShanIce::StrValues::iterator it = keysNeedToIntersection.begin(); it < keysNeedToIntersection.end(); it++)
	{
		std::string key = *it;
		::TianShanIce::Variant resultVal;
		bool isNarrowed;
		if (!IntersectVariant(res1.resourceData.find(key)->second, res2.resourceData.find(key)->second, resultVal, isNarrowed))
			return false;

		bNegotiable = bNegotiable && !isNarrowed; 
		
		result.resourceData.insert(::TianShanIce::ValueMap::value_type(key, resultVal));
	}

	switch((bMandatory?1:0) <<1 | (bNegotiable?1:0))
	{
	case 0: result.attr = ::TianShanIce::SRM::raNonMandatoryNonNegotiable; break;
	case 1: result.attr = ::TianShanIce::SRM::raNonMandatoryNegotiable; break;
	case 2: result.attr = ::TianShanIce::SRM::raMandatoryNonNegotiable; break;
	case 3: result.attr = ::TianShanIce::SRM::raMandatoryNegotiable; break;
	}

	return true;
}

::Ice::Long now()
{
    return ZQ::common::now();
}

// return the UTC formatted time string
const char* TimeToUTC(::Ice::Long time, char* buf, const int maxlen , bool bLocalZone )
{
    return ZQ::common::TimeUtil::TimeToUTC(time, buf, maxlen, bLocalZone);
}

/// convert UTC string format to SYSTEMTIME format
::Ice::Long ISO8601ToTime(const char* ISO8601Str)
{
    return ZQ::common::TimeUtil::ISO8601ToTime(ISO8601Str);
}

/// validate request speed
bool isDoSAttack(::Ice::Long& timeEdge, int& reqCount, int windowSize, int maxRequests)
{
	::Ice::Long stampPrev = timeEdge;
	if (windowSize <=0 || maxRequests <=0)
	{
		// unlimited
		reqCount = 0;
		return true;
	}

	if (++reqCount <0)
		reqCount =0;

	if (reqCount > maxRequests *2)
	{
		reqCount = maxRequests +1;
		return true;
	}
	
	timeEdge = now();
	if (stampPrev <=0)
		return true;

	int step =(int) (timeEdge - stampPrev) * maxRequests / windowSize / 1000;
	reqCount -= step;

	return (reqCount <= maxRequests);
}

ZQ::common::Mutex _pathLocker;
static std::string _programPath;
static std::string _programRoot;
static std::string _modulesPath;

const char* getProgramPath()
{
	if (!_programPath.empty())
		return _programPath.c_str();

	ZQ::common::MutexGuard g(_pathLocker);
	if (!_programPath.empty())
		return _programPath.c_str();
	
	char path[MAX_PATH];
#ifdef ZQ_OS_MSWIN
	if (::GetModuleFileName(NULL, path, MAX_PATH-2) >0)
	{
		char* p = strrchr(path, FNSEPC);
		if (NULL !=p)
		{
			*p='\0';
			p = strrchr(path, FNSEPC);
			if (NULL !=p && (0==stricmp(FNSEPS "bin", p) || 0==stricmp(FNSEPS "exe", p)))
			{
				*p='\0';
				_programRoot = path;
				_modulesPath = _programRoot + FNSEPS "modules";
			}
			else if (NULL !=p && (0==stricmp(FNSEPS "bin64", p) || 0==stricmp(FNSEPS "exe64", p)))
			{
				*p='\0';
				_programRoot = path;
				_modulesPath = _programRoot + FNSEPS "modules64";
			}
		}
        else
        {
            _programRoot = _modulesPath = path;
        }
	}
	else _programRoot = _modulesPath = "." ;

#else
	int res = readlink("/proc/self/exe", path, sizeof(path));
	if(res < 0 || res > (int)sizeof(path))
	{
		_programRoot = _modulesPath = "." ;
	}
	else
	{
		char* p = strrchr(path, FNSEPC);
		if (NULL !=p)
		{
			*p='\0';
			p = strrchr(path, FNSEPC);
			if (NULL !=p && (0==strcasecmp(FNSEPS "bin", p) || 0==strcasecmp(FNSEPS "exe", p)))
			{
				*p='\0';
				_programRoot = path;
				_modulesPath = _programRoot + FNSEPS "modules";
			}
			else if (NULL !=p && (0==strcasecmp(FNSEPS "bin64", p) || 0==strcasecmp(FNSEPS "exe64", p)))
			{
				*p='\0';
				_programRoot = path;
				_modulesPath = _programRoot + FNSEPS "modules64";
			}
		}
        else
        {
            _programRoot = _modulesPath = path;
        }		
	}
#endif

	return _programPath.c_str();
}

const char* getProgramRoot()
{
	if (_programRoot.empty())
		getProgramPath();

	return _programRoot.c_str();
}

const char* getModulesPath()
{
	if (_modulesPath.empty())
		getProgramPath();

	return _modulesPath.c_str();
}

std::string IceCurrentToStr(const ::Ice::Current& c)
{
	::Ice::Context::const_iterator it = c.ctx.find("signature");
	if (c.ctx.end() != it)
		return it->second;

	std::string temp = "";
	if(!c.con || c.requestId <=0)
		return temp; // empty string

	temp = c.con->toString();
	std::replace(temp.begin(), temp.end(), '\n', ';');
//	std::replace(temp.begin(), temp.end(), '\r', '');
	std::replace(temp.begin(), temp.end(), '\t', ' ');

	char result[256] = "\0";
	snprintf(result, sizeof(result)-1, "%s(): %s; requestId=%d", c.operation.c_str(), temp.c_str(), c.requestId);
	return std::string(result);
}

/// create DB folder and prepare the DB_CONFIG
#define CACHE_SEG_SIZE (64*1024*1024) // 64MB per cache segement
bool createDBFolderEx(ZQ::common::Log& log, const std::string& path,
							 const long cacheSizeKB, const long maxLocks, const long maxObjects, const long maxLockers)
{
	if (!path.empty())
		FS::createDirectory(path, true);
	
	// preparing the DB_CONFIG file
	std::string dbConfFile = path + "DB_CONFIG";
	uint64 cacheSize = (cacheSizeKB > 1024*1023) ? 1024*1023 : cacheSizeKB;
	cacheSize *= 1024;
	uint segments = (uint)(cacheSize / CACHE_SEG_SIZE);

	if ( -1 == access(dbConfFile.c_str(), 0))
	{
		log(ZQ::common::Log::L_DEBUG, CLOGFMT(createDBFolder, "initializing %s"), dbConfFile.c_str());
		FILE* f = ::fopen(dbConfFile.c_str(), "w+");
		if (NULL != f)
		{
			::fprintf(f, "set_lk_max_locks %ld\n",   maxLocks);
			::fprintf(f, "set_lk_max_objects %ld\n", maxObjects);
			::fprintf(f, "set_lk_max_lockers %ld\n", maxLockers);
			::fprintf(f, "set_cachesize 0 %lld %d\n", cacheSize, segments);
			::fclose(f);
		}
	}

	return true;
}

std::string createDBFolder(ZQ::common::Log& log, const char* serviceName, const char* dataRoot, const char* databaseName,
		    const long cacheSizeKB, const long maxLocks, const long maxObjects, const long maxLockers)
{
	if (NULL==serviceName || strlen(serviceName)<=0 || 
	    NULL==dataRoot    || strlen(dataRoot)<=0    || 
	    NULL==databaseName || strlen(databaseName)<=0)
	{
		log(ZQ::common::Log::L_INFO, CLOGFMT(createDBFolder, 
				"invalid input values: serviceName[%s] dataRoot[%s], databaseName[%s]"), serviceName, dataRoot, databaseName);
		return "";
	}
	std::string path = std::string(dataRoot)+serviceName+FNSEPS+databaseName+FNSEPS;
	createDBFolderEx(log, path.c_str(), cacheSizeKB, maxLocks, maxObjects, maxLockers);
	return path;
}

///MemoryServantLocator
MemoryServantLocator::MemoryServantLocator(Ice::ObjectAdapterPtr a , const std::string& dbName )
:mAdapater(a),
mSize(100),
mDbName(dbName)
{
}

::Ice::ObjectPrx MemoryServantLocator::add(const ::Ice::ObjectPtr& obj, const ::Ice::Identity& id)
{
	{
		ZQ::common::MutexGuard gd(mLocker);
		if(mServants.find(id.name) != mServants.end())
		{
			throw Ice::AlreadyRegisteredException("Object already exist",0);
		}
		mServants.insert( std::map<std::string, Ice::ObjectPtr>::value_type(id.name,obj));
	}
	return mAdapater->createProxy(id);
}


::Ice::ObjectPtr MemoryServantLocator::remove(const ::Ice::Identity& id)
{
	::Ice::ObjectPtr p;
	{
		ZQ::common::MutexGuard gd(mLocker);
		std::map<std::string , Ice::ObjectPtr>::iterator it = mServants.find(id.name);
		if(it != mServants.end() )
		{			
			p = it->second;
			mServants.erase(it);
		}
	}
	return p;
}

std::vector<Ice::Identity> MemoryServantLocator::getIds() const
{
	std::vector<Ice::Identity> ids;
	{
		ZQ::common::MutexGuard gd(mLocker);
		std::map<std::string , Ice::ObjectPtr>::const_iterator it = mServants.begin();
		for( ; it != mServants.end() ; it ++ )
		{
			Ice::Identity id ;id.category = mDbName;
			id.name = it->first;
			ids.push_back(id);
		}
	}
	return ids;
}


void MemoryServantLocator::keep(const ::Ice::Identity&){}	

void MemoryServantLocator::release(const ::Ice::Identity&){}

bool MemoryServantLocator::hasObject(const ::Ice::Identity& id)
{
	ZQ::common::MutexGuard gd(mLocker);
	return mServants.find(id.name) != mServants.end();
}

Freeze::EvictorIteratorPtr MemoryServantLocator::getIterator(const ::std::string&, ::Ice::Int)
{
	return new MemoryServantLocatorIterator(*this);
}

Ice::ObjectPtr MemoryServantLocator::locate(const ::Ice::Current& ic, ::Ice::LocalObjectPtr& cookie)
{
	if( ic.id.category != mDbName )
		return NULL;
	cookie = NULL;
	Ice::ObjectPtr p = NULL;
	{
		ZQ::common::MutexGuard gd(mLocker);
		std::map<std::string , Ice::ObjectPtr>::iterator it = mServants.find(ic.id.name);
		if( it !=mServants.end())
			p =it->second;
	}
	return p;
}

void MemoryServantLocator::finished(const ::Ice::Current&, const ::Ice::ObjectPtr&, const ::Ice::LocalObjectPtr&)
{

}

void MemoryServantLocator::deactivate(const ::std::string&) 
{

}

////////////////////
MemoryServantLocatorIterator::MemoryServantLocatorIterator(MemoryServantLocator& locater)
:mIndex(0)
{
	mIds = locater.getIds();
}

bool MemoryServantLocatorIterator::hasNext()
{
	return mIndex < mIds.size();
}

Ice::Identity MemoryServantLocatorIterator::next()
{
	Ice::Identity id;
	if(hasNext())
	{
		id = mIds[mIndex++];		
	}
	return id;
}

size_t tokenize(std::vector<std::string>& tokens, const char* str, const char* delimiters)
{
	std::string::size_type delimPos = 0, tokenPos = 0, pos = 0;
	std::string str1 = str ? str : "";

	if (str1.length() <1) 
		return 0;

	if (NULL == delimiters)
		delimiters = " \t\n\r";

	tokens.clear();

	while(1)
	{
		delimPos = str1.find_first_of(delimiters, pos);
		tokenPos = str1.find_first_not_of(delimiters, pos);

		if (std::string::npos == delimPos)
		{
			tokens.push_back(std::string::npos == tokenPos ? "" : str1.substr(pos));
			break;
		}

		if (std::string::npos == tokenPos)
			tokens.push_back("");
		else
			tokens.push_back(tokenPos < delimPos ? str1.substr(pos, delimPos-pos) :"");

		pos = delimPos+1;
	}

	return tokens.size();
}

}

