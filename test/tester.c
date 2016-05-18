#include <background-process.h>
#include <curl-intf.h>

int main(int argc, char** argv)
{
	int status = 0;
	int errorNum;
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

	curlInit();

	if (argc > 1) {

		if (strcmp(argv[1], "background") == 0) {
			if (argc > 2) {
				#ifdef _TESTING_
				printf("Calling backgroundLaunchTest\n");
				status = backgroundLaunchTest(array);
				#endif
			} 
			else {
				printf("Calling backgroundLaunch\n");
				status = backgroundLaunch(array);	
				//status = backgroundLaunchVerbose(array, errMsg);	
			}


			printf("backgroundLaunch complete, return %d\n", status);
			//sleep(10);
		}
		else if (strcmp(argv[1], "download") == 0) {
			printf("running download function: from url '%s' to '%s'\n", argv[2], argv[3]);

			status = downloadFile(argv[2], argv[3], &errorNum);

			printf("downloadFile status = %d\n", status);
		}
	}

	curlCleanup();
	printf("main thread exiting\n");

	return 0;
}