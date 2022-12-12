#include "../include/resource.h"
#include "../include/task.h"

int core_resource[8] = {0, 0, 0, 0, 0, 0, 0, 0};

int resource_available(int count, int *resources){
    int is_not_legal = 0;
    for(int i = 0; i < count; i++){
        if(core_resource[resources[i]])
        {
            is_not_legal = 1;
            break;
        }
    }
    return is_not_legal;
}

void waiting_resources(int count, int *resources){
    Task_Now->state = 2;
    Task_Now->usleep = 0x7FFFFFFF;

    for(int i = 0; i < count; i++)
        Task_Now->waiting_resource[resources[i]] = 1;

    printf("Task %s is waiting resource\n", Task_Now->name);
    while(resource_available(count, resources));
}

void check_resources_require(){
    int virtual_resources[8];
    for(int i = 0; i < 8; i++){
        virtual_resources[i] = core_resource[i];
    }

    for(List *i = Task_List->next; i != NULL; i = i->next){
        Task *task = (Task *)i->value;
        if(task->state == 2){
            int is_not_legal = 0;
            int applying_resource = 0;
            for(int i = 0; i < 8; i++){
                applying_resource += task->waiting_resource[i];
                if(task->waiting_resource[i] && virtual_resources[i])
                {
                    is_not_legal = 1;
                    break;
                }
            }

            if(!is_not_legal && applying_resource){
                task->state = 0;
                task->usleep = 0;
                for(int i = 0; i < 8; i++){
                    virtual_resources[i] += task->waiting_resource[i];
                    task->waiting_resource[i] = 0;
                }
            }
        }
    }
}

void update_task_resource(int count, int *resources, int flag){
    for(int i = 0; i < count; i++){
        Task_Now->resources[resources[i]] = flag;
    }
}

void get_resources(int count, int *resources)
{
    int is_not_legal = resource_available(count, resources);


    if(is_not_legal) {
        waiting_resources(count, resources);
    }

    update_task_resource(count, resources, 1);
    for(int i = 0; i < count; i ++){
        printf("Task %s gets resource %d\n", Task_Now->name, resources[i]);
        core_resource[resources[i]] = 1;
    }
}

void release_resources(int count, int *resources)
{
    for(int i = 0; i < count; i ++){
        printf("Task %s release resource %d\n", Task_Now->name, resources[i]);
        core_resource[resources[i]] = 0;
    }

    update_task_resource(count, resources, 0);
    check_resources_require();
}
