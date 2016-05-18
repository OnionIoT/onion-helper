#include <curl-intf.h>

#include "curl/curl.h"
//#include <unistd.h>
//#include <fcntl.h>


// global curl init
void curlInit() 
{
	curl_global_init(CURL_GLOBAL_SSL);
}

// global curl cleanup
void curlCleanup() 
{
	curl_global_cleanup();
}


// callback function to write to a file
static size_t _writeDataToFile(void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
	return written;
	
	// if (write((int)stream, ptr, size) >= 0) {
	// 	return size;
	// }
	// else {
	// 	return 0;
	// }
}

// function to download a file
int downloadFile(char* url, char* fileName)
{
	int 	status 	= -1;

	CURL 	*handle;
	CURLcode res;
	
	FILE 	*fileHandle;
	int 	fd;

	// init the curl session 
	handle = curl_easy_init();
 
	// set URL to get here
	_ONION_HELPER_DEBUG("downloading '%s' to '%s'\n", url, fileName);
	curl_easy_setopt(handle, CURLOPT_URL, url);
 
#ifdef _ONION_HELPER_DEBUG
	curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
#endif
 
 	// set options
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);	// disable progress meter
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, _writeDataToFile);	// send all data to this function  
 
	// open the file 
	fileHandle = fopen(fileName, "wb");
	if(fileHandle) {
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, fileHandle);	// write the data to this file handle

		// perform the download
		res = curl_easy_perform(handle);
		status = (int)res;

		// close the header file 
		fclose(fileHandle);
	}

	// fd = open(fileName, O_CREAT | O_WRONLY | O_TRUNC, 0666);
	// if(fd >= 0) {
	// 	curl_easy_setopt(handle, CURLOPT_WRITEDATA, fileHandle);	// write the data to this file handle
 
	// 	// perform the download
	// 	res = curl_easy_perform(handle);
	// 	status = (int)res;
 
	// 	// close the header file 
	// 	close(fd);
	// }
 
	// clean-up 
	curl_easy_cleanup(handle);
 
	return status;
}
