#include "FileSystemOp.h"
#include "strHelper.h"

using namespace ZQ::common::stringHelper;

namespace FS {


#if defined(ZQ_OS_MSWIN)

	bool createDirectory(const std::string& path, bool recursive) {
		if(!recursive) {
			if(CreateDirectory(path.c_str(), 0)) {
				return true;
			}

			return (GetLastError() == ERROR_ALREADY_EXISTS); 
		}
		else {
			std::string subPath = path;
			std::vector<std::string> pathToCreate;

			while(true) {
				if(CreateDirectory(subPath.c_str(), 0)) {
					break;
				}

				if(GetLastError() == ERROR_PATH_NOT_FOUND) {
					pathToCreate.push_back(subPath);
					subPath = rsplit(subPath, '\\', 1).at(0);		

					continue;
				}
				break;
			}

			std::vector<std::string>::reverse_iterator iter = pathToCreate.rbegin();
			for(; iter != pathToCreate.rend(); ++iter) {
				CreateDirectory((*iter).c_str(), 0);
			}
		}

		return true;
	}

	std::string getWorkingDirectory() {
		char buffer[MAX_PATH];
		memset(buffer, '\0', MAX_PATH);
		GetCurrentDirectory(MAX_PATH-1, buffer);

		return buffer;
	}

	std::string getImagePath() {
		char buffer[MAX_PATH];
		memset(buffer, '\0', MAX_PATH-1);

		GetModuleFileName(NULL, buffer, MAX_PATH-1);

		return buffer;
	}

	bool getDiskSpace(const std::string& path, int64& avail, int64& total, int64& free) {
		if (GetDiskFreeSpaceEx(path.c_str(), (ULARGE_INTEGER*)&avail, (ULARGE_INTEGER*)&total, (ULARGE_INTEGER*)&free)) {
			return true;
		}
		return false;
	}

	bool remove(const std::string& path, bool recursive) {
		{
			FileAttributes f(path);
			if(f.isRegular()) {
				return (DeleteFile(path.c_str()) != 0);
			}
		}

		if(!recursive) {
			return (RemoveDirectory(path.c_str()) != 0);
		}

		/* force deleting all subdir and files within. */

		/* x:\xxx --> x:\xxx\*.* */
		std::string searchPath = path + "\\*.*";

		WIN32_FIND_DATA data;
		HANDLE findHandle = FindFirstFile(searchPath.c_str(), &data);

		if(findHandle == INVALID_HANDLE_VALUE) {
			return false;
		}

		do{
			if(data.cFileName[0] == '.') {
				continue;
			}

			/* X:\xxx\yyy */
			searchPath = path + "\\" + data.cFileName;

			/* recursively enter directories until all regular files deleted */
			if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				remove(searchPath, true);
			}
			else {
				if(!DeleteFile(searchPath.c_str())) {
					return false;
				}
			}
		} while(FindNextFile(findHandle, &data));

		FindClose(findHandle);

		/* must be an empty directory */
		if(!RemoveDirectory(path.c_str())) {
			return false;
		}

		return true;
	}

	std::vector<std::string> searchFiles(const std::string& path, const std::string& pattern) { 

		std::vector<std::string> result;

		std::string target = path + '\\' + pattern;

		std::string local;

		if(path.at(0) == '.') {
			local = getWorkingDirectory();
		}

		if(path.at(path.length()-1) != '\\') {
			local = path + '\\'; 	
		}

		if(local.empty()) {
			local = path;
		}

		WIN32_FIND_DATA findData;
		HANDLE find = FindFirstFile(target.c_str(), &findData);

		if(find != INVALID_HANDLE_VALUE) {
			result.push_back(local + findData.cFileName);

			while(FindNextFile(find, &findData)) {
				result.push_back(local + findData.cFileName);
			}

			FindClose(find);
		}

		return result;
	}


	std::vector<std::string> matchFiles(const std::vector<std::string>& names, const std::string& pattern) {
		std::vector<std::string> result;
		std::vector<std::string>::const_iterator iter = names.begin();
		for(; iter != names.end(); ++iter) {
			if(PathMatchSpec(iter->c_str(), pattern.c_str())) {
				result.push_back(*iter);
			}
		}

		return result;
	}

	std::string getErrorMessage() {
		char msg[256];
		memset(msg, '\0', 256);

		FormatMessage(
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			GetLastError(),
			0,
			msg,
			255,
			NULL);
		msg[std::strlen(msg)-1] = '\0';

		return msg;
	}


