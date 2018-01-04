#include "includes.h"
#include "mainfunc.h"
#include "uniq.h"

int main(int argc, char** argv){

    char* cmd, prompt[256];
    printf("Welcome to the CShell project for Operating Systems I!\n");

    while(RUNNING){
        //Printing and storing the command symbol
		char* wd = (char*)malloc(sizeof(char) * 512);
		getcwd(wd, 512);
      	snprintf(prompt, sizeof(prompt), "\n%s$> ", wd);

        //Prompt for inputing the command
        cmd = readline(prompt);


        if(!cmd)
            break;

        //Adding command to history
        add_history(cmd);

        //If command is exit breaks the while else does other stuff
        if(strcmp(cmd, "exit") == 0){
            printf("\nBye!\n");
            break;
        }else{
            analyzeCommand(cmd);
        }


        //free(cmd);
    }


    return 0;
}

void analyzeCommand(char* cmd){
    int p = 0;
    int r = 0;
	char uniq[] = "uniq";
	

    if(strcmp(cmd, "ver") == 0){

        printf("\nCShell version: 0.1");

    }else{
        findSymbols(&p, &r, cmd);
		

		if(strstr(cmd, "cd") == cmd){

        	cdCommand(cmd);

		}else if(strncmp(cmd, uniq, 4) == 0){
			parseUniq(cmd);
		}else{
			if(p > 0 && r == 0){

				pipeCommands(&p, cmd);

			}else if(p == 0 && r == 1){

				redirectCommand(cmd);

			}else if(p == 0 && r == 0){

				linuxCommand(cmd);

			}
		}
    }
}

void redirectCommand(char* cmd){
	int saved_stdout = dup(1);

	char* com = (char*)malloc(sizeof(char) * strlen(cmd) + 1);
	memcpy(com, cmd, strlen(cmd) + 1);
	
	char *tok = strtok(com, ">");

	char* actualCom = (char*)malloc(sizeof(char) * strlen(tok) + 1);
	memcpy(actualCom, tok, strlen(tok) + 1);

	actualCom[strlen(tok)] = '\0';

	tok = strtok(NULL, ">");

	char* flname = (char*)malloc(sizeof(char) * 256);
	memcpy(flname, tok, strlen(tok));

	removeSpaces(flname);
	
	int file = open(flname, O_WRONLY | O_CREAT, 0666);
	
	dup2(file, STDOUT_FILENO);
	
	linuxCommand(actualCom);

	dup2(saved_stdout, 1);
	close(saved_stdout);
}

void removeSpaces(char* source){

  char* i = source;
  char* j = source;
  while(*j != 0){
    *i = *j++;

    if(*i != ' ')
      i++;

  }

  *i = '\0';
}

void cdCommand(char* cmd){
        char* com = (char*)malloc(sizeof(char) * strlen(cmd) + 1);

		memcpy(com, cmd, strlen(cmd) + 1);

		char* dir = (char*)malloc(sizeof(char) * 512);

        char *p = strtok(com, " ");
		p = strtok(NULL, " ");
		int i = 0;

		while(p){
			if(i==0){
				memcpy(dir, p, strlen(p) + 1);
				//dir[strlen(p)+1] == '\0';
			}else{
				strcat(dir, " ");
				strcat(dir, p);
			}	
			i += strlen(p);
			p = strtok(NULL, " ");
		}

		
        chdir(dir);
		
		free(com);
		free(dir);
}

int pipeCommands(int* p, char* cmd){
	
	int i;
	pid_t f;
	char** split = splitCommand(cmd, p);

	if((f = fork()) < 0){
		printf("Couldn't do first fork");
		return -2;
	}

	if(f > 0){ /* parent */
		wait(NULL);
		
		for(i = 0; i <= *p; i++)
			free(split[i]);
		free(split);
			
		
	}else{ /* child */		
		pid_t ch;
		int fd1, fd2;		

		i = 0;
		int pip[2];

		while(i <= *p){
			if(pipe(pip) < 0){
				printf("Couldn't pipe.");
				exit(0);
			}

			ch = fork();

			if(ch < 0){
				printf("Couldn't fork");
				exit(0);
			}

			if(ch == 0){
				fd2 = pip[1];

				
				if(i != 0){
					dup2(fd1, 0);
					close(fd1);
				}

				if(i != *p){
					dup2(fd2, 1);
					close(fd2);
				}

				
				linuxCommand(split[i]);

				exit(0);
			}

			close(pip[1]);

			fd1 = pip[0];

			wait(NULL);
			i++;
		}


		return 0;	
	}
	
	wait(NULL);
	return 0;
}

