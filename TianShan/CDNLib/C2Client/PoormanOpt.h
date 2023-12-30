#ifndef _option_parser_poorman_option_parser_header_file_h__
#define _option_parser_poorman_option_parser_header_file_h__

#include <string>
#include <map>
#include <vector>
#include <stdlib.h>

class OptDescription;

class OptInit
{
public:	
	OptInit( OptDescription* owner );
	//OptInit& operator( )( const char* shortname , const char* description  , const char* longname = NULL ,bool bStatic = false );
	OptInit& operator( )( const char* shortname , const char* value , const char* description = NULL , const char* longname = NULL ,bool bStatic =false );
	OptInit& operator( )( const char* shortname , bool value , const char* description = NULL , const char* longname = NULL ,bool bStatic =false );
private:
	
	OptDescription*	mOwner;

};


class OptDescription
{
public:
	OptDescription( );
	virtual ~OptDescription( );

public:
	
	OptInit			addOptions( const std::string& desc = ("") );

	bool			add( const std::string& shortName , const std::string& description , 
						const std::string& defaultValue, const std::string& longName,
						bool bStatic);
	bool			add( const std::string& shortName , const std::string& description , 
						const bool& defaultValue, const std::string& longName,
						bool bStatic);

	size_t			getOptionsCount( ) const;

	bool			findOption( std::string& name , bool *bStatic , bool* bFlag ,std::string* defaultValue = NULL ) const;

	size_t			getStaticOptionCount( ) const;

	std::string			getStaticOptionName( size_t index ) const;

	std::string			format( ) const;
	
	const std::string&	getDescription( ) const
	{
		return		mDesciption;
	}

	typedef struct _OptionValue
	{
		std::string			description;
		std::string			defaultValue;
		bool				bStatic;
		bool				bFlag;
	}OptionValue;
	typedef std::map<std::string,OptionValue>			OptionS;
	typedef std::map<std::string , std::string>			SlRelation;
	///long name to short name map
	typedef std::vector<std::string>					StaticOptNames;

protected:

	std::string			fillBlank( size_t maxBlank ) const;

	bool				findOptionWithShortName( const std::string& name , bool *bStatic ,bool* bFlag ,std::string* defaultValue = NULL	) const;

private:
	std::string			mDesciption;
	friend class OptResult;

	StaticOptNames			mStaticNames;
	SlRelation				mRelation;
	OptionS					mOpts;
};


class OptResult
{
public:
	OptResult( );
	virtual ~OptResult( );
private:
	class converter
	{
	public:
		converter( const std::string& str ):mStr(str){}
		~converter(){}
		void operator >> ( bool& t)
		{
			if( !mStr.empty() )
				t = atoi( mStr.c_str() )!= 0 ;
			else
				t = false;					
		}
		void operator >> ( int & t)
		{
			if( mStr.empty() )			
				t = 0;
			else
				t = atoi( mStr.c_str() ) ;
		}
		void operator >> ( float & t)
		{
			if( mStr.empty() )
				t = 0.0f;
			else
				t = (float)atof(mStr.c_str());
		}
		void operator >> ( double & t)
		{
			if( mStr.empty() )
				t = 0.0f;
			else
				t = atof(mStr.c_str());			
		}
		void operator >> ( std::string& t)
		{
			t = mStr;
		}

	private:
		std::string mStr;
	};
public:

	class NoSuchOptionException : public std::exception
	{
	public:
		NoSuchOptionException(const char* what) throw()
		{
			if(what) mWhat = what;
		}
		~NoSuchOptionException() throw(){}
		const char* what() const throw()
		{
			return mWhat.c_str();
		}
	private:
		std::string mWhat;

	};
	template<class T> T as( const std::string& name )
	{
		ResultMap::const_iterator it = mResults.find( name );
		if( it != mResults.end() )
		{
			converter iss(it->second);
			T t;
			
			iss >> t;
			return t;
		}		
		NoSuchOptionException ex("on such an option");
		throw ex;
//		return T();
	}
	
	//导入那些还没有被写入的含有缺省值的options
	size_t		import( const OptDescription& opt );

protected:

	void		addResult( const std::string& name , const std::string& value);

private:
	friend bool ParseOptions( const OptDescription& desc , OptResult& result ,const std::vector<std::string>& vars );
	
	typedef std::map<std::string, std::string > ResultMap;
	///         short name to value map
	ResultMap		mResults;
};


bool ParseOptions( const OptDescription& desc , OptResult& result , int argc , const char* argv[] );
bool ParseOptions( const OptDescription& desc , OptResult& result , const std::vector<std::string>& vars );


#endif//_option_parser_poorman_option_parser_header_file_h__
