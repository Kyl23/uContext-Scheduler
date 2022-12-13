#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "../include/command.h"
#include "../include/builtin.h"
#include "../include/task.h"
#include "../include/list.h"

int execute(struct pipes *p)
{
	for (int i = 0; i < num_builtins(); ++i)
		if (strcmp(p->args[0], builtin_str[i]) == 0)
    		return (*builtin_func[i])(p->args);
	return execvp(p->args[0], p->args);
}

int spawn_proc(int in, int out, struct cmd *cmd, struct pipes *p)
{
  	pid_t pid;
  	int status, fd;
  	if ((pid = fork()) == 0) {
      	if (in != 0) {
          	dup2(in, 0);
          	close(in);
        } else {
			if (cmd->in_file) {
				fd = open(cmd->in_file, O_RDONLY);
				dup2(fd, 0);
				close(fd);
			}
		}
	    if (out != 1) {
          	dup2(out, 1);
          	close(out);
        } else {
			if (cmd->out_file) {
				fd = open(cmd->out_file, O_RDWR | O_CREAT, 0644);
				dup2(fd, 1);
				close(fd);
			}
		}
    	if (execute(p) == -1)
       		perror("lsh");
    	exit(EXIT_FAILURE);
    } else {
   		if(cmd->background) {
			if (!p->next)
      			printf("[pid]: %d\n", pid);
    	} else {
      		do {
          		waitpid(pid, &status, WUNTRACED);
        	} while (!WIFEXITED(status) && !WIFSIGNALED(status));
    	}
  	}
  	return 1;
}

int fork_pipes(struct cmd *cmd)
{
  	int in = 0, fd[2];
	struct pipes *temp = cmd->head;
  	while (temp->next != NULL) {
      	pipe(fd);
      	spawn_proc(in, fd[1], cmd, temp);
      	close(fd[1]);
      	in = fd[0];
      	temp = temp->next;
  	}
  	if (in != 0) {
    	spawn_proc(in, 1, cmd, temp);
    	return 1;
  	}

  	spawn_proc(0, 1, cmd, cmd->head);
	return 1;
}

List *Task_List = NULL;
Task *Task_Now = NULL;

int now_TID = 1;

void copyTask(void *t, void *v){
	Task *t_task = (Task *)t;
	Task *v_task = (Task *)v;

	memcpy(t_task, v_task, sizeof(Task));
}

void freeTask(void *t){
	Task *task = (Task *)t;
	free(task);

	return;
}

void shell()
{
	Task_List = spawn_list(sizeof(Task), &copyTask, &freeTask);
	
	Task *task = (Task *) Task_List->value;
	task->in = task->out = task->fd = -1;
	
	getcontext(&task->context);
	
	while (1) {
		Task *task = (Task *)Task_List->value;
		task->state = 0;

		if (task->in != -1){
			dup2(task->in, 0);
			close(task->in);
			task->in = -1;
		}  
		if (task->out != -1){
			dup2(task->out, 1);
			close(task->out);
			task->out = -1;
		}
		printf(">>> $ ");
		
		char *buffer = read_line();
		if (buffer == NULL)
			continue;

		struct cmd *cmd = split_line(buffer);
		
		int status = -1;
		

		if (!cmd->background && cmd->head->next == NULL) {
			task->in = dup(0), task->out = dup(1);

			if (cmd->in_file) {
                task->fd = open(cmd->in_file, O_RDONLY);
                dup2(task->fd, 0);
                close(task->fd);
            }
			if (cmd->out_file) {
                task->fd = open(cmd->out_file, O_RDWR | O_CREAT, 0644);
                dup2(task->fd, 1);
                close(task->fd);
			}
			for (int i = 0; i < num_builtins(); ++i)
				if (strcmp(cmd->head->args[0], builtin_str[i]) == 0)
					status = (*builtin_func[i])(cmd->head->args);
			if (cmd->in_file)  dup2(task->in, 0);
			if (cmd->out_file) dup2(task->out, 1);
			close(task->in);
			close(task->out);
		}
		if (status == -1)
			status = fork_pipes(cmd);

		while (cmd->head) {
			struct pipes *temp = cmd->head;
      		cmd->head = cmd->head->next;
			free(temp->args);
   	    	free(temp);
   		}
		free(cmd);
		free(buffer);
		
		if (status == 0)
			break;
	}
}
