#include <CRMInterface.h>
#include "../ZipHelper.h"
#include <FileLog.h>
#include <Locks.h>
#include <algorithm>
#include <io.h>
#include <fcntl.h>

class SymbolDownloader;
SymbolDownloader* gDownloader;
ZQ::common::FileLog* gLog;
#define LOG if(gLog) (*gLog)

template <class StringColl>
StringColl& split(StringColl& strs, const std::string& s, const std::string& delimiter = " ")
{
    strs.clear();

    std::string::size_type pos_from = 0;
    while((pos_from = s.find_first_not_of(delimiter, pos_from)) != std::string::npos)
    {
        std::string::size_type pos_to = s.find_first_of(delimiter, pos_from);
        if(pos_to != std::string::npos)
        {
            strs.push_back(s.substr(pos_from, pos_to - pos_from));
        }
        else
        {
            strs.push_back(s.substr(pos_from));
            break;
        }
        pos_from = pos_to;
    }
    return strs;
}
class ICharEqual {
public:
    bool operator()(char l, char r) {
        return tolower(l) == tolower(r);
    }
};
class ResourceDb {
public:
    // FMT: path1;path2;path3
    ResourceDb(const std::string& root) {
        split(dbList_, root, ";");
        for(size_t i = 0; i < dbList_.size(); ++i) {
            std::string& rootDir = dbList_[i];
            if(!rootDir.empty() && rootDir[rootDir.size() - 1] != '\\')
                rootDir += "\\";
        }
    }
    bool queryPackage(const std::string& version, std::string& packagePath) {
        for(size_t i = 0; i < dbList_.size(); ++i) {
            packagePath = dbList_[i] + "TianShanSymbols.V" + version + ".zip";
            if(0 == access(packagePath.c_str(), 4)) { // check read permission
                return true;
            }
        }
        packagePath.clear();
        return false;
    }
    bool query(const std::string& name, const std::string& version, bool x64,  std::string& fileName) {
        fileName.clear();
        ZQ::common::MutexGuard guard(lock_);
        if(!buildIndex(version)) {
            return false;
        }
        const ZipHelper::FileList& files = index_[version];
        for(ZipHelper::FileList::const_iterator it = files.begin(); it != files.end(); ++it) {
            // find the first file name that end with the target
            if(x64 == (it->find("64/") != std::string::npos)) {
                if (it->end() != std::find_end(it->begin(), it->end(), name.begin(), name.end(), ICharEqual())) {
                    fileName = *it;
                    break;
                }
            }
        }
        return !fileName.empty();
    }
private:
    bool buildIndex(const std::string& version) {
        if(index_.find(version) != index_.end()) {
            return true;
        }
        std::string packagePath;
        if(!queryPackage(version, packagePath)) {
            return false;
        }
        // check the existence
        ZipHelper::FileList fileList;
        ZipHelper::Error e;
        // build the index
        if(ZipHelper::listFilesInZipFile(packagePath.c_str(), fileList, e)) {
            index_[version].swap(fileList);
            return true;
        } else {
            return false;
        }
    }
private:
    ZQ::common::Mutex lock_;
    std::map<std::string, ZipHelper::FileList> index_;
    std::vector<std::string> dbList_;
};

