#include "cli.h"
#include "libdpa.h"
#include <argp.h> // opt parsing
#include <stdio.h> // printf, fgets
#include <stdlib.h> // atoi
#include <string.h> // strtok

/* options parsing */

static char doc[] = "dynamic program analyser";

static char args_doc[] = "[PATH | -p PID | -]";

static struct argp_option options[] = 
{
    {"help", 'h', 0, 0, "Prints help message", 0},
    {"pid", 'p', "PID", 0, "Attach to a process pid", 0},
    {0}
};

typedef struct
{
    int pid;
    char *path;
} arguments_t;

void print_arg_help()
{
    printf("usage: dpa [-p PID | -h, --help | PATH | -]\n");
}

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    arguments_t *arguments = state->input;
    switch (key)
    {
    case 'h':
        print_arg_help();
        exit(EXIT_SUCCESS);
    case 'p':
        arguments->pid = atoi(arg);
        break;
    case ARGP_KEY_ARG:
        arguments->path = arg;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

/* main code */

void print_cmd_help()
{
    printf("available commands:\n");
    printf("  help,     h\n");
    printf("  attach,   a,  [pid]\n");
    printf("  continue, c\n");
    printf("  detach,   d\n");
    printf("  regread,  rr, [reg]\n");
    printf("  regwrite, rw, [reg]   [val]\n");
    printf("  memread,  mr, [vaddr] [size]\n");
    printf("  exit,     e\n");
}

int convert_str_into_id(char *cmd)
{
    if (strcmp(cmd, "help") == 0 || 
        strcmp(cmd, "h") == 0)
        return HELP;
    if (strcmp(cmd, "attach") == 0 || 
        strcmp(cmd, "a") == 0)
        return ATTACH;
    if (strcmp(cmd, "continue") == 0 || 
        strcmp(cmd, "c") == 0)
        return CONTINUE;
    if (strcmp(cmd, "detach") == 0 || 
        strcmp(cmd, "d") == 0)
        return DETACH;
    if (strcmp(cmd, "exit") == 0 || 
        strcmp(cmd, "e") == 0)
        return EXIT;
    if (strcmp(cmd, "regread") == 0 ||
        strcmp(cmd, "rr") == 0)
        return REG_READ;
    if (strcmp(cmd, "regwrite") == 0 ||
        strcmp(cmd, "rw") == 0)
        return REG_WRITE;
    if (strcmp(cmd, "memread") == 0 ||
        strcmp(cmd, "mr") == 0)
        return REG_WRITE;
    return ERROR;
}

int main(int argc, char **argv)
{
    /* arg parsing */
    arguments_t args;
    args.path = NULL;
    args.pid = -1;
    if (argc > 3)
    {
        exit(EXIT_FAILURE);
    }
    argp_parse(&argp, argc, argv, 0, 0, &args);

    /* check args */
    printf("---------CHECKING-ARGUMENTS---------\n");
    printf("pid: %d\n", args.pid);
    printf("path: %s\n", args.path);

    /* if args */
    printf("\n----------------DPA-----------------\n");
    process_t *proc = NULL;
    if (args.pid != -1) // pid mode
    {
        printf("attaching to pid...\n");
        if (attach(args.pid, true, &proc) != 0)
        {
            printf("error: issue with attach\n");
            return EXIT_FAILURE;
        }
        if (!proc)
        {
            printf("error: proc = NULL\n");
            return EXIT_FAILURE;
        }
        printf("successful attachment\n");
    }
    else if (args.path) // software path mode
    {
        printf("lauching software...\n");
        if (launch(args.path, true, &proc) != 0)
        {
            printf("error: issue with launch\n");
            return EXIT_FAILURE;
        }
        if (!proc)
        {
            printf("error: proc = NULL\n");
            return EXIT_FAILURE;
        }
        printf("successful launch\npid: %d\n", proc->pid);
    }

    /* routine */
    char input[128];
    while(1)
    {
        /* get cmd + args */
        printf("dpa> ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0; // to remove '\n'
        char *cmd = strtok(input, " ");
        char *arg1 = strtok(NULL, " ");
        char *arg2 = strtok(NULL, " ");
        int cmd_id = -1;
        if (cmd) cmd_id = convert_str_into_id(cmd);

        /* exec command */
        switch(cmd_id)
        {
        /* help */
        case HELP:
            print_cmd_help();
            break;

        /* attach */
        case ATTACH:
            if (!arg1)
            {
                printf("error: 'attach' cmd requires 1 arg\n");
                print_cmd_help();
                break;
            }
            else
            {
                if (proc)
                {
                    printf("detaching...\n");
                    if (detach(proc) != 0)
                    {
                        printf("error: issue with detach\n");
                        return EXIT_FAILURE;
                    }
                    free(proc);
                    proc = NULL;
                }
                printf("attaching...\n");
                pid_t pid = atoi(arg1);
                printf("%d\n", pid);
                if (pid <= 0)
                {
                    printf("error: wrong pid\n");
                    break;
                }
                if (attach(pid, true, &proc) != 0)
                {
                    printf("error: issue with attach\n");
                    break;
                }
                break;
            }

        /* continue */
        case CONTINUE:
            if (!proc)
            {
                printf("error: no proc attached\n");
                break;
            }
            else 
            {
                if (proc->status == TERMINATED)
                {
                    printf("program is already terminated\n");
                    break;
                }
                if (proc->status == STOPPED)
                {
                    printf("continuing...\n");
                    if (resume(proc) != 0)
                    {
                        printf("error: issue with resume\n");
                        return EXIT_FAILURE;
                    }
                }
                printf("waiting for program to stop...\n");
                /*
                we made the choice to directly wait for the child 
                to finish after resuming it.
                */
                wait_status(proc); 
                if (proc->status == TERMINATED)
                    printf("program terminated!\n");
                else
                    printf("program stopped\n");
            }
            break;

        /* detach */
        case DETACH:
            if (!proc)
            {
                printf("error: no proc attached\n");
                break;
            }
            printf("detaching...\n");
            if (detach(proc) != 0)
            {
                printf("error: issue with detach\n");
                return EXIT_FAILURE;
            }
            break;
        
        /* register_read */
        case REG_READ:
            if (!proc)
            {
                printf("error: no proc attached\n");
                break;
            }
            if (!arg1)
            {
                printf("error: 1 argument is required\n");
                print_cmd_help();
                break;
            }
            printf("reading register...\n");
            char *reg_to_read = arg1;
            reg_t val_reg = 0;
            if (register_read(proc, arg1, &val_reg) != 0)
            {
                printf("error: issue with register_read\n");
                return EXIT_FAILURE;
            }
            printf("%s=0x%llx\n", arg1, val_reg);
            break;

        /* register_write */
        case REG_WRITE:
            if (!proc)
            {
                printf("error: no proc attached\n");
                break;
            }
            if (!arg1 || !arg2)
            {
                printf("error: 2 arguments are required\n");
                print_cmd_help();
                break;
            }
            printf("writing register...\n");
            char *reg_to_write = arg1;
            reg_t new_val = strtoll(arg2, NULL, 0);
            if (register_write(proc, reg_to_write, new_val) != 0)
            {
                printf("error: issue with register_write\n");
                return EXIT_FAILURE;
            }
            printf("%s=0x%llx\n", arg1, new_val);
            break;

        /* memory_read */
        case MEM_READ:
            if (!proc)
            {
                printf("error: no proc attached\n");
                break;
            }
            if (!arg1 || !arg2)
            {
                printf("error: 2 arguments are required\n");
                print_cmd_help();
                break;
            }
            printf("reading memory...\n");
            addr_t vaddr = strtoll(arg1, NULL, 0);
            size_t size = atoi(arg2);
            if (size > 0 || size < 64) // size limited to 64 bytes
            {
                unsigned char buffer[0];
                if (memory_read(proc, vaddr, buffer, size) != 0)
                {
                    printf("error: issue with memory_read\n");
                    return EXIT_FAILURE;
                }
                // print read bytes
                printf("0x%lx: ", vaddr);
                for (int i = 0; i < size; i++)
                {
                    printf("%x", buffer[i]);
                }
                printf("\n");
            }
            
            break;

        /* exit */
        case EXIT:
            if (proc)
            {
                printf("detaching...\n");
                if (detach(proc) != 0)
                {
                    printf("error: issue with detach\n");
                    return EXIT_FAILURE;
                }
                free(proc);
                proc = NULL;
            }
            printf("exiting...\n");
            goto _end;
            break;
        
        /* error well */
        case ERROR:
            printf("error: wrong command\n");
            print_cmd_help();
            break;
        }
    }

_end:
    return EXIT_SUCCESS;
}
