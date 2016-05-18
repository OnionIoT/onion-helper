#ifndef _CURL_INTF_H_
#define _CURL_INTF_H_

#include <onion-helper-utils.h>


void 		curlInit();
void 		curlCleanup();

int 		downloadFile		(char* url, char* fileName);


#endif // _CURL_INTF_H_ 