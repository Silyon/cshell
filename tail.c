#include "includes.h"
#include "tail.h"
#define SIZE (1<<12)

int parseTail(char* cmd){
	char* com = (char*)malloc(sizeof(char) * strlen(cmd) + 1);
	memcpy(com, cmd, strlen(cmd) + 1);

	char* result[5];
	char* tok = strtok(com, " ");
	int i = 0;

	while(tok != NULL){
		result[i] = (char*)malloc(sizeof(char) * strlen(tok) + 1);
		memcpy(result[i], tok, strlen(tok) + 1);		

		i++;
		tok = strtok(NULL, " ");
	}

	int n = i;
	int j = 0;
	int opts = -1;
	char* option = (char*)malloc(sizeof(char) * 10);
	int files[5];
	char* filenames[5];

	for(i = 0; i < n; i++){
		if(result[i][0] == '-' && opts == -1){
			memcpy(option, result[i], strlen(result[i]) + 1);
			option[strlen(result[i])] = '\0';
			printf("Option: %s\n", option);
			opts = i;
		}else if(opts != -1 && result[i][0] == '-'){
			printf("You can only have one option.\n Available options: -c, -n, -q, -v");
			return -1;
		}
		if(strcmp(result[i], "tail") != 0 && result[i][0] != '-'){
			filenames[j] = result[i];			
			files[j++] = open(result[i], O_RDONLY);
		}
	}

	if(opts == -1)
		strcpy(option, "-N");

	execTail(option, files, &j, filenames);

	free(option);

	return 0;
}

int execTail(char* option, int* files, int* n, char** filenames){
	int i = 0;	
	int fd[2];
	int nrLines = 10;
	int nrBytes = 10;
	pid_t pid;

	if(option[1] == 'c'){
		nrBytes = atoi(option + 2);
	}else if(option[1] == 'n'){
		nrLines = atoi(option + 2);
	}else if(option[1] == 'N'){
		printf("Hey");
	}else if(option[1] != 'q' || option[1] != 'c'){
		printf("Option not available.\n Available options: -c, -n, -q, -v");
		return -1;
	}

	if(pipe(fd) < 0){
		printf("Piping error");
		return -1;
	}

	pid = fork();

	if(pid < 0){
		printf("Fork error.");
		return -1;
	}else if(pid == 0){
		char* buff[*n];
		int m;
		int size;

		i = 0;

		close(fd[1]);
		while(i < *n){
			buff[i] = (char*)malloc(sizeof(char) * SIZE);
			
			read(fd[0], &size, sizeof(size));

			if((m = read(fd[0], buff[i], size)) > 0){
				buff[i][m] = '\0';
			}

			i++;
		}
		close(fd[0]);

		i = 0;

		if(*n == 1){
			if(option[1] == 'c')
				printf("%s", buff[i] + (strlen(buff[i]) - nrBytes));
			else if(option[1] == 'n')
				exit(0);
		}else{
			for(i = 0; i < *n; i ++){
				if(option[1] == 'q'){
					printf("\n%s\n", buff[i]);
				}else{
					printf("\n====\n%s\n====\n%s\n", filenames[i], buff[i]);
				}
			}
		}
		
		exit(0);
	}else{

		//Parent reads files and sends it to child
		char* buff;
		int m;
		int size;

		close(fd[0]);		
		for(i = 0; i < *n; i++){
			buff = (char*)malloc(sizeof(char) * SIZE);

			while((m = read(files[i], buff, SIZE)) > 0){
				size = strlen(buff);

				write(fd[1], &size, sizeof(size));
				write(fd[1], buff, m);
			}
		}
		close(fd[1]);
		wait(NULL);
		
		if(buff)
			free(buff);

	}

	return 0;
}