#elif defined(ZQ_OS_LINUX)

	bool createDirectory(const std::string& path, bool recursive) {
		if(!mkdir(path.c_str(), 0755)) {
			return true;
		}

		if(errno == EEXIST) {
			FileAttributes f(path);
			return f.isDirectory();
		}

		if(errno != ENOENT || !recursive) {
			return false;
		}

		std::string subPath = path;
		std::vector<std::string> pathToCreate;

		do {
			pathToCreate.push_back(subPath);
			subPath = rsplit(subPath, '/', 1).at(0);		

			if(!mkdir(subPath.c_str(), 0755)) {
				break;
			}

			if(errno == ENOENT) {
				continue;
			}
			return false;
		} while(true);

		std::vector<std::string>::reverse_iterator iter = pathToCreate.rbegin();
		for(; iter != pathToCreate.rend(); ++iter) {
			if(mkdir((*iter).c_str(), 0755)) {
				return false;
			}
		}

		return true;
	}

	std::string getWorkingDirectory() {
		return get_current_dir_name();
	}

	std::string getImagePath() {
		std::ostringstream os;
		os << "/proc/" << getpid() << "/exe";

		char buffer[PATH_MAX];
		memset(buffer, '\0', PATH_MAX);
		readlink(os.str().c_str(), buffer, PATH_MAX-1);

		return buffer;
	}

	bool getDiskSpace(const std::string& path, int64& avail, int64& total, int64& free) {
		struct statvfs info;
		if(!statvfs(path.c_str(), &info)) {
			avail = (int64_t)((double)info.f_bavail*(double)info.f_frsize);
			free  = (int64_t)((double)info.f_bfree* (double)info.f_frsize);
			total = (int64_t)((double)info.f_blocks*(double)info.f_frsize);

			return true;
		}

		return false;
	}

	bool remove(const std::string& path, bool recursive) {
		if(::remove(path.c_str()) == (-1)) {
			if(errno != ENOTEMPTY || !recursive) {
				return false;
			}
		}

		DIR* dir = opendir(path.c_str());
		if(!dir) {
			return false;
		}

		struct dirent* entry;
		while(entry = readdir(dir)) {
			if(!strcmp(entry->d_name, ".") || 
				!strcmp(entry->d_name, "..")) {
					continue;
			}

			std::string tmp = path + "/" + entry->d_name;
			FileAttributes f(tmp);
			if(f.isDirectory()) {
				remove(tmp, true);
			}
			else {
				if(::remove(tmp.c_str()) == (-1)) {
					closedir(dir);
					return false;
				}
			}
		}

		if(::remove(path.c_str()) == (-1)) {
			closedir(dir);
			return false;
		}

		closedir(dir);
		return true;
	}


	std::vector<std::string> searchFiles(const std::string& path, const std::string& pattern) { 
		std::vector<std::string> result;

		glob_t gl;

		std::string target = path + '/' + pattern;

		if(!glob(target.c_str(), GLOB_PERIOD|GLOB_TILDE, 0, &gl)) {
			for(size_t i = 0; i < gl.gl_pathc; ++i) {
				if(gl.gl_pathv[i][0] == '.') {
					result.push_back(getWorkingDirectory() + (gl.gl_pathv[i]+1));
				}
				else {
					result.push_back(gl.gl_pathv[i]);
				}
			}
			globfree(&gl);
		}

		return result;
	}


	std::vector<std::string> matchFiles(const std::vector<std::string>& names, const std::string& pattern) {
		std::vector<std::string> result;
		std::vector<std::string>::const_iterator iter = names.begin();
		for(; iter != names.end(); ++iter) {
			if(!fnmatch(pattern.c_str(), iter->c_str(), 0)) {
				result.push_back(*iter);
			}
		}

		return result;
	}

	std::string getErrorMessage() {
		char msg[256];
		memset(msg, '\0', 256);

		strerror_r(errno, msg, 255);

		return msg;
	}

#endif
}