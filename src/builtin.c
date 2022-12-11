#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include "../include/builtin.h"
#include "../include/command.h"
#include "../include/function.h"
#include "../include/task.h"
#include "../include/list.h"

int help(char **args)
{
	int i;
    printf("--------------------------------------------------\n");
  	printf("My Little Shell!!\n");
	printf("The following are built in:\n");
	for (i = 0; i < num_builtins(); i++) {
    	printf("%d: %s\n", i, builtin_str[i]);
  	}
	printf("%d: replay\n", i);
    printf("--------------------------------------------------\n");
	return 1;
}

int cd(char **args)
{
	if (args[1] == NULL) {
    	fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  	} else {
    	if (chdir(args[1]) != 0)
      		perror("lsh");
	}
	return 1;
}

int echo(char **args)
{
	bool newline = true;
	for (int i = 1; args[i]; ++i) {
		if (i == 1 && strcmp(args[i], "-n") == 0) {
			newline = false;
			continue;
		}
		printf("%s", args[i]);
		if (args[i + 1])
			printf(" ");
	}
	if (newline)
		printf("\n");

	return 1;
}

int exit_shell(char **args)
{
	return 0;
}

int record(char **args)
{
	if (history_count < MAX_RECORD_NUM) {
		for (int i = 0; i < history_count; ++i)
			printf("%2d: %s\n", i + 1, history[i]);
	} else {
		for (int i = history_count % MAX_RECORD_NUM; i < history_count % MAX_RECORD_NUM + MAX_RECORD_NUM; ++i)
			printf("%2d: %s\n", i - history_count % MAX_RECORD_NUM + 1, history[i % MAX_RECORD_NUM]);
	}
	return 1;
}

bool isnum(char *str)
{
	for (int i = 0; i < strlen(str); ++i) {
    	if(str[i] >= 48 && str[i] <= 57)
			continue;
        else
		    return false;
  	}
  	return true;
}

int mypid(char **args)
{
	char fname[BUF_SIZE];
	char buffer[BUF_SIZE];
	if(strcmp(args[1], "-i") == 0) {

	    pid_t pid = getpid();
	    printf("%d\n", pid);
	
	} else if (strcmp(args[1], "-p") == 0) {
	
		if (args[2] == NULL) {
      		printf("mypid -p: too few argument\n");
      		return 1;
    	}

    	sprintf(fname, "/proc/%s/stat", args[2]);
    	int fd = open(fname, O_RDONLY);
    	if(fd == -1) {
      		printf("mypid -p: process id not exist\n");
     		return 1;
    	}

    	read(fd, buffer, BUF_SIZE);
	    strtok(buffer, " ");
    	strtok(NULL, " ");
	    strtok(NULL, " ");
    	char *s_ppid = strtok(NULL, " ");
	    int ppid = strtol(s_ppid, NULL, 10);
    	printf("%d\n", ppid);
	    
		close(fd);

  	} else if (strcmp(args[1], "-c") == 0) {

		if (args[2] == NULL) {
      		printf("mypid -c: too few argument\n");
      		return 1;
    	}

    	DIR *dirp;
    	if ((dirp = opendir("/proc/")) == NULL){
      		printf("open directory error!\n");
      		return 1;
    	}

    	struct dirent *direntp;
    	while ((direntp = readdir(dirp)) != NULL) {
      		if (!isnum(direntp->d_name)) {
        		continue;
      		} else {
        		sprintf(fname, "/proc/%s/stat", direntp->d_name);
		        int fd = open(fname, O_RDONLY);
        		if (fd == -1) {
          			printf("mypid -p: process id not exist\n");
          			return 1;
        		}

        		read(fd, buffer, BUF_SIZE);
        		strtok(buffer, " ");
        		strtok(NULL, " ");
        		strtok(NULL, " ");
		        char *s_ppid = strtok(NULL, " ");
		        if(strcmp(s_ppid, args[2]) == 0)
		            printf("%s\n", direntp->d_name);

        		close(fd);
     		}
	   	}
    	
		closedir(dirp);
	
	} else {
    	printf("wrong type! Please type again!\n");
  	}
	
	return 1;
}

int add(char **args)
{
	Task t;

	char *name = args[1];
	char *func_name = args[2];
	
	task_init(&t, func_name);

	strcpy(t.name, name);

	if(task_algorithm == 2){
		t.priority = atoi(args[3]);
	}
	
	list_push(Task_List, &t);
	
	printf("Task %s  is ready.\n", name);

	return 1;
}

int del(char **args)
{
	char *task_name = args[1];

	for(List *i = Task_List->next; i != NULL; i = i->next){
		Task *task = (Task *) i->value;
		if(!strcmp(task->name, task_name)){
			task->state = 3;
		}
	}

	printf("Task %s is killed.\n", task_name);
	
	return 1;
}

int ps(char **args)
{
	printf("%4s|%11s|%11s|%8s|%8s|%11s|%10s|%10s\n", "TID", "name", "state", "running", "waiting", "turnaround", "resources", "priority");
	for(int i = 0; i < 80; i++) putc('-', stdout);
	puts("");
	for(List *i = Task_List->next; i != NULL; i = i->next){
		Task *task = (Task *)i->value;
		if(task->turnaround)
			printf("%4d|%11s|%11s|%8d|%8d|%11d|", task->TID, task->name, state_mapper[task->state], task->running, task->waiting, task->turnaround);
		else
			printf("%4d|%11s|%11s|%8d|%8d|%11s|", task->TID, task->name, state_mapper[task->state], task->running, task->waiting, "none");
		char resource_str[1024] = {'\0'};
		int resource_count = 0;
		for(int i = 0; i < 8; i++){
			if(resource_str[i]){
				sprintf(resource_str,"%s %d",resource_str, i);
				resource_count++;
			} 
		}
		if(!resource_count) strcpy(resource_str, "none");
		printf("%10s|%10d\n", resource_str, task->priority);
	}
	return 1;
}

int start(char **args)
{
	printf("Start simulation.\n");
	task_scheduler();
	return 1;
}

const char *builtin_str[] = {
 	"help",
 	"cd",
	"echo",
 	"exit",
 	"record",
	"mypid",
	"add",
	"del",
	"ps",
	"start"
};

const int (*builtin_func[]) (char **) = {
	&help,
	&cd,
	&echo,
	&exit_shell,
  	&record,
	&mypid,
	&add,
	&del,
	&ps,
	&start
};

int num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}
