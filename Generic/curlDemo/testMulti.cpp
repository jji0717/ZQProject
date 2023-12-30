/*****************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * $Id: multi-single.c,v 1.6 2006-10-13 14:01:20 bagder Exp $
 *
 * This is a very simple example using the multi interface.
 */

#include <stdio.h>
/* curl stuff */
#include <curl/curl.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;
static char errorBuffer[CURL_ERROR_SIZE];
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
//	printf("Body:\n%s \n", buffer.c_str());
}
static int writer(char *data, size_t size, size_t nmemb, string *writerData)
{
	unsigned long sizes = size * nmemb;
	if (writerData == NULL) return 0;
	writerData->append(data, sizes);
	return sizes;
}
///CURLOPT_SOCKOPTFUNCTION 
//Pass a pointer to a function that matches the following prototype: 
//int function(void *clientp, curl_socket_t curlfd, curlsocktype purpose);.
//This function gets called by libcurl after the socket() call but before the connect() call. 
//The callback's purpose argument identifies the exact purpose for this particular socket
///CURLOPT_SOCKOPTDATA 
///Pass a pointer that will be untouched by libcurl and passed as the first argument 
///in the sockopt callback set with CURLOPT_SOCKOPTFUNCTION. 
int cbSocket(void *clientp, curl_socket_t curlfd, curlsocktype purpose)
{
	printf("cbSocket: %d, %d\n", curlfd, purpose);
	return 0;
}

///CURLOPT_OPENSOCKETFUNCTION  
///Pass a pointer to a function that matches the following prototype: 
///int function(void *clientp, curl_socket_t item);. 
///This function gets called by libcurl instead of the close(3) or closesocket(3) 
///call when sockets are closed (not for any other file descriptors). This is pretty much the reverse
///to the CURLOPT_OPENSOCKETFUNCTION option. Return 0 to signal success and 1 if there was an error. 
///CURLOPT_OPENSOCKETDATA 
///Pass a pointer that will be untouched by libcurl and passed as the first argument 
//in the opensocket callback set with CURLOPT_OPENSOCKETFUNCTION. 
curl_socket_t cbOpenSocket(void *clientp, curlsocktype purpose, struct curl_sockaddr *address)
{
	printf("cbOpenSocket: family:%d, socktype:%d, protocol:%d\n", address->family, address->socktype, address->protocol);
	return socket(address->family, address->socktype, address->protocol); 
}

///CURLOPT_CLOSESOCKETFUNCTION 
///Pass a pointer to a function that matches the following prototype: 
///int function(void *clientp, curl_socket_t item);.
///This function gets called by libcurl instead of the close(3) or closesocket(3) call 
///when sockets are closed (not for any other file descriptors). This is pretty much
///the reverse to the CURLOPT_OPENSOCKETFUNCTION option. 
///Return 0 to signal success and 1 if there was an error.

///CURLOPT_CLOSESOCKETDATA
///Pass a pointer that will be untouched by libcurl and passed as the first argument
///in the closesocket callback set with CURLOPT_CLOSESOCKETFUNCTION. 
static int cbSocketClosed(void *clientp, curl_socket_t item)
{
	printf("cbSocketClosed: %d\n", item);
	return item;
}
///CURLOPT_PROGRESSFUNCTION.
///Pass a pointer to a function that matches the following prototype: 
///int function(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow); . 
///This function gets called by libcurl instead of its internal equivalent with a frequent 
///interval during operation (roughly once per second or sooner) no matter if data is being
///transferred or not. Unknown/unused argument values passed to the callback will be set to 
///zero (like if you only download data, the upload size will remain 0). Returning a non-zero 
///value from this callback will cause libcurl to abort the transfer and return CURLE_ABORTED_BY_CALLBACK. 
///If you transfer data with the multi interface, this function will not be called during 
///periods of idleness unless you call the appropriate libcurl function that performs transfers. 
///CURLOPT_NOPROGRESS must be set to 0 to make this function actually get called.

