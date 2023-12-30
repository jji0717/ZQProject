// MuxItemImpl.h: interface for the MuxItemImpl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MUXITEMIMPL_H__882BB4AA_ABC3_4F0A_A0D1_2B7118F3B28B__INCLUDED_)
#define AFX_MUXITEMIMPL_H__882BB4AA_ABC3_4F0A_A0D1_2B7118F3B28B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <DataStream.h>
#include <IceUtil/IceUtil.h>

namespace DataStream {

class DataSource;
class DataReader;

} // namespace DataStream {

namespace DataOnDemand {

class MuxItemImpl : virtual public MuxItem, 
	public IceUtil::AbstractMutexI<IceUtil::RecMutex> {
	
	friend class DataStreamImpl;
public:

	MuxItemImpl(Ice::ObjectAdapterPtr& adapter);
	virtual ~MuxItemImpl();
	
	bool init(const MuxItemInfo& info);

    virtual ::std::string getName(
		const Ice::Current& = Ice::Current()) const;

    virtual void notifyFullUpdate(const ::std::string&, 
		const Ice::Current& = Ice::Current());

    virtual void notifyFileAdded(const ::std::string&,
		const Ice::Current& = Ice::Current());

    virtual void notifyFileDeleted(const ::std::string&,
		const Ice::Current& = Ice::Current());

    virtual ::DataOnDemand::MuxItemInfo getInfo(
		const Ice::Current& = Ice::Current()) const;

    virtual void destroy(const Ice::Current& = Ice::Current());

	virtual void setProperies(const ::TianShanIce::Properties&, 
		const ::Ice::Current& = ::Ice::Current());

	virtual ::TianShanIce::Properties getProperties(
		const ::Ice::Current& = ::Ice::Current()) const;

protected:
	::DataStream::DataSource* getDataSource();
	void setDataSource(::DataStream::DataSource* dataSource);

	bool fetchFile(const std::string& fileName);
	bool deleteFile(const std::string& fileName);

	enum {
		UpdateFull, 
		UpdateAdded,
		UpdateDeleted,
	};

	::DataStream::DataReader* rebuildReader(const std::string& fileName, 
		int type);

	bool appendFile(FILE* dest, FILE* src);

protected:
	Ice::ObjectAdapterPtr&	_adapter;
	MuxItemInfo				_info;
	DataStreamImpl*			_parent;
	TianShanIce::Properties	_props;
	::DataStream::DataSource*	_dataSource;

	typedef std::vector<std::string> FileList;
	FileList				_fileList;
	bool					_destroy;
};

} // namespace DataOnDemand {

#endif // !defined(AFX_MUXITEMIMPL_H__882BB4AA_ABC3_4F0A_A0D1_2B7118F3B28B__INCLUDED_)
