#ifndef __ZQ_Snmp_H__
#define __ZQ_Snmp_H__

#include <ZQ_common_conf.h>
#include <boost/shared_ptr.hpp>
#include <string>
#include <Log.h>

namespace ZQ{
namespace Snmp{

enum Status
    {
        // v1 status
        noError,
        tooBig,
        noSuchName,
        badValue,
        readOnly,
        genErr
    };

typedef uint32 AsnType;

// supported standard ASN.1 type
const AsnType AsnType_None = 0x00;
const AsnType AsnType_Integer = 0x02;
const AsnType AsnType_Counter64 = 0x46;
const AsnType AsnType_Oid = 0x06;
const AsnType AsnType_Octets = 0x04; // string, displaystring

enum Access
    {
        aReadOnly,
        aReadWrite
    };

class Oid;
template <typename T>
bool oidFrom(Oid& id, const T& from);

template <typename T>
bool oidTo(const Oid& id, T& to);

class SmiValue;
template <typename T>
bool smivalFrom(SmiValue& val, const T& from, AsnType t);

template <typename T>
bool smivalTo(const SmiValue& val, T& to);

class IManaged
{
 public:

	IManaged(){}

    virtual ~IManaged() {}

    // get the value of the object with the specified sub-id
    virtual Status get(const Oid& subid, SmiValue& val) = 0;

    // set the value of the object with the specified sub-id
    virtual Status set(const Oid& subid, const SmiValue& val) = 0;

    // get the next id in the managed object
    virtual Status next(const Oid& subid, Oid& nextId) const = 0;

    // get the first id in the managed object
    virtual Status first(Oid& firstId) const = 0;
};
typedef boost::shared_ptr< IManaged > ManagedPtr;

class IVariable
{
public:
	IVariable(){}
    virtual ~IVariable() {}

    virtual bool get(SmiValue& val, AsnType desiredType) = 0;
    virtual bool set(const SmiValue& val) = 0;
    virtual bool validate(const SmiValue& val) const = 0;
};
typedef boost::shared_ptr< IVariable > VariablePtr;


class NullType;

struct EmptyType{};

template <class T, class U>
struct Typelist
{
	typedef T Head;
	typedef U Tail;
};

#define TYPELIST_1(T1)          ZQ::Snmp::Typelist<T1, ZQ::Snmp::NullType> 
#define TYPELIST_2(T1, T2)	    ZQ::Snmp::Typelist<T1, TYPELIST_1(T2) > 
#define TYPELIST_3(T1, T2, T3)	ZQ::Snmp::Typelist<T1, TYPELIST_2(T2, T3) > 

template <class TList> struct TypeStruct;

template< class Object, class Method, class DesiredType >
struct TypeStruct<ZQ::Snmp::Typelist<Object, ZQ::Snmp::Typelist<Method, DesiredType> > > 
{
	typedef  ZQ::Snmp::Typelist<Object, ZQ::Snmp::Typelist<Method, DesiredType> >  List;
	typedef  typename List::Head              BindObj;
	typedef  typename List::Tail::Head        BindMethod;
	typedef  typename List::Tail::Tail::Head  ReturnDesired;
};


template<class Obj, class ObjMethod, class ReturnDesired>
ReturnDesired  assistSnmpGet(Obj& obj, ObjMethod objMethod, ReturnDesired )
{
	return (obj.*objMethod)();
};

template<>
inline int  assistSnmpGet(int & obj, int objMethod, int returnDesired)
{
	return obj;
}

template<>
inline std::string  assistSnmpGet(std::string & obj, std::string objMethod, std::string returnDesired)
{
	return obj;
}

template<class Obj, class ObjMethod, class ReturnDesired>
ReturnDesired  assistSnmpSet(Obj & obj, ObjMethod objMethod, ReturnDesired returnDesired)
{
	(obj.*objMethod)(returnDesired);
	return returnDesired;
};

template<class Obj>
inline int assistSnmpSet(Obj & obj, void (Obj::*objMethod)() , int)
{
	(obj.*objMethod)();
	return true;
};

template<>
inline int  assistSnmpSet(int & obj, int objMethod, int returnDesired)
{
	obj = returnDesired;
	return obj;
}

template<>
inline std::string  assistSnmpSet(std::string & obj, std::string objMethod, std::string returnDesired)
{
	obj = returnDesired;
	return obj;
}

template <Access rwAccess, class type>class VarCommon;

template <class GetTypes>
class VarCommon<ZQ::Snmp::aReadWrite, GetTypes> : public ZQ::Snmp::IVariable
{
public:
	typedef typename GetTypes::Head        GetType;
	typedef typename GetTypes::Tail::Head  SetType;

public:
	VarCommon(typename GetType::BindObj getObj, typename GetType::BindMethod method, typename SetType::BindObj setObj, typename SetType::BindMethod setMethod)
		:_getObj(getObj), _getMethod(method), _setObj(setObj), _setMethod(setMethod)
	{	}


