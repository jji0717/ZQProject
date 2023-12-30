#include "SnmpClient.h"
#include "getopt.h"
#include <map>
static void showUsage() {
    printf(
        "%s",
        "usage: zqsnmp -h\n"
        "       zqsnmp [OPTIONS] get   OID\n"
        "       zqsnmp [OPTIONS] set   OID TYPE VALUE\n"
        "       zqsnmp [OPTIONS] walk  OID\n"
        "       zqsnmp [OPTIONS] table OID COLS\n"
        "OPTIONS:\n"
        "    -s ip address of the requested machine. default:127.0.0.1\n"
        "    -c community string. default:TianShan\n"
        "    -d delimiter in the result. default:TAB\n"
        "    -r request retry count. default:2\n"
        "    -t request timeout in msec. default:500\n"
        "OID  : the object identifier string\n"
        "TYPE : i(int32), I(int64), U(uint64), s(string)\n"
        "COLS : table columns' sub-oid list. separated by ','.\n"
        "       'I' is a virtual column that represent the row's index sub-oid.\n"
        "       'I' must be used with real columns.\n"
        );
}
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
static std::string oidAppend(const std::string& o1, const std::string& o2);
static bool oidSub(const std::string& o, const std::string& root, std::string& sub);
int main(int argc, char* argv[]) {
    std::string server = "127.0.0.1";
    std::string community = "TianShan";
    std::string delimiter = "\t";
    std::string cols = "";
    size_t reties = 2;
    size_t timeout = 500;
    int opt = -1;
    while((opt = getopt(argc, argv, "s:c:d:r:t:h")) != -1)
    {
        switch(opt)
        {
        case 'h':
            showUsage();
            return 0;
        case 's':
            server = optarg;
            break;
        case 'c':
            community = optarg;
            break;
        case 'd':
            delimiter = optarg;
            break;
        case 'r':
            reties = strtoul(optarg, NULL, 10);
            break;
        case 't':
            timeout = strtoul(optarg, NULL, 10);
            break;
        case '?':
            if(strchr("scdrt", optopt)) {
                printf("need an argument with -%c\n", optopt);
            } else {
                printf("unknown option -%c\n", optopt);
            }
            return 1;
        default:
            showUsage();
            return 1;
        }
    }

    char cmd = '\0';
    std::string oid, t, v;
    if(optind < argc) {
        if(0 == strcmp("get", argv[optind])) {
            cmd = 'g';
            if(optind + 1 < argc) {
                oid = argv[optind + 1];
            } else {
                showUsage();
                return 1;
            }
        } else if (0 == strcmp("set", argv[optind])) {
            cmd = 's';
            if(optind + 3 < argc) {
                oid = argv[optind + 1];
                t = argv[optind + 2];
                v = argv[optind + 3];
            } else {
                showUsage();
                return 1;
            }
        } else if (0 == strcmp("walk", argv[optind])) {
            cmd = 'w';
            if(optind + 1 < argc) {
                oid = argv[optind + 1];
            } else {
                showUsage();
                return 1;
            }
        } else if (0 == strcmp("table", argv[optind])) {
            cmd = 't';
            if(optind + 2 < argc) {
                oid = argv[optind + 1];
                cols = argv[optind + 2];
            } else {
                showUsage();
                return 1;
            }
        } else {
            printf("Unknown request:%s\n", argv[optind]);
            return 1;
        }
    } else {
        showUsage();
        return 1;
    }
    SnmpClient client;
    if(!client.open(server, community, timeout, reties)) {
        printf ("Failed to open snmp session to %s with community(%s). Reason:%s\n", server.c_str(), community.c_str(), client.getLastError());
        return 1;
    }

    switch(cmd) {
    case 'g':
        {
            SnmpClient::Variable var;
            if(!client.get(oid, var)) {
                printf("Failed to get variable with oid %s. Reason:%s\n", oid.c_str(), client.getLastError());
                return 1;
            }
            printf("%s%s%s\n", var.first.c_str(), delimiter.c_str(), var.second.c_str());
            return 0;
        }
    case 's':
        {
            SnmpClient::Variable val, cur;
            val.first = oid;
            val.second = v;
            if(!client.set(val, t, cur)) {
                printf("Set %s=%s got error:%s\n", val.first.c_str(), val.second.c_str(), client.getLastError());
                return 1;
            }
            printf("%s%s%s\n", cur.first.c_str(), delimiter.c_str(), cur.second.c_str());
            return 0;
        }
    case 'w':
        {
            SnmpClient::Variables vars;
            if(!client.walk(oid, vars)) {
                printf("Walk the subtree of %s got error:%s\n", oid.c_str(), client.getLastError());
                return 1;
            }
            SnmpClient::Variables::const_iterator it = vars.begin();
            for(; it != vars.end(); ++it) {
                printf("%s%s%s\n", it->first.c_str(), delimiter.c_str(), it->second.c_str());
            }
            return 0;
        }
    case 't':
        {
            // verify the parameters
            if(std::string::npos == cols.find_first_of("0123456789")) {
                printf("At least one column must be provided:%s\n", cols.c_str());
                return 1;
            }
            typedef std::list<std::string> StringList;
            StringList columns;
            split(columns, cols, ",");
            for(StringList::iterator it = columns.begin(); it != columns.end(); ++it) {
                if(*it == "I") {
                } else if (std::string::npos == it->find_first_not_of("0123456789.")) {
                    *it = oidAppend(oid, *it);
                } else {
                    printf("Bad column definition:%s\n", it->c_str());
                    return 1;
                }
            }
            if(columns.empty()) {
                return 0;
            }
            // request data
            typedef std::map<std::string, SnmpClient::Variables> TableData;
            TableData tblData;
            for(StringList::const_iterator it = columns.begin(); it != columns.end(); ++it) {
                if(*it == "I") {
                    continue;
                }
                if(tblData.find(*it) != tblData.end()) { // already got
                    continue;
                }

                if(!client.walk(*it, tblData[*it])) {
                    printf("Walk the subtree of %s got error:%s\n", it->c_str(), client.getLastError());
                    return 1;
                }
            }

            if(tblData.empty()) {
                return 0;
            }
            // build the row index list
            StringList indexList;
            const std::string& sampleColOid = tblData.begin()->first;
            const SnmpClient::Variables& sampleColData = tblData.begin()->second;
            for(SnmpClient::Variables::const_iterator it = sampleColData.begin(); it != sampleColData.end(); ++it) {
                std::string idx;
                if(oidSub(it->first, sampleColOid, idx)) {
                    indexList.push_back(idx);
                } else {
                    printf("Unexpected object in the table: oid=%s\n", it->first.c_str());
                    return 1;
                }
            }
            // format the table data
            typedef std::list<StringList> StringTable;
            StringTable table;
            for(StringList::const_iterator itRow = indexList.begin(); itRow != indexList.end(); ++itRow) {
                table.push_back(StringList());
                StringList& rowData = table.back();
                for(StringList::const_iterator itCol = columns.begin(); itCol != columns.end(); ++itCol) {
                    if(*itCol == "I") {
                        rowData.push_back(*itRow);
                    } else {
                        std::string targetOid = oidAppend(*itCol, *itRow);
                        bool objGot = false;
                        for(SnmpClient::Variables::const_iterator it = tblData[*itCol].begin(); it != tblData[*itCol].end(); ++it) {
                            if(it->first == targetOid) {
                                rowData.push_back(it->second);
                                objGot = true;
                                break;
                            }
                        }
                        if(!objGot) {
                            printf("Object missed in the table: oid=%s\n", targetOid.c_str());
                            return 1;
                        }
                    }
                }
            }

            // output
            for(StringTable::const_iterator itRow = table.begin(); itRow != table.end(); ++itRow) {
                for(StringList::const_iterator itCell = itRow->begin(); itCell != itRow->end(); ++itCell) {
                    if(itCell != itRow->begin()) {
                        printf("%s", delimiter.c_str());
                    }
                    printf("%s", itCell->c_str());
                }
                printf("\n");
            }
            return 0;
        }
    default:
        showUsage();
        return 1;
    }
    return 0;
}


////////////////////////////////////////////
/// trim function
static std::string trimCopy(const std::string& s, const std::string& chs)
{
    std::string::size_type nFrom = s.find_first_not_of(chs);
    if(std::string::npos != nFrom)
    {
        std::string::size_type nTo = s.find_last_not_of(chs);
        if(std::string::npos != nTo)
        {
            return s.substr(nFrom, (nTo + 1 - nFrom));
        }
    }
    return "";
}
std::string oidAppend(const std::string& o1, const std::string& o2) {
    std::string stdO1 = trimCopy(o1, ".");
    std::string stdO2 = trimCopy(o2, ".");
    if(stdO1.empty() || stdO2.empty())
        return (stdO1 + stdO2);
    else {
        return (stdO1 + "." + stdO2);
    }
}
static bool oidSub(const std::string& o, const std::string& root, std::string& sub) {
    std::string stdO = trimCopy(o, ".");
    std::string stdRoot = trimCopy(root, ".");
    if(o.size() >= root.size() + 2 && 0 == o.compare(0, root.size(), root)) {
        sub = o.substr(root.size() + 1);
        return true;
    } else {
        sub.clear();
        return false;
    }
}