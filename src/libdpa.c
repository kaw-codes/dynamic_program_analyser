#include "libdpa.h"
#include <stdlib.h> // malloc
#include <signal.h> // SIGTERM
#include <sys/ptrace.h> // ptrace
#include <sys/wait.h> // waitpid
#include <unistd.h> // fork, access

int launch(char *path, bool kill_on_exit, process_t **proc)
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
        if (ptrace(PTRACE_TRACEME) < 0)
        {
            return EXIT_FAILURE;
        }
        
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
        // init process_t object
        (*proc) = malloc(sizeof(process_t));
        (*proc)->pid = pid;
        (*proc)->status = STOPPED;
        (*proc)->kill_on_exit = kill_on_exit;

        // wait for child to stop
        wait_status((*proc));
    }

    return EXIT_SUCCESS;
    
}

int wait_status(process_t *proc)
{
    // check
    if (!proc)
    {
        EXIT_FAILURE;
    }

    // waitpid
    int status;
    waitpid(proc->pid, &status, 0);

    // edit status
    if (status < 0)
    {
        return EXIT_FAILURE;
    }
    else if (WIFSTOPPED(status))
    {
        proc->status = STOPPED;
    }
    else if (WIFSIGNALED(status) || WIFEXITED(status))
    {
        proc->status = TERMINATED;
    }
    else
    {
        proc->status = RUNNING;
    }
    return EXIT_SUCCESS;
}

int attach(pid_t pid, bool kill_on_exit, process_t **proc)
{
    // check is pid exists
    if (kill(pid, 0) != 0)
    {
        return EXIT_FAILURE;
    }

    // attach 
    if (ptrace(PTRACE_ATTACH, pid) < 0)
    {
        return EXIT_FAILURE;
    } 
    wait_status((*proc));

    // set struct proc
    (*proc) = malloc(sizeof(process_t));
    (*proc)->pid = pid;
    (*proc)->kill_on_exit = kill_on_exit;

    // end
    return EXIT_SUCCESS;
}

int resume(process_t *proc)
{
    // check
    if (!proc)
    {
        return EXIT_FAILURE;
    }

    // resume
    if (proc->status == STOPPED)
    {
        ptrace(PTRACE_CONT, proc->pid);
        proc->status = RUNNING;
    }
    return EXIT_SUCCESS;
}

int detach(process_t *proc)
{
    // check
    if (!proc)
    {
        return EXIT_FAILURE;
    }
    if (proc->status != TERMINATED || proc->status != STOPPED)
    {
        kill(proc->pid, SIGSTOP);
    }

    // detach
    ptrace(PTRACE_DETACH, proc->pid);

    // kill or resume
    if (proc->kill_on_exit)
    {
        kill(proc->pid, SIGTERM);
    }
    else
    {
        kill(proc->pid, SIGCONT);
    }

    return EXIT_SUCCESS;
}
