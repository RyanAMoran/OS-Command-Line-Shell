//Operating Systems
//Project 2
//Ryan Moran
// Custom implementation of a command line shell for an OS. Various commands possible, such as run, stop, continue, kill, etc.
// For example, the run the ls command, start the shell, then type "start ls" or "run ls", then press enter.

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>


int main (int argc, char *argv[] ){
	if(argv[1]!=NULL){ //make sure no command line arguments passed when starting myshell
		printf("myshell must be invoked without any arguments\n");
		exit(1);
	}

	char input[4096];
	char inputCopy[4096];
	char* words[100];
	char* fgetVal;
	char* p;
	int i=0;
	int j=0;
	int k=1;
	int newPID;
	int waitPID;
	int status = -1000;
	int hadError=0;
	while(1){
		if(fgetVal==NULL){
			break;
		}
		printf("myshell> ");
		i=0;
		j=0;
		k=1;
		hadError=0;
		status=-1000;
		fflush(stdout);
		fgetVal=fgets(input,1000,stdin);

		for(j=0;j<1000;j++){
			inputCopy[j]=input[j]; //need second identical input array for first if statement below
		}
		if(strtok(inputCopy, " \t\n")!=NULL){ //added this so that pressing enter works
		words[i]=strtok(input, " \t\n");
		//printf("%s\n", words[i]);
		//fflush(stdout);
		while(words[i]!=NULL){
			words[i+1] = strtok(NULL, " \t\n");
			i+=1;
		}
		words[i+1] = NULL;

		if(strcmp(words[0], "start")==0){ //everything for start user command
			if(words[1]==NULL){
				printf("myshell: Error. Missing job name for start.\n");
			}
			else if (strcmp(words[1], "myshell")==0){ //don't let them do shell within shell 
				printf("myshell: not a good idea to start myshell within myshell\n");
			}
			else{
				newPID = fork();
				if(newPID<0){ //failed fork
					printf("myshell: Fork failed\n");
				}
				else if(newPID==0){//you are the child
					if(execvp(words[1], &words[1]) < 0){
						printf("myshell: Unable to start %s: %s\n",words[1],strerror(errno));
					}
				}
				else if(newPID>0){// you are the parent
					printf("myshell: process %d started\n", newPID);
				}
				usleep(100000); //sleep needed so that next prompt line doesn't print before output of potential program being run like "start ls" for example. Without sleep ls command would print out on same line as next myshell > prompt
			}
		}
		else if (strcmp(words[0], "wait")==0){ //handle wait command
			while ((waitPID = wait(&status))>0){
				if (waitPID>0){
					break;
				}
			}
			if(waitPID>0 && WIFEXITED(status)){ //normal exit
				printf("myshell: Process with id %d exited normally with status %d\n", waitPID, status);
			}
			else if (waitPID>0 && !(WIFEXITED(status))){ //did not exit normally
				printf("myshell: Process with id %d exited abornmally with signal: %d : %s \n", waitPID, status, strsignal(status));
			}
			else{
				printf("myshell: No processes left.\n");
			}
		}
		else if (strcmp(words[0], "run")==0){ //handle run user command
			if(words[1]==NULL){
				printf("myshell: Error. Missing job name for run.\n");
				hadError=1;
			}
			else if (strcmp(words[1], "myshell")==0){ //again don't allow shell within shell
				printf("myshell: not a good idea to run myshell within myshell\n");
				hadError=1;
			}
			else{
				newPID = fork();
				if (newPID<0){ //failed fork
					printf("myshell: Fork failed\n");
				}
				else if(newPID==0){ //if you are the child
					if(execvp(words[1], &words[1]) < 0){
						printf("myhsell: Unable to run %s: %s\n",words[1],strerror(errno));
						hadError=1;
					}
				}
				else if(newPID>0){ //you are the parent
					printf("myshell: process %d started\n", newPID);
					fflush(stdout);
				}
			}
			while ((waitPID = waitpid(newPID, &status,0))>0){ //wait portion of run command
				if (waitPID>0){
					break;
				}
			}
			if(hadError!=1){
				if((waitPID>0 && WIFEXITED(status))){
					printf("Process with id %d exited normally with status %d\n", waitPID, status);
				}
				else if (waitPID>0 && !(WIFEXITED(status))){
					printf("Process with id %d exited abornmally with status: %d : %s \n", waitPID, status, strsignal(status));
				}
				else{
					printf("No processes left.\n");
				}
			}
		}
		else if (strcmp(words[0], "kill")==0){ //handle kill user command
			if(words[1]==NULL){
				printf("myshell: Error. Missing process id argument for kill.\n");
			}
			else{
				while(words[k]!=NULL){ //loop allows for killing multiple processes at once with one kill call: kill 123 456 ...
					errno=0; //errno needs to be reset each time through loop; without it gets stuck on !=0 below
					strtol(words[k], &p, 0); //p will be '\0' if every character of words[k] was processed, i.e. was a number
					if(*p!='\0'){// skip arguments to kill that are not pid numbers
						printf("myshell: Skipping illegal pid '%s'\n", words[k]);
					}
					else{
						if (kill(atoi(words[k]), 0) ==0 && errno==0){
							kill(atoi(words[k]), SIGKILL);
							printf("myshell: Process %s killed.\n", words[k]);
						}
						else if (errno!=0){
							printf("myshell: Error for %s : %s \n", words[k], strerror(errno));
						}
					}
					k+=1;
				}
			}
		}
		else if (strcmp(words[0], "stop")==0){ //handle stop user command
			if(words[1]==NULL){
				printf("myshell: Error. Missing process id argument for stop.\n");
			}
			else{
				while(words[k]!=NULL){ //loop allows for stopping multiple processes at once.
					errno = 0;
					strtol(words[k], &p, 0); //p will be '\0' if every character of words[k] was processed, i.e. was a number
					if(*p!='\0'){// skip arguments to kill that are not pid numbers
						printf("myshell: Skipping illegal pid '%s'\n", words[k]);
					}
					else{
						if(kill(atoi(words[k]), 0)==0 && errno==0){
							kill(atoi(words[k]), SIGSTOP);
							printf("myshell: Process %s stopped.\n", words[k]);
						}
						else if (errno!=0){
							printf("myshell: Error for %s : %s \n", words[k], strerror(errno));
						}
					}
					k+=1;
				}
			}
		}
		else if (strcmp(words[0], "continue")==0){//handle user continue command
			if(words[1]==NULL){
				printf("myshell: Error. Missing process id argument for stop.\n");
			}
			else{
				while(words[k]!=NULL){
					errno=0;
					strtol(words[k], &p, 0); //p will be '\0' if every character of words[k] was processed, i.e. was a number
					if(*p!='\0'){// skip arguments to kill that are not pid numbers
						printf("myshell: Skipping illegal pid '%s'\n", words[k]);
					}
					else{
						if(kill(atoi(words[k]),0)==0 && errno==0){
							kill(atoi(words[k]), SIGCONT);
							printf("myshell: Process %s continued.\n", words[k]);
						}
						else if (errno!=0){
							printf("myshell: Error for %s : %s \n", words[k], strerror(errno));
						}
					}
					k+=1;
				}
			}
		}
		else if (strcmp(words[0], "quit")==0 || strcmp(words[0], "exit")==0){ //handle user exit commands
			exit(1);
		}
		else{ //catch all other illegal commands
			printf("myshell: Unreognized shell command\n");
		}
		}
	}
	
	
	exit (0);
}
