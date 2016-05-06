#include <background-process.h>

// function prototypes
void 	backgroundProcess 		(char **commandArray);


// function to be run in child process:
//	executes the command contained in the commandArray argument
void backgroundProcess (char **commandArray)
{
	int 	status;
	int 	errorNum;
	OH_DBG_PRINT("Spawn Process:\n");

	// DBG
	status 	= 0;
	OH_DBG_PRINT("thread: argument listing:\n");
	while (commandArray[status] != NULL) {
		OH_DBG_PRINT("argument[%d] = %s\n", status, commandArray[status]);
		status++;
	}

	// cmd:  ssh -T -R 2228:localhost:22 cloudterm@zh.onion.io -i /root/.ssh/cloudterm.key
	//status = execl("/usr/bin/ssh", "/usr/bin/ssh", "-T", "-R", "2228:localhost:22", "cloudterm@zh.onion.io", "-i", "/root/.ssh/cloudterm.key", (char *) 0);
	//status = execlp("ssh", "ssh", "-y", "-T", "-R", "2228:localhost:22", "cloudterm@zh.onion.io", "-i", "/root/.ssh/cloudterm.key", (char *) 0);
	return execvp(commandArray[0], commandArray);
	
	/*errorNum = errno;

	OH_DBG_PRINT("execl returned: %d\n", status);
	OH_DBG_PRINT("error: %d\n", errorNum);
	OH_DBG_PRINT("error: %s\n", strerror( errorNum ));

	return;*/
}



// function to fork a process and run a command in it
int backgroundLaunch(char** commandArray)
{
	pid_t  	pid;

	// creat a new process
	pid 	= fork();

	if (pid == 0) {
		backgroundProcess(commandArray);
	}
	else {
		OH_DBG_PRINT("> Returning to main thread\n");
	}

	return (int)pid;
}
