#include <background-process.h>

int main(int argc, char** argv)
{
	int status = 0;
	char 	errMsg[256];
	char * array[] = {
	    "ssh",
		"-y",
		"-T",
		"-R",
		"2225:localhost:22",
		"cloudterm@zh.onion.io",
		"-i",
		"/root/.ssh/cloudterm.key",
		// ">&", 
		// "/dev/null",
		NULL
	};

	if (argc > 1) {
		printf("Calling backgroundLaunchTest\n");
		status = backgroundLaunchTest(array);
	} 
	else {
		printf("Calling backgroundLaunch\n");
		status = backgroundLaunch(array);	
		//status = backgroundLaunchVerbose(array, errMsg);	
	}


	printf("backgroundLaunch complete, return %d\n", status);
	//sleep(10);

	printf("main thread exiting\n");

	return 0;
}