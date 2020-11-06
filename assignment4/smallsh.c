/***************************************************************
 * Filename: smallsh.c
 * Author: Wei Huang
 * Date: 11/19/2019
 * Description: CS 344 assignment4
 *reference: https://github.com/palmerja-osu/cs344/blob/master/smallsh.c
 						https://github.com/brentirwin/cs344-smallsh/blob/master/smallsh.c

***************************************************************/



#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>


#define MAX_LENGTH 2048
#define MAX_ARGS 512

int foregroundOnly = 0;


//ctrl + c exit signal
void catchSIGINT(int signo) {
	puts("\nForeground signal terminating.\n");
	fflush(stdout);
	waitpid(signo);
}


//ctrl + z exit signal
void catchSIGTSTP(int signo) {
	if (foregroundOnly == 0) {
		puts("\nEntering foreground-only mode (& is now ignored)\n");
		fflush(stdout);
		foregroundOnly = 1;
	}

	else {
		puts("\nExiting foreground-only mode\n");
		fflush(stdout);
		foregroundOnly = 0;
	}
}


//remaining processes
void exit_oper(int numBgProcesses, int bgProcesses[]){
	int i = 0;
	if (numBgProcesses > 0) {
		for (i = 0; i < numBgProcesses; i++) {
			kill(bgProcesses[i], SIGKILL);
			printf("Bg process terminated\n");
			fflush(stdout);
		}
	}
		exit(0);
}


//this function is for change directory
void cd_operation(int Args, char* args[]){
	if (Args == 1 || Args == 2)
		if (Args == 1) {
			if (chdir(getenv("HOME"))) {
				perror("Cannot find HOME environment variable.\n");
			}
		}
		else {
			if (chdir(args[1])) {
				perror("Cannot find directory.\n");
			}
		}
}


// print exit value and terminated sinal
void print_exit(int shellStatus){
	if (WIFEXITED(shellStatus)) {
		printf("exit value %d\n", WEXITSTATUS(shellStatus));
		fflush(stdout);
	}
	else {
		printf("terminated by signal %d\n", WTERMSIG(shellStatus));
		fflush(stdout);
	}
}



// '<' within bounds
void input_file(char* inputFile, char* args[], int shellStatus,int inputIndex){
	int sourceFD,inputResult;

	inputFile = args[inputIndex + 1];
	sourceFD = open(inputFile, O_RDONLY);
	if (sourceFD == -1) {
	  printf("cannot open %s for input\n", inputFile);
	  fflush(stdout);
	  shellStatus = 1;
	  exit(1);
	}
	else {
	  inputResult = dup2(sourceFD, 0);
	  if (inputResult == -1) { perror("dup2()"); shellStatus = 1; }
	  close(sourceFD);
	}
}

// '>' within bounds
void out_file(char* outputFile, char* args[], int shellStatus,int outputIndex){
	int targetFD, outputResult;
	outputFile = args[outputIndex + 1];
	targetFD = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (targetFD == -1) { perror("target open()"); shellStatus = 1; }
	outputResult = dup2(targetFD, 1);
	if (outputResult == -1) { perror("dup2()"); shellStatus = 1; }
	close(targetFD);
}

//if no input specified
void not_inp(char* inputFile, int shellStatus){
	int sourceFD, inputResult;
	inputFile = "/dev/null";
	sourceFD = open(inputFile, O_RDONLY);
	if (sourceFD == -1) { perror("source open()"); shellStatus = 1; }
	inputResult = dup2(sourceFD, 0);
	if (inputResult == -1) { perror("dup2()"); shellStatus = 1; }
	close(sourceFD);
}

// output of no input specified
void not_output(char* outputFile, int shellStatus){
	int targetFD, outputResult;
	outputFile = "/dev/null";
	targetFD = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (targetFD == -1) { perror("target open()"); shellStatus = 1; }
	outputResult = dup2(targetFD, 1);
	if (outputResult == -1) { perror("dup2()"); shellStatus = 1; }
	close(targetFD);
}

//check comment
void ec_bgpro(char* filteredArgs[]){
	execvp(filteredArgs[0], filteredArgs);
	printf("Could not find command\n");
	fflush(stdout);
	exit(1);
}

//check comment only take 1st argument
void check_arg(char* args[],char *ioArg[]){

	ioArg[0] = args[0];
	execvp(args[0], ioArg);
	printf("%s: no such file or directory\n", args[0]);
	fflush(stdout);
	exit(1);
}

//check normal comment
void normal_co(char* args[]){
	execvp(args[0], args);
	printf("%s: no such file or directory\n", args[0]);
	fflush(stdout);
	exit(1);
}



void check_term(pid_t childPid, int shellStatus, int numBgProcesses){
	childPid = waitpid(-1, &shellStatus, WNOHANG);
	while (childPid > 0) {

		if (WIFEXITED(shellStatus)) {
			printf("background pid %d is done: exit value %d\n", childPid, WEXITSTATUS(shellStatus));
			fflush(stdout);
			numBgProcesses--;
		}

		else if (WIFSIGNALED(shellStatus)) {
			printf("background pid %d is done: terminated by signal %d\n", childPid, WTERMSIG(shellStatus));
			fflush(stdout);
			numBgProcesses--;
		}
		childPid = waitpid(-1, &shellStatus, WNOHANG);
	}

}


