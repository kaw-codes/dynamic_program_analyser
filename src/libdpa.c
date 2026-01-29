#include "libdpa.h"
#include <stdlib.h> // malloc
#include <sys/ptrace.h> // ptrace
#include <sys/wait.h> // waitpid
#include <unistd.h> // fork, access

int launch(const char *path, bool kill_on_exit, process_t *proc)
{
    // verif that program exists
    if (access(path, F_OK) != 0)
    {
        return EXIT_FAILURE;
    }

    // fork
    pid_t pid = fork();
    if (pid < 0)
    { /* error */
        return EXIT_FAILURE;
    }
    else if (pid == 0)
    { /* child */
        ptrace(PTRACE_TRACEME, 0, 0, NULL);
        
        // exec software from path
        char *my_args[2];
        my_args[0] = path;
        my_args[1] = NULL;
        execvp(my_args[0], my_args);

        // if exevp fails
        return EXIT_FAILURE;
    }
    else 
    { /* parent */
        // wait for child to stop
        waitpid(pid, NULL, 0);

        // init process_t object
        proc = malloc(sizeof(process_t));
        proc->pid = pid;
        proc->status = STOPPED;
        proc->kill_on_exit = kill_on_exit;
    }

    return EXIT_SUCCESS;
    
}
