#include "libdpa.h"
#include <stdio.h> // perror
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
        (*proc) = malloc(sizeof(process_t)); // TODO ctrl malloc
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
        perror("ptrace");
        return EXIT_FAILURE;
    } 
    wait_status((*proc));

    // set struct proc
    (*proc) = malloc(sizeof(process_t)); // TODO ctrl malloc
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

/**
 * convert the register's name in paramater in an identifier
 */
static int convert_reg_into_id(const char *reg)
{
    if (strcmp(reg, "r15") == 0) return R15;
    if (strcmp(reg, "r14") == 0) return R14;
    if (strcmp(reg, "r13") == 0) return R13;
    if (strcmp(reg, "r12") == 0) return R12;
    if (strcmp(reg, "rbp") == 0) return RBP;
    if (strcmp(reg, "rbx") == 0) return RBX;
    if (strcmp(reg, "r11") == 0) return R11;
    if (strcmp(reg, "r10") == 0) return R10;
    if (strcmp(reg, "r9") == 0) return R9;
    if (strcmp(reg, "r8") == 0) return R8;
    if (strcmp(reg, "rax") == 0) return RAX;
    if (strcmp(reg, "rcx") == 0) return RCX;
    if (strcmp(reg, "rdx") == 0) return RDX;
    if (strcmp(reg, "rsi") == 0) return RSI;
    if (strcmp(reg, "rdi") == 0) return RDI;
    if (strcmp(reg, "orig_rax") == 0) return ORIG_RAX;
    if (strcmp(reg, "rip") == 0) return RIP;
    if (strcmp(reg, "cs") == 0) return CS;
    if (strcmp(reg, "eflags") == 0) return EFLAGS;
    if (strcmp(reg, "rsp") == 0) return RSP;
    if (strcmp(reg, "ss") == 0) return SS;
    if (strcmp(reg, "fs_base") == 0) return FS_BASE;
    if (strcmp(reg, "gs_base") == 0) return GS_BASE;
    if (strcmp(reg, "ds") == 0) return DS;
    if (strcmp(reg, "es") == 0) return ES;
    if (strcmp(reg, "fs") == 0) return FS;
    if (strcmp(reg, "gs") == 0) return GS;
    return -1;
}

int register_read(process_t *proc, const char *reg, reg_t *val_reg)
{
    // check
    if (!proc)
    {
        return EXIT_FAILURE;
    }

    // register_read
    int reg_id = convert_reg_into_id(reg);
    switch(reg_id)
    {
    case R15:
        (*val_reg) = proc->regs.r15;
        return EXIT_SUCCESS;
    case R14:
        (*val_reg) = proc->regs.r14;
        return EXIT_SUCCESS;
    case R13:
        (*val_reg) = proc->regs.r13;
        return EXIT_SUCCESS;
    case R12:
        (*val_reg) = proc->regs.r12;
        return EXIT_SUCCESS;
    case RBP:
        (*val_reg) = proc->regs.rbp;
        return EXIT_SUCCESS;
    case RBX:
        (*val_reg) = proc->regs.rbx;
        return EXIT_SUCCESS;
    case R11:
        (*val_reg) = proc->regs.r11;
        return EXIT_SUCCESS;
    case R10:
        (*val_reg) = proc->regs.r10;
        return EXIT_SUCCESS;
    case R9:
        (*val_reg) = proc->regs.r9;
        return EXIT_SUCCESS;
    case R8:
        (*val_reg) = proc->regs.r8;
        return EXIT_SUCCESS;
    case RAX:
        (*val_reg) = proc->regs.rax;
        return EXIT_SUCCESS;
    case RCX:
        (*val_reg) = proc->regs.rcx;
        return EXIT_SUCCESS;
    case RDX:
        (*val_reg) = proc->regs.rdx;
        return EXIT_SUCCESS;
    case RSI:
        (*val_reg) = proc->regs.rsi;
        return EXIT_SUCCESS;
    case RDI:
        (*val_reg) = proc->regs.rdi;
        return EXIT_SUCCESS;
    case ORIG_RAX:
        (*val_reg) = proc->regs.orig_rax;
        return EXIT_SUCCESS;
    case RIP:
        (*val_reg) = proc->regs.rip;
        return EXIT_SUCCESS;
    case CS:
        (*val_reg) = proc->regs.cs;
        return EXIT_SUCCESS;
    case EFLAGS:
        (*val_reg) = proc->regs.eflags;
        return EXIT_SUCCESS;
    case RSP:
        (*val_reg) = proc->regs.rsp;
        return EXIT_SUCCESS;
    case SS:
        (*val_reg) = proc->regs.ss;
        return EXIT_SUCCESS;
    case FS_BASE:
        (*val_reg) = proc->regs.fs_base;
        return EXIT_SUCCESS;
    case GS_BASE:
        (*val_reg) = proc->regs.gs_base;
        return EXIT_SUCCESS;
    case DS:
        (*val_reg) = proc->regs.ds;
        return EXIT_SUCCESS;
    case ES:
        (*val_reg) = proc->regs.es;
        return EXIT_SUCCESS;
    case FS:
        (*val_reg) = proc->regs.fs;
        return EXIT_SUCCESS;
    case GS:
        (*val_reg) = proc->regs.gs;
        return EXIT_SUCCESS;
    default:
        return EXIT_FAILURE;
    }

    // end
    return EXIT_FAILURE;
}
