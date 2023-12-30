// DataProfile.h: interface for the DataProfile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAPROFILE_H__50F77714_1E36_47BA_887C_9461505DCEB4__INCLUDED_)
#define AFX_DATAPROFILE_H__50F77714_1E36_47BA_887C_9461505DCEB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ProfileCategory {
public:

	bool getValue(const std::string& name, long& value)
	{
		SLMap::iterator it = _slMap.find(name);
		if (it == _slMap.end()) 
			return false;
		value = it->second;
		return true;
	}

	bool getValue(const std::string& name, std::string& value)
	{
		SSMap::iterator it = _ssMap.find(name);
		if (it == _ssMap.end()) 
			return false;
		value = it->second;
		return true;
	}

	void clear()
	{
		_slMap.clear();
		_ssMap.clear();
	}

protected:

	bool report(const std::string& name, long value)
	{
		SLMap::iterator it = _slMap.find(name);
		if (it == _slMap.end()) {
			std::pair<SLMap::iterator, bool> r;
			r = _slMap.insert(SLMap::value_type(name, value));
			if (!r.second) {

				// log error
				return false;
			}

			return true;
		} else {
			it->second = value;
			return true;
		}
	}

	bool report(const std::string& name, const std::string& value)
	{
		SSMap::iterator it = _ssMap.find(name);
		if (it == _ssMap.end()) {
			std::pair<SSMap::iterator, bool> r;
			r = _ssMap.insert(SSMap::value_type(name, value));
			if (!r.second) {

				// log error
				return false;
			}

			return true;
		} else {
			it->second = value;
			return true;
		}
	}
protected:

	typedef std::map<std::string, long> SLMap;
	typedef std::map<std::string, std::string> SSMap;

	SLMap		_slMap;
	SSMap		_ssMap;
};

class DataProfile {
public:
	DataProfile();
	virtual ~DataProfile();

//////////////////////////////////////////////////////////////////////////
	
	bool retisterCategory(const std::string& name, 
		ProfileCategory& category);
	bool unretisterCategory(const std::string& category);

	ProfileCategory* getCategory(const std::string& category);
	bool getCategoryNames(std::vector<std::string>& categoryNames);
};

#endif // !defined(AFX_DATAPROFILE_H__50F77714_1E36_47BA_887C_9461505DCEB4__INCLUDED_)
