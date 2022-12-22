#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include "../include/task.h"
#include "../include/function.h"

char *state_mapper[] = {"READY", "RUNNING", "WAITING", "TERMINATED"};

void (*task[])() = {&task1, &task2, &task3, &task4, &task5, &task6, &task7, &task8, &task9, &test_exit, &test_sleep, &test_resource1, &test_resource2, &idle};
char *task_n[] = {"task1", "task2", "task3", "task4", "task5", "task6", "task7", "task8", "task9", "test_exit", "test_sleep", "test_resource1", "test_resource2", "idle"};
int task_len = 15;
void stop_timer();
void init_timer();

void task_sleep(int ms_10)
{
  Task_Now->state = 2;
  printf("Task %s goes to sleep\n", Task_Now->name);
  Task_Now->usleep = ms_10 * 10;
  Task_Now->running++;
  while(Task_Now->usleep);
}

void task_exit()
{
  Task_Now->state = 3;
  Task_Now->running++;
  printf("Task %s has terminated\n", Task_Now->name);
}

void task_init(Task *t, char *task_name){
  for(int i = 0; i < task_len; i++){
    if(!strcmp(task_n[i], task_name)){
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
        t->waiting_resource[i] = 0;
      }

      t->waiting_resource_flag = 0;
      
      makecontext(&t->context, task[i], 0);
      break;
    }
  }
} 


struct itimerval timer_attr;
int looping;
int idling = 0;

void FCFS(){
  if(Task_Now)
    getcontext(&Task_Now->context);
  
  if(!Task_Now || Task_Now->state != 1){
    for(List *i = Task_List->next; i != NULL; i = i->next){
      Task *task = (Task *)i->value;
      if(task->state == 0){
        task->state = 1;
        Task_Now = task;
        idling = 0;
        printf("Task %s is running\n", Task_Now->name);
        setcontext(&task->context);
      }
    }
    if(!idling){
      puts("CPU idle.");
      idling = 1;
    }
  }
}

int RR_counter = 0;
List *Task_List_p = NULL;

void RR(){
  if(Task_Now){
    getcontext(&Task_Now->context);
  }

  if(Task_List_p == NULL){
    Task_List_p = Task_List;
    RR_counter = 3;
  }
  
  if(idling) RR_counter = 3;

  if(RR_counter == 3 || (Task_Now && Task_Now->state != 1)){
    RR_counter = 0;

    if(Task_Now && Task_Now->state == 1) 
      Task_Now->state = 0;

    Task *temp = NULL;

    for(List *i = Task_List_p->next; i != NULL; i = i->next){
      Task *task = (Task *) i->value;
      if(task->state == 0){
        Task_List_p = i;
        temp = task;
        break;
      }
    }

    if(!temp && Task_List != Task_List_p){
      for(List *i = Task_List->next; i != Task_List_p->next; i = i->next){
        Task *task = (Task *) i->value;
        if(task->state == 0){
          Task_List_p = i;
          temp = task;
          break;
        }
      }
    }

    if(!temp){
      if(!idling){
        puts("CPU idle.");
        idling = 1;
      }
      return;
    }

    idling = 0;
    Task_Now = temp;
    Task_Now->state = 1;
    printf("Task %s is running\n", Task_Now->name);
    setcontext(&Task_Now->context);
  }


  RR_counter += 1;
}

int PP_context_checked = 0;
void PP(){
  Task *temp;
  if(Task_Now){
    getcontext(&Task_Now->context);
  }
  
  if(PP_context_checked) {
    PP_context_checked = 0;
    return;
  }

  temp = NULL;

  int alive = 0;

  for(List *i = Task_List->next; i != NULL; i = i->next){
    Task *task = (Task *)i->value;
    if(task->state != 3) alive = 1;

    if(temp == NULL && (task->state == 0 || task->state == 1)){
      temp = task;
    }

    if(temp && (temp->priority >= task->priority) && (task->state == 0 || task->state == 1)){
      // printf("%s %s %d %d\n", temp->name, task->name, temp->priority, task->priority);
      temp = task;
    }
  }
  if(temp == Task_Now) {
    PP_context_checked = 1;
    return;
  }
  if(temp == NULL && alive ){
    if(!idling){
      puts("CPU idle.");
      idling = 1;
    }
    return;
  }

  if(Task_Now && Task_Now->state == 1)
    Task_Now->state = 0;
 
  Task_Now = temp;
  
  if(Task_Now){
    Task_Now->state = 1;
    idling = 0;
    printf("Task %s is running\n", Task_Now->name);
    setcontext(&Task_Now->context);
  }
}

void (*algorithm[])() = {FCFS, RR, PP};

void stop_looping(int signo){
  looping = 0;

  Task *task = (Task *)Task_List->value;
  if(Task_Now){
    swapcontext(&Task_Now->context, &task->context);
  }
}

void work_time_cal(){
  for(List *i = Task_List->next; i != NULL; i = i->next){
    Task *task = (Task *)i->value;
    
    if(task->state == 1) task->running++;
    else if(task->state == 0) task->waiting++;

    if(task->state != 3) task->turnaround++;
  }
}

void sleep_redundance(){
  if(Task_List->next == NULL)
    return;

  for(List *i = Task_List->next; i != NULL; i = i->next){
    Task *task = (Task *)i->value;
    
    if(task->state == 2 && !task->waiting_resource_flag){
      task->usleep -= 10;
      if(task->usleep <= 0){
        task->usleep = 0; 
        task->state = 0;
      }  
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

  Task *task = (Task *)Task_List->value;
  task->state = 3;
  printf("Simulation over.\n");

  stop_timer();
  stop_looping(0);

  return 0;
}

void task_dispatch(int signo){
  work_time_cal();
  
  sleep_redundance();
  
  alive_task();

  Task *task = (Task *)Task_List->value;

  if(task->state != 3)
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

void stop_timer(){
  signal(SIGVTALRM, NULL);
  signal(SIGTSTP, NULL);
  timer_attr.it_value.tv_sec = 0;
  timer_attr.it_value.tv_usec = 0;
  timer_attr.it_interval.tv_sec = 0;
  timer_attr.it_interval.tv_usec = 0;
  setitimer(ITIMER_VIRTUAL, &timer_attr, NULL);
}

void task_scheduler(){  
  looping = 1;

  if(Task_Now && Task_Now->state == 1) Task_Now->state = 0;
  
  Task_Now = NULL;
  timer_init();
}