#define ValidCStr(p) (NULL != (p) && '\0' != *(p))
class SymbolDownloader: public CRG::IContentHandler
{
public:
    SymbolDownloader(const std::string& dbroot, const std::string& tmpdir)
        :resDb_(dbroot), tmpdir_(tmpdir) {
        if(!tmpdir_.empty() && tmpdir_[tmpdir_.size() - 1] != '\\')
            tmpdir_ += "\\";
    }
    virtual ~SymbolDownloader(){
    }
    virtual void onRequest(const CRG::IRequest* req, CRG::IResponse* resp) {
        using namespace ZQ::common;
        const char* ver = req->queryArgument("v"); // version
        if(!ValidCStr(ver)) {
            LOG(Log::L_DEBUG, "Parameter missed: v");
            resp->setStatus(400);
            return;
        }

        const char* f = req->queryArgument("f"); // files
        if(!ValidCStr(f)) {
            LOG(Log::L_DEBUG, "Parameter missed: f");
            resp->setStatus(400);
            return;
        }

        bool x64 = NULL != req->queryArgument("x64"); // x86|x64
        std::string packagePath;
        if(!resDb_.queryPackage(ver, packagePath)) {
            LOG(Log::L_DEBUG, "Package not found: Version(%s)", ver);
            resp->setStatus(404);
            return;
        }
        // chose the content encoding
        // only support *deflate* and *gzip*
        enum ContentEncoding {
            enIdentity, // default:no encoding
            enGzip, // gzipped
        };
        ContentEncoding encoding = enIdentity;
        // Accept-Encoding: xxx;q=1.0, yyy, zzz:q=0
        const char* acceptEncoding = req->header("Accept-Encoding");
        // TODO: parse the AcceptEncoding and chose a proper one
        if(ValidCStr(acceptEncoding)) {
            // not parse the header string but simply check the key word
            // NOTICE: this behavior don't conform to the protocol.
            std::string s = acceptEncoding;
            std::transform(s.begin(), s.end(), s.begin(), tolower);
            if (s.find("gzip") != std::string::npos) {
                encoding = enGzip;
            } else {
                encoding = enIdentity;
            }
        }
        std::string fileFullName;
        if(!resDb_.query(f, ver, x64, fileFullName)) {
            LOG(Log::L_DEBUG, "Module not found:Module(%s) Version(%s)%s", f, ver, (x64 ? " x64" : ""));
            resp->setStatus(404);
            return;
        }
        std::string fileName;
        {
            std::string::size_type pos = fileFullName.rfind("/");
            if(pos != std::string::npos) {
                fileName = fileFullName.substr(pos + 1);
            } else {
                fileName = fileFullName;
            }
        }

        char buf[256];
        sprintf(buf, "%s%u", tmpdir_.c_str(), GetCurrentThreadId());
        std::string tmppath = buf;
        unlink(tmppath.c_str());
        if (enGzip == encoding) {
            ZipHelper::GzipFileOutput out;
            if(!out.open(tmppath.c_str())) {
                LOG(Log::L_ERROR, "Failed to create temp file %s", tmppath.c_str());
                resp->setStatus(500);
                return;
            }
            ZipHelper::Error e;
            if(!extractFileFromZipFile(packagePath.c_str(), fileFullName.c_str(), out, e)) {
                LOG(Log::L_ERROR, "Failed to extract file %s from zip file %s", fileFullName.c_str(), packagePath.c_str());
                resp->setStatus(500);
                return;
            }
            out.close();
        } else { // identity
            ZipHelper::RawFileOutput out;
            if(!out.open(tmppath.c_str())) {
                LOG(Log::L_ERROR, "Failed to create temp file %s", tmppath.c_str());
                resp->setStatus(500);
                return;
            }
            ZipHelper::Error e;
            if(!extractFileFromZipFile(packagePath.c_str(), fileFullName.c_str(), out, e)) {
                LOG(Log::L_ERROR, "Failed to extract file %s from zip file %s", fileFullName.c_str(), packagePath.c_str());
                resp->setStatus(500);
                return;
            }
            out.close();
        }

        HANDLE hFile = CreateFile(tmppath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
        if(INVALID_HANDLE_VALUE == hFile) {
            LOG(Log::L_ERROR, "Failed to open temp file %s", tmppath.c_str());
            resp->setStatus(500);
            unlink(tmppath.c_str());
            return;
        }
        DWORD len = GetFileSize(hFile, NULL);
        if(len < 0) {
            LOG(Log::L_ERROR, "Failed get the file size of %s", tmppath.c_str());
            resp->setStatus(500);
            CloseHandle(hFile);
            unlink(tmppath.c_str());
            return;
        }
        HANDLE hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
        if(NULL == hMap) {
            LOG(Log::L_ERROR, "Failed create file mapping of %s", tmppath.c_str());
            resp->setStatus(500);
            CloseHandle(hFile);
            unlink(tmppath.c_str());
            return;
        }
        const char* data = (const char*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
        if(NULL == data) {
            LOG(Log::L_ERROR, "Failed map view of file %s", tmppath.c_str());
            resp->setStatus(500);
            CloseHandle(hMap);
            CloseHandle(hFile);
            unlink(tmppath.c_str());
            return;
        }
        resp->setStatus(200);
        sprintf(buf, "%u", len);
        resp->setHeader("Content-Length", buf);
        resp->setHeader("Content-Type", "application/octet-stream");
        if (enGzip == encoding) {
            resp->setHeader("Content-Encoding", "gzip");
        } // else: no this header
        resp->setHeader("Content-Disposition", ("attachment; filename=" + fileName).c_str());
        resp->setContent(data, len);
        UnmapViewOfFile(data);
        CloseHandle(hMap);
        CloseHandle(hFile);
        unlink(tmppath.c_str());

        LOG(Log::L_INFO, "Request successfully:Module(%s) Version(%s)%s", f, ver, (x64 ? " x64" : ""));
        return;
    }
private:
    ResourceDb resDb_;
    std::string tmpdir_;
};
BOOL APIENTRY DllMain( HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                      )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        gDownloader = NULL;
        gLog = NULL;
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        if(gDownloader) {
            delete gDownloader;
            gDownloader = NULL;
        }
        if(gLog) {
            delete gLog;
            gLog = NULL;
        }
        break;
    default:
        return FALSE;
    }
    return TRUE;
}
#define DownloadURI "/?symbols"
#define ConfFileName "SymbolServer.ini"
#define LogFileName "CRM_SymbolServer.log"
extern "C"
{
    __EXPORT bool CRM_Entry_Init(CRG::ICRMManager* mgr)
    {
        if(gDownloader) {
            mgr->superLogger()(ZQ::common::Log::L_WARNING, CLOGFMT(CRM_SymbolServer, "Already registered"));
            return false;
        }
        std::string logpath = mgr->getLogFolder() + LogFileName;
        try
        {
            gLog = new ZQ::common::FileLog(logpath.c_str(), 7, 5, 10 * 1024 * 1024);
        }catch(ZQ::common::FileLogException& e)
        {
            mgr->superLogger()(ZQ::common::Log::L_ERROR, CLOGFMT(C2Locator, "Failed to create logger at %s. detail:%s"), logpath.c_str(), e.getString());
            return false;
        }
        std::string conffile = mgr->getConfigFolder() + ConfFileName;
        char buf[512];
        int bufn = sizeof(buf);
        GetPrivateProfileString("TianShan", "DBRoot", "", buf, bufn, conffile.c_str());
        std::string dbRoot = buf;
        GetPrivateProfileString("TianShan", "TMPDir", "", buf, bufn, conffile.c_str());
        std::string tmpDir = buf;
        if(dbRoot.empty() || tmpDir.empty()) {
            mgr->superLogger()(ZQ::common::Log::L_WARNING, CLOGFMT(CRM_SymbolServer, "Got invalid conf: DBRoot(%s) TMPDir(%s)"), dbRoot.c_str(), tmpDir.c_str(), conffile.c_str());
            return false;
        }
        gDownloader = new SymbolDownloader(dbRoot, tmpDir);
        LOG(ZQ::common::Log::L_INFO, "Created SymbolServer with:DBRoot(%s) TMPDir(%s)", dbRoot.c_str(), tmpDir.c_str());
        mgr->registerContentHandler(DownloadURI, gDownloader);
        return true;
    }

    __EXPORT void CRM_Entry_Uninit(CRG::ICRMManager* mgr)
    {
        if(NULL == gDownloader) {
            return;
        }
        mgr->unregisterContentHandler(DownloadURI, gDownloader);
        delete gDownloader;
        gDownloader = NULL;
        delete gLog;
        gLog = NULL;
    }
}