	virtual bool get(SmiValue& val, AsnType desiredType)
	{
		typename GetType::ReturnDesired  finalGet = assistSnmpGet(_getObj, _getMethod, _getReturn);
		return  ZQ::Snmp::smivalFrom(val, finalGet, desiredType);
	}

	virtual bool set(const SmiValue& val)
	{
		if(! ZQ::Snmp::smivalTo(val, _setReturn)) 
			return false;

		_setReturn = assistSnmpSet(_setObj, _setMethod, _setReturn);
		return true;
	}

	virtual bool validate(const SmiValue&) const
	{
		return true;
	}

private:
	typename GetType::BindObj        _getObj;
	typename GetType::BindMethod	 _getMethod;
	typename GetType::ReturnDesired  _getReturn;

	typename SetType::BindObj        _setObj;
	typename SetType::BindMethod	 _setMethod;
	typename SetType::ReturnDesired  _setReturn;
};

template <class GetTypes>
class VarCommon<ZQ::Snmp::aReadOnly, GetTypes > : public ZQ::Snmp::IVariable
{
public:
	typedef typename GetTypes::Head  GetType;

public:
	VarCommon(typename GetType::BindObj getObj)
		:_getObj(getObj)
	{
	}

	VarCommon(typename GetType::BindObj getObj, typename GetType::BindMethod method)
		:_getObj(getObj), _getMethod(method)
	{
	}

	virtual bool get(SmiValue& val, AsnType desiredType)
	{
		typename GetType::ReturnDesired  finalGet = assistSnmpGet(_getObj, _getMethod, _getReturn);
		return ZQ::Snmp::smivalFrom(val, finalGet, desiredType);
	}

	virtual bool set(const SmiValue& )
	{
		return false;
	}

	virtual bool validate(const SmiValue& ) const
	{
		return true;
	}

private:
	typename GetType::BindObj        _getObj;
	typename GetType::BindMethod	 _getMethod;
	typename GetType::ReturnDesired  _getReturn;
};

typedef  struct ZQ::Snmp::TypeStruct<TYPELIST_3(int&, int, int) >	                         thrIntList;
typedef  struct ZQ::Snmp::TypeStruct<TYPELIST_3(std::string&, std::string, std::string) >	 thrStringList;

typedef  ZQ::Snmp::VarCommon<ZQ::Snmp::aReadOnly,  TYPELIST_1(thrIntList) >               roIntSnmpType;
typedef  ZQ::Snmp::VarCommon<ZQ::Snmp::aReadWrite, TYPELIST_2(thrIntList, thrIntList) >   rwIntSnmpType;

typedef  ZQ::Snmp::VarCommon<ZQ::Snmp::aReadOnly,  TYPELIST_1(thrStringList) >                  roStringSnmpType;
typedef  ZQ::Snmp::VarCommon<ZQ::Snmp::aReadWrite, TYPELIST_2(thrStringList, thrStringList) >   rwStringSnmpType;


typedef	 void  (ZQ::common::Log::*SetLevelType)(int level);
typedef	 int   (ZQ::common::Log::*GetVerbosityType)(void);
typedef  struct ZQ::Snmp::TypeStruct<TYPELIST_3(ZQ::common::Log&, GetVerbosityType, int)>            GetLoglevelList;
typedef  struct ZQ::Snmp::TypeStruct<TYPELIST_3(ZQ::common::Log&, SetLevelType, int) >               SetLoglevelList;
typedef  ZQ::Snmp::VarCommon<ZQ::Snmp::aReadWrite, TYPELIST_2(GetLoglevelList, SetLoglevelList) >    rwSnmpLogType;

#define DECLARE_SNMP_RO_TYPE(store, method, retType)  \
	ZQ::Snmp::VarCommon<ZQ::Snmp::aReadOnly,TYPELIST_1( struct ZQ::Snmp::TypeStruct< TYPELIST_3(store, method, retType) > ) >


#define DECLARE_SNMP_RW_TYPE(storeGet, methodGet, retTypeGet, storeSet, methodSet, retTypeSet)\
	ZQ::Snmp::VarCommon< ZQ::Snmp::aReadWrite, TYPELIST_2(\
                                         struct ZQ::Snmp::TypeStruct< TYPELIST_3(storeGet, methodGet, retTypeGet) >, \
										 struct ZQ::Snmp::TypeStruct< TYPELIST_3(storeSet, methodSet, retTypeSet) >  \
                                         ) >
}} // namespace ZQ::Snmp

#endif
