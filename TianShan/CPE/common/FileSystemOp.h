#ifndef __FILESYSTEM__
#define __FILESYSTEM__


#include "ZQ_common_conf.h"
#include <string>
#include <vector>


#if defined(ZQ_OS_MSWIN)

#include <process.h>
#include <direct.h>
#include <shlwapi.h>

#elif defined(ZQ_OS_LINUX)

#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <dirent.h>
#include <glob.h>
#include <inttypes.h>

#endif


namespace FS {

#if defined(ZQ_OS_MSWIN)

	class FileAttributes {

	public:

		FileAttributes(const std::string& path): _isValid(true) {
			if(!GetFileAttributesEx(path.c_str(), ::GetFileExInfoStandard, &_attrs)) {
				_isValid = false;
			}
		}

	public:

		bool isDirectory() const {
			return (_isValid && (_attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
		}

		bool isRegular() const {
			return (!isDirectory());
		}

		bool exists() const {
			return _isValid;
		}

		int64 size() const {
			return _isValid ? ((_attrs.nFileSizeHigh << sizeof(DWORD))+_attrs.nFileSizeLow) : 0;
		}

	private:

		WIN32_FILE_ATTRIBUTE_DATA _attrs;
		bool _isValid;

	};

#elif defined(ZQ_OS_LINUX)

	class FileAttributes {

	public:

		FileAttributes(const std::string& path):_isValid(true) {
			if(lstat64(path.c_str(), &_attrs) == (-1)) {
				_isValid = false;
			}
		}

	public:

		bool isDirectory() const {
			return (_isValid && (S_ISDIR(_attrs.st_mode)));
		}

		bool isRegular() const {
			return (_isValid && (S_ISREG(_attrs.st_mode)));
		}

		bool exists() const {
			return _isValid;
		}

		int64 size() const {
			return _isValid ? _attrs.st_size : 0;
		}

	private:

		struct stat64 _attrs;
		bool _isValid;
	};

#endif

	extern bool createDirectory(const std::string& path, bool recursive=false);

	extern std::string getWorkingDirectory();

	extern std::string getImagePath();

	extern bool remove(const std::string& path, bool recursive=false);

	extern bool getDiskSpace(const std::string& path, int64& free, int64& avail, int64& total); 

	extern std::vector<std::string> searchFiles(const std::string& path, const std::string& pattern); 

	extern std::vector<std::string> matchFiles(const std::vector<std::string>& names, const std::string& pattern);
	
	extern std::string getErrorMessage();
}



#endif