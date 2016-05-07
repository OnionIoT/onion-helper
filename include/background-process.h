#ifndef _BACKGROUND_PROCESS_H_
#define _BACKGROUND_PROCESS_H_

#include <onion-helper-utils.h>

#include <unistd.h>
#include <errno.h>


int 		backgroundLaunch		(char** commandArray);


// testing code
#ifdef _TESTING_
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int 		backgroundLaunchTest	(char** commandArray);
int 		backgroundLaunchVerbose	(char** commandArray, char* msg);
#endif

#endif // _BACKGROUND_PROCESS_H_ 