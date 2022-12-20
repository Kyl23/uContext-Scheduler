#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/shell.h"
#include "include/command.h"
#include "include/task.h"

int task_algorithm; // 0: FCFS, 1: RR, 2: PP

int main(int argc, char *argv[])
{
	char *algo = argv[1];

	if(!strcmp("FCFS", algo)){
		task_algorithm = 0;
	}
	else if(!strcmp("RR", algo)){
		task_algorithm = 1;
	}
	else if(!strcmp("PP", algo)){
		task_algorithm = 2;
	}

	history_count = 0;
	for (int i = 0; i < MAX_RECORD_NUM; ++i)
    	history[i] = (char *)malloc(BUF_SIZE * sizeof(char));

	shell();

	for (int i = 0; i < MAX_RECORD_NUM; ++i)
    	free(history[i]);

	return 0;
}