///CURLOPT_PROGRESSDATA 
///Pass a pointer that will be untouched by libcurl and passed as the first argument 
///in the progress callback set with CURLOPT_PROGRESSFUNCTION. 
int cbProgress(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	printf("cbProgress: dltotal:%f, dlnow:%f, %f, ultotal:%f, ulnow:%f\n", dltotal, dlnow, ultotal, ulnow);
	return 0;
}
std::string bufHeader;
int pIoReadHeader_cb(char *data, size_t size, size_t nmemb, string* userdata)
{
	//	printf("********HeadResponse: %s\n", ptr);
	unsigned long sizes = size * nmemb;
	if (userdata == NULL) return 0;
	userdata->append(data, sizes);
	return sizes;
}
std::string sockebuf;
CURL * createConn(std::string url, std::string& buffer, std::string& transBuf)
{
	CURLcode code;
	CURL *curlHandle = NULL;
	curlHandle = curl_easy_init();

	if (curlHandle == NULL)
	{
		fprintf(stderr, "Failed to create CURL connection\n");
		return NULL;
	}
	curl_easy_setopt(curlHandle, CURLOPT_ERRORBUFFER, errorBuffer);
	curl_easy_setopt(curlHandle, CURLOPT_URL, (char*)url.c_str());
	curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, 1);
	char* timeout = "20000";
	curl_easy_setopt(curlHandle, CURLOPT_TIMEOUT_MS, timeout);
	char* conncetTimeout = "20000";
	curl_easy_setopt(curlHandle, CURLOPT_CONNECTTIMEOUT_MS, conncetTimeout);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, writer);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curlHandle, CURLOPT_POST, 1L);
	curl_easy_setopt(curlHandle, CURLOPT_POSTFIELDS, transBuf.c_str());
	curl_easy_setopt(curlHandle, CURLOPT_POSTFIELDSIZE, (curl_off_t)transBuf.size());

	curl_easy_setopt(curlHandle, CURLOPT_SOCKOPTFUNCTION, cbSocket);
	curl_easy_setopt(curlHandle, CURLOPT_SOCKOPTDATA, &sockebuf);

	curl_easy_setopt(curlHandle, CURLOPT_OPENSOCKETFUNCTION, cbOpenSocket);
	curl_easy_setopt(curlHandle, CURLOPT_OPENSOCKETDATA, &sockebuf);

	curl_easy_setopt(curlHandle, CURLOPT_CLOSESOCKETFUNCTION, cbSocketClosed);
    curl_easy_setopt(curlHandle, CURLOPT_CLOSESOCKETDATA, &sockebuf);

	curl_easy_setopt(curlHandle, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curlHandle, CURLOPT_PROGRESSFUNCTION, cbProgress);
	curl_easy_setopt(curlHandle, CURLOPT_PROGRESSDATA, &buffer);

	curl_easy_setopt(curlHandle, CURLOPT_HEADERFUNCTION, pIoReadHeader_cb);   
	curl_easy_setopt(curlHandle, CURLOPT_HEADERDATA, &bufHeader);

	struct curl_slist *chunk = NULL;
	chunk = curl_slist_append(chunk, "User-Agent: Test User Agent");
	chunk = curl_slist_append(chunk, "Content-Type: application/atom+xml");

	curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, chunk);

	return curlHandle;
}