// main function, the make the bash system can run and put all check function together
int main() {
	int Args, inputIndex, outputIndex, pidIndex, pid, pidLen, lenWithPid, i, j;
	int numBgProcesses = 0;
	int bgProcesses[MAX_ARGS];
	int shellStatus = 0;
	char *str, *inputFile, *outputFile;
	char input[MAX_LENGTH];
	char *filteredArgs[MAX_ARGS];
	char *args[MAX_ARGS];
	char pidBuffer[50];
	char* argWithPid;
	char *ioArg[1];
  bool isBackgroundProcess;
  pid_t childPid;

	struct sigaction SIGINT_action = { 0 }, SIGTSTP_action = { 0 };

	//ctrl c
	SIGINT_action.sa_handler = catchSIGINT;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;

//ctrl z
	SIGTSTP_action.sa_handler = catchSIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = 0;

	sigaction(SIGINT, &SIGINT_action, NULL);
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

// creat the bash system
	while (1) {
		Args = 0;
		inputIndex = outputIndex = pidIndex = -1;
		isBackgroundProcess = false;
		printf(": ");
		fflush(stdout);	fflush(stdin);
		fgets(input, MAX_LENGTH, stdin);
		if (input != NULL)
			str = strtok(input, " \n");
			if (str != NULL && str[0] != '#') {
		    while (str != NULL) {
					if (strcmp(str, "<") == 0) {
						inputIndex = Args;
					}
					else if (strcmp(str, ">") == 0) {
						outputIndex = Args;
					}
		      if (strlen(str) > 1) {
		        for (i = 1; i < strlen(str); i++) {
		          if ((str[i - 1] == '$') && (str[i] == '$')) {
		            pidIndex = i - 1;
		            break;
		          }
		        }
		      }
		      if (pidIndex >= 0) {
		        pid = getpid();
		        snprintf(pidBuffer, 50, "%d", pid);
		        pidLen = strlen(pidBuffer);
		        lenWithPid = strlen(pidBuffer) + strlen(str) - 2;
		        argWithPid = malloc(lenWithPid * sizeof(char));
		        memset(argWithPid, '\0', lenWithPid);
		        for (i = 0; i < pidIndex; i++)
		          argWithPid[i] = str[i];
		        j = 0;
		        for (i = pidIndex; i <= pidLen; i++)
		          argWithPid[i] = pidBuffer[j++];
		        if (pidIndex + pidLen < lenWithPid) {
		          j = pidIndex + 2;
		          for (i = pidIndex + pidLen; i < lenWithPid; i++)
		            argWithPid[i] = str[j++];
		        }
		        str = argWithPid;
		      }
		      args[Args] = str;
		      str = strtok(NULL, " \n");
		      Args++;
		    }
		    args[Args] = str;

		    if (strcmp(args[0], "exit") == 0) {
					exit_oper(numBgProcesses, bgProcesses);
		    }
		    else if (strcmp(args[0], "cd") == 0) {
					cd_operation(Args, args);
		    }
		    else if (strcmp(args[0], "status") == 0) {
					print_exit(shellStatus);
		    }
		    else {
		      if (strcmp(args[Args - 1], "&") == 0) {
		        isBackgroundProcess = true;
		       	for (i = 0; i < Args - 1; i++) {
		          filteredArgs[i] = args[i];
		        }
		        Args--;
		      }
		      pid_t childPid = -5;
		      childPid = fork();
		      if (childPid == -1) {
		        perror("Hull breach!\n");
		        exit(1);
		      }
		      else if (childPid == 0) {

		        if (inputIndex >= 0 && inputIndex < Args - 1) {
							input_file(inputFile, args, shellStatus, inputIndex);
		        }
		        if (outputIndex >= 0 && outputIndex < Args - 1) {
							 out_file(outputFile, args, shellStatus, outputIndex);
		        }

		        if (foregroundOnly == 0) {
		          if (isBackgroundProcess == true && inputFile != NULL) {
								not_inp(inputFile, shellStatus);
		          }
		          if (isBackgroundProcess == true && outputFile != NULL) {
		            not_output( outputFile, shellStatus);
		          }
		        }
		        if (isBackgroundProcess == true) {

							ec_bgpro(filteredArgs);
		        }
		        if ((inputIndex >= 0) || (outputIndex >= 0)) {
		         check_arg(args, ioArg);
		        }
		        else {
							normal_co(args);
		        }
		      }
		      if (isBackgroundProcess == false || foregroundOnly != 0) {
		        waitpid(childPid, &shellStatus, 0);
		      }
		      else {
		        printf("background pid is %d\n", childPid);
		        fflush(stdout);
		        bgProcesses[numBgProcesses] = childPid;
		        numBgProcesses++;
		        fflush(stdout);
		      }

		      if (WIFEXITED(shellStatus)) {
		        WEXITSTATUS(shellStatus);
		      }
		      else {
		        WTERMSIG(shellStatus);
		      }
		    }
		  }
			check_term( childPid, shellStatus, numBgProcesses);
	}

	if (argWithPid != NULL)
		free(argWithPid);
	return 0;
}
