#include <string>
#include <curl/curl.h>
#include <sstream>
#include <algorithm>
#include <fcntl.h>
#include <sys/stat.h>
//#include <unistd.h>

using namespace std;
static char errorBuffer[CURL_ERROR_SIZE];
static int writer(char *, size_t, size_t, string *);
static int writerFile(char *, size_t, size_t, FILE *);
static bool init(CURL *&, char *,string *);

long chunkBegin(const void *transfer_info, void *ptr, int remains)
{
  return -1;
}
long chunkEnd(void *ptr)
{
 return -1;
}
struct WriteThis {
	const char *readptr;
	int sizeleft;
};

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
	struct WriteThis *pooh = (struct WriteThis *)userp;

	if(size*nmemb < 1)
		return 0;

	if(pooh->sizeleft) {
		*(char *)ptr = pooh->readptr[0]; /* copy one single byte */
		pooh->readptr++;                 /* advance pointer */
		pooh->sizeleft--;                /* less data left */
		return 1;                        /* we return 1 byte at a time! */
	}

	return 0;                          /* no more data left to deliver */
}

const std::string XML_HEADER = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";

void transferBuf(std::string& buffer)
{
	std::ostringstream buf;
    std::string paid = "cdntest1234567895521";
	std::string pid = "zq.com";
	std::string subFile= "index";
	int  bitRate = 375000;
	int  IngressCapacity = 1000000000;
	std::string _exclustionlist = "";
	int beginPos = 0;
	int endPos = -1;
	int transferDelay = 100;

	buf << XML_HEADER ;
	buf << "<LocateRequest>\n";
	buf << "  <Object>\n" ;
	buf << "    <Name>\n";
	buf << "     <AssetID>" << paid << "</AssetID>\n";
	buf << "     <ProviderID>" << pid << "</ProviderID>\n";
	buf << "    </Name>\n";
	buf << "    <SubType>"<< subFile << "</SubType>\n";
	buf << "  </Object>\n";
	buf << "  <TransferRate>"<< bitRate<< "</TransferRate>\n";
	buf << "  <IngressCapacity>" << IngressCapacity << "</IngressCapacity>\n";
	buf << "  <ExclusionList>";
	buf << _exclustionlist;
	buf << "  </ExclusionList>\n";
	buf << "  <TransferDelay>";
	buf << transferDelay;
	buf << "  </TransferDelay>\n";
	if(beginPos >= 0)
	{
		buf << "  <Range> ";
		buf << beginPos << " - ";
		if(endPos > 0)
			buf<< endPos ;
		buf << "\n  </Range>\n";
	}
	buf << "</LocateRequest>\n";
	buffer = buf.str();
	printf("Body:\n%s \n", buffer.c_str());
}
int pIoReadHeader_cb(char *data, size_t size, size_t nmemb, string* userdata)
{
//	printf("********HeadResponse: %s\n", ptr);
	unsigned long sizes = size * nmemb;
	if (userdata == NULL) return 0;
	userdata->append(data, sizes);
	return sizes;
}
void post()
{
	CURL *conn = NULL;
	CURLcode code;
	string buffer;

	std::string transBuf;
	transferBuf(transBuf);	

//	struct WriteThis pooh;
//	pooh.readptr = transBuf.c_str();
//	pooh.sizeleft = transBuf.size();

	curl_global_init(CURL_GLOBAL_DEFAULT);

	std::string strServer = "10.15.10.85";
	std::string strPort = "10080";
	std::string url = std::string("http://") + strServer + ":" + strPort + "/vodadi.cgi";

	if (!init(conn, (char*)url.c_str(),&buffer))
	{
		fprintf(stderr, "Connection initializion failed\n");
		exit(EXIT_FAILURE);
	}

	curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, writer);
	curl_easy_setopt(conn, CURLOPT_WRITEDATA, &buffer);

	curl_easy_setopt(conn, CURLOPT_POST, 1L);
