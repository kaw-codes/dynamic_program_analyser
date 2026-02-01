#pragma once
#include <sys/types.h>
#include <stdbool.h>

typedef enum 
{
    RUNNING,
    STOPPED,
    TERMINATED
} 
status_t;

typedef struct 
{
    pid_t pid;
    status_t status;
    bool kill_on_exit;
} 
process_t;

/**
 * fork 
 * child: 
 *  - executes the program from the path given in parameter
 *  - enables the parent to trace him
 * parent:
 *  - waits for the child to stop
 *  - sets up the process_t struct
 */
int launch(char *path, bool kill_on_exit, process_t **proc);

/**
 * attach to the software using an existing pid
 */
int attach(pid_t pid, bool kill_on_exit, process_t **proc);

int resume(process_t *proc);

int detach(process_t *proc);