int main(int argc, char **argv)
{

  CURL *http_handle1 = NULL;
  CURL *http_handle2 = NULL;
  CURL *http_handle3 = NULL;
  CURL *http_handle4 = NULL;
  CURLM *multi_handle;

  int still_running; /* keep number of running handles */

  std::string transBuf;
  transferBuf(transBuf);	
  std::string buf1, buf2, buf3, buf4;

  std::string strServer = "10.15.10.85";
  std::string strPort = "10080";
  std::string url = std::string("http://") + strServer + ":" + strPort + "/vodadi.cgi";

  curl_global_init(CURL_GLOBAL_DEFAULT);

  http_handle1 = createConn(url, buf1, transBuf);
//  http_handle2 = createConn(url, buf2, transBuf);
//  http_handle3 = createConn(url, buf3, transBuf);
 // http_handle4 = createConn(url, buf4, transBuf);

  /* init a multi stack */
  multi_handle = curl_multi_init();

  /* add the individual transfers */
  curl_multi_add_handle(multi_handle, http_handle1);
 // curl_multi_add_handle(multi_handle, http_handle2);
//  curl_multi_add_handle(multi_handle, http_handle3);
 // curl_multi_add_handle(multi_handle, http_handle4);

  /* we start some action by calling perform right away */
 // while(CURLM_CALL_MULTI_PERFORM ==
 //       curl_multi_perform(multi_handle, &still_running));

  still_running = 1;

  while (still_running > 0)
  {
	  if (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(multi_handle, &still_running))
		  continue;

	  if (still_running <=0)
		  break;

    struct timeval timeout;
    int rc; /* select() return code */

    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    int maxfd;

    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);

    /* set a suitable timeout to play around with */
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    /* get file descriptors from the transfers */
    curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

    /* In a real-world program you OF COURSE check the return code of the
       function calls, *and* you make sure that maxfd is bigger than -1 so
       that the call to select() below makes sense! */

	// TODO: initilize fdsets

    rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);

	if (rc > 0)
	{
		printf("*****read[%d], write[%d],excep[%d]*******\n", fdread.fd_count, fdwrite.fd_count,fdexcep.fd_count);
		printf("Fdread: ");
		for(int i = 0; i < fdread.fd_count; ++i)
		{
			printf("%d	", fdread.fd_array[i]);
		}
		printf("\nFdwrite:");
		for(int i = 0; i < fdwrite.fd_count; ++i)
		{
			printf("%d	", fdwrite.fd_array[i]);
		}
		printf("\nFdexcep:");
		for(int i = 0; i < fdexcep.fd_count; ++i)
		{
			printf("%d	", fdexcep.fd_array[i]);
		}
		printf("\n");

/*		int sockfd1 = 0; 
		curl_easy_getinfo(http_handle1, CURLINFO_LASTSOCKET, &sockfd1);
		printf("socket: %d\n", sockfd1);
		int sockfd2 = 0;
		curl_easy_getinfo(http_handle2, CURLINFO_LASTSOCKET, &sockfd2);
		int sockfd3 = 0;
		curl_easy_getinfo(http_handle3, CURLINFO_LASTSOCKET, &sockfd3);
		int sockfd4 = 0;
		curl_easy_getinfo(http_handle1, CURLINFO_LASTSOCKET, &sockfd4);

		printf("socket: %d, %d, %d, %d\n", sockfd1, sockfd2, sockfd3, sockfd4);*/
		/*	
		for each fdpair in fdIndex
			{
				if (FD_ISSET(fdpair.fd, &fdread))
					fdpair.client.OnDataArrived(...);

				if (FD_ISSET(fdpair.fd, &fdwrite))
					fdpair.client.OnDataSent(...);

				if (FD_ISSET(fdpair.fd, &fdexcep))
					fdpair.client.OnError(...);
			}
		*/
	}
	else if(rc < 0)
	{
		/* select error */
		still_running = 0;
		printf("select() returns error, this is badness\n");
		//quit();
	}
	else
	{
		/* timeout or readable/writable sockets */
		//printf log
	}
   
	/*if(!add)
	{
	 http_handle3 = curl_easy_init();
	 curl_easy_setopt(http_handle3, CURLOPT_URL, "http://www.haxx.se/");
	 curl_multi_add_handle(multi_handle, http_handle3);
	}
	else
	{
		http_handle4 = curl_easy_init();
		curl_easy_setopt(http_handle4, CURLOPT_URL, "http://www.haxx.se/");
		curl_multi_add_handle(multi_handle, http_handle4);
	}*/
  }// while

  long retcode = 0;

  if ( (curl_easy_getinfo(http_handle1, CURLINFO_RESPONSE_CODE , &retcode) != CURLE_OK))
  {
	  printf("failed to getinfo CURLINFO_RESPONSE_CODE\n");
  }
  else 
	  printf("StatusCode:%d\n", retcode);


  curl_multi_cleanup(multi_handle);

  if(http_handle1)
	  curl_easy_cleanup(http_handle1);
  if(http_handle2)
	  curl_easy_cleanup(http_handle2);
  if(http_handle3)
	  curl_easy_cleanup(http_handle3);
  if(http_handle4)
	  curl_easy_cleanup(http_handle4);

  printf("ERROR: %s \n", errorBuffer);
  printf("buf1: %s \n", buf1.c_str());
  printf("buf2: %s \n", buf2.c_str());
  printf("buf3: %s \n", buf3.c_str());
  printf("buf4: %s \n", buf4.c_str());
  printf("Header: %s", bufHeader.c_str());
  return 0;
}



/*	
	struct curl_waitfd waitfd[100];
	int numfds = sizeof(waitfd) / sizeof(struct curl_waitfd);

	curl_multi_wait(CURLM *multi_handle, waitfd, 0, 10000, &numfds); 
*/
