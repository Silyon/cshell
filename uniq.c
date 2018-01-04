#include "includes.h"
#include "uniq.h"
#include <errno.h>
#define SIZE (1<<12)

/* 2. The "uniq" command. Parameters that need to be implemented are: -i, -d, -u */

int parseUniq(char* cmd){
	char* copy = (char*)malloc(sizeof(char) * strlen(cmd) + 1);

	memcpy(copy, cmd, strlen(cmd) + 1);

	char* tok;

	tok = strtok(copy, " ");

	char* args[3];
	int i = 0;
	while(tok != NULL){
		args[i++] = tok;
		tok = strtok(NULL, " ");
	}

	int k = i;

	
	if((args[1][0] == '-') || k > 3){
		printf("\nUsage: uniq [Filename] [-Option]\n Available options: -i, -d, -u\n");
		return 1;
	}

	int file = open(args[1], O_RDONLY);
	
	if(errno != 0)
		perror("Error");
	
	if(errno == 0){
		if(args[2]){
			char* option = (char*)malloc(sizeof(char) * 3);
			memcpy(option, args[2], 3);

			execUniq(option, &file);
		}else{
			char option = '\0';

			execUniq(&option, &file);
		}
	}
	
	
	return 0;
}

int execUniq(char* option, int* file){
	pid_t pid;
	int fd[2];
	int n;

	if(pipe(fd) < 0){
		printf("Couldn't pipe for uniq");
		return -1;
	}
	
	pid = fork();

	if(pid < 0){
		printf("Couldn't fork for uniq");
		return -1;
	}

	if(pid == 0){
		char* buff = (char*)malloc(sizeof(char) * (SIZE));

		close(fd[1]);
		while((n=read(fd[0], buff, SIZE-1)) > 0){
			buff[n] = '\0';
		}
		close(fd[0]);

		selectUniq(buff, option);
		
		if(buff)
			free(buff);

		exit(0);

	}else{
		char* buff = (char*)malloc(sizeof(char) * (SIZE));

		close(fd[0]);
		while((n = read(*file, buff, SIZE)) > 0){
			write(fd[1], buff, n);
		}
		close(fd[1]);

		wait(NULL);
		
		if(buff)
			free(buff);
	}

	return 0;
}

int selectUniq(char* text, char* option){
	char* cache = (char*)malloc(sizeof(char) * 512);
	char* textcpy = (char*)malloc(sizeof(char) * strlen(text) + 1);

	memcpy(textcpy, text, strlen(text) + 1);

	char* tok = strtok(textcpy, "\n");
	int i = 0;

	while(tok != NULL){
		i++;		
		tok = strtok(NULL, "\n");
	}

	free(textcpy);

	textcpy = (char*)malloc(sizeof(char) * strlen(text + 1));

	memcpy(textcpy, text, strlen(text) + 1);


	int n = i;
	char* result[n];
	int counter[n];

	for(i = 0; i < n; i++){
		counter[i] = 0;
	}

	int j = 0;

	char* tok2 = strtok(textcpy, "\n");

	i = 0;

	while(tok2 != NULL){
		result[i++] = tok2;
		tok2 = strtok(NULL, "\n");
	}

	free(textcpy);

	i = 0;
	cache = result[0];
	counter[0]++;

	for(i = 1; i < n; i++){
		if(strcmp(cache, result[i]) == 0){
			counter[j]++;
		}else{
			cache = result[i];
			j += counter[j];
			counter[j]++;
		}
	}

	i = 0;
	int o = 0;

	if(strcmp(option, "-u") == 0){
		o = 1;
	}else if(strcmp(option, "-d") == 0){
		o = 2;
	}else{
		printf("Invalid option");
		return -1;
	}

	while(i < n){
		if(o == 1){
			if(counter[i] == 1){
				printf("\n%s", result[i]);
			}
		}else if(o == 2){
			if(counter[i] > 1){
				printf("\n%s", result[i]);
			}
		}
		i++;
	}

	return 0;
}
