//	curl_easy_setopt(conn, CURLOPT_READFUNCTION, read_callback);
//	curl_easy_setopt(conn, CURLOPT_READDATA, &pooh);
//  curl_easy_setopt(conn, CURLOPT_POSTFIELDSIZE, (curl_off_t)pooh.sizeleft);

	curl_easy_setopt(conn, CURLOPT_POSTFIELDS, transBuf.c_str());
	curl_easy_setopt(conn, CURLOPT_POSTFIELDSIZE, (curl_off_t)transBuf.size());

 //	curl_easy_setopt(conn, CURLOPT_POSTFIELDSIZE, -1);

	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "User-Agent: CPE User Agent");
	headers = curl_slist_append(headers, "Content-Type: application/atom+xml");
//	headers = curl_slist_append(headers, "Content-Length:"); //É¾³ýContent-Length: Header

//	curl_easy_setopt(conn, CURLOPT_RANGE,"100-");   

	curl_easy_setopt(conn, CURLOPT_HTTPHEADER, headers);

	code = curl_easy_perform(conn);
	if (code != CURLE_OK)
	{
		fprintf(stderr, "Failed to get '%s' [%s]\n", url.c_str(), errorBuffer);
		exit(EXIT_FAILURE);
	}


	long retcode = 0;
	code = curl_easy_getinfo(conn, CURLINFO_RESPONSE_CODE , &retcode); 
	if ( (code == CURLE_OK) && retcode != 200 )
	{
		printf("Error: locate file,retCode = (%d)\n", retcode);
	}

	struct curl_slist *chunkRes = NULL;
	long headerSize = 0;
	curl_easy_getinfo(conn, CURLINFO_HEADER_SIZE , &headerSize);

	char *ct;
	curl_easy_getinfo(conn, CURLINFO_CONTENT_TYPE, &ct);

	curl_easy_cleanup(conn);
	printf("%s\n",buffer.c_str());
}

void downLoadByChuck()
{
    CURL *conn = NULL;
	CURLcode code;
	string buffer;

	FILE* downloadfile = NULL;;
	downloadfile = fopen("c:\\testdownfile", "wb");
	if(downloadfile == NULL)
		return;
	fseek(downloadfile, 0,SEEK_SET );

	curl_global_init(CURL_GLOBAL_DEFAULT);

	std::string url = "http://10.15.10.85:12000/scs/getfile?file=cdntest1234567895521zq.com.FF&ic=1000000000&rate=16000000";
	if (!init(conn, (char*)url.c_str(),&buffer))
	{
		fprintf(stderr, "Connection initializion failed\n");
		exit(EXIT_FAILURE);
	}
    string bufHeader;
//	curl_easy_setopt(conn, CURLOPT_NOBODY, 1L);


	curl_easy_setopt(conn, CURLOPT_HEADERFUNCTION, pIoReadHeader_cb);   
	curl_easy_setopt(conn, CURLOPT_HEADERDATA, &bufHeader);

	curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, writerFile);
	curl_easy_setopt(conn, CURLOPT_WRITEDATA, downloadfile);

/*	std::string transBuf;
	transferBuf(transBuf);	

	curl_easy_setopt(conn, CURLOPT_POSTFIELDS, transBuf.c_str());
	curl_easy_setopt(conn, CURLOPT_POSTFIELDSIZE, (curl_off_t)transBuf.size());

	curl_easy_setopt(conn, CURLOPT_TRANSFERTEXT, transBuf.c_str());

	std::string chunkData;
	curl_easy_setopt(conn, CURLOPT_CHUNK_BGN_FUNCTION, chunkBegin);
	curl_easy_setopt(conn, CURLOPT_CHUNK_END_FUNCTION, chunkEnd);
	curl_easy_setopt(conn, CURLOPT_CHUNK_END_FUNCTION, &chunkData);*/


	struct curl_slist *chunk = NULL;
	chunk = curl_slist_append(chunk, "User-Agent: CPE User Agent");
	chunk = curl_slist_append(chunk, "Content-Type: application/atom+xml");

	curl_easy_setopt(conn, CURLOPT_RANGE,"100-");   

	curl_easy_setopt(conn, CURLOPT_HTTPHEADER, chunk);

	code = curl_easy_perform(conn);
	if (code != CURLE_OK)
	{
		fprintf(stderr, "Failed to get '%s' [%s]\n", url.c_str(), errorBuffer);
		exit(EXIT_FAILURE);
	}

	long retcode = 0;
	code = curl_easy_getinfo(conn, CURLINFO_RESPONSE_CODE , &retcode); 
	if ( (code == CURLE_OK) && retcode != 200 )
	{
		printf("Error: locate file,retCode = (%d)\n", retcode);
	}

    curl_easy_cleanup(conn);
