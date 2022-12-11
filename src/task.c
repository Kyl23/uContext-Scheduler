#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include "../include/task.h"
#include "../include/function.h"

char *state_mapper[] = {"READY", "RUNNING", "WAITING", "TERMINATED"};

void (*task[])() = {&task1, &task2, &task3, &task4, &task5, &task6, &task7, &task8, &task9, &test_exit, &test_sleep, &test_resource1, &test_resource2, &idle};
char *task_name[] = {"task1", "task2", "task3", "task4", "task5", "task6", "task7", "task8", "task9", "test_exit", "test_sleep", "test_resource1", "test_resource2", "idle"};
int task_len = 15;

void task_sleep(int ms)
{
  Task_Now->state = 2;
  printf("Task %s goes to sleep\n", Task_Now->name);
  Task_Now->usleep = ms;
  while(1);
}

void task_exit()
{
  Task_Now->state = 3;
  Task_Now->turnaround = Task_Now->running + Task_Now->waiting;
  printf("Task %s has terminated\n", Task_Now->name);
}

void task_init(Task *t, char *task_name){
    for(int i = 0; i < task_len; i++){
      if(!strcmp(&task_name[i], task_name)){
        t->TID = now_TID++;
        t->state = 0;
        t->running = 0;
        t->waiting = 0;
        t->turnaround = 0;
        t->priority = -1;
        
        getcontext(&t->context);
        t->context.uc_stack.ss_sp = malloc(1024 * 128 * sizeof(char));
        t->context.uc_stack.ss_size = 1024 * 128 * sizeof(char);
        t->context.uc_stack.ss_flags = 0;

        t->usleep = 0;

        for(int i = 0; i < 8; i++){
          t->resources[i] = 0;
      
        makecontext(&t->context, task[i], 0);
			  break;
      }
		}
	} 
}

struct itimerval timer_attr;
int looping;

void FCFS(){
  if(Task_List_p == NULL){
    return;
  }
  
  if(!Task_Now){
    Task_List_p = Task_List_p->next;
    Task_Now = (Task *)Task_List_p->value;
    Task_Now->state = 1;
    setcontext(&(Task_Now->context));
  }

  if(Task_Now && Task_Now->state == 3){
    Task_List_p = Task_List_p->next;
    Task_Now = Task_List_p ? (Task *)Task_List_p->value : NULL;

    if(!Task_List_p || !Task_Now)
      return;
    Task_Now->state = 1;
    setcontext(&(Task_Now->context));
  }
}

void RR(){
  printf("RR");
}

void PP(){
  printf("PP");
}

void (*algorithm[])() = {FCFS, RR, PP};

void stop_looping(int signo){
  looping = 0;
}

void work_time_cal(){
  for(List *i = Task_List->next; i != NULL; i = i->next){
    Task *task = (Task *)i->value;
    
    if(task->state == 1) task->running++;
    else if(task->state == 2) task->waiting++;
  }
}

void sleep_redundance(){
  if(Task_List->next == NULL)
    return;

  for(List *i = Task_List->next; i != NULL; i = i->next){
    Task *task = (Task *)i->value;
    
    if(task->state == 2){
      task->usleep -= 10;
      if(task->usleep == 0) 
        task->state = 0;
    }
  }
}

int alive_task(){
  for(List *i = Task_List->next; i != NULL; i = i->next){
    Task *task = (Task *)i->value;
    if(task->state != 3){
      return 1;
    }
  }
  
  stop_looping(0);
  printf("Simulation over.\n");

  Task *task = (Task *)Task_List->value;
  setcontext(&task->context);
  
  return 0;
}

void task_dispatch(int signo){
  work_time_cal();
  sleep_redundance();
  if(!alive_task())
    return;

  algorithm[task_algorithm]();
}

void timer_init(){
  signal(SIGVTALRM, task_dispatch);
  signal(SIGTSTP, stop_looping);
  timer_attr.it_value.tv_sec = 0;
  timer_attr.it_value.tv_usec = 1;
  timer_attr.it_interval.tv_sec = 0;
  timer_attr.it_interval.tv_usec = 10000;
  setitimer(ITIMER_VIRTUAL, &timer_attr, NULL);
  while(looping);
}

void task_scheduler(){
  looping = 1;
  timer_init();
}