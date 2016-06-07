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

	// close stdin and open /dev/null
	// fclose(stdin);
	// if( open("/dev/null", O_RDONLY) != 0) {
	// 	//syslog(LOG_ERR, "failed to open /dev/null");
	// 	printf("failed to open /dev/null");
	// 	exit(EXIT_FAILURE);
	// }

	// DBG
	// status 	= 0;
	// OH_DBG_PRINT("thread: argument listing:\n");
	// while (commandArray[status] != NULL) {
	// 	OH_DBG_PRINT("argument[%d] = %s\n", status, commandArray[status]);
	// 	status++;
	// }

	// execute the command
	execvp(commandArray[0], commandArray);

	// free the memory used by the command array
	free(commandArray);
}



// function to fork a process and run a command in it
int backgroundLaunch(char** commandArray)
{
	pid_t  	pid;
	pid_t  	waitPidReturn;
	int 	status;

	// create a new process
	pid 	= fork();

	if (pid == 0) {
		//// child process
		// put the child into a new process group
		status = setpgid(0, 0);
		OH_DBG_PRINT("child process, setpgid returned %d\n", status);

		// execute the command
		backgroundProcess(commandArray);

		OH_DBG_PRINT("child process: backgroundProcess returned\n");
	}
	else {
	#ifdef _TESTING_	
		waitPidReturn 	= waitpid(pid, &status, WNOHANG);
		OH_DBG_PRINT("waitpid returns: %d, status is %d\n", (int)waitPidReturn, status);
	#endif
		OH_DBG_PRINT("> Returning to main thread\n");
	}

	return (int)pid;
}




//// TESTING CODE
#ifdef _TESTING_
// function to fork a process and run a command in it
int backgroundLaunchTest(char** commandArray)
{
	pid_t  	pid;


	// execute the command
	backgroundProcess(commandArray);

	return (int)pid;
}

// testing code: useful for checking the status of a child process
int backgroundLaunchVerbose(char** commandArray, char* msg)
{
	pid_t  	pid;
	pid_t  	waitPidReturn;
	int 	status;

	// create a new process
	pid 	= fork();

	if (pid == 0) {
		//// child process
		// put the child into a new process group
		status = setpgid(0, 0);
		printf("child process, setpgid returned %d\n", status);

		// execute the command
		backgroundProcess(commandArray);
	}
	else {
		// dropping in code //
		printf("Child has pid %ld\n", (long)pid);

		    if (wait(&status) == -1)
		    {
		      perror("wait()");
		    }
		    else
		    {
		      // did the child terminate normally? //
		      if(WIFEXITED(status))
		      {
		        sprintf(msg, "%ld exited with return code %d\n",
		               (long)pid, WEXITSTATUS(status));
		      }
		      // was the child terminated by a signal? //
		      else if (WIFSIGNALED(status))
		      {
		        sprintf(msg, "%ld terminated because it didn't catch signal number %d\n",
		               (long)pid, WTERMSIG(status));
		        
		      }
		    }

		waitPidReturn 	= waitpid(pid, &status, WNOHANG);
		printf("waitpid returns: %d, status is %d\n", (int)waitPidReturn, status);
		OH_DBG_PRINT("> Returning to main thread\n");
	}

	return (int)pid;
}

#endif