char** splitCommand(char* cmd, int* p){
	char* copy = (char*)malloc(sizeof(char) * strlen(cmd) + 1);
	memcpy(copy, cmd, strlen(cmd) + 1);

	char** result;
	int i;

	result = (char**)malloc(sizeof(char*) * (*p));
		

	char* tok = strtok(copy, "|");
	i = 0;
	while(tok != NULL){
		result[i] = (char*)malloc(sizeof(char) * strlen(tok));
		memcpy(result[i], tok, strlen(tok) + 1);
		result[strlen(tok)] = '\0';
		i++;
		tok = strtok(NULL, "|");
	}

	free(copy);
	free(tok);

	return result;
}


void findSymbols(int* p, int* r, char* cmd){
    char* copy = (char*)malloc(strlen(cmd) + 1);
    memcpy(copy, cmd, strlen(cmd) + 1);

	char* tok = strtok(copy, "|");
	
	while(tok != NULL){
		*p += 1;		
		tok = strtok(NULL, "|");	
	}

	*p -= 1;

	free(tok);
	free(copy);
	copy = (char*)malloc(strlen(cmd) + 1);
    memcpy(copy, cmd, strlen(cmd) + 1);

	tok = strtok(copy, ">");	

	while(tok != NULL){
		*r += 1;
		tok = strtok(NULL, ">");
	}

	*r -= 1;

	free(tok);
	free(copy);
}

void childLinuxCommand(char* cmd){
        char* com = (char*)malloc(sizeof(char) * strlen(cmd) + 1);

		memcpy(com, cmd, strlen(cmd) + 1);

        //For strtok purposes
        char* com2 = (char*)malloc(sizeof(char) * strlen(cmd) + 1);

		memcpy(com2, cmd, strlen(cmd) + 1);

        int count = 0;

        char sep[] = " ";

        char* p = strtok(com, sep);

        //Counting the words for arg list
        while(p){
            count++;
            p = strtok(NULL, sep);
        }

        char* arg[count + 1]; //+1 for NULL

        arg[count] = NULL;

        char* pp = strtok(com2, sep);
        int i = 0;

        //Putting arguments into an array
        while(pp){
            arg[i++] = pp;
            pp = strtok(NULL, sep);
        }


        //Execvp with NULL terminated argument list
		
        execvp(arg[0], arg);

        exit(1);
}


void linuxCommand(char* cmd){
    int fd[2];
    pid_t pid;

    if(pipe(fd) == -1){
        printf("Couldn't create pipe");
        exit(1);
    }



    if((pid = fork()) == -1){
        printf("Couldn't fork");
        exit(1);
    }

    if(pid == 0){
        //Child

        int size;
        char* com;

        //For strtok purposes
        char* com2;

        close(fd[1]);

        //Reading the size of the string
        read(fd[0], &size, sizeof(size));

        com = (char*)malloc(sizeof(char) * size + 1);
        com2 = (char*)malloc(sizeof(char) * size + 1);

        //Reading the string and null terminating it
        read(fd[0], com, size + 1);
        com[size] = '\0';

        close(fd[0]);

        //Copy for strtok
        strcpy(com2, com);

        int count = 0;

        char sep[] = " ";

        char* p = strtok(com, sep);

        //Counting the words for arg list
        while(p){
            count++;
            p = strtok(NULL, sep);
        }

        char* arg[count + 1]; //+1 for NULL

        arg[count] = NULL;

        char* pp = strtok(com2, sep);
        int i = 0;

        //Putting arguments into an array
        while(pp){
            arg[i++] = pp;
            pp = strtok(NULL, sep);
        }


        //Execvp with NULL terminated argument list

		free(com);
		free(com2);

        execvp(arg[0], arg);
		
		perror("Exec error: ");
        exit(1);
    }else{
        //Parent
        int len = strlen(cmd);

        close(fd[0]);
        //Sending size and actual command
        write(fd[1], &len, sizeof(len));
        write(fd[1], cmd, len);
        close(fd[1]);

        wait(NULL);
    }
}