//	printf("%s\n", buffer.c_str());

	if(downloadfile)
		fclose(downloadfile);
	printf("%s\n", bufHeader.c_str());
}

static size_t read_callbackput(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t retcode;

  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */
  retcode = fread(ptr, size, nmemb, (FILE*)stream);

  fprintf(stderr, "*** We read %d bytes from file\n", retcode);

  return retcode;
} 
int put()
{
  CURL *curl;
  CURLcode res;
  FILE * hd_src ;
  struct _stat file_info;

  char *file = "c:\\modserver.log";
  std::string strServer = "10.15.10.85";
  std::string strPort = "10080";
  std::string url = std::string("http://") + strServer + ":" + strPort + "/vodadi.cgi";

  /* get the file size of the local file */
  _stat(file,  &file_info);

  /* get a FILE * of the same file, could also be made with
     fdopen() from the previous descriptor, but hey this is just
     an example! */
  hd_src = fopen(file, "rb");

  /* In windows, this will init the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);

  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {

     /* enable uploading */
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    /* HTTP PUT please */
    curl_easy_setopt(curl, CURLOPT_PUT, 1L);

    /* specify target URL, and note that this URL should include a file
       name, not only a directory */
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /* we want to use our own read function */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callbackput);
    /* now specify which file to upload */
    curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);

    /* provide the size of the upload, we specicially typecast the value
       to curl_off_t since we must be sure to use the correct data size */
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)file_info.st_size);

	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

    /* Now run off and do what you've been told! */
    res = curl_easy_perform(curl);

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  fclose(hd_src); /* close the local file */

  curl_global_cleanup();
  return 0;
}
void get(std::string response)
{
  
}
int main()
{
  post();
//    put();
//  downLoadByChuck();
   return -1;
}

static bool init(CURL *&conn, char *url,string *p_buffer)
{
	CURLcode code;
	conn = curl_easy_init();
	if (conn == NULL)
	{
		fprintf(stderr, "Failed to create CURL connection\n");
		exit(EXIT_FAILURE);
	}
	code = curl_easy_setopt(conn, CURLOPT_ERRORBUFFER, errorBuffer);
	if (code != CURLE_OK)
	{
		fprintf(stderr, "Failed to set error buffer [%d]\n", code);
		return false;
	}
	code = curl_easy_setopt(conn, CURLOPT_URL, url);
	if (code != CURLE_OK)
	{
		fprintf(stderr, "Failed to set URL [%s]\n", errorBuffer);
		return false;
	}
	code = curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1);
	if (code != CURLE_OK)
	{
		fprintf(stderr, "Failed to set redirect option [%s]\n", errorBuffer);
		return false;
	}
	char* timeout = "20000";
	code = curl_easy_setopt(conn, CURLOPT_TIMEOUT_MS, timeout);
	if (code != CURLE_OK)
	{
		fprintf(stderr, "Failed to set timeout [%s]\n", timeout);
		return false;
	}
	char* conncetTimeout = "20000";
	code = curl_easy_setopt(conn, CURLOPT_CONNECTTIMEOUT_MS, conncetTimeout);
	if (code != CURLE_OK)
	{
		fprintf(stderr, "Failed to set connect timout [%s]\n", conncetTimeout);
		return false;
	}

	curl_easy_setopt(conn, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(conn, CURLOPT_HEADER, 1L);
	return true;
}

static int writer(char *data, size_t size, size_t nmemb, string *writerData)
{
	unsigned long sizes = size * nmemb;
	if (writerData == NULL) return 0;
	writerData->append(data, sizes);
	return sizes;
}
static int writerFile(char *data, size_t size, size_t nmemb, FILE* file)
{
	static int i = 0;
	unsigned int nWrite = 0;
	unsigned long sizes = size * nmemb;
	if(file)
		nWrite = fwrite(data, 1 , sizes, file);
	i += nWrite;
//	printf("%d  %d  %d total %d\n", nWrite, size, nmemb, i);
	return sizes;
}

