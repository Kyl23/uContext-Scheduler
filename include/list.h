# ifndef LIST_H
# define LIST_H

#include <stdio.h>
#include <stdlib.h>


typedef struct List{
	void *value;
	long long size_value;
	void (*free_value_func)(void *);
    void (*cpy_value_func)(void *, void *);
	struct List *next;
}List;

List *spawn_list(long long, void(*)(void *, void *), void(*)(void *));

void *get_list_head(List *);

void *get_list_rear(List *);

void list_push(List *, void *);

void list_rm_head(List *);

void list_rm_rear(List *);

# endif
