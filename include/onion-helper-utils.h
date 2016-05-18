#ifndef _ONION_HELPER_UTILS_H_
#define _ONION_HELPER_UTILS_H_


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>


#define _ONION_HELPER_DEBUG

#ifdef _ONION_HELPER_DEBUG
	#define OH_DBG_PRINT(a,...)		printf(a,##__VA_ARGS__)
#else
	#define OH_DBG_PRINT(a,...)
#endif


#define BUFFER_SIZE 		1024

#endif // _ONION_HELPER_UTILS_H_ 