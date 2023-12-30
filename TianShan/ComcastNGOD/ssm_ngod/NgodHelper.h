#ifndef _ngod_helper_header_file_h__
#define _ngod_helper_header_file_h__

#include <string>
#include <map>
#include <vector>

class KVPairGroupWalker
{
public:
	KVPairGroupWalker( const std::string& kvPairContent , const std::string& delimeter="\r\n" );
	virtual ~KVPairGroupWalker( );	
public:
	typedef struct _KVAttr
	{
		std::string		key;
		std::string		value;
	}KVAttr;	

	class const_iterator
	{
	public:		
		const_iterator( std::vector<KVAttr>::const_iterator itBegin , 
						std::vector<KVAttr>::const_iterator itEnd)
			:mItCur(itBegin),
			mItEnd(itEnd)
		{
		}
		~const_iterator( )
		{
		}
		const KVAttr& operator*( ) const
		{
			assert( mItCur != mItEnd );
			return *mItCur;
		}
		const KVAttr* operator->( ) const
		{
			assert( mItCur != mItEnd );
			return &(*mItCur);
		}
		const_iterator&	operator++( ) 
		{
			assert( mItCur != mItEnd );
			mItCur++;
			return *this;
		}

		const_iterator operator++(int)
		{
			assert( mItCur != mItEnd );
			const_iterator tmp = *this;
			mItCur++;
			return tmp;
		}

		bool operator == ( const const_iterator& it)
		{
			return it.mItCur == mItCur;
		}
		bool operator != ( const const_iterator& it)
		{
			return it.mItCur != mItCur;
		}
	private:
		std::vector<KVAttr>::const_iterator mItCur;
		std::vector<KVAttr>::const_iterator mItEnd;
	};

	const_iterator	begin( ) const
	{
		return const_iterator( mSdpLine.begin() , mSdpLine.end() );
	}
	const_iterator	end() const
	{
		return const_iterator( mSdpLine.end() , mSdpLine.end() );
	}
protected:
	
	void			parseSdp( const std::string& strSdp );

private:		

	std::vector<KVAttr>		mSdpLine;
	std::string				mDelimiter;
};

#define POSOK(pos) (pos != std::string::npos )

class SettingDispatcher;

typedef void ( SettingDispatcher::*SETTINGFUNC )( const std::string& );

class SettingDispatcher
{
public:
	SettingDispatcher();
	virtual ~SettingDispatcher( );
	
	
public:
	
	void			regSetting( const std::string& key , SETTINGFUNC func );

	void			dispatch( const std::string& key ,const std::string& value);
	
	virtual	void	onComplete( ){;}

protected:	
	struct keyCmp
	{
		bool operator()( const std::string& a , const std::string& b ) const
		{
			return stricmp( a.c_str() , b.c_str() ) < 0 ;
		}
	};
	typedef std::map< std::string, SETTINGFUNC , keyCmp >  SETTINGMAP;
	SETTINGMAP				mMap;
};

#define REGSETTING( key , className , func ) regSetting( key , (&className:func) )


enum NgodProtoclVersionCode
{
	NgodVerCode_R2 = 0, 
	NgodVerCode_R2_DecNpt, 
	NgodVerCode_C1, 
	NgodVerCode_C1_DecNpt, 
	NgodVerCode_UNKNOWN
};


void		walkSettings( SettingDispatcher& dispatcher , KVPairGroupWalker& walker );

std::string getGMTString();

std::string	getISOTimeString( );

std::string generatorOffsetString( int64 offset , int require );

std::string generatorNoticeString(	const std::string strNoticeCode, const std::string strNoticeString, const std::string strNpt = "" );

std::string convertIntToNptString( int64 npt );


#endif//_ngod_helper_header_file_h__

