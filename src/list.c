#include "../include/list.h"

List *spawn_list(long long value_size, void(*cpy_value_func)(void *target, void *value), void(*free_value_func)(void *value)){
	List *list = (List *)malloc(sizeof(List));
	list->size_value = value_size;
    list->free_value_func = free_value_func;
    list->cpy_value_func = cpy_value_func;
	list->value = malloc(value_size);
	
	return list;
}

void *get_list_head(List *list){
    if(!list->next){
		puts("List is empty");
		return NULL;
	}

	return list->next->value;
}

void *get_list_rear(List *list){
    if(!list->next){
		puts("List is empty");
		return NULL;
	}

	List *i;
	for(i = list; i->next != NULL; i = i->next);
	return  i->value;
}

void list_push(List *list, void *value){
	List *i;
	for(i = list; i->next != NULL; i = i->next);
	
	i->next = malloc(sizeof(List));
	i = i->next;
	i->value = malloc(list->size_value);
    list->cpy_value_func(i->value, value);
}

void list_rm_head(List *list){
	if(!list->next){
		puts("List is empty");
		return;
	}

	List *t = list->next;
    list->next = list->next->next;
    t->next = NULL;
	list->free_value_func(t->value);
	free(t);
}

void list_rm_rear(List *list){
	if(!list->next){
		puts("List is empty");
		return;
	}

	List *i;
	
	for(i = list; i->next->next != NULL; i = i->next);

	list->free_value_func(i->next->value);
	free(i->next);
	i->next = NULL;
}