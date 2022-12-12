#ifndef TASK_H
#define TASK_H

#include <ucontext.h>
#include "./list.h"

typedef struct task{
    int TID;
	char name[1024];
    int state;
    int running;
    int waiting;
    int turnaround;
    int resources[8];
    int waiting_resource;
	int priority;
    ucontext_t context;
    int usleep;
}Task;

void task_sleep(int);
void task_exit();
void task_init(Task *, char *task_name);
void task_scheduler();

extern int now_TID;
extern int task_algorithm;
extern char *state_mapper[];

extern List *Task_List;
extern List *Task_List_p;
extern Task *Task_Now;

#endif
