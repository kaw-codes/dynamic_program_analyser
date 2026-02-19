#pragma once
#include <sys/types.h>
#include <sys/user.h> // struct user_regs_struct
#include <stdbool.h>

/* registers id */
#define R15 1
#define R14 2
#define R13 3
#define R12 4
#define RBP 5
#define RBX 6
#define R11 7
#define R10 8
#define R9  9
#define R8  10
#define RAX 11
#define RCX 12
#define RDX 13
#define RSI 14
#define RDI 15
#define ORIG_RAX 16
#define RIP 17
#define CS  18
#define EFLAGS 19
#define RSP 20
#define SS  21
#define FS_BASE 22
#define GS_BASE 23
#define DS  24
#define ES  25
#define FS  26
#define GS  27

typedef unsigned long long int reg_t;

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
    struct user_regs_struct regs;
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
 * to wait and edit status
 */
int wait_status(process_t *proc);

/**
 * to attach to the software using an existing pid
 * 
 * if you obtain "operation not permitted", then run:
 * cat /proc/sys/kernel/yama/ptrace_scope
 * - 0 : authorized
 * - 1 : only on children
 * - 2 : quite never allowed
 */
int attach(pid_t pid, bool kill_on_exit, process_t **proc);

/**
 * to resume the program from when it stopped
 */
int resume(process_t *proc);

/**
 * detach from the child proc and either kill or resume it
 */
int detach(process_t *proc);

/**
 * gets reg's content of proc and put it in val_reg
 */
int register_read(process_t *proc, const char *reg, reg_t *val_reg);